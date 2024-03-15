#pragma once


#include <Evo.h>

namespace llvm{
	class LLVMContext;

	class NoFolder;
	class IRBuilderDefaultInserter;
	class IRBuilderBase;

	class Module;
	class TargetMachine;
	class FunctionType;
	class Function;

	class BasicBlock;
	class IntegerType;
	class PointerType;
	class Type;
	class Value;
	class Constant;
	class ConstantInt;
	class GlobalVariable;
	class AllocaInst;
	class LoadInst;
	class StoreInst;
	class ReturnInst;
	class CallInst;

	class ExecutionEngine;
};


// custom interfaces
namespace panther{
	namespace llvmint{
			
		// llvm::GlobalValue::LinkageTypes
		enum class LinkageTypes{
			ExternalLinkage = 0,
			AvailableExternallyLinkage,
			LinkOnceAnyLinkage,
			LinkOnceODRLinkage,
			WeakAnyLinkage,
			WeakODRLinkage,
			AppendingLinkage,
			InternalLinkage,
			PrivateLinkage,
			ExternalWeakLinkage,
			CommonLinkage,
		};



		// Typesafe way to convert between llvm pointer types
		// 	Example usage: panther::llvmint::ptrcast<llvm::Value>(constant)
		template<class To, class From>
		inline auto ptrcast(From* from) noexcept -> To* {
			// If no template specializations exist, this assert fails
			static_assert(sizeof(From*) == 0, "Invalid or unsupported conversion");
		};





		EVO_NODISCARD auto _ptrcast_to_constant(llvm::ConstantInt* from) noexcept -> llvm::Constant*;

		EVO_NODISCARD auto _ptrcast_to_value(llvm::LoadInst* from) noexcept -> llvm::Value*;
		EVO_NODISCARD auto _ptrcast_to_value(llvm::GlobalVariable* from) noexcept -> llvm::Value*;
		EVO_NODISCARD auto _ptrcast_to_value(llvm::ConstantInt* from) noexcept -> llvm::Value*;
		EVO_NODISCARD auto _ptrcast_to_value(llvm::AllocaInst* from) noexcept -> llvm::Value*;




		template<> inline auto ptrcast(llvm::ConstantInt* from) noexcept -> llvm::Constant* { return _ptrcast_to_constant(from); };



		template<> inline auto ptrcast(llvm::LoadInst* from) noexcept -> llvm::Value* { return _ptrcast_to_value(from); };
		template<> inline auto ptrcast(llvm::GlobalVariable* from) noexcept -> llvm::Value* { return _ptrcast_to_value(from); };
		template<> inline auto ptrcast(llvm::ConstantInt* from) noexcept -> llvm::Value* { return _ptrcast_to_value(from); };
		template<> inline auto ptrcast(llvm::AllocaInst* from) noexcept -> llvm::Value* { return _ptrcast_to_value(from); };



	};
};