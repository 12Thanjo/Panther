#pragma once


#include <Evo.h>

#include "frontend/Token.h"
#include "frontend/AST.h"
#include "frontend/SourceID.h"

#include "LLVM_interface/llvm_protos.h"

namespace panther{
	class Source;

	namespace PIR{

		//////////////////////////////////////////////////////////////////////
		// types

		struct TypeID{ // typesafe identifier
			uint32_t id;
			explicit TypeID(uint32_t _id) noexcept : id(_id) {};
			EVO_NODISCARD auto operator==(TypeID rhs) const noexcept -> bool { return this->id == rhs.id; };
		};

		struct IntrinsicID{ // typesafe identifier
			uint32_t id;
			explicit IntrinsicID(uint32_t _id) noexcept : id(_id) {};
			EVO_NODISCARD auto operator==(IntrinsicID rhs) const noexcept -> bool { return this->id == rhs.id; };
		};

		struct FuncID{ // typesafe identifier
			Source& source;
			uint32_t id;
			FuncID(Source& _source, uint32_t _id) noexcept : source(_source), id(_id) {};
		};



		class TypeVoidableID{
			public:
				TypeVoidableID(TypeID type_id) : id(type_id) {};
				~TypeVoidableID() = default;

				EVO_NODISCARD static inline auto Void() noexcept -> TypeVoidableID { return TypeVoidableID(); };

				EVO_NODISCARD inline auto operator==(const TypeVoidableID& rhs) const noexcept -> bool { return this->id == rhs.id; };

				EVO_NODISCARD inline auto typeID() const noexcept -> const TypeID& {
					evo::debugAssert(this->isVoid() == false, "type is void");
					return *this->id;
				};

				EVO_NODISCARD inline auto typeID() noexcept -> TypeID& {
					evo::debugAssert(this->isVoid() == false, "type is void");
					return *this->id;
				};


				EVO_NODISCARD inline auto isVoid() const noexcept -> bool { return !this->id.has_value(); };


			private:
				TypeVoidableID() : id(std::nullopt) {};
		
			private:
				std::optional<TypeID> id;
		};



		class BaseType{
			public:
				struct ID{ // typesafe identifier
					uint32_t id;
					explicit ID(uint32_t _id) noexcept : id(_id) {};
					EVO_NODISCARD auto operator==(const ID& rhs) const noexcept -> bool { return this->id == rhs.id; };
				};

				enum class Kind{
					Import, // for the unnameable type that is returned by @import()
					Builtin,
					Function,
				};


				struct Operator{
					struct Param{
						TypeID type;
						AST::FuncParams::Param::Kind kind;

						EVO_NODISCARD auto operator==(const Param& rhs) const noexcept -> bool;
					};


					std::vector<Param> params;
					PIR::TypeVoidableID returnType;

					EVO_NODISCARD auto operator==(const Operator& rhs) const noexcept -> bool;
				};


				union OverloadedOperator {
					IntrinsicID intrinsic;
					FuncID func;
				};

			public:

				// builtin
				BaseType(Kind _kind, Token::Kind builtin_kind) : kind(_kind){
					evo::debugAssert(_kind == Kind::Builtin, "This constructor must be only used for builtin kind");

					this->builtin.kind = builtin_kind;
				}

				// function / import
				explicit BaseType(Kind _kind) : kind(_kind){
					evo::debugAssert(_kind == Kind::Function || _kind == Kind::Import, "This constructor must be only used for function or import kind");
				}

				~BaseType() = default;
		

				EVO_NODISCARD auto operator==(Token::Kind tok_kind) const noexcept -> bool;

				EVO_NODISCARD auto operator==(const BaseType& rhs) const noexcept -> bool;


			public:
				Kind kind;


				union {
					struct /* import */ {
											
					} import;

					struct /* builtin */ {
						Token::Kind kind;
					} builtin;

					struct /* function */ {
											
					} function;
				};



				std::optional<Operator> callOperator{};

				struct /* ops */ {
					std::vector<OverloadedOperator> add{};
					std::vector<OverloadedOperator> addWrap{};
					std::vector<OverloadedOperator> sub{};
					std::vector<OverloadedOperator> subWrap{};
					std::vector<OverloadedOperator> mul{};
					std::vector<OverloadedOperator> mulWrap{};
					std::vector<OverloadedOperator> div{};
					std::vector<OverloadedOperator> negate{};

					std::vector<OverloadedOperator> logicalEqual{};
					std::vector<OverloadedOperator> notEqual{};
					std::vector<OverloadedOperator> lessThan{};
					std::vector<OverloadedOperator> lessThanEqual{};
					std::vector<OverloadedOperator> greaterThan{};
					std::vector<OverloadedOperator> greaterThanEqual{};
					std::vector<OverloadedOperator> logicalNot{};
					std::vector<OverloadedOperator> logicalAnd{};
					std::vector<OverloadedOperator> logicalOr{};
				} ops;






