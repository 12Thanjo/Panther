#pragma once


#include <Evo.h>

#include "middleend/llvm_protos.h"


namespace panther{
	namespace llvmint{


		class IRBuilder{
			public:
				IRBuilder(llvm::LLVMContext& context);
				~IRBuilder();


				//////////////////////////////////////////////////////////////////////
				// create

				EVO_NODISCARD auto createBasicBlock(llvm::Function* func, evo::CStrProxy name = '\0') noexcept -> llvm::BasicBlock*;

				EVO_NODISCARD auto createAlloca(llvm::Type* type, llvm::Value* array_length, evo::CStrProxy name = '\0') noexcept -> llvm::AllocaInst*;
				EVO_NODISCARD auto createAlloca(llvm::Type* type, evo::CStrProxy name = '\0') noexcept -> llvm::AllocaInst*;

				EVO_NODISCARD auto createLoad(llvm::Value* value, llvm::Type* type, evo::CStrProxy name = '\0') noexcept -> llvm::LoadInst*;
				EVO_NODISCARD auto createLoad(llvm::AllocaInst* alloca, evo::CStrProxy name = '\0') noexcept -> llvm::LoadInst*;

				auto createStore(llvm::AllocaInst* dst, llvm::Value* source, bool is_volatile = false) noexcept -> llvm::StoreInst*;
				auto createStore(llvm::Value* dst, llvm::Value* source, bool is_volatile = false) noexcept -> llvm::StoreInst*;

				auto createRet(llvm::Value* value) noexcept -> llvm::ReturnInst*;
				auto createRet() noexcept -> llvm::ReturnInst*;

				auto createCall(llvm::Function* func, evo::ArrayProxy<llvm::Value*> params, evo::CStrProxy name = '\0') noexcept -> llvm::CallInst*;


				//////////////////////////////////////////////////////////////////////
				// set

				auto setInsertionPoint(llvm::BasicBlock* block) noexcept -> void;


				//////////////////////////////////////////////////////////////////////
				// values

				EVO_NODISCARD auto valueUI32(uint32_t num) noexcept -> llvm::ConstantInt*;
				EVO_NODISCARD auto valueUI64(uint64_t num) noexcept -> llvm::ConstantInt*;
				EVO_NODISCARD auto valueUI_N(unsigned n, uint64_t num) noexcept -> llvm::ConstantInt*;

				EVO_NODISCARD auto valueBool(bool val) noexcept -> llvm::ConstantInt*;

				EVO_NODISCARD auto valueString(evo::CStrProxy str, evo::CStrProxy name = '\0') noexcept -> llvm::GlobalVariable*;

				// TODO: linkage
				EVO_NODISCARD auto valueGlobal(
					class Module& module, llvm::Constant* value, llvm::Type* type, bool is_constant, evo::CStrProxy name = '\0'
				) noexcept -> llvm::GlobalVariable*;


				//////////////////////////////////////////////////////////////////////
				// types

				EVO_NODISCARD auto getFuncProto(llvm::Type* return_type, evo::ArrayProxy<llvm::Type*> params, bool is_var_args) noexcept -> llvm::FunctionType*;


				EVO_NODISCARD auto getTypeBool() noexcept -> llvm::IntegerType*;


				EVO_NODISCARD auto getTypeI8()  noexcept -> llvm::IntegerType*;
				EVO_NODISCARD auto getTypeI16() noexcept -> llvm::IntegerType*;
				EVO_NODISCARD auto getTypeI32() noexcept -> llvm::IntegerType*;
				EVO_NODISCARD auto getTypeI64() noexcept -> llvm::IntegerType*;
				EVO_NODISCARD auto getTypeI128() noexcept -> llvm::IntegerType*;

				EVO_NODISCARD auto getTypeI_N(unsigned n) noexcept -> llvm::IntegerType*;



				EVO_NODISCARD auto getTypePtr() noexcept -> llvm::PointerType*;

				EVO_NODISCARD auto getTypeVoid() noexcept -> llvm::Type*;



				//////////////////////////////////////////////////////////////////////
				// getters


				EVO_NODISCARD inline auto getBuilder() const noexcept -> llvm::IRBuilderBase& { return *this->builder; };
				EVO_NODISCARD auto getContext() const noexcept -> llvm::LLVMContext&;


		
			private:
				llvm::IRBuilderBase* builder = nullptr;

				llvm::NoFolder* folder = nullptr;
				llvm::IRBuilderDefaultInserter* inserter = nullptr;
		};


	};
};