#pragma once


#include <Evo.h>

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
			// LiteralChar,
			// LiteralString,


			///////////////////////////////////
			// types

			TypeVoid,

			TypeInt,
			TypeBool,


			///////////////////////////////////
			// keywords

			KeywordVar,
			KeywordFunc,

			KeywordReturn,

			KeywordUninit,


			///////////////////////////////////
			// operators
			
			Equal, // =

			DoubleEqual, // ==

			RightArrow, // ->


			///////////////////////////////////
			// punctuation

			OpenParen,
			CloseParen,
			OpenBracket,
			CloseBracket,
			OpenBrace,
			CloseBrace,

			Comma,
			Period,
			SemiColon,
			Colon,
		};

		using enum class Kind;


		///////////////////////////////////
		// properties / constructors

		Kind kind;

		// TODO; make Location
		uint32_t line_start;
		uint32_t line_end;
		uint32_t collumn_start;
		uint32_t collumn_end;

		union {
			evo::byte none = 0;

			bool boolean;
			std::string_view string;
			uint64_t integer;
			float64_t floating_point;
		} value;


		Token(Kind _kind, uint32_t start_line, uint32_t end_line, uint32_t start_collumn, uint32_t end_collumn) noexcept
			: kind(_kind), line_start(start_line), line_end(end_line), collumn_start(start_collumn), collumn_end(end_collumn) {};

		Token(Kind _kind, uint32_t start_line, uint32_t end_line, uint32_t start_collumn, uint32_t end_collumn, std::string_view value) noexcept
			: kind(_kind), line_start(start_line), line_end(end_line), collumn_start(start_collumn), collumn_end(end_collumn) {
			this->value.string = value;
		};


		Token(Kind _kind, uint32_t start_line, uint32_t end_line, uint32_t start_collumn, uint32_t end_collumn, bool value) noexcept
			: kind(_kind), line_start(start_line), line_end(end_line), collumn_start(start_collumn), collumn_end(end_collumn) {
			this->value.boolean = value;
		};


		Token(Kind _kind, uint32_t start_line, uint32_t end_line, uint32_t start_collumn, uint32_t end_collumn, uint64_t value) noexcept
			: kind(_kind), line_start(start_line), line_end(end_line), collumn_start(start_collumn), collumn_end(end_collumn) {
			this->value.integer = value;
		};


		Token(Kind _kind, uint32_t start_line, uint32_t end_line, uint32_t start_collumn, uint32_t end_collumn, float64_t value) noexcept
			: kind(_kind), line_start(start_line), line_end(end_line), collumn_start(start_collumn), collumn_end(end_collumn) {
			this->value.floating_point = value;
		};



		//////////////////////////////////////////////////////////////////////
		// helper functions

		EVO_NODISCARD static constexpr auto get(const char* token_str) noexcept -> Kind {
			auto is_token = [&](std::string_view str){
				return *evo::stringsEqual(token_str, str.data(), str.size());
			};


			///////////////////////////////////
			// operators

			// length 2
			if(is_token("==")){ return Token::DoubleEqual; }
			if(is_token("->")){ return Token::RightArrow; }

			// length 1
			if(is_token("=")){ return Token::Equal; }
			if(is_token("(")){ return Token::OpenParen; }
			if(is_token(")")){ return Token::CloseParen; }
			if(is_token("[")){ return Token::OpenBracket; }
			if(is_token("]")){ return Token::CloseBracket; }
			if(is_token("{")){ return Token::OpenBrace; }
			if(is_token("}")){ return Token::CloseBrace; }

			if(is_token(",")){ return Token::Comma; }
			if(is_token(".")){ return Token::Period; }
			if(is_token(";")){ return Token::SemiColon; }
			if(is_token(":")){ return Token::Colon; }


			EVO_FATAL_BREAK(std::format("Unknown token kind ({}) => {}", token_str, __FUNCTION__));
		};



		#if defined(EVO_COMPILER_MSVC)
			#pragma warning(disable: 4702)
		#endif

		EVO_NODISCARD static constexpr auto printKind(Kind kind) noexcept -> const char* {
			switch(kind){
				break; case Kind::None: evo::unreachable(); return "{{ERROR}}";

				break; case Kind::Ident: return "Ident";
				break; case Kind::Intrinsic: return "Intrinsic";
				break; case Kind::Attribute: return "Attribute";


				///////////////////////////////////
				// literals

				break; case Kind::LiteralBool: return "LiteralBool";
				break; case Kind::LiteralInt: return "LiteralInt";
				break; case Kind::LiteralFloat: return "LiteralFloat";


				///////////////////////////////////
				// types

				break; case Kind::TypeVoid: return "Void";

				break; case Kind::TypeInt: return "Int";
				break; case Kind::TypeBool: return "Bool";


				///////////////////////////////////
				// keywords

				break; case Kind::KeywordVar: return "var";
				break; case Kind::KeywordFunc: return "func";

				break; case Kind::KeywordReturn: return "return";

				break; case Kind::KeywordUninit: return "uninit";


				///////////////////////////////////
				// operators
				
				break; case Kind::Equal: return "=";

				break; case Kind::DoubleEqual: return "==";

				break; case Kind::RightArrow: return "->";


				///////////////////////////////////
				// punctuation

				break; case Kind::OpenParen: return "(";
				break; case Kind::CloseParen: return ")";
				break; case Kind::OpenBracket: return "[";
				break; case Kind::CloseBracket: return "]";
				break; case Kind::OpenBrace: return "{";
				break; case Kind::CloseBrace: return "}";

				break; case Kind::Comma: return ",";
				break; case Kind::Period: return ".";
				break; case Kind::SemiColon: return ";";
				break; case Kind::Colon: return ":";
				
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