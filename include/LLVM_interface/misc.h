#pragma once


#include <Evo.h>

#include "LLVM_interface/llvm_protos.h"


namespace panther{
	namespace llvmint{
		
		struct ParamInfo{
			std::string_view name;
			bool readonly = false;
			bool nonnull = false;
			bool noalias = false;

			struct Dereferenceable{
				uint64_t bytes = 0; // size{0} means can't dereferenced
				bool can_be_null = true;
			} dereferenceable = {};

			// TODO: align?
		};

		auto setupFuncParams(llvm::Function* func, evo::ArrayProxy<ParamInfo> param_infos) noexcept -> void;

		EVO_NODISCARD auto getFuncArguments(llvm::Function* func) noexcept -> std::vector<llvm::Argument*>;


	};
};