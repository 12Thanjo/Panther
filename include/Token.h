#pragma once


#include <Evo.h>

namespace panther{


	struct Token{

		///////////////////////////////////
		// kind 

		enum class Kind{
			None, // for use as an optional

			Ident,

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

			///////////////////////////////////
			// keywords

			KeywordVar,
			KeywordFunc,


			///////////////////////////////////
			// operators
			
			Assign,

			LogicalEquals,


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

		uint32_t line;
		uint32_t collumn;

		union {
			evo::byte none = 0;

			bool boolean;
			std::string_view string;
			uint64_t integer;
			float64_t floating_point;
		} value;


		Token(Kind _kind, uint32_t _line, uint32_t _collumn) noexcept
			: kind(_kind), line(_line), collumn(_collumn) {};

		Token(Kind _kind, uint32_t _line, uint32_t _collumn, std::string_view value) noexcept
			: kind(_kind), line(_line), collumn(_collumn) {
			this->value.string = value;
		};


		Token(Kind _kind, uint32_t _line, uint32_t _collumn, bool value) noexcept
			: kind(_kind), line(_line), collumn(_collumn) {
			this->value.boolean = value;
		};


		Token(Kind _kind, uint32_t _line, uint32_t _collumn, uint64_t value) noexcept
			: kind(_kind), line(_line), collumn(_collumn) {
			this->value.integer = value;
		};


		Token(Kind _kind, uint32_t _line, uint32_t _collumn, float64_t value) noexcept
			: kind(_kind), line(_line), collumn(_collumn) {
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
			if(is_token("==")){ return Token::LogicalEquals; }

			// length 1
			if(is_token("=")){ return Token::Assign; }


			EVO_FATAL_BREAK(std::format("Unknown token kind ({}) => {}", token_str, __FUNCTION__));
		};



		EVO_NODISCARD static constexpr auto printKind(Kind kind) noexcept -> const char* {
			switch(kind){
				break; case Kind::None: return  "None";

				break; case Kind::Ident: return "Ident";

				///////////////////////////////////
				// literals

				break; case Kind::LiteralBool: return "LiteralBool";
				break; case Kind::LiteralInt: return "LiteralInt";
				break; case Kind::LiteralFloat: return "LiteralFloat";

				///////////////////////////////////
				// types

				break; case Kind::TypeVoid: return "Void";

				break; case Kind::TypeInt: return "Int";

				///////////////////////////////////
				// keywords

				break; case Kind::KeywordVar: return "var";
				break; case Kind::KeywordFunc: return "func";


				///////////////////////////////////
				// operators
				
				break; case Kind::Assign: return "=";

				break; case Kind::LogicalEquals: return "==";


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


	};


};