#include "middleend/Context.h"

#include <LLVM.h>


namespace panther{
	namespace llvmint{
		

		auto Context::init() noexcept -> void {
			evo::debugAssert(this->isInitialized() == false, "Context is already initialized");

			// LLVMLinkInInterpreter();
			// auto force = ForceMCJITLinking();
			LLVMLinkInMCJIT();
			llvm::InitializeNativeTarget();
			llvm::InitializeNativeTargetAsmPrinter();
			llvm::InitializeNativeTargetAsmParser();


			// llvm::InitializeAllTargetInfos();
			// llvm::InitializeAllTargets();
			// llvm::InitializeAllTargetMCs();
			// llvm::InitializeAllAsmParsers();
			// llvm::InitializeAllAsmPrinters();


			this->context = new llvm::LLVMContext();
		};



		auto Context::shutdown() noexcept -> void {
			evo::debugAssert(this->isInitialized(), "Cannot shutdown context when not initialized");

			delete this->context;
			this->context = nullptr;
		};

	
	};
};