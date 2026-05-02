/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "AnnotationCheckAction.h"
#include "MissingAnnotationCallback.h"

#include "HeaderCompilationDatabase.h"

#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

namespace {

llvm::cl::OptionCategory ToolCategory("public-api-fixer options");

llvm::cl::opt<std::string> AnnotationOpt(
    "annotation",
    llvm::cl::desc("Macro name considered as the public-API marker (default: "
                   "COS_API)."),
    llvm::cl::value_desc("identifier"),
    llvm::cl::init("COS_API"),
    llvm::cl::cat(ToolCategory));

llvm::cl::opt<std::string> HeaderFilterOpt(
    "header-filter",
    llvm::cl::desc("Only check declarations whose source path matches this "
                   "regex (default: \".*/include/libcos/.*\\.h$\")."),
    llvm::cl::value_desc("regex"),
    llvm::cl::init(R"(.*/include/libcos/.*\.h$)"),
    llvm::cl::cat(ToolCategory));

llvm::cl::opt<bool> FixOpt(
    "fix",
    llvm::cl::desc("Rewrite headers in place to insert the missing "
                   "annotations."),
    llvm::cl::init(false),
    llvm::cl::cat(ToolCategory));

llvm::cl::opt<std::string> AnnotationHeaderOpt(
    "annotation-header",
    llvm::cl::desc("In --fix mode, also #include this header in any file "
                   "that gets a new annotation. Empty (default) skips "
                   "include insertion."),
    llvm::cl::value_desc("path"),
    llvm::cl::init(""),
    llvm::cl::cat(ToolCategory));

enum class IncludeStyle { Angled, Quoted };

llvm::cl::opt<IncludeStyle> IncludeStyleOpt(
    "include-style",
    llvm::cl::desc("Quote style for inserted #include directives:"),
    llvm::cl::values(
        clEnumValN(IncludeStyle::Angled, "angled", "<header>"),
        clEnumValN(IncludeStyle::Quoted, "quoted", "\"header\"")),
    llvm::cl::init(IncludeStyle::Angled),
    llvm::cl::cat(ToolCategory));

} // namespace

int
main(int argc, const char **argv)
{
    auto ExpectedParser = clang::tooling::CommonOptionsParser::create(
        argc, argv, ToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 2;
    }
    clang::tooling::CommonOptionsParser &OptionsParser = *ExpectedParser;

    libcos::tooling::HeaderCompilationDatabase HeaderDB(
        OptionsParser.getCompilations());
    clang::tooling::RefactoringTool Tool(HeaderDB,
                                         OptionsParser.getSourcePathList());

    // If the user supplied --annotation-header, force-include it via
    // `-include <header>` so the tool parses cleanly even on headers that
    // do not yet include it themselves. clang resolves the path against
    // the existing -I search list.
    if (!AnnotationHeaderOpt.empty()) {
        Tool.appendArgumentsAdjuster(
            clang::tooling::getInsertArgumentAdjuster(
                {"-include", AnnotationHeaderOpt},
                clang::tooling::ArgumentInsertPosition::END));
    }

    // Pass the replacements map only when --fix is requested. The callback
    // uses its presence to decide between emitting a diagnostic and
    // recording a fix-it.
    auto *Replacements = FixOpt ? &Tool.getReplacements() : nullptr;
    const bool IncludeIsAngled = (IncludeStyleOpt == IncludeStyle::Angled);
    libcos::tooling::public_api_fixer::AnnotationCheckActionFactory Factory(
        AnnotationOpt, HeaderFilterOpt, Replacements,
        AnnotationHeaderOpt, IncludeIsAngled);

    const int RunResult =
        FixOpt ? Tool.runAndSave(&Factory) : Tool.run(&Factory);
    if (RunResult != 0) {
        return RunResult;
    }

    if (FixOpt) {
        size_t NumFixes = 0;
        for (const auto &Entry : Tool.getReplacements()) {
            NumFixes += Entry.second.size();
        }
        llvm::outs() << "public_api_fixer: applied " << NumFixes
                     << " annotation(s) across "
                     << Tool.getReplacements().size() << " file(s).\n";
        return 0;
    }

    return libcos::tooling::public_api_fixer::MissingAnnotationCallback::
                       getWarningCount() > 0
               ? 1
               : 0;
}
