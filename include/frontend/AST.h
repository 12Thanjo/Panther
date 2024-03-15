#pragma once


#include <Evo.h>

namespace panther{
	namespace AST{

		enum class Kind{
			VarDecl,
			Func,
			
			Type,
			Block,

			// tokens
			Ident,
			Literal,
			Uninit,
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
		};


		struct Type{
			Token::ID token;
		};

		struct Block{
			std::vector<Node::ID> nodes;
		};
	

	};
};