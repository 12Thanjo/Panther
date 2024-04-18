#include "LLVM_interface/llvm_protos.h"

#include <LLVM.h>


namespace panther{
	namespace llvmint{


		auto _ptrcast_to_constant(llvm::ConstantInt* from) noexcept -> llvm::Constant* {
			return static_cast<llvm::Constant*>(from);
		};

		auto _ptrcast_to_constant(llvm::GlobalVariable* from) noexcept -> llvm::Constant* {
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

		auto _ptrcast_to_value(llvm::CallInst* from) noexcept -> llvm::Value* {
			return static_cast<llvm::Value*>(from);
		};

		auto _ptrcast_to_value(llvm::Argument* from) noexcept -> llvm::Value* {
			return static_cast<llvm::Value*>(from);
		};



		auto _ptrcast_to_type(llvm::IntegerType* from) noexcept -> llvm::Type* {
			return static_cast<llvm::Type*>(from);
		};


		auto _ptrcast_to_type(llvm::PointerType* from) noexcept -> llvm::Type* {
			return static_cast<llvm::Type*>(from);
		};
		
	};
};