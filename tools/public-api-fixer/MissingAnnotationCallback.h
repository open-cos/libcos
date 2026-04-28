/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_TOOLS_PUBLIC_API_FIXER_MISSING_ANNOTATION_CALLBACK_H
#define LIBCOS_TOOLS_PUBLIC_API_FIXER_MISSING_ANNOTATION_CALLBACK_H

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Tooling/Core/Replacement.h"
#include "llvm/ADT/StringRef.h"

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace libcos::tooling::public_api_fixer {

/**
 * MatchCallback that, for each matched declaration, checks whether a
 * recorded macro expansion (the "annotation") sits on the same line or the
 * line immediately preceding the declaration. Declarations without such an
 * annotation are reported via the supplied DiagnosticsEngine.
 *
 * Matched declarations must be bound to the name @c "decl".
 *
 * Reports are de-duplicated across translation units by (file, line, name)
 * via process-wide static state, so the same header included from multiple
 * `.c` files yields a single warning per missing annotation.
 */
class MissingAnnotationCallback
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    /**
     * @param FileReplacements  Optional pointer to a per-file Replacements
     *   map. When non-null, every flagged declaration is recorded as an
     *   insertion that can later be persisted via RefactoringTool.
     */
    MissingAnnotationCallback(
        clang::DiagnosticsEngine &Diags,
        const std::vector<clang::SourceLocation> &Expansions,
        llvm::StringRef Annotation,
        std::map<std::string, clang::tooling::Replacements> *FileReplacements);

    void
    run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

    static unsigned getWarningCount();
    static void resetWarningState();

private:
    clang::DiagnosticsEngine &Diags;
    const std::vector<clang::SourceLocation> &Expansions;
    std::string Annotation;
    std::map<std::string, clang::tooling::Replacements> *FileReplacements;
    unsigned DiagID = 0;

    static unsigned WarningCount;
    static std::set<std::tuple<std::string, unsigned, std::string>> Reported;
};

} // namespace libcos::tooling::public_api_fixer

#endif // LIBCOS_TOOLS_PUBLIC_API_FIXER_MISSING_ANNOTATION_CALLBACK_H
