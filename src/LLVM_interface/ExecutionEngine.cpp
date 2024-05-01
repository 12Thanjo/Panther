#include "LLVM_interface/ExecutionEngine.h"

#include <LLVM.h>

#include "LLVM_interface/Module.h"


namespace panther{
	namespace llvmint{
		
		
		auto ExecutionEngine::createEngine(const Module& module) noexcept -> void {
			evo::debugAssert(this->hasCreatedEngine() == false, "Execution engine already created");

			this->engine = llvm::EngineBuilder(module.get_clone()).setEngineKind(llvm::EngineKind::JIT).create();
		};


		auto ExecutionEngine::shutdownEngine() noexcept -> void {
			evo::debugAssert(this->hasCreatedEngine(), "Execution engine is not created and cannot be shutdown");

			delete this->engine;
			this->engine = nullptr;
		};



		auto ExecutionEngine::getFuncAddress(std::string_view func_name) noexcept -> uint64_t {
			const std::string func_name_str = std::string(func_name);
			return this->engine->getFunctionAddress(func_name_str);
		};




		template<>
		auto ExecutionEngine::runFunction<void>(std::string_view func_name) noexcept -> void {
			const uint64_t func_addr = this->getFuncAddress(func_name);

			using FuncType = void(*)(void);
			const FuncType func = (FuncType)func_addr;
			func();
		};
	
	};
};