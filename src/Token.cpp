#include "Token.h"


namespace panther{


	auto Token::print_kind(Token::Kind kind) noexcept -> const char* {
		switch(kind){
			break; case Token::Kind::Ident: return "Ident";

			///////////////////////////////////
			// literals

			break; case Token::Kind::LiteralInt: return "LiteralInt";


			///////////////////////////////////
			// keywords


			///////////////////////////////////
			// opereators


			///////////////////////////////////
			// punctuation

		};


		// TODO: errors
		evo::logFatal("Unknown token kind");
		evo::unreachable();


		// return "[[Unknown Error]]";
	};


	
};
