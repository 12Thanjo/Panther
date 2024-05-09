#pragma once


#include <Evo.h>

#include "frontend/Message.h"

namespace panther{


	struct Token{
		struct ID{ // typesafe identifier
			uint32_t id;
			explicit ID(uint32_t _id) noexcept : id(_id) {};
		};



		///////////////////////////////////
		// kind 

		enum class Kind{
			None, // for use as an optional

			Ident,
			Attribute,
			Intrinsic,

			///////////////////////////////////
			// literals

			LiteralBool,
			LiteralInt,
			LiteralFloat,
			LiteralChar,
			LiteralString,


			///////////////////////////////////
			// types

			TypeVoid,

			TypeBool,
			TypeInt,
			TypeUInt,
			TypeString,


			///////////////////////////////////
			// keywords

			KeywordVar,
			KeywordDef,
			KeywordFunc,

			KeywordReturn,
			KeywordUnreachable,
			KeywordIf,
			KeywordElse,

			KeywordCopy,
			KeywordUninit,
			KeywordAddr,

			KeywordRead,
			KeywordWrite,
			KeywordIn,

			KeywordAlias,


			///////////////////////////////////
			// operators
			
			Equal, // =

			LogicalEqual, // ==
			NotEqual, // !=
			LessThan, // <
			LessThanEqual, // <=
			GreaterThan, // <
			GreaterThanEqual, // <=
			LogicalNot, // !
			LogicalAnd, // &&
			LogicalOr, // ||

			Plus, // +
			PlusWrap, // +@
			Minus, // -
			MinusWrap, // -@
			Multiply, // *
			MultiplyWrap, // *@
			Divide, // /

			RightArrow, // ->

			Pointer, // ^
			Dereference, // .^
			Accessor, // .


			///////////////////////////////////
			// punctuation

			OpenParen, // (
			CloseParen, // )
			OpenBracket, // [
			CloseBracket, // ]
			OpenBrace, // {
			CloseBrace, // }

			Comma, // ,
			SemiColon, // ;
			Colon, // :

			Pipe, // |
		};

		using enum class Kind;


		///////////////////////////////////
		// properties / constructors

		Kind kind;

		Location location;

		union {
			evo::byte none = 0;

			bool boolean;
			std::string_view string;
			uint64_t integer;
			float64_t floating_point;
		} value;


		Token(Kind _kind, Location _location) noexcept
			: kind(_kind), location(_location) {};

		Token(Kind _kind, Location _location, std::string_view value) noexcept
			: kind(_kind), location(_location) {
			this->value.string = value;
		};


		Token(Kind _kind, Location _location, bool value) noexcept
			: kind(_kind), location(_location) {
			this->value.boolean = value;
		};


		Token(Kind _kind, Location _location, uint64_t value) noexcept
			: kind(_kind), location(_location) {
			this->value.integer = value;
		};


		Token(Kind _kind, Location _location, float64_t value) noexcept
			: kind(_kind), location(_location) {
			this->value.floating_point = value;
		};


		Token(Kind _kind, std::string_view value) noexcept
			: kind(_kind), location(0, 0, 0, 0) {
			evo::debugAssert(kind == Kind::Intrinsic, "This constructor should only be used for intrinsics (builtin-initialization)");

			this->value.string = value;
		}



		//////////////////////////////////////////////////////////////////////
		// helper functions

		EVO_NODISCARD static constexpr auto get(const char* token_str) noexcept -> Kind {
			auto is_token = [&](std::string_view str){
				return evo::stringsEqual(token_str, str.data(), str.size()).value();
			};


			///////////////////////////////////
			// operators

			// length 2
			if(is_token("==")){ return Token::LogicalEqual; }
			if(is_token("!=")){ return Token::NotEqual; }
			if(is_token("<=")){ return Token::LessThanEqual; }
			if(is_token(">=")){ return Token::GreaterThanEqual; }

			if(is_token("&&")){ return Token::LogicalAnd; }
			if(is_token("||")){ return Token::LogicalOr; }

			if(is_token("->")){ return Token::RightArrow; }
			if(is_token(".^")){ return Token::Dereference; }

			if(is_token("+@")){ return Token::PlusWrap; }
			if(is_token("-@")){ return Token::MinusWrap; }
			if(is_token("*@")){ return Token::MultiplyWrap; }

			// length 1
			if(is_token("=")){ return Token::Equal; }

			if(is_token("!")){ return Token::LogicalNot; }

			if(is_token("<")){ return Token::LessThan; }
			if(is_token(">")){ return Token::GreaterThan; }			

			if(is_token("+")){ return Token::Plus; }
			if(is_token("-")){ return Token::Minus; }
			if(is_token("*")){ return Token::Multiply; }
			if(is_token("/")){ return Token::Divide; }

			if(is_token("^")){ return Token::Pointer; }

			if(is_token(".")){ return Token::Accessor; }

			if(is_token("(")){ return Token::OpenParen; }
			if(is_token(")")){ return Token::CloseParen; }
			if(is_token("[")){ return Token::OpenBracket; }
			if(is_token("]")){ return Token::CloseBracket; }
			if(is_token("{")){ return Token::OpenBrace; }
			if(is_token("}")){ return Token::CloseBrace; }

			if(is_token(",")){ return Token::Comma; }
			if(is_token(";")){ return Token::SemiColon; }
			if(is_token(":")){ return Token::Colon; }

			if(is_token("|")){ return Token::Pipe; }


			EVO_FATAL_BREAK(std::format("Unknown token kind ({}) => {}", token_str, __FUNCTION__));
		};



