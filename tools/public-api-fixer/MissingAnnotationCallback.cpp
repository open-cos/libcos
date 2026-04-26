/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "MissingAnnotationCallback.h"

#include "clang/AST/Decl.h"
#include "clang/Basic/SourceManager.h"

#include <utility>

namespace libcos::tooling::public_api_fixer {

using clang::DiagnosticsEngine;
using clang::FileID;
using clang::NamedDecl;
using clang::PresumedLoc;
using clang::SourceLocation;
using clang::SourceManager;
using clang::ast_matchers::MatchFinder;

unsigned MissingAnnotationCallback::WarningCount = 0;
std::set<std::tuple<std::string, unsigned, std::string>>
    MissingAnnotationCallback::Reported;

MissingAnnotationCallback::MissingAnnotationCallback(
    DiagnosticsEngine &Diags,
    const std::vector<SourceLocation> &Expansions,
    llvm::StringRef Annotation)
    : Diags(Diags),
      Expansions(Expansions),
      Annotation(Annotation.str())
{
    DiagID = Diags.getCustomDiagID(
        DiagnosticsEngine::Warning,
        "public declaration %0 is missing %1 annotation [public-api-fixer]");
}

void
MissingAnnotationCallback::run(const MatchFinder::MatchResult &Result)
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

    Diags.Report(BeginExp, DiagID) << ND->getNameAsString() << Annotation;
    ++WarningCount;
}

unsigned
MissingAnnotationCallback::getWarningCount()
{
    return WarningCount;
}

void
MissingAnnotationCallback::resetWarningState()
{
    WarningCount = 0;
    Reported.clear();
}

} // namespace libcos::tooling::public_api_fixer
