/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "AnnotationCheckAction.h"
#include "MissingAnnotationCallback.h"

#include "HeaderCompilationDatabase.h"

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

    libcos::tooling::public_api_fixer::AnnotationCheckActionFactory Factory(
        AnnotationOpt, HeaderFilterOpt, &Tool.getReplacements());

    const int RunResult =
        FixOpt ? Tool.runAndSave(&Factory) : Tool.run(&Factory);
    if (RunResult != 0) {
        return RunResult;
    }
    return libcos::tooling::public_api_fixer::MissingAnnotationCallback::
                       getWarningCount() > 0
               ? 1
               : 0;
}
