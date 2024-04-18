#pragma once


#include <Evo.h>

#include "LLVM_interface/llvm_protos.h"


namespace panther{
	namespace llvmint{
		
		struct ParamInfo{
			std::string_view name;
		};

		auto setupFuncParams(llvm::Function* func, evo::ArrayProxy<ParamInfo> param_infos) noexcept -> void;

		EVO_NODISCARD auto getFuncArguments(llvm::Function* func) noexcept -> std::vector<llvm::Argument*>;


	};
};