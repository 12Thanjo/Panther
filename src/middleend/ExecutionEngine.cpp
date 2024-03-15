#include "middleend/ExecutionEngine.h"

#include <LLVM.h>

#include "middleend/Module.h"


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


		static auto run_function(llvm::ExecutionEngine* engine, std::string_view func_name) noexcept -> llvm::GenericValue {
			llvm::Function* func = engine->FindFunctionNamed(func_name);
			return engine->runFunction(func, {});	
		};


		template<>
		auto ExecutionEngine::runFunction<uint64_t>(std::string_view func_name) noexcept -> uint64_t {
			return run_function(this->engine, func_name).IntVal.getLimitedValue();
		};

		template<>
		auto ExecutionEngine::runFunction<uint32_t>(std::string_view func_name) noexcept -> uint32_t {
			uint64_t output = run_function(this->engine, func_name).IntVal.getLimitedValue();
			return uint32_t(output);
		};



		template<>
		auto ExecutionEngine::runFunction<void>(std::string_view func_name) noexcept -> void {
			run_function(this->engine, func_name);
		};
	
	};
};