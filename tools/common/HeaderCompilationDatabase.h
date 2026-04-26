/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_TOOLS_COMMON_HEADER_COMPILATION_DATABASE_H
#define LIBCOS_TOOLS_COMMON_HEADER_COMPILATION_DATABASE_H

#include "clang/Tooling/CompilationDatabase.h"
#include "llvm/ADT/StringRef.h"

#include <optional>
#include <string>
#include <vector>

namespace libcos::tooling {

/**
 * Wraps an underlying CompilationDatabase so that lookups for header files
 * (which normally have no entry of their own) succeed by synthesizing a
 * command derived from a representative entry in the inner database.
 *
 * This lets a libclang-based tool be invoked directly on `.h` paths
 * instead of going through the `.c` files that include them. Existing
 * `.c` lookups still resolve through the inner database unchanged.
 */
class HeaderCompilationDatabase : public clang::tooling::CompilationDatabase {
public:
    explicit HeaderCompilationDatabase(
        const clang::tooling::CompilationDatabase &Inner);

    std::vector<clang::tooling::CompileCommand>
    getCompileCommands(llvm::StringRef FilePath) const override;

    std::vector<std::string>
    getAllFiles() const override;

    std::vector<clang::tooling::CompileCommand>
    getAllCompileCommands() const override;

private:
    static bool
    isHeaderPath(llvm::StringRef Path);

    clang::tooling::CompileCommand
    synthesizeCommand(llvm::StringRef HeaderPath) const;

    const clang::tooling::CompilationDatabase &Inner;
    std::optional<clang::tooling::CompileCommand> Template;
};

} // namespace libcos::tooling

#endif // LIBCOS_TOOLS_COMMON_HEADER_COMPILATION_DATABASE_H
