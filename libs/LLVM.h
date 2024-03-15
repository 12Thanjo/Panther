#pragma once


#pragma warning(disable : 4244)
#pragma warning (push, 0)

    #pragma warning(disable : 4996)
    #define _SILENCE_CXX20_CISO646_REMOVED_WARNING

        #include <llvm/ExecutionEngine/ExecutionEngine.h>
        #include <llvm/ExecutionEngine/GenericValue.h>

        #include <llvm/IR/IRBuilder.h>
        #include <llvm/IR/NoFolder.h>

        #include <llvm/Transforms/Utils/Cloning.h>

        #include <llvm/Support/TargetSelect.h>


        #include <llvm/IR/LegacyPassManager.h>

        #include <llvm/MC/TargetRegistry.h>
        #include <llvm/Support/FileSystem.h>
        // #include <llvm/Support/TargetSelect.h>
        // #include <llvm/Support/raw_ostream.h>
        // #include <llvm/Target/TargetMachine.h>
        // #include <llvm/Target/TargetOptions.h>
        #include <llvm/TargetParser/Host.h>





        // #include <clang/Frontend/CompilerInstance.h>
        // #include <clang/Basic/DiagnosticOptions.h>
        // #include <clang/Frontend/TextDiagnosticPrinter.h>
        // #include <clang/CodeGen/CodeGenAction.h>
        // #include <clang/Basic/TargetInfo.h>
        // #include <llvm/Support/TargetSelect.h>

        
    #undef _SILENCE_CXX20_CISO646_REMOVED_WARNING
    #pragma warning(default : 4996)

#pragma warning (pop)
#pragma warning(default : 4244)

