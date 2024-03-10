#pragma once


#include <Evo.h>

#include "Token.h"
#include "Source.h"

namespace panther{
	namespace object{
		

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
			};


			BaseType::ID base_type;


			EVO_NODISCARD auto operator==(const Type& rhs) const noexcept -> bool;
		};




		struct Var{
			Source::ID source_id;

			Token::ID ident;
			Type::ID type;
		};




	};
};