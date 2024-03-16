#pragma once


#include <Evo.h>

#include "Token.h"
#include "frontend/AST.h"

#include "middleend/llvm_protos.h"

namespace panther{
	class Source;

	namespace object{

		//////////////////////////////////////////////////////////////////////
		// types
		
		struct BaseType{
			struct ID{ // typesafe identifier
				uint32_t id;
				explicit ID(uint32_t _id) noexcept : id(_id) {};
			};


			bool is_builtin;

			union {
				Token::Kind builtin;
			};


			EVO_NODISCARD auto operator==(Token::Kind kind) const noexcept -> bool;
		};


		struct Type{
			struct ID{ // typesafe identifier
				uint32_t id;
				explicit ID(uint32_t _id) noexcept : id(_id) {};
				EVO_NODISCARD auto operator==(ID rhs) const noexcept -> bool { return this->id == rhs.id; };
			};


			BaseType::ID base_type;


			EVO_NODISCARD auto operator==(const Type& rhs) const noexcept -> bool;
		};




		//////////////////////////////////////////////////////////////////////
		// statements / expressions

		struct VarID{ // typesafe identifier
			uint32_t id;
			explicit VarID(uint32_t _id) noexcept : id(_id) {};
		};


		struct Expr{
			enum class Kind{
				Var,
				ASTNode,
			} kind;

			union {
				VarID var;
				AST::Node::ID ast_node;
			};

			Expr(VarID id) : kind(Kind::Var), var(id) {};
			Expr(AST::Node::ID node) : kind(Kind::ASTNode), ast_node(node) {};
		};


		struct Var{
			using ID = VarID;

			Token::ID ident;
			Type::ID type;
			// AST::Node::ID value;
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

			Var::ID var;
			Token::ID op;
			Expr value;
		};




		struct Stmt{
			enum class Kind{
				Var,
				Return,
				Assignment,
			} kind;

			union {
				Var::ID var;
				Return::ID ret;
				Assignment::ID assignment;
			};

			Stmt(Var::ID id) : kind(Kind::Var), var(id) {};
			Stmt(Return::ID id) : kind(Kind::Return), ret(id) {};
			Stmt(Assignment::ID id) : kind(Kind::Assignment), assignment(id) {};
		};


		struct Func{
			struct ID{ // typesafe identifier
				uint32_t id;
				explicit ID(uint32_t _id) noexcept : id(_id) {};
			};


			Token::ID ident;
			std::optional<Type::ID> return_type; // nullopt means Void

			bool is_export;

			llvm::Function* llvm_func = nullptr;
			std::vector<Stmt> stmts{};

			bool returns = false;
		};


	};
};