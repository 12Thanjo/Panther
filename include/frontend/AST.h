#pragma once


#include <Evo.h>

namespace panther{
	namespace AST{

		enum class Kind{
			VarDecl,
			TemplatePack,
			FuncParams,
			Func,
			Struct,
			Return,
			Conditional,
			Alias,
			Cast,
			
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
			std::optional<Node::ID> expr;
		};

		struct TemplatePack{
			struct Template{
				Node::ID ident;

				bool is_type_keyword;
				union{
					Node::ID type_node;
					Token::ID keyword;
				};

				Template(Node::ID _ident, Node::ID node) noexcept : ident(_ident), is_type_keyword(false), type_node(node) {};
				Template(Node::ID _ident, Token::ID _keyword) noexcept : ident(_ident), is_type_keyword(true),  keyword(_keyword) {};
			};

			std::vector<Template> templates;
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

			std::optional<Node::ID> template_pack;
			Node::ID params;
			std::vector<Token::ID> attributes;
			Node::ID returnType;
			
			Node::ID block;
		};

		struct Struct{
			Node::ID ident;
			std::vector<Token::ID> attributes;
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


		struct Alias{
			Node::ID ident;
			std::vector<Token::ID> attributes;
			Node::ID type;
		};


		struct Cast{
			Node::ID value;
			Node::ID type;

			enum class Kind{
				As,
				Cast,
			} kind;
		};




		struct Type{
			union Base{
				Token::ID token;
				Node::ID node;
			} base;

			bool isBuiltin;

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
			std::optional<std::vector<Node::ID>> template_args;
			std::vector<Node::ID> args;
		};


	};
};