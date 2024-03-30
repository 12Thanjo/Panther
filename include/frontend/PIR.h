#pragma once


#include <Evo.h>

#include "Token.h"
#include "frontend/AST.h"

#include "middleend/llvm_protos.h"

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



		class BaseType{
			public:
				struct ID{ // typesafe identifier
					uint32_t id;
					explicit ID(uint32_t _id) noexcept : id(_id) {};
				};

				enum class Kind{
					Builtin,
					Function,
				};


				struct Operator{
					std::vector<TypeID> params;
					std::optional<TypeID> return_type; // nullopt means Void

					EVO_NODISCARD auto operator==(const Operator& rhs) const noexcept -> bool;
				};

			public:

				// builtin
				BaseType(Kind _kind, Token::Kind builtin_kind) : kind(_kind){
					evo::debugAssert(_kind == Kind::Builtin, "This constructor must be only used for builtin kind");

					this->builtin.kind = builtin_kind;
				}


				// function
				BaseType(Kind _kind) : kind(_kind){
					evo::debugAssert(_kind == Kind::Function, "This constructor must be only used for function kind");
				}

				~BaseType() = default;
		

				EVO_NODISCARD auto operator==(Token::Kind tok_kind) const noexcept -> bool;

				EVO_NODISCARD auto operator==(const BaseType& rhs) const noexcept -> bool;


			public:
				Kind kind;


				union {
					struct /* builtin */ {
						Token::Kind kind;
					} builtin;


					struct /* function */ {
											
					} function;
				};


				std::vector<Operator> call_operators{};

			private:
				
		};



		struct Type{
			using ID = TypeID;


			BaseType::ID base_type;

			std::vector<AST::Type::Qualifier> qualifiers;


			EVO_NODISCARD auto operator==(const Type& rhs) const noexcept -> bool;
		};




		//////////////////////////////////////////////////////////////////////
		// statements / expressions

		struct VarID{ // typesafe identifier
			uint32_t id;
			explicit VarID(uint32_t _id) noexcept : id(_id) {};
		};

		struct FuncID{ // typesafe identifier
			uint32_t id;
			explicit FuncID(uint32_t _id) noexcept : id(_id) {};
		};


		struct PrefixID{ // typesafe identifier
			uint32_t id;
			explicit PrefixID(uint32_t _id) noexcept : id(_id) {};
		};

		struct DerefID{ // typesafe identifier
			uint32_t id;
			explicit DerefID(uint32_t _id) noexcept : id(_id) {};
		};



		///////////////////////////////////
		// expressions

		struct FuncCall{
			struct ID{ // typesafe identifier
				uint32_t id;
				explicit ID(uint32_t _id) noexcept : id(_id) {};
			};

			FuncID func;
		};



		struct Expr{
			enum class Kind{
				Var,
				ASTNode,
				FuncCall,
				Prefix,
				Deref,
			} kind;

			union {
				VarID var;
				AST::Node::ID ast_node;
				FuncCall::ID func_call;
				PrefixID prefix;
				DerefID deref;
			};

			explicit Expr(VarID id) : kind(Kind::Var), var(id) {};
			explicit Expr(AST::Node::ID node) : kind(Kind::ASTNode), ast_node(node) {};
			explicit Expr(FuncCall::ID func_call_id) : kind(Kind::FuncCall), func_call(func_call_id) {};
			explicit Expr(PrefixID prefix_id) : kind(Kind::Prefix), prefix(prefix_id) {};
			explicit Expr(DerefID deref_id) : kind(Kind::Deref), deref(deref_id) {};
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

			union {
				llvm::Value* value = nullptr;
				llvm::AllocaInst* alloca;
			} llvm;
			bool is_alloca = false;

			EVO_NODISCARD inline auto isGlobal() const noexcept -> bool { return !this->is_alloca; };
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




		struct Stmt{
			enum class Kind{
				Var,
				Return,
				Assignment,
				FuncCall,
			} kind;

			union {
				Var::ID var;
				Return::ID ret;
				Assignment::ID assignment;
				FuncCall::ID func_call;
			};

			Stmt(Var::ID id) : kind(Kind::Var), var(id) {};
			Stmt(Return::ID id) : kind(Kind::Return), ret(id) {};
			Stmt(Assignment::ID id) : kind(Kind::Assignment), assignment(id) {};
			Stmt(FuncCall::ID id) : kind(Kind::FuncCall), func_call(id) {};
		};





		struct Func{
			using ID = FuncID;


			Token::ID ident;

			BaseType::ID base_type;
			std::optional<Type::ID> return_type; // nullopt means Void

			bool is_export;
			

			llvm::Function* llvm_func = nullptr;
			std::vector<Stmt> stmts{};

			bool returns = false;
		};



	};
};