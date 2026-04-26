/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "clang/AST/Decl.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

namespace {

cl::OptionCategory ToolCategory("public-api-fixer options");

cl::opt<std::string> AnnotationOpt(
    "annotation",
    cl::desc("Macro name considered as the public-API marker (default: "
             "COS_API)."),
    cl::value_desc("identifier"),
    cl::init("COS_API"),
    cl::cat(ToolCategory));

cl::opt<std::string> HeaderFilterOpt(
    "header-filter",
    cl::desc("Only check declarations whose source path matches this regex "
             "(default: \".*/include/libcos/.*\\.h$\")."),
    cl::value_desc("regex"),
    cl::init(R"(.*/include/libcos/.*\.h$)"),
    cl::cat(ToolCategory));

cl::opt<bool> FixOpt(
    "fix",
    cl::desc("Apply fix-its to insert missing annotations (not yet "
             "implemented)."),
    cl::init(false),
    cl::cat(ToolCategory));

unsigned WarningCount = 0;

// Records source locations where the configured annotation macro is expanded
// in the current translation unit.
class AnnotationTracker : public PPCallbacks {
public:
    AnnotationTracker(StringRef Annotation,
                      std::vector<SourceLocation> &Expansions)
        : Annotation(Annotation.str()),
          Expansions(Expansions) {}

    void
    MacroExpands(const Token &MacroNameTok,
                 const MacroDefinition &,
                 SourceRange Range,
                 const MacroArgs *) override
    {
        const IdentifierInfo *II = MacroNameTok.getIdentifierInfo();
        if (II != nullptr && II->getName() == Annotation) {
            Expansions.push_back(Range.getBegin());
        }
    }

private:
    std::string Annotation;
    std::vector<SourceLocation> &Expansions;
};

class MissingAnnotationCallback : public MatchFinder::MatchCallback {
public:
    MissingAnnotationCallback(DiagnosticsEngine &Diags,
                              const std::vector<SourceLocation> &Expansions,
                              StringRef Annotation)
        : Diags(Diags),
          Expansions(Expansions),
          Annotation(Annotation.str())
    {
        DiagID = Diags.getCustomDiagID(
            DiagnosticsEngine::Warning,
            "public declaration %0 is missing %1 annotation "
            "[public-api-fixer]");
    }

    void
    run(const MatchFinder::MatchResult &Result) override
    {
        const NamedDecl *ND = Result.Nodes.getNodeAs<NamedDecl>("decl");
        if (ND == nullptr) {
            return;
        }
        // Skip declarations whose identifier was produced by a macro
        // (e.g. DECLARE_FOO(name)). Source-level reasoning doesn't apply.
        if (ND->getLocation().isMacroID()) {
            return;
        }

        const SourceManager &SM = *Result.SourceManager;
        const SourceLocation BeginLoc = ND->getBeginLoc();
        if (BeginLoc.isInvalid()) {
            return;
        }
        // Use the *expansion* location, not the spelling location: when the
        // declaration's leading token came from a macro (e.g. `COS_API` or
        // `bool`), spelling resolves into the macro definition; expansion
        // resolves to the macro invocation site, which is what we want.
        const SourceLocation BeginExp = SM.getExpansionLoc(BeginLoc);
        const FileID DeclFile = SM.getFileID(BeginExp);
        const unsigned BeginLine = SM.getPresumedLineNumber(BeginExp);

        for (SourceLocation ExpLoc : Expansions) {
            if (ExpLoc.isInvalid()) {
                continue;
            }
            const SourceLocation MacroExp = SM.getExpansionLoc(ExpLoc);
            if (SM.getFileID(MacroExp) != DeclFile) {
                continue;
            }
            const unsigned ExpLine = SM.getPresumedLineNumber(MacroExp);
            // Annotation may sit on the same line as the declaration or on
            // the immediately preceding line.
            if (ExpLine == BeginLine || ExpLine + 1 == BeginLine) {
                return;
            }
        }

        // De-duplicate across TUs that include the same header.
        const PresumedLoc PLoc = SM.getPresumedLoc(BeginExp);
        if (PLoc.isInvalid() || PLoc.getFilename() == nullptr) {
            return;
        }
        auto Key = std::make_tuple(std::string(PLoc.getFilename()),
                                   BeginLine,
                                   ND->getNameAsString());
        if (!Reported.insert(std::move(Key)).second) {
            return;
        }

        Diags.Report(BeginExp, DiagID)
            << ND->getNameAsString() << Annotation;
        ++WarningCount;
    }

private:
    DiagnosticsEngine &Diags;
    const std::vector<SourceLocation> &Expansions;
    std::string Annotation;
    unsigned DiagID = 0;

    static std::set<std::tuple<std::string, unsigned, std::string>> Reported;
};

std::set<std::tuple<std::string, unsigned, std::string>>
    MissingAnnotationCallback::Reported;

class AnnotationCheckAction : public ASTFrontendAction {
public:
    AnnotationCheckAction(StringRef Annotation, StringRef HeaderFilter)
        : Annotation(Annotation.str()),
          HeaderFilter(HeaderFilter.str()) {}

