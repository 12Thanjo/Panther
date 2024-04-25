#pragma once


#include <Evo.h>

namespace panther{
	namespace AST{

		enum class Kind{
			VarDecl,
			FuncParams,
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
			bool isDef;
			Node::ID ident;
			std::vector<Token::ID> attributes;
			std::optional<Node::ID> type;
			Node::ID expr;
		};


		struct FuncParams{
			struct Param{
				Node::ID ident;
				Node::ID type;

				enum class Kind{
					Read,
					Write,
					In,
				} kind;
			};

			std::vector<Param> params;
		};

		struct Func{
			Node::ID ident;
			Node::ID params;
			std::vector<Token::ID> attributes;
			Node::ID returnType;
			Node::ID block;
		};

		struct Return{
			Token::ID keyword;
			std::optional<Node::ID> value;
		};

		struct Conditional{
			Token::ID ifTok;
			Node::ID ifExpr;
			Node::ID thenBlock;
			std::optional<Node::ID> elseBlock;
		};


		struct Type{
			Token::ID token;

			struct Qualifier{
				bool isPtr;
				bool isConst;
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
			std::vector<Node::ID> args;
		};


	};
};