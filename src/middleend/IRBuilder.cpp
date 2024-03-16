#include "middleend/IRBuilder.h"

#include <LLVM.h>

#include "middleend/Module.h"

namespace panther{
	namespace llvmint{
		

		IRBuilder::IRBuilder(llvm::LLVMContext& context){
			this->folder = new llvm::NoFolder();
			this->inserter = new llvm::IRBuilderDefaultInserter();


			llvm::MDNode* FPMathTag = nullptr;
			llvm::ArrayRef<llvm::OperandBundleDef> OpBundles = std::nullopt;

			this->builder = new llvm::IRBuilderBase(context, *this->folder, *this->inserter, FPMathTag, OpBundles);
		}
		
		IRBuilder::~IRBuilder(){
			delete this->builder;
			delete this->folder;
			delete this->inserter;
		}




		//////////////////////////////////////////////////////////////////////
		// create

		auto IRBuilder::createBasicBlock(llvm::Function* func, evo::CStrProxy name) noexcept -> llvm::BasicBlock* {
			return llvm::BasicBlock::Create(this->getContext(), name.data(), func);
		};


		auto IRBuilder::createAlloca(llvm::Type* type, llvm::Value* array_length, evo::CStrProxy name) noexcept -> llvm::AllocaInst* {
			return this->builder->CreateAlloca(type, array_length, name.data());
		};
		auto IRBuilder::createAlloca(llvm::Type* type, evo::CStrProxy name) noexcept -> llvm::AllocaInst* {
			return this->createAlloca(type, nullptr, name);
		};


		auto IRBuilder::createLoad(llvm::Value* value, llvm::Type* type, evo::CStrProxy name) noexcept -> llvm::LoadInst* {
			return this->builder->CreateLoad(type, value, name.data());
		};
		auto IRBuilder::createLoad(llvm::AllocaInst* alloca, evo::CStrProxy name) noexcept -> llvm::LoadInst* {
			return this->createLoad(alloca, alloca->getAllocatedType(), name);
		};


		auto IRBuilder::createStore(llvm::AllocaInst* dst, llvm::Value* source, bool is_volatile) noexcept -> llvm::StoreInst* {
			return this->builder->CreateStore(source, dst, is_volatile);
		};
		auto IRBuilder::createStore(llvm::Value* dst, llvm::Value* source, bool is_volatile) noexcept -> llvm::StoreInst* {
			return this->builder->CreateStore(source, dst, is_volatile);
		};


		auto IRBuilder::createRet(llvm::Value* value) noexcept -> llvm::ReturnInst* {
			return this->builder->CreateRet(value);
		};

		auto IRBuilder::createRet() noexcept -> llvm::ReturnInst* {
			return this->builder->CreateRetVoid();
		};



		auto IRBuilder::createCall(llvm::Function* func, evo::ArrayProxy<llvm::Value*> params, evo::CStrProxy name) noexcept -> llvm::CallInst* {
			return this->builder->CreateCall(func, llvm::ArrayRef<llvm::Value*>{params.data(), params.size()}, name.data());
		};



		//////////////////////////////////////////////////////////////////////
		// set

		auto IRBuilder::setInsertionPoint(llvm::BasicBlock* block) noexcept -> void {
			this->builder->SetInsertPoint(block);
		};


		//////////////////////////////////////////////////////////////////////
		// values

		auto IRBuilder::valueUI32(uint32_t num) noexcept -> llvm::ConstantInt* {
			return this->builder->getInt32(num);
		};

		auto IRBuilder::valueUI64(uint64_t num) noexcept -> llvm::ConstantInt* {
			return this->builder->getInt64(num);
		};

		auto IRBuilder::valueUI_N(unsigned n, uint64_t num) noexcept -> llvm::ConstantInt* {
			return this->builder->getIntN(n, num);
		};


		auto IRBuilder::valueBool(bool val) noexcept -> llvm::ConstantInt* {
			return this->builder->getInt1(val);
		};



		auto IRBuilder::valueString(evo::CStrProxy str, evo::CStrProxy name) noexcept -> llvm::GlobalVariable* {
			return this->builder->CreateGlobalString(str.data(), name.data());;
		};


		auto IRBuilder::valueGlobal(
			llvmint::Module& module, llvm::Constant* value, llvm::Type* type, bool is_constant, evo::CStrProxy name
		) noexcept -> llvm::GlobalVariable* {
			// this gets freed automatically in the destructor of the module
			llvm::GlobalVariable* global = new llvm::GlobalVariable(
				module.getModule(), type, is_constant, llvm::GlobalValue::PrivateLinkage, value, name.data()
			);

			global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
			// TODO: set alignment?

			return global;
		};


		//////////////////////////////////////////////////////////////////////
		// types

		auto IRBuilder::getFuncProto(llvm::Type* return_type, evo::ArrayProxy<llvm::Type*> params, bool is_var_args) noexcept -> llvm::FunctionType* {
			return llvm::FunctionType::get(return_type, llvm::ArrayRef(params.data(), params.size()), is_var_args);
		};


		auto IRBuilder::getTypeBool()  noexcept -> llvm::IntegerType* {
			 return this->builder->getInt1Ty();
		};


		auto IRBuilder::getTypeI8()  noexcept -> llvm::IntegerType* {
			 return this->builder->getInt8Ty();
		};

		auto IRBuilder::getTypeI16() noexcept -> llvm::IntegerType* {
			 return this->builder->getInt16Ty();
		};

		auto IRBuilder::getTypeI32() noexcept -> llvm::IntegerType* {
			 return this->builder->getInt32Ty();
		};

		auto IRBuilder::getTypeI64() noexcept -> llvm::IntegerType* {
			 return this->builder->getInt64Ty();
		};

		auto IRBuilder::getTypeI128() noexcept -> llvm::IntegerType* {
			 return this->builder->getInt128Ty();
		};


		auto IRBuilder::getTypeI_N(unsigned n) noexcept -> llvm::IntegerType* {
			 return this->builder->getIntNTy(n);
		};




		auto IRBuilder::getTypePtr() noexcept -> llvm::PointerType* {
			return this->builder->getPtrTy();
		};

		auto IRBuilder::getTypeVoid() noexcept -> llvm::Type* { return this->builder->getVoidTy(); };



		//////////////////////////////////////////////////////////////////////
		// getters


		auto IRBuilder::getContext() const noexcept -> llvm::LLVMContext& {
			return this->builder->getContext();
		};



	};
};