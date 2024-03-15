#include "middleend/llvm_protos.h"

#include <LLVM.h>


namespace panther{
	namespace llvmint{


		auto _ptrcast_to_constant(llvm::ConstantInt* from) noexcept -> llvm::Constant* {
			return static_cast<llvm::Constant*>(from);
		};





		auto _ptrcast_to_value(llvm::LoadInst* from) noexcept -> llvm::Value* {
			return static_cast<llvm::Value*>(from);
		};

		auto _ptrcast_to_value(llvm::GlobalVariable* from) noexcept -> llvm::Value* {
			return static_cast<llvm::Value*>(from);
		};

		auto _ptrcast_to_value(llvm::ConstantInt* from) noexcept -> llvm::Value* {
			return static_cast<llvm::Value*>(from);
		};

		auto _ptrcast_to_value(llvm::AllocaInst* from) noexcept -> llvm::Value* {
			return static_cast<llvm::Value*>(from);
		};

		
	};
};