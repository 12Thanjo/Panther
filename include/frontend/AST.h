#pragma once


#include <Evo.h>


#include "./tokenizer.h"

namespace panther{
	namespace AST{

		enum class Kind{
			VarDecl,

			Ident,
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




		struct Ident{
			TokenID token;
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
