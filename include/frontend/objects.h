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
		};




		struct Stmt{
			enum class Kind{
				Var,
			} kind;

			union {
				Var::ID var;
			};

			Stmt(Var::ID id) : kind(Kind::Var), var(id) {};
		};


		struct Func{
			struct ID{ // typesafe identifier
				uint32_t id;
				explicit ID(uint32_t _id) noexcept : id(_id) {};
			};


			Token::ID ident;
			// Type::ID return_type;

			llvm::Function* llvm_func = nullptr;
			std::vector<Stmt> stmts{};
		};


	};
};