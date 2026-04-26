/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "HeaderCompilationDatabase.h"

#include "llvm/Support/Path.h"

#include <utility>

namespace libcos::tooling {

using clang::tooling::CompilationDatabase;
using clang::tooling::CompileCommand;

HeaderCompilationDatabase::HeaderCompilationDatabase(
    const CompilationDatabase &Inner)
    : Inner(Inner)
{
    std::vector<CompileCommand> All = Inner.getAllCompileCommands();
    if (!All.empty()) {
        Template = std::move(All.front());
    }
}

std::vector<CompileCommand>
HeaderCompilationDatabase::getCompileCommands(llvm::StringRef FilePath) const
{
    std::vector<CompileCommand> Found = Inner.getCompileCommands(FilePath);
    if (!Found.empty()) {
        return Found;
    }
    if (!Template || !isHeaderPath(FilePath)) {
        return {};
    }
    return { synthesizeCommand(FilePath) };
}

std::vector<std::string>
HeaderCompilationDatabase::getAllFiles() const
{
    return Inner.getAllFiles();
}

std::vector<CompileCommand>
HeaderCompilationDatabase::getAllCompileCommands() const
{
    return Inner.getAllCompileCommands();
}

bool
HeaderCompilationDatabase::isHeaderPath(llvm::StringRef Path)
{
    const llvm::StringRef Ext = llvm::sys::path::extension(Path);
    return Ext == ".h" || Ext == ".hh" || Ext == ".hpp" || Ext == ".hxx";
}

CompileCommand
HeaderCompilationDatabase::synthesizeCommand(llvm::StringRef HeaderPath) const
{
    CompileCommand C = *Template;
    const std::string OldFilename = C.Filename;
    // Rewrite the source-file argument to point at the header.
    for (std::string &Arg : C.CommandLine) {
        if (Arg == OldFilename) {
            Arg = HeaderPath.str();
        }
    }
    C.Filename = HeaderPath.str();
    // Force header parsing as C so a bare .h file is treated as a TU.
    C.CommandLine.emplace_back("-x");
    C.CommandLine.emplace_back("c-header");
    return C;
}

} // namespace libcos::tooling
