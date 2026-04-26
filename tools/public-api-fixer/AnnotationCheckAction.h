/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_TOOLS_PUBLIC_API_FIXER_ANNOTATION_CHECK_ACTION_H
#define LIBCOS_TOOLS_PUBLIC_API_FIXER_ANNOTATION_CHECK_ACTION_H

#include "MissingAnnotationCallback.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringRef.h"

#include <memory>
#include <string>
#include <vector>

namespace libcos::tooling::public_api_fixer {

/**
 * Frontend action that, for each translation unit:
 *   1. Installs a MacroExpansionTracker for the configured annotation name.
 *   2. Registers AST matchers for free function and global variable
 *      declarations whose source path matches the configured header
 *      filter regex.
 *   3. Routes each match to a MissingAnnotationCallback that compares the
 *      declaration site against the recorded annotation expansions and
 *      emits a diagnostic when none is found.
 */
class AnnotationCheckAction : public clang::ASTFrontendAction {
public:
    AnnotationCheckAction(llvm::StringRef Annotation,
                          llvm::StringRef HeaderFilter);

    bool BeginSourceFileAction(clang::CompilerInstance &CI) override;

    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI,
                      llvm::StringRef File) override;

private:
    std::string Annotation;
    std::string HeaderFilter;
    std::vector<clang::SourceLocation> Expansions;
    clang::ast_matchers::MatchFinder Finder;
    std::unique_ptr<MissingAnnotationCallback> Callback;
};

class AnnotationCheckActionFactory : public clang::tooling::FrontendActionFactory {
public:
    AnnotationCheckActionFactory(llvm::StringRef Annotation,
                                 llvm::StringRef HeaderFilter);

    std::unique_ptr<clang::FrontendAction> create() override;

private:
    std::string Annotation;
    std::string HeaderFilter;
};

} // namespace libcos::tooling::public_api_fixer

#endif // LIBCOS_TOOLS_PUBLIC_API_FIXER_ANNOTATION_CHECK_ACTION_H