			private:
				
		};



		struct Type{
			using ID = TypeID;
			using VoidableID = TypeVoidableID;


			BaseType::ID baseType;

			std::vector<AST::Type::Qualifier> qualifiers;


			EVO_NODISCARD auto operator==(const Type& rhs) const noexcept -> bool;
			EVO_NODISCARD auto isImplicitlyConvertableTo(const Type& rhs) const noexcept -> bool;
		};




		//////////////////////////////////////////////////////////////////////
		// statements / expressions

		struct VarID{ // typesafe identifier
			Source& source;
			uint32_t id;
			explicit VarID(Source& _source, uint32_t _id) noexcept : source(_source), id(_id) {};
		};

		struct ParamID{ // typesafe identifier
			uint32_t id;
			explicit ParamID(uint32_t _id) noexcept : id(_id) {};
		};


		struct PrefixID{ // typesafe identifier
			uint32_t id;
			explicit PrefixID(uint32_t _id) noexcept : id(_id) {};
		};

		struct DerefID{ // typesafe identifier
			uint32_t id;
			explicit DerefID(uint32_t _id) noexcept : id(_id) {};
		};

		struct FuncCallID{
			uint32_t id;
			explicit FuncCallID(uint32_t _id) noexcept : id(_id){};
		};



		///////////////////////////////////
		// expressions


		struct Expr{
			enum class Kind{
				Var,
				Param,
				ASTNode,
				FuncCall,
				Prefix,
				Deref,
				Import,
			} kind;

			union {
				VarID var;
				ParamID param;
				AST::Node::ID astNode;
				FuncCallID funcCall;
				PrefixID prefix;
				DerefID deref;
				SourceID import;
			};

			explicit Expr(VarID id) : kind(Kind::Var), var(id) {};
			explicit Expr(ParamID id) : kind(Kind::Param), param(id) {};
			explicit Expr(AST::Node::ID node) : kind(Kind::ASTNode), astNode(node) {};
			explicit Expr(FuncCallID func_call_id) : kind(Kind::FuncCall), funcCall(func_call_id) {};
			explicit Expr(PrefixID prefix_id) : kind(Kind::Prefix), prefix(prefix_id) {};
			explicit Expr(DerefID deref_id) : kind(Kind::Deref), deref(deref_id) {};
			explicit Expr(SourceID src_id) : kind(Kind::Import), import(src_id) {};
		};


		struct FuncCall{
			using ID = FuncCallID;

			enum class Kind{
				Func,
				Intrinsic
			} kind;


			union{
				FuncID func;
				IntrinsicID intrinsic;
			};

			std::vector<Expr> args;

			FuncCall(FuncID func_id, std::vector<Expr> _args) : kind(Kind::Func), func(func_id), args(_args) {};
			FuncCall(IntrinsicID intrinsic_id, std::vector<Expr> _args) : kind(Kind::Intrinsic), intrinsic(intrinsic_id), args(_args) {};
		};



		struct Prefix{
			using ID = PrefixID;

			Token::ID op;
			Expr rhs;
		};

		struct Deref{
			using ID = DerefID;

			Expr ptr;
			Type::ID type;
		};


		///////////////////////////////////
		// statements

		struct Var{
			using ID = VarID;

			Token::ID ident;
			Type::ID type;
			Expr value;
			bool isDef;
			bool isExport;

			union {
				llvm::GlobalVariable* global = nullptr;
				llvm::AllocaInst* alloca;
			} llvm;
			bool is_alloca = false;

			EVO_NODISCARD inline auto isGlobal() const noexcept -> bool { return !this->is_alloca; };
		};

		struct Param{
			using ID = ParamID;

			Token::ID ident;
			Type::ID type;
			AST::FuncParams::Param::Kind kind;

			llvm::AllocaInst* alloca = nullptr;
			bool mayHaveBeenEdited = false; // it's impossible to detech this 100%
		};


		


		struct Return{
			struct ID{ // typesafe identifier
				uint32_t id;
				explicit ID(uint32_t _id) noexcept : id(_id) {};
			};

			std::optional<Expr> value;
		};


		struct Assignment{
			struct ID{ // typesafe identifier
				uint32_t id;
				explicit ID(uint32_t _id) noexcept : id(_id) {};
			};

			Expr dst;
			Token::ID op;
			Expr value;
		};



