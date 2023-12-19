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

			LiteralNull,


			///////////////////////////////////
			// types

			TypeBool,

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
			KeywordUninit,

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



			///////////////////////////////////
			// operators
			
			Equals, // =

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


		// TODO: maybe make all of constexpr?
		EVO_NODISCARD static auto print_kind(Token::Kind kind) noexcept -> const char*;
		EVO_NODISCARD static auto get(const char* token_str) noexcept -> Token::Kind;

	};



};
