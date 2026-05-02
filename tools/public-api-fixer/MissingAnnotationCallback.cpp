/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "MissingAnnotationCallback.h"

#include "clang/AST/Decl.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Tooling/Inclusions/HeaderIncludes.h"
#include "clang/Tooling/Inclusions/IncludeStyle.h"
#include "llvm/Support/Error.h"

#include <utility>

namespace libcos::tooling::public_api_fixer {

using clang::DiagnosticsEngine;
using clang::FileID;
using clang::FixItHint;
using clang::NamedDecl;
using clang::PresumedLoc;
using clang::SourceLocation;
using clang::SourceManager;
using clang::ast_matchers::MatchFinder;
using clang::tooling::Replacement;
using clang::tooling::Replacements;

unsigned MissingAnnotationCallback::WarningCount = 0;
std::set<std::tuple<std::string, unsigned, std::string>>
    MissingAnnotationCallback::Reported;
std::set<std::string> MissingAnnotationCallback::IncludesAlreadyConsidered;

MissingAnnotationCallback::MissingAnnotationCallback(
    DiagnosticsEngine &Diags,
    const std::vector<SourceLocation> &Expansions,
    llvm::StringRef Annotation,
    std::map<std::string, Replacements> *FileReplacements,
    llvm::StringRef AnnotationHeader,
    bool IncludeIsAngled)
    : Diags(Diags),
      Expansions(Expansions),
      Annotation(Annotation.str()),
      FileReplacements(FileReplacements),
      AnnotationHeader(AnnotationHeader.str()),
      IncludeIsAngled(IncludeIsAngled)
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

    const std::string InsertText = Annotation + " ";

    // Two modes, decided by whether the caller supplied a Replacements map:
    //   - check mode (FileReplacements == nullptr): emit a diagnostic with
    //     a fix-it hint so the user sees what's missing.
    //   - fix mode  (FileReplacements != nullptr): record a Replacement for
    //     RefactoringTool::runAndSave. Stay silent here - the fix itself is
    //     the report, and main() prints a one-line summary at the end.
    if (FileReplacements != nullptr) {
        const Replacement R(SM, BeginExp, /*Length=*/0, InsertText);
        const llvm::StringRef Path = R.getFilePath();
        if (Path.empty()) {
            return;
        }
        Replacements &FileR = (*FileReplacements)[std::string(Path)];

        // First time we touch this file: also plant the include of the
        // annotation's defining header, if requested. HeaderIncludes::insert
        // is itself idempotent (returns nullopt when the spelling is
        // already present), so a re-run of --fix won't add duplicates.
        if (!AnnotationHeader.empty()
            && IncludesAlreadyConsidered.insert(std::string(Path)).second) {
            const llvm::StringRef Code = SM.getBufferData(DeclFile);
            const clang::tooling::IncludeStyle Style{};
            const clang::tooling::HeaderIncludes Includes(Path, Code, Style);
            std::optional<Replacement> IncR = Includes.insert(
                AnnotationHeader,
                IncludeIsAngled,
                clang::tooling::IncludeDirective::Include);
            if (IncR) {
                if (auto Err = FileR.add(*IncR)) {
                    llvm::consumeError(std::move(Err));
                }
            }
        }

        if (auto Err = FileR.add(R)) {
            // Conflicting/overlapping replacements should not occur because
            // of the dedup above, but stay defensive: drop the error.
            llvm::consumeError(std::move(Err));
        }
        return;
    }

    Diags.Report(BeginExp, DiagID)
        << ND->getNameAsString()
        << Annotation
        << FixItHint::CreateInsertion(BeginExp, InsertText);
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
