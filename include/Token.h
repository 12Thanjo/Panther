#pragma once



#include <Evo.h>

namespace panther{
	


	struct Token{
		enum class Kind{
			Ident,

			///////////////////////////////////
			// literals

			LiteralInt


			///////////////////////////////////
			// keywords


			///////////////////////////////////
			// opereators


			///////////////////////////////////
			// punctuation



		};


		// TODO: maybe constexpr?
		EVO_NODISCARD static auto print_kind(Token::Kind kind) noexcept -> const char*;


	};



};
