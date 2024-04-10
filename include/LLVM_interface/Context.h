#pragma once


#include <Evo.h>

#include "LLVM_interface/llvm_protos.h"


namespace panther{
	namespace llvmint{
		

		class Context{
			public:
				Context() = default;
				~Context() {
					evo::debugAssert(this->context == nullptr, "Did not call shutdown() before destructor");
				};

				auto init() noexcept -> void;
				auto shutdown() noexcept -> void;



				EVO_NODISCARD inline auto isInitialized() const noexcept -> bool { return this->context != nullptr; };

				EVO_NODISCARD inline auto getContext() noexcept -> llvm::LLVMContext& {
					evo::debugAssert(this->isInitialized(), "Cannot get context when not initialized");
					return *this->context;
				};

		
			private:
				llvm::LLVMContext* context = nullptr;
		};


	};
};