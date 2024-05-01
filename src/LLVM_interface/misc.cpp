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

				if(param_info.readonly){ arg.addAttr(llvm::Attribute::ReadOnly); }
				if(param_info.nonnull){ arg.addAttr(llvm::Attribute::NonNull); }
				if(param_info.noalias){ arg.addAttr(llvm::Attribute::NoAlias); }

				auto attr_builder = llvm::AttrBuilder(func->getContext());

				if(param_info.dereferenceable.bytes != 0){
					if(param_info.dereferenceable.can_be_null){
						attr_builder.addDereferenceableOrNullAttr(param_info.dereferenceable.bytes);
					}else{
						attr_builder.addDereferenceableAttr(param_info.dereferenceable.bytes);
					}
				}

				arg.addAttrs(attr_builder);

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



		auto setFuncNoReturn(llvm::Function* func) noexcept -> void {
			func->addFnAttr(llvm::Attribute::AttrKind::NoReturn);
		};

	
	};
};