    bool
    BeginSourceFileAction(CompilerInstance &CI) override
    {
        Expansions.clear();
        CI.getPreprocessor().addPPCallbacks(
            std::make_unique<AnnotationTracker>(Annotation, Expansions));
        return true;
    }

    std::unique_ptr<ASTConsumer>
    CreateASTConsumer(CompilerInstance &CI, StringRef) override
    {
        Callback = std::make_unique<MissingAnnotationCallback>(
            CI.getDiagnostics(), Expansions, Annotation);

        Finder.addMatcher(
            functionDecl(
                isExpansionInFileMatching(HeaderFilter),
                unless(isExpansionInSystemHeader()),
                unless(isStaticStorageClass()),
                unless(cxxMethodDecl()),
                unless(isImplicit()))
                .bind("decl"),
            Callback.get());

        Finder.addMatcher(
            varDecl(
                isExpansionInFileMatching(HeaderFilter),
                hasGlobalStorage(),
                unless(isStaticStorageClass()),
                unless(isImplicit()))
                .bind("decl"),
            Callback.get());

        return Finder.newASTConsumer();
    }

private:
    std::string Annotation;
    std::string HeaderFilter;
    std::vector<SourceLocation> Expansions;
    MatchFinder Finder;
    std::unique_ptr<MissingAnnotationCallback> Callback;
};

class AnnotationCheckActionFactory : public FrontendActionFactory {
public:
    AnnotationCheckActionFactory(StringRef Annotation, StringRef HeaderFilter)
        : Annotation(Annotation.str()),
          HeaderFilter(HeaderFilter.str()) {}

    std::unique_ptr<FrontendAction>
    create() override
    {
        return std::make_unique<AnnotationCheckAction>(Annotation,
                                                       HeaderFilter);
    }

private:
    std::string Annotation;
    std::string HeaderFilter;
};

// Wraps an underlying CompilationDatabase so that lookups for header files
// (which normally have no entry of their own) succeed by synthesizing a
// command from a representative entry in the inner DB. The header is then
// compiled as a standalone translation unit, letting us run the tool over
// public headers directly without going through their `.c` includers.
class HeaderCompilationDatabase : public CompilationDatabase {
public:
    explicit HeaderCompilationDatabase(const CompilationDatabase &Inner)
        : Inner(Inner)
    {
        std::vector<CompileCommand> All = Inner.getAllCompileCommands();
        if (!All.empty()) {
            Template = std::move(All.front());
        }
    }

    std::vector<CompileCommand>
    getCompileCommands(StringRef FilePath) const override
    {
        std::vector<CompileCommand> Found = Inner.getCompileCommands(FilePath);
        if (!Found.empty()) {
            return Found;
        }
        if (!Template || !isHeaderPath(FilePath)) {
            return {};
        }
        return { synthesizeCommand(FilePath) };
    }

    std::vector<std::string>
    getAllFiles() const override
    {
        return Inner.getAllFiles();
    }

    std::vector<CompileCommand>
    getAllCompileCommands() const override
    {
        return Inner.getAllCompileCommands();
    }

private:
    static bool
    isHeaderPath(StringRef Path)
    {
        const StringRef Ext = llvm::sys::path::extension(Path);
        return Ext == ".h" || Ext == ".hh" || Ext == ".hpp" || Ext == ".hxx";
    }

    CompileCommand
    synthesizeCommand(StringRef HeaderPath) const
    {
        CompileCommand C = *Template;
        const std::string OldFilename = C.Filename;
        // Rewrite the source-file argument to point at the header.
        for (std::string &Arg : C.CommandLine) {
            if (Arg == OldFilename) {
                Arg = HeaderPath.str();
            }
        }
        C.Filename = HeaderPath.str();
        // Force header parsing as C so a bare .h file is treated as a TU.
        C.CommandLine.push_back("-x");
        C.CommandLine.push_back("c-header");
        return C;
    }

    const CompilationDatabase &Inner;
    std::optional<CompileCommand> Template;
};

} // namespace

int
main(int argc, const char **argv)
{
    auto ExpectedParser =
        CommonOptionsParser::create(argc, argv, ToolCategory);
    if (!ExpectedParser) {
        errs() << ExpectedParser.takeError();
        return 2;
    }
    CommonOptionsParser &OptionsParser = *ExpectedParser;
    HeaderCompilationDatabase HeaderDB(OptionsParser.getCompilations());
    ClangTool Tool(HeaderDB, OptionsParser.getSourcePathList());

    if (FixOpt) {
        errs() << "warning: --fix is not yet implemented; running in "
                  "report-only mode\n";
    }

    AnnotationCheckActionFactory Factory(AnnotationOpt, HeaderFilterOpt);
    const int RunResult = Tool.run(&Factory);
    if (RunResult != 0) {
        return RunResult;
    }
    return WarningCount > 0 ? 1 : 0;
}
