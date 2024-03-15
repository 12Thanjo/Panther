#pragma once


#include <Evo.h>

#include "middleend/llvm_protos.h"


namespace panther{
	namespace llvmint{
		

		class ExecutionEngine{
			public:
				ExecutionEngine() = default;
				~ExecutionEngine(){
					evo::debugAssert(this->hasCreatedEngine() == false, "ExecutionEngine destructor run without shutting down");
				};

				// creates copy of the module (module.getClone())
				auto createEngine(const class Module& module) noexcept -> void;

				auto shutdownEngine() noexcept -> void;


				template<typename T>
				EVO_NODISCARD auto runFunction(std::string_view func_name) noexcept -> T;

				template<>
				EVO_NODISCARD auto runFunction<void>(std::string_view func_name) noexcept -> void;


				EVO_NODISCARD inline auto hasCreatedEngine() const noexcept -> bool { return this->engine != nullptr; };
		
			private:
				llvm::ExecutionEngine* engine = nullptr;
		};


	};
};