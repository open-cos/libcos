/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

static llvm::cl::OptionCategory ToolCategory("public-api-fixer options");

class PublicAPIInserter : public MatchFinder::MatchCallback {
public:
    PublicAPIInserter(Rewriter &R) : Rewrite(R) {}

    void
    run(const MatchFinder::MatchResult &Result) override
    {
        // FunctionDecl
        if (auto *FD = Result.Nodes.getNodeAs<FunctionDecl>("func")) {
            // don't re-process if already has macro
            SourceLocation loc = FD->getSourceRange().getBegin();
            Rewrite.InsertText(loc, "PUBLIC_API ", /*InsertAfter=*/false);
        }
        // VarDecl (globals)
        if (auto *VD = Result.Nodes.getNodeAs<VarDecl>("var")) {
            SourceLocation loc = VD->getSourceRange().getBegin();
            Rewrite.InsertText(loc, "PUBLIC_API ", /*InsertAfter=*/false);
        }
    }

private:
    Rewriter &Rewrite;
};

int
main(int argc, const char **argv)
{
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, ToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser &OptionsParser = *ExpectedParser;
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());

    Rewriter Rewrite;
    PublicAPIInserter Inserter(Rewrite);
    MatchFinder Finder;

    // Match free functions with external linkage in headers
    Finder.addMatcher(
        functionDecl(
            unless(isExpansionInSystemHeader()),
            isDefinition(), // or just declaration?
            unless(cxxMethodDecl()),
            unless(isStaticStorageClass()),        // not static
            unless(hasAttr(attr::Visibility)),     // or custom check
            unless(hasAncestor(linkageSpecDecl())) // not inside extern "C"?
                                                   //                     unless(hasMacroAnnotation("PUBLIC_API"))
            )
            .bind("func"),
        &Inserter);

    // Match global variables
    Finder.addMatcher(
        varDecl(hasGlobalStorage(),
                isDefinition(),
                unless(isExpansionInSystemHeader())
                //                unless(hasMacroAnnotation("PUBLIC_API")))
                )
            .bind("var"),
        &Inserter);

    return Tool.run(newFrontendActionFactory(&Finder).get());
}
