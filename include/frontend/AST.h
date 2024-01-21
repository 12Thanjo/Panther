#pragma once


#include <Evo.h>


#include "./tokenizer.h"

namespace panther{
	namespace AST{

		enum class Kind{
			VarDecl,
			MultipleAssignment,

			FuncDef,
			FuncParams,
			FuncOutputs,

			Block,

			Conditional,
			WhileLoop,
			Alias,
			Return,
			Struct,
			TryCatch,

			Prefix,
			Infix,
			Postfix,
			IndexOp,
			FuncCall,

			Ident,
			Intrinsic,
			Attributes,
			Literal,
			Type,
			Operator,

			// no data
			Uninit,
			Null,
			This,
			Underscore,
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
					|| this->kind == Kind::Underscore
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
			std::optional<NodeID> expr;
		};


		struct MultipleAssignment{
			bool is_public;
			bool is_static;

			enum class DeclType : uint8_t{
				Var,
				Def,
				None,
			};

			DeclType decl_type;

			std::vector<NodeID> targets;
			NodeID value;
		};


		


		struct FuncDef{
			bool is_public;
			bool is_static;
			NodeID name;
			bool is_operator;
			NodeID func_params;

			// struct Capture{
			// 	NodeID ident;

			// 	enum class Kind: uint8_t{
			// 		Read,
			// 		Write,
			// 		In,
			// 	};

			// 	Kind kind;
			// };
			// std::optional<std::vector<Capture>> captures;

			NodeID attributes;

			NodeID returns;
			NodeID errors;

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

			std::optional<Param::Kind> this_param; // should only be read, write, or nullopt
			std::vector<Param> params;
		};


		struct FuncOutputs{
			struct Value{
				std::optional<NodeID> name;
				NodeID type;
			};

			std::vector<Value> values;
		};


		



		struct Block{
			std::vector<NodeID> stmts;
		};


		struct Conditional{
			NodeID if_stmt;
			NodeID then_stmt;
			std::optional<NodeID> else_stmt;
		};


		struct WhileLoop{
			bool is_do_while;
			NodeID condition;
			NodeID block;
		};


		struct Alias{
			bool is_public;
			bool is_typedef;
			NodeID name;
			NodeID type;
		};


		struct Return{
			bool is_throw;

			enum class Kind{
				Nothing,
				Ellipsis,
				Expr
			};

			Kind kind;

			std::optional<NodeID> expr; // is std::nullopt when kind is Nohting or Ellipsis
		};

		struct Struct{
			bool is_public;
			NodeID name;
			NodeID block;
		};


		struct TryCatch{
			NodeID try_block;

			struct Catch{
				struct Param{
					NodeID ident;
					NodeID type;
				};

				std::vector<Param> params;
				NodeID block;
			};

			std::vector<Catch> catches;
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

		struct IndexOp{
			NodeID target;
			NodeID index;
		};

		struct FuncCall{
			NodeID target;
			std::vector<NodeID> arguments;
		};




		struct Ident{
			TokenID token;
		};

		struct Intrinsic{
			TokenID token;
		};


		struct Attributes{
			std::vector<TokenID> tokens;
		};

		struct Literal{
			TokenID token;
		};


		// this is just for functions that are operator overloading
		struct Operator{
			TokenID token;
			std::optional<NodeID> type; // if the operator is `as` or `cast`
		};


		struct Type{
			enum class Kind{
				Basic,
				Array,
				Func,
			};

			Kind kind;

			union {
				struct {
					TokenID token;
				} basic;

				struct {
					NodeID type;
					NodeID length;
				} array;

				struct {
					NodeID func_params;
					NodeID attributes;
					NodeID returns;
					NodeID errors;
				} func;
			} value;


			struct Qualifier{
				bool is_pointer;
				bool is_const;
				bool is_optional;
			};

			std::vector<Qualifier> qualifiers;



			Type(Kind _kind, TokenID token, std::vector<Qualifier>&& _qualifers)
				: kind(_kind), qualifiers(std::move(_qualifers)) { this->value.basic.token = token; };


			Type(Kind _kind, NodeID type, NodeID length, std::vector<Qualifier>&& _qualifers)
				: kind(_kind), qualifiers(std::move(_qualifers))
			{
				this->value.array.type = type;
				this->value.array.length = length;
			};


			Type(
				Kind _kind,
				NodeID func_params,
				NodeID attributes,
				NodeID returns,
				NodeID errors,
				std::vector<Qualifier>&& _qualifers
			)
				: kind(_kind), qualifiers(std::move(_qualifers))
			{
				this->value.func.func_params = func_params;
				this->value.func.attributes = attributes;
				this->value.func.returns = returns;
				this->value.func.errors = errors;
			};

		};




	};
};
