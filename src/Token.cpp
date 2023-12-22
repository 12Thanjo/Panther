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
			break; case Token::Kind::LiteralInt: return "LiteralInt";
			break; case Token::Kind::LiteralFloat: return "LiteralFloat";



			///////////////////////////////////
			// types

			break; case Token::Kind::TypeVoid: return "Void";
			break; case Token::Kind::TypeBool: return "Bool";
			break; case Token::Kind::TypeString: return "String";
			break; case Token::Kind::TypeChar: return "Char";

			break; case Token::Kind::TypeInt: return "Int";
			break; case Token::Kind::TypeIntN: return "IntN";
			break; case Token::Kind::TypeUInt: return "UInt";
			break; case Token::Kind::TypeUIntN: return "UIntN";

			break; case Token::Kind::TypeF16: return "F16";
			break; case Token::Kind::TypeF32: return "F32";
			break; case Token::Kind::TypeF64: return "F64";
			break; case Token::Kind::TypeF128: return "F128";

			break; case Token::Kind::TypeUSize: return "USize";
			break; case Token::Kind::TypeRawptr: return "Rawptr";
			break; case Token::Kind::TypeBool32: return "Bool32";

			break; case Token::Kind::TypeCInt: return "CInt";
			break; case Token::Kind::TypeCUInt: return "CUInt";



			///////////////////////////////////
			// keywords

			break; case Token::Kind::KeywordVar: return "var";
			break; case Token::Kind::KeywordDef: return "def";
			break; case Token::Kind::KeywordTypedef: return "typedef";
			break; case Token::Kind::KeywordAlias: return "alias";

			break; case Token::Kind::KeywordPub: return "pub";
			break; case Token::Kind::KeywordStatic: return "static";

			break; case Token::Kind::KeywordFunc: return "func";
			break; case Token::Kind::KeywordStruct: return "struct";
			break; case Token::Kind::KeywordEnum: return "enum";
			break; case Token::Kind::KeywordUnion: return "union";
			break; case Token::Kind::KeywordFlags: return "flags";

			break; case Token::Kind::KeywordIf: return "if";
			break; case Token::Kind::KeywordElse: return "else";
			break; case Token::Kind::KeywordDo: return "do";
			break; case Token::Kind::KeywordWhile: return "while";

			break; case Token::Kind::KeywordCopy: return "copy";
			break; case Token::Kind::KeywordMove: return "move";
			break; case Token::Kind::KeywordAddr: return "addr";

			break; case Token::Kind::KeywordRead: return "read";
			break; case Token::Kind::KeywordWrite: return "write";
			break; case Token::Kind::KeywordIn: return "in";

			break; case Token::Kind::KeywordReturn: return "return";
			break; case Token::Kind::KeywordError: return "error";

			break; case Token::Kind::KeywordTry: return "try";
			break; case Token::Kind::KeywordCatch: return "catch";

			break; case Token::Kind::KeywordThis: return "this";
			break; case Token::Kind::KeywordUnderscore: return "_";
			break; case Token::Kind::KeywordUninit: return "uninit";
			break; case Token::Kind::KeywordNull: return "LiteralNull";


			///////////////////////////////////
			// operators

			break; case Token::Kind::Equals: return "=";

			break; case Token::Kind::Accessor: return ".";
			break; case Token::Kind::Dereference: return ".*";
			break; case Token::Kind::Unwrap: return ".?";

			break; case Token::Kind::Plus: return "+";
			break; case Token::Kind::Minus: return "-";
			break; case Token::Kind::Asterisk: return "*";
			break; case Token::Kind::ForwardSlash: return "/";
			break; case Token::Kind::Percent: return "%";

			break; case Token::Kind::PlusEquals: return "+=";
			break; case Token::Kind::MinusEquals: return "-=";
			break; case Token::Kind::TimesEquals: return "*=";
			break; case Token::Kind::DivideEquals: return "/=";
			break; case Token::Kind::ModEquals: return "%=";

			break; case Token::Kind::LessThan: return "<";
			break; case Token::Kind::LessThanEqual: return "<=";
			break; case Token::Kind::GreaterThan: return ">";
			break; case Token::Kind::GreaterThanEqual: return ">=";
			break; case Token::Kind::LogicalEquals: return "==";
			break; case Token::Kind::LogicalNotEquals: return "!=";


			break; case Token::Kind::LogicalAnd: return "&&";
			break; case Token::Kind::LogicalOr: return "||";

			break; case Token::Kind::BitwiseAnd: return "&";
			break; case Token::Kind::BitwiseOr: return "|";
			break; case Token::Kind::BitwiseNot: return "~";
			
			break; case Token::Kind::BitshiftLeft: return "<<";
			break; case Token::Kind::BitshiftRight: return ">>";


			break; case Token::Kind::LeftArrow: return "->";


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


		///////////////////////////////////
		// operators


		// length 2
			 if(IS_TOKEN(".*")){ return Token::Kind::Dereference; }
		else if(IS_TOKEN(".?")){ return Token::Kind::Unwrap; }

		else if(IS_TOKEN("+=")){ return Token::Kind::PlusEquals; }
		else if(IS_TOKEN("-=")){ return Token::Kind::MinusEquals; }
		else if(IS_TOKEN("*=")){ return Token::Kind::TimesEquals; }
		else if(IS_TOKEN("/=")){ return Token::Kind::DivideEquals; }
		else if(IS_TOKEN("%=")){ return Token::Kind::ModEquals; }

		else if(IS_TOKEN("&&")){ return Token::Kind::LogicalAnd; }
		else if(IS_TOKEN("||")){ return Token::Kind::LogicalOr; }

		else if(IS_TOKEN("<=")){ return Token::Kind::LessThanEqual; }
		else if(IS_TOKEN(">=")){ return Token::Kind::GreaterThanEqual; }
		else if(IS_TOKEN("==")){ return Token::Kind::LogicalEquals; }
		else if(IS_TOKEN("!=")){ return Token::Kind::LogicalNotEquals; }

		else if(IS_TOKEN("<<")){ return Token::Kind::BitshiftLeft; }
		else if(IS_TOKEN(">>")){ return Token::Kind::BitshiftRight; }


		else if(IS_TOKEN("->")){ return Token::Kind::LeftArrow; }



		// length 1

		else if(IS_TOKEN("=")){ return Token::Kind::Equals; }

		else if(IS_TOKEN(".")){ return Token::Kind::Accessor; }

		else if(IS_TOKEN("+")){ return Token::Kind::Plus; }
		else if(IS_TOKEN("-")){ return Token::Kind::Minus; }
		else if(IS_TOKEN("*")){ return Token::Kind::Asterisk; }
		else if(IS_TOKEN("/")){ return Token::Kind::ForwardSlash; }
		else if(IS_TOKEN("%")){ return Token::Kind::Percent; }


		else if(IS_TOKEN("<")){ return Token::Kind::LessThan; }
		else if(IS_TOKEN(">")){ return Token::Kind::GreaterThan; }



		else if(IS_TOKEN("&")){ return Token::Kind::BitwiseAnd; }
		else if(IS_TOKEN("|")){ return Token::Kind::BitwiseOr; }
		else if(IS_TOKEN("~")){ return Token::Kind::BitwiseNot; }
		


		///////////////////////////////////
		// punctuation

		else if(IS_TOKEN(";")){ return Token::Kind::Semicolon; }
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
