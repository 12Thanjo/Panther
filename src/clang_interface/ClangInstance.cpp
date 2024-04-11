#include "backend/ClangInstance.h"

#include <LLVM.h>

// #include <clang/Frontend/CompilerInstance.h>
// #include <clang/Basic/DiagnosticOptions.h>
// #include <clang/Frontend/TextDiagnosticPrinter.h>
// #include <clang/CodeGen/CodeGenAction.h>
// #include <clang/Basic/TargetInfo.h>
// #include <llvm/Support/TargetSelect.h>

// TOOD: remove
// using namespace llvm;
// using namespace clang;



namespace panther{


    // https://stackoverflow.com/questions/34828480/generate-assembly-from-c-code-in-memory-using-libclang/34866053#34866053

    auto ClangInstance::init() noexcept -> void {
        // llvm::InitializeNativeTargetMC();
        // llvm::InitializeNativeTargetAsmPrinter();
    };

    
    auto ClangInstance::run() noexcept -> void {
        // constexpr auto testCodeFileName = "test.cpp";
        // constexpr auto testCode = "int main() { printf(\"hi\"); return 0; }";

        // // Prepare compilation arguments
        // std::vector<const char *> args;
        // args.push_back(testCodeFileName);

        // // Prepare DiagnosticEngine 
        // clang::DiagnosticOptions DiagOpts;
        // TextDiagnosticPrinter *textDiagPrinter = new clang::TextDiagnosticPrinter(errs(), &DiagOpts);
        // IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;
        // DiagnosticsEngine *pDiagnosticsEngine = new DiagnosticsEngine(pDiagIDs, &DiagOpts, textDiagPrinter);


        // // Initialize CompilerInvocation
        // // CompilerInvocation* CI = new CompilerInvocation();
        // std::shared_ptr<CompilerInvocation> CI = std::make_shared<CompilerInvocation>();
        // // TODO: check "*CI" arg here
        // CompilerInvocation::CreateFromArgs(*CI, llvm::ArrayRef<const char*>{&args[0], &args[0] + args.size()}, *pDiagnosticsEngine);

        // // Map code filename to a memoryBuffer
        // StringRef testCodeData(testCode);
        // std::unique_ptr<MemoryBuffer> buffer = MemoryBuffer::getMemBufferCopy(testCodeData);
        // CI->getPreprocessorOpts().addRemappedFile(testCodeFileName, buffer.get());


        // // Create and initialize CompilerInstance
        // CompilerInstance Clang;
        // Clang.setInvocation(CI);
        // Clang.createDiagnostics();

        // // Set target (I guess I can initialize only the BPF target, but I don't know how)
        // // InitializeAllTargets();
        // const std::shared_ptr<clang::TargetOptions> targetOptions = std::make_shared<clang::TargetOptions>();
        // targetOptions->Triple = std::string("x86_64-pc-windows-msvc");
        // TargetInfo *pTargetInfo = TargetInfo::CreateTargetInfo(*pDiagnosticsEngine,targetOptions);
        // Clang.setTarget(pTargetInfo);

        // // Create and execute action
        // // CodeGenAction *compilerAction = new EmitLLVMOnlyAction();
        // CodeGenAction *compilerAction = new EmitAssemblyAction();
        // Clang.ExecuteAction(*compilerAction);

        // buffer.release();
    };

    



};