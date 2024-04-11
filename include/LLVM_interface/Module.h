#pragma once


#include <Evo.h>

#include "LLVM_interface/llvm_protos.h"


namespace panther{
	namespace llvmint{
		

		class Module{
			public:
				Module(std::string_view name, llvm::LLVMContext& context);
				~Module();

				EVO_NODISCARD auto createFunction(
					evo::CStrProxy name, llvm::FunctionType* prototype, llvmint::LinkageTypes linkage, bool nothrow, bool fast_call_conv
				) noexcept -> llvm::Function*;


				EVO_NODISCARD auto getPointerSize() const noexcept -> unsigned;
				EVO_NODISCARD auto getTypeSize(llvm::Type* type) const noexcept -> uint64_t;

				EVO_NODISCARD auto print() const noexcept -> std::string;


				static auto getDefaultTargetTriple() noexcept -> std::string;

				auto setTargetTriple(const std::string& target_triple) noexcept -> void;

				// return is error message (empty if no error)
				EVO_NODISCARD auto setDataLayout(const std::string& target_triple, std::string_view cpu, std::string_view features) noexcept -> std::string;


				// return nullopt means target machine cannot output object file
				EVO_NODISCARD auto compileToObjectFile() noexcept -> std::optional<std::vector<evo::byte>>;



				EVO_NODISCARD inline auto getModule() const noexcept -> llvm::Module& { return *this->module; };

			private:

				EVO_NODISCARD auto get_clone() const noexcept -> std::unique_ptr<llvm::Module>;


				friend class ExecutionEngine;
		

			private:
				llvm::Module* module = nullptr;
				llvm::TargetMachine* target_machine = nullptr;
		};


	};
};