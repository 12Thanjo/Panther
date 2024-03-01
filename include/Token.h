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



	};


};