		#if defined(EVO_COMPILER_MSVC)
			#pragma warning(disable: 4702)
		#endif

		EVO_NODISCARD static constexpr auto printKind(Kind kind) noexcept -> const char* {
			switch(kind){
				break; case Kind::None: evo::unreachable(); return "{{ERROR}}";

				break; case Kind::Ident:     return "Ident";
				break; case Kind::Intrinsic: return "Intrinsic";
				break; case Kind::Attribute: return "Attribute";


				///////////////////////////////////
				// literals

				break; case Kind::LiteralBool:   return "LiteralBool";
				break; case Kind::LiteralInt:    return "LiteralInt";
				break; case Kind::LiteralFloat:  return "LiteralFloat";
				break; case Kind::LiteralChar:   return "LiteralChar";
				break; case Kind::LiteralString: return "LiteralString";


				///////////////////////////////////
				// types

				break; case Kind::TypeVoid:   return "Void";

				break; case Kind::TypeInt:    return "Int";
				break; case Kind::TypeUInt:   return "UInt";

				break; case Kind::TypeBool:   return "Bool";
				break; case Kind::TypeString: return "String";


				///////////////////////////////////
				// keywords

				break; case Kind::KeywordVar:         return "var";
				break; case Kind::KeywordDef:         return "def";
				break; case Kind::KeywordFunc:        return "func";

				break; case Kind::KeywordReturn:      return "return";
				break; case Kind::KeywordUnreachable: return "unreachable";
				break; case Kind::KeywordIf:          return "if";
				break; case Kind::KeywordElse:        return "else";

				break; case Kind::KeywordCopy:        return "copy";
				break; case Kind::KeywordUninit:      return "uninit";
				break; case Kind::KeywordAddr:        return "addr";

				break; case Kind::KeywordRead:        return "read";
				break; case Kind::KeywordWrite:       return "write";
				break; case Kind::KeywordIn:          return "in";

				break; case Kind::KeywordAlias:       return "alias";


				///////////////////////////////////
				// operators
				
				break; case Kind::Equal:                 return "=";

				break; case Kind::LogicalEqual:          return "==";
				break; case Kind::NotEqual:              return "!=";
				break; case Kind::LessThan:              return "<";
				break; case Kind::LessThanEqual:         return "<=";
				break; case Kind::GreaterThan:           return ">";
				break; case Kind::GreaterThanEqual:      return ">=";
				break; case Kind::LogicalNot:            return "!";
				break; case Kind::LogicalAnd:            return "&&";
				break; case Kind::LogicalOr:             return "||";

				break; case Kind::Plus:                  return "+";
				break; case Kind::PlusWrap:              return "+@";
				break; case Kind::Minus:                 return "-";
				break; case Kind::MinusWrap:             return "-@";
				break; case Kind::Multiply:              return "*";
				break; case Kind::MultiplyWrap:          return "*@";
				break; case Kind::Divide:                return "/";

				break; case Kind::RightArrow:            return "->";

				break; case Kind::Pointer:               return "^";
				break; case Kind::Dereference:           return ".^";

				break; case Kind::Accessor:              return ".";


				///////////////////////////////////
				// punctuation

				break; case Kind::OpenParen:    return "(";
				break; case Kind::CloseParen:   return ")";
				break; case Kind::OpenBracket:  return "[";
				break; case Kind::CloseBracket: return "]";
				break; case Kind::OpenBrace:    return "{";
				break; case Kind::CloseBrace:   return "}";

				break; case Kind::Comma:        return ",";
				break; case Kind::SemiColon:    return ";";
				break; case Kind::Colon:        return ":";

				break; case Kind::Pipe:         return "|";
				
			};


			evo::unreachable();


			// Literally just here to appease constexpr
			return "{{ERROR}}";
		};

		#if defined(EVO_COMPILER_MSVC)
			#pragma warning(default: 4702)
		#endif


	};


};