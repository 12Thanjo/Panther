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
			// LiteralInt,


			///////////////////////////////////
			// keywords

			KeywordVar,
			KeywordDef,
			KeywordFunc,
			KeywordStruct,
			KeywordEnum,
			KeywordUnion,


			///////////////////////////////////
			// operators




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


		// TODO: maybe make all of constexpr?
		EVO_NODISCARD static auto print_kind(Token::Kind kind) noexcept -> const char*;
		EVO_NODISCARD static auto get(const char* token_str) noexcept -> Token::Kind;

	};



};
