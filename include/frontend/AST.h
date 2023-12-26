#pragma once


#include <Evo.h>


#include "./tokenizer.h"

namespace panther{
	namespace AST{

		enum class Kind{
			VarDecl,
			FuncDef,
			FuncParams,

			Block,

			Prefix,
			Infix,
			Postfix,

			Ident,
			Attributes,
			Literal,
			Type,
			Term,

			// no data
			Uninit,
			Null,
			This,
		};

		struct NodeID{ uint32_t id; };


		struct Node{
			Kind kind;
			uint32_t value_index;

			Node(Kind _kind, uint32_t index) : kind(_kind), value_index(index) {};

			Node(Kind _kind) : kind(_kind) {
				evo::debugAssert(
					this->kind == Kind::Uninit
					|| this->kind == Kind::Null
					|| this->kind == Kind::This
					, 
					"This node kind must have a value"
				);
			};
		};





		struct VarDecl{
			bool is_public;
			bool is_static;

			enum class DeclType : uint8_t{
				Var,
				Def,
			};

			DeclType decl_type;

			NodeID ident;
			std::optional<NodeID> type; // null optional means type needs to be inferenced
			NodeID expr;
		};


		

		struct FuncDef{
			bool is_public;
			bool is_static;
			NodeID ident;
			NodeID func_params;

			struct Capture{
				NodeID ident;

				enum class Kind: uint8_t{
					Read,
					Write,
					In,
				};

				Kind kind;
			};
			std::optional<std::vector<Capture>> captures;

			NodeID attributes;

			struct ReturnType{
				std::optional<NodeID> name;
				NodeID type;
			};
			std::vector<ReturnType> returns;
			std::vector<ReturnType> errors;

			NodeID block;
		};

		struct FuncParams{
			struct Param{
				NodeID ident;
				NodeID type;

				enum class Kind: uint8_t{
					Read,
					Write,
					In,
				};

				Kind kind;

				std::optional<NodeID> default_value;
			};

			std::vector<Param> params;
		};



		struct Block{
			std::vector<NodeID> stmts;
		};




		struct Prefix{
			TokenID op;
			NodeID rhs;
		};

		struct Infix{
			NodeID lhs;
			TokenID op;
			NodeID rhs;
		};

		struct Postfix{
			NodeID lhs;
			TokenID op;
		};




		struct Ident{
			TokenID token;
		};

		struct Attributes{
			std::vector<TokenID> tokens;
		};

		struct Literal{
			TokenID token;
		};



		struct Type{
			enum class Kind{
				Ident,
				Builtin,
			};

			Kind kind;


			union {
				Ident ident;

				struct {
					TokenID token;
				} builtin;
			} value;


			explicit Type(Kind _kind, Ident ident) : kind(_kind), value(ident) {};
			explicit Type(Kind _kind, TokenID token) : kind(_kind) { this->value.builtin.token = token; };
		};



		struct Term{
			
		};




	};
};
