#pragma once



#include <Evo.h>

namespace panther{
	


	struct Token{
		enum class Kind{
			None,

			Ident,
			Intrinsic,
			Attribute,

			///////////////////////////////////
			// literals

			LiteralBool,
			LiteralString,
			LiteralChar,
			LiteralInt,
			LiteralFloat,


			///////////////////////////////////
			// types

			TypeVoid,
			TypeBool,
			TypeString,
			TypeChar,

			TypeInt,
			TypeIntN,
			TypeUInt,
			TypeUIntN,

			TypeF16,
			TypeF32,
			TypeF64,
			TypeF128,

			TypeUSize,
			TypeRawptr,
			TypeBool32,

			TypeCInt,
			TypeCUInt,



			///////////////////////////////////
			// keywords

			KeywordVar,
			KeywordDef,
			KeywordTypedef,
			KeywordAlias,

			KeywordPub,
			KeywordStatic,

			KeywordFunc,
			KeywordStruct,
			KeywordEnum,
			KeywordUnion,
			KeywordFlags,

			KeywordIf,
			KeywordElse,
			KeywordDo,
			KeywordWhile,
			KeywordSwitch,
			KeywordCase,
			KeywordDefault,

			KeywordCopy,
			KeywordMove,
			KeywordAddr,
			KeywordAs,
			KeywordCast,

			KeywordRead,
			KeywordWrite,
			KeywordIn,

			KeywordReturn,
			KeywordError,
			KeywordDefer,
			KeywordBreak,

			KeywordTry,
			KeywordCatch,

			KeywordThis,
			KeywordUnderscore,
			KeywordUninit,
			KeywordNull,




			///////////////////////////////////
			// operators
			
			Equals, // =
			Optional, // ?

			Accessor, // .
			Dereference, // .*
			Unwrap, // .?

			Plus, // +
			Minus, // -
			Asterisk, // *
			ForwardSlash, // /
			Percent, // %

			PlusEquals, // +=
			MinusEquals, // -=
			TimesEquals, // *=
			DivideEquals, // /=
			ModEquals, // %=

			LessThan, // <
			LessThanEqual, // <=
			GreaterThan, // >
			GreaterThanEqual, // >=
			LogicalEquals, // ==
			LogicalNotEquals, // !=


			LogicalAnd, // &&
			LogicalOr, // ||

			BitwiseAnd, // &
			BitwiseOr, // |
			BitwiseNot, // ~
			
			BitshiftLeft, // <<
			BitshiftRight, // >>


			LeftArrow, // ->


			///////////////////////////////////
			// punctuation

			Semicolon,
			Colon,
			Comma,

			OpenParen,
			CloseParen,
			OpenBracket,
			CloseBracket,
			OpenBrace,
			CloseBrace,

		};

		using enum class Kind;


		// TODO: maybe make all of constexpr?
		EVO_NODISCARD static auto print_kind(Token::Kind kind) noexcept -> const char*;


		EVO_NODISCARD static constexpr auto get(const char* token_str) noexcept -> Token::Kind {
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
			else if(IS_TOKEN("?")){ return Token::Kind::Optional; }

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
			evo::logFatal("Unknown token kind => " __FUNCTION__);
			evo::unreachable();
		};

	};



};
