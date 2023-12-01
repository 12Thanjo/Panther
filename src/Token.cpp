#include "Token.h"


namespace panther{


	auto Token::print_kind(Token::Kind kind) noexcept -> const char* {
		switch(kind){
			break; case Token::Kind::Ident: return "Ident";
			break; case Token::Kind::Intrinsic: return "Intrinsic";
			break; case Token::Kind::Attribute: return "Attribute";

			///////////////////////////////////
			// literals

			break; case Token::Kind::LiteralBool: return "LiteralBool";
			break; case Token::Kind::LiteralString: return "LiteralString";
			break; case Token::Kind::LiteralChar: return "LiteralChar";
			// break; case Token::Kind::LiteralInt: return "LiteralInt";


			///////////////////////////////////
			// keywords

			break; case Token::Kind::KeywordVar: return "KeywordVar";
			break; case Token::Kind::KeywordDef: return "KeywordDef";
			break; case Token::Kind::KeywordFunc: return "KeywordFunc";
			break; case Token::Kind::KeywordStruct: return "KeywordStruct";
			break; case Token::Kind::KeywordEnum: return "KeywordEnum";
			break; case Token::Kind::KeywordUnion: return "KeywordUnion";


			///////////////////////////////////
			// operators


			///////////////////////////////////
			// punctuation

			break; case Token::Kind::Semicolon: return ";";
			break; case Token::Kind::Colon: return ":";
			break; case Token::Kind::Comma: return ",";

			break; case Token::Kind::OpenParen: return "(";
			break; case Token::Kind::CloseParen: return ")";
			break; case Token::Kind::OpenBracket: return "[";
			break; case Token::Kind::CloseBracket: return "]";
			break; case Token::Kind::OpenBrace: return "{";
			break; case Token::Kind::CloseBrace: return "}";

		};


		// TODO: errors
		evo::logFatal("Unknown token kind");
		evo::unreachable();


		// return "[[Unknown Error]]";
	};




	auto Token::get(const char* token_str) noexcept -> Token::Kind {
		#define IS_TOKEN(str) *evo::stringsEqual(token_str, str, sizeof(str))


		     if(IS_TOKEN(";")){ return Token::Kind::Semicolon; }
		else if(IS_TOKEN(":")){ return Token::Kind::Colon; }
		else if(IS_TOKEN(",")){ return Token::Kind::Comma; }

		else if(IS_TOKEN("(")){ return Token::Kind::OpenParen; }
		else if(IS_TOKEN(")")){ return Token::Kind::CloseParen; }
		else if(IS_TOKEN("[")){ return Token::Kind::OpenBracket; }
		else if(IS_TOKEN("]")){ return Token::Kind::CloseBracket; }
		else if(IS_TOKEN("{")){ return Token::Kind::OpenBrace; }
		else if(IS_TOKEN("}")){ return Token::Kind::CloseBrace; }



		// TODO: errors
		evo::logFatal("Unknown token kind");
		evo::unreachable();
	};


	
};
