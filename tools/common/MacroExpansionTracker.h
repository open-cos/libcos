/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_TOOLS_COMMON_MACRO_EXPANSION_TRACKER_H
#define LIBCOS_TOOLS_COMMON_MACRO_EXPANSION_TRACKER_H

#include "clang/Lex/PPCallbacks.h"
#include "llvm/ADT/StringRef.h"

#include <string>
#include <vector>

namespace libcos::tooling {

/**
 * PPCallbacks specialization that records the source location of every
 * expansion of a specific macro name into a caller-owned vector.
 *
 * The caller is responsible for the lifetime of the destination vector and
 * may inspect or clear it between translation units.
 */
class MacroExpansionTracker : public clang::PPCallbacks {
public:
    MacroExpansionTracker(llvm::StringRef MacroName,
                          std::vector<clang::SourceLocation> &Expansions);

    void
    MacroExpands(const clang::Token &MacroNameTok,
                 const clang::MacroDefinition &MD,
                 clang::SourceRange Range,
                 const clang::MacroArgs *Args) override;

private:
    std::string MacroName;
    std::vector<clang::SourceLocation> &Expansions;
};

} // namespace libcos::tooling

#endif // LIBCOS_TOOLS_COMMON_MACRO_EXPANSION_TRACKER_H
