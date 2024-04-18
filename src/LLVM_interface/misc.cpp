#include "LLVM_interface/misc.h"

#include <LLVM.h>


namespace panther{
	namespace llvmint{
		
		
		auto setupFuncParams(llvm::Function* func, evo::ArrayProxy<ParamInfo> param_infos) noexcept -> void {
			size_t i = 0;
			for(llvm::Argument& arg : func->args()){
				const ParamInfo& param_info = param_infos[i];

				arg.setName(param_info.name);
				arg.addAttr(llvm::Attribute::NoUndef);

				i += 1;
			}
		};

		auto getFuncArguments(llvm::Function* func) noexcept -> std::vector<llvm::Argument*> {
			auto args = std::vector<llvm::Argument*>();

			for(llvm::Argument& arg : func->args()){
				args.emplace_back(&arg);

			}

			return args;
		};

	
	};
};