/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "AnnotationCheckAction.h"

#include "MacroExpansionTracker.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"

namespace libcos::tooling::public_api_fixer {

using clang::ASTConsumer;
using clang::CompilerInstance;
using clang::FrontendAction;
using clang::ast_matchers::cxxMethodDecl;
using clang::ast_matchers::functionDecl;
using clang::ast_matchers::hasGlobalStorage;
using clang::ast_matchers::isExpansionInFileMatching;
using clang::ast_matchers::isExpansionInSystemHeader;
using clang::ast_matchers::isImplicit;
using clang::ast_matchers::isStaticStorageClass;
using clang::ast_matchers::unless;
using clang::ast_matchers::varDecl;

AnnotationCheckAction::AnnotationCheckAction(llvm::StringRef Annotation,
                                             llvm::StringRef HeaderFilter)
    : Annotation(Annotation.str()),
      HeaderFilter(HeaderFilter.str()) {}

bool
AnnotationCheckAction::BeginSourceFileAction(CompilerInstance &CI)
{
    Expansions.clear();
    CI.getPreprocessor().addPPCallbacks(
        std::make_unique<MacroExpansionTracker>(Annotation, Expansions));
    return true;
}

std::unique_ptr<ASTConsumer>
AnnotationCheckAction::CreateASTConsumer(CompilerInstance &CI, llvm::StringRef)
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

AnnotationCheckActionFactory::AnnotationCheckActionFactory(
    llvm::StringRef Annotation,
    llvm::StringRef HeaderFilter)
    : Annotation(Annotation.str()),
      HeaderFilter(HeaderFilter.str()) {}

std::unique_ptr<FrontendAction>
AnnotationCheckActionFactory::create()
{
    return std::make_unique<AnnotationCheckAction>(Annotation, HeaderFilter);
}

} // namespace libcos::tooling::public_api_fixer