		struct ConditionalID{ // typesafe identifier
			uint32_t id;
			explicit ConditionalID(uint32_t _id) noexcept : id(_id) {};
		};

		struct Stmt{
			enum class Kind{
				Var,
				Return,
				Assignment,
				FuncCall,
				Conditional,
				Unreachable,
			} kind;

			union {
				evo::byte dummy;

				Var::ID var;
				Return::ID ret;
				Assignment::ID assignment;
				FuncCall::ID funcCall;
				ConditionalID conditional;
			};

			explicit Stmt(Var::ID id) : kind(Kind::Var), var(id) {};
			explicit Stmt(Return::ID id) : kind(Kind::Return), ret(id) {};
			explicit Stmt(Assignment::ID id) : kind(Kind::Assignment), assignment(id) {};
			explicit Stmt(FuncCall::ID id) : kind(Kind::FuncCall), funcCall(id) {};
			explicit Stmt(ConditionalID id) : kind(Kind::Conditional), conditional(id) {};

			EVO_NODISCARD static inline auto getUnreachable() noexcept -> Stmt { return Stmt(Kind::Unreachable); };


			private:
				Stmt(Kind _kind) : kind(_kind) {};
		};



		class StmtBlock{
			public:
				StmtBlock() noexcept : stmts() {};
				~StmtBlock() = default;

				StmtBlock(const StmtBlock& rhs) noexcept : stmts(rhs.stmts) {};
				StmtBlock(StmtBlock&& rhs) noexcept : stmts(std::move(rhs.stmts)), is_terminated(rhs.is_terminated) {
					rhs.is_terminated = false;
				};

				

				EVO_NODISCARD inline auto setTerminated() noexcept -> void { this->is_terminated = true; };
				EVO_NODISCARD inline auto isTerminated() const noexcept -> bool { return this->is_terminated; };



				///////////////////////////////////
				// interface with the stmts vector
				
				inline auto emplace_back(auto&&... args) noexcept -> Stmt& {
					return this->stmts.emplace_back(std::forward<decltype(args)>(args)...);
				};


				inline auto begin()       noexcept -> std::vector<Stmt>::iterator       { return this->stmts.begin(); };
				inline auto begin() const noexcept -> std::vector<Stmt>::const_iterator { return this->stmts.begin(); };

				inline auto end()       noexcept -> std::vector<Stmt>::iterator       { return this->stmts.end(); };
				inline auto end() const noexcept -> std::vector<Stmt>::const_iterator { return this->stmts.end(); };

				EVO_NODISCARD inline auto empty() const noexcept -> bool { return this->stmts.empty(); };

		
			private:
				std::vector<Stmt> stmts;
				bool is_terminated = false;
		};




		struct Conditional{
			using ID = ConditionalID;

			Expr ifCond;
			StmtBlock thenStmts;
			StmtBlock elseStmts;
		};



		struct Func{
			using ID = FuncID;


			Token::ID ident;

			BaseType::ID baseType;
			std::vector<Param::ID> params;
			Type::VoidableID returnType; // nullopt means Void

			bool isExport;
			

			llvm::Function* llvmFunc = nullptr;
			StmtBlock stmts{};
			bool terminatesInBaseScope = false;
		};




		struct Intrinsic{
			using ID = IntrinsicID;

			//////////////////////////////////////////////////////////////////////
			// 																	//
			// IMPORTANT: if the order of Kind ever changes, the order in 		//
			// 		SourceManager::initIntrinsics() must be made to match		//
			// 																	//
			//////////////////////////////////////////////////////////////////////

			enum class Kind{
				import,
				breakpoint,

				// add
				addInt,
				addUInt,

				// add wrap
				addWrapInt,
				addWrapUInt,

				// sub
				subInt,
				subUInt,

				// sub wrap
				subWrapInt,
				subWrapUInt,

				// mul
				mulInt,
				mulUInt,

				// mul wrap
				mulWrapInt,
				mulWrapUInt,

				// div
				divInt,
				divUInt,

				// negate
				negateInt,


				// Logical Int
				equalInt,
				notEqualInt,
				lessThanInt,
				lessThanEqualInt,
				greaterThanInt,
				greaterThanEqualInt,

				// logical UInt
				equalUInt,
				notEqualUInt,
				lessThanUInt,
				lessThanEqualUInt,
				greaterThanUInt,
				greaterThanEqualUInt,

				// logical Bool
				equalBool,
				notEqualBool,
				logicalAnd,
				logicalOr,
				logicalNot,



				__printHelloWorld,
				__printSeparator,
				__printInt,
				__printUInt,


				_MAX_, // not an actual intrinsic
			} kind;


			std::string_view ident;

			BaseType::ID baseType;
		};



	};
};