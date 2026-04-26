/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "MacroExpansionTracker.h"

#include "clang/Basic/IdentifierTable.h"
#include "clang/Lex/Token.h"

namespace libcos::tooling {

MacroExpansionTracker::MacroExpansionTracker(
    llvm::StringRef MacroName,
    std::vector<clang::SourceLocation> &Expansions)
    : MacroName(MacroName.str()),
      Expansions(Expansions) {}

void
MacroExpansionTracker::MacroExpands(const clang::Token &MacroNameTok,
                                    const clang::MacroDefinition &,
                                    clang::SourceRange Range,
                                    const clang::MacroArgs *)
{
    const clang::IdentifierInfo *II = MacroNameTok.getIdentifierInfo();
    if (II != nullptr && II->getName() == MacroName) {
        Expansions.push_back(Range.getBegin());
    }
}

} // namespace libcos::tooling
