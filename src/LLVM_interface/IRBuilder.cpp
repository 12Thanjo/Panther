#include "LLVM_interface/IRBuilder.h"

#include <LLVM.h>

#include "LLVM_interface/Module.h"

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
			return llvm::BasicBlock::Create(this->getContext(), name.c_str(), func);
		};


		auto IRBuilder::createAlloca(llvm::Type* type, llvm::Value* array_length, evo::CStrProxy name) noexcept -> llvm::AllocaInst* {
			return this->builder->CreateAlloca(type, array_length, name.c_str());
		};
		auto IRBuilder::createAlloca(llvm::Type* type, evo::CStrProxy name) noexcept -> llvm::AllocaInst* {
			return this->createAlloca(type, nullptr, name);
		};


		auto IRBuilder::createLoad(llvm::Value* value, llvm::Type* type, evo::CStrProxy name) noexcept -> llvm::LoadInst* {
			return this->builder->CreateLoad(type, value, name.c_str());
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

		auto IRBuilder::createUnreachable() noexcept -> llvm::UnreachableInst* {
			return this->builder->CreateUnreachable();
		};


		auto IRBuilder::createBranch(llvm::BasicBlock* block) noexcept -> llvm::BranchInst* {
			return this->builder->CreateBr(block);
		};

		auto IRBuilder::createCondBranch(llvm::Value* cond, llvm::BasicBlock* then_block, llvm::BasicBlock* else_block) noexcept -> llvm::BranchInst* {
			return this->builder->CreateCondBr(cond, then_block, else_block);
		};




		auto IRBuilder::createCall(llvm::Function* func, evo::ArrayProxy<llvm::Value*> params, evo::CStrProxy name) noexcept -> llvm::CallInst* {
			llvm::CallInst* call_inst = this->builder->CreateCall(func, llvm::ArrayRef<llvm::Value*>{params.data(), params.size()}, name.c_str());
			call_inst->setDoesNotThrow();

			return call_inst;
		};

		auto IRBuilder::createIntrinsicCall(IntrinsicID id, evo::ArrayProxy<llvm::Value*> params) noexcept -> llvm::CallInst* {
			// llvm/IR/IntrinsicEnums.inc
			const llvm::Intrinsic::ID intrinsic_id = [&]() noexcept {
				switch(id){
					case IntrinsicID::debugtrap: return llvm::Intrinsic::IndependentIntrinsics::debugtrap;
					default: EVO_FATAL_BREAK("Unknown llvm intrinsic");
				};
			}();

			return this->builder->CreateIntrinsic(this->getTypeVoid(), intrinsic_id, llvm::ArrayRef<llvm::Value*>{params.data(), params.size()});
		};


		auto IRBuilder::createPhi(llvm::Type* type, evo::ArrayProxy<PhiIncoming> incoming, evo::CStrProxy name) noexcept -> llvm::Value* {
			llvm::PHINode* phi = this->builder->CreatePHI(type, evo::uint(incoming.size()), name.c_str());

			for(const PhiIncoming& incom : incoming){
				phi->addIncoming(incom.value, incom.basic_block);
			}

			return static_cast<llvm::Value*>(phi);
		};



		///////////////////////////////////
		// operators

		auto IRBuilder::createAdd(llvm::Value* lhs, llvm::Value* rhs, bool nuw, bool nsw, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateAdd(lhs, rhs, name.c_str(), nuw, nsw);
		};



		auto IRBuilder::createSub(llvm::Value* lhs, llvm::Value* rhs, bool nuw, bool nsw, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateSub(lhs, rhs, name.c_str(), nuw, nsw);
		};


		auto IRBuilder::createMul(llvm::Value* lhs, llvm::Value* rhs, bool nuw, bool nsw, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateMul(lhs, rhs, name.c_str(), nuw, nsw);
		};



		auto IRBuilder::createUDiv(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateUDiv(lhs, rhs, name.c_str(), false);
		};

		auto IRBuilder::createSDiv(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateSDiv(lhs, rhs, name.c_str(), false);
		};





		auto IRBuilder::createICmpEQ(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateICmpEQ(lhs, rhs, name.c_str());
		};

		auto IRBuilder::createICmpNE(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateICmpNE(lhs, rhs, name.c_str());
		};

		auto IRBuilder::createICmpUGT(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateICmpUGT(lhs, rhs, name.c_str());
		};

		auto IRBuilder::createICmpUGE(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateICmpUGE(lhs, rhs, name.c_str());
		};

		auto IRBuilder::createICmpULT(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateICmpULT(lhs, rhs, name.c_str());
		};

		auto IRBuilder::createICmpULE(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateICmpULE(lhs, rhs, name.c_str());
		};

		auto IRBuilder::createICmpSGT(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateICmpSGT(lhs, rhs, name.c_str());
		};

		auto IRBuilder::createICmpSGE(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateICmpSGE(lhs, rhs, name.c_str());
		};

		auto IRBuilder::createICmpSLT(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateICmpSLT(lhs, rhs, name.c_str());
		};

		auto IRBuilder::createICmpSLE(llvm::Value* lhs, llvm::Value* rhs, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateICmpSLE(lhs, rhs, name.c_str());
		};



		auto IRBuilder::createNot(llvm::Value* value, evo::CStrProxy name) noexcept -> llvm::Value* {
			return this->builder->CreateNot(value, name.c_str());
		};



		//////////////////////////////////////////////////////////////////////
		// set

		auto IRBuilder::setInsertionPoint(llvm::BasicBlock* block) noexcept -> void {
			this->builder->SetInsertPoint(block);
		};

		auto IRBuilder::setInsertionPointAtBack(llvm::Function* function) noexcept -> void {
			this->setInsertionPoint(&function->back());
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

		auto IRBuilder::valueCInt(int num) noexcept -> llvm::ConstantInt* {
			return this->builder->getInt32(unsigned(num));
		};


		auto IRBuilder::valueBool(bool val) noexcept -> llvm::ConstantInt* {
			return this->builder->getInt1(val);
		};



		auto IRBuilder::valueString(evo::CStrProxy str, evo::CStrProxy name) noexcept -> llvm::GlobalVariable* {
			return this->builder->CreateGlobalString(str.c_str(), name.c_str());;
		};


		auto IRBuilder::valueGlobal(
			llvmint::Module& module, llvm::Constant* value, llvm::Type* type, llvmint::LinkageTypes linkage, bool is_constant, evo::CStrProxy name
		) noexcept -> llvm::GlobalVariable* {
			// this gets freed automatically in the destructor of the module
			llvm::GlobalVariable* global = new llvm::GlobalVariable(
				module.getModule(), type, is_constant, static_cast<llvm::GlobalValue::LinkageTypes>(linkage), value, name.c_str()
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


		auto IRBuilder::getTypeCInt() noexcept -> llvm::IntegerType* {
			 return this->builder->getInt32Ty();
		};




		auto IRBuilder::getTypePtr() noexcept -> llvm::PointerType* {
			return this->builder->getPtrTy();
		};

		auto IRBuilder::getTypeVoid() noexcept -> llvm::Type* { return this->builder->getVoidTy(); };



		//////////////////////////////////////////////////////////////////////
		// getters


		auto IRBuilder::getContext() noexcept -> llvm::LLVMContext& {
			return this->builder->getContext();
		};

		auto IRBuilder::getInsertPoint() noexcept -> llvm::BasicBlock* {
			return this->builder->GetInsertBlock();
		};



	};
};