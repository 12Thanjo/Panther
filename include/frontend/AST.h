#pragma once


#include <Evo.h>

namespace panther{
	namespace AST{

		enum class Kind{
			VarDecl,
			Func,
			Return,
			Conditional,
			
			Type,
			Block,
			Prefix,
			Infix,
			Postfix,
			FuncCall,

			// tokens
			Ident,
			Literal,
			Intrinsic,
			Uninit,
			Unreachable,
		};


		struct Node{
			struct ID{ // typesafe identifier
				uint32_t id;
				explicit ID(uint32_t _id) noexcept : id(_id) {};
			};

			Kind kind;

			union {
				uint32_t index;
				Token::ID token;
			};

			Node(Kind node_kind, uint32_t node_index) : kind(node_kind), index(node_index){};
			Node(Kind node_kind, Token::ID token_id) : kind(node_kind), token(token_id){};
		};



		struct VarDecl{
			Node::ID ident;
			Node::ID type;
			Node::ID expr;
		};


		struct Func{
			Node::ID ident;
			Node::ID return_type;
			Node::ID block;

			std::vector<Token::ID> attributes;
		};

		struct Return{
			Token::ID keyword;
			std::optional<Node::ID> value;
		};

		struct Conditional{
			Token::ID if_tok;
			Node::ID if_expr;
			Node::ID then_block;
			std::optional<Node::ID> else_block;
		};


		struct Type{
			Token::ID token;

			struct Qualifier{
				bool is_ptr;	
			};
			std::vector<Qualifier> qualifiers;
		};

		struct Block{
			std::vector<Node::ID> nodes;
		};
	


		struct Prefix{
			Token::ID op;
			Node::ID rhs;
		};

		struct Infix{
			Node::ID lhs;
			Token::ID op;
			Node::ID rhs;
		};

		struct Postfix{
			Node::ID lhs;
			Token::ID op;
		};


		struct FuncCall{
			Node::ID target;
		};


	};
};