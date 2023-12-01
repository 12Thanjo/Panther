#pragma once


#include <Evo.h>

#include "CharStream.h"
#include "Token.h"


namespace panther{


	class Tokenizer{
		public:
			Tokenizer(CharStream& char_stream) : stream(char_stream) {};
			~Tokenizer() = default;

			EVO_NODISCARD auto tokenize() noexcept -> bool; // returns if tokenizing was successful


		private:
			// these return true if they tokenized
			EVO_NODISCARD auto tokenize_whitespace() noexcept -> bool;
			EVO_NODISCARD auto tokenize_comment() noexcept -> bool;
			EVO_NODISCARD auto tokenize_ident() noexcept -> bool;
			EVO_NODISCARD auto tokenize_punctuation() noexcept -> bool;
			EVO_NODISCARD auto tokenize_string_literal() noexcept -> bool;


		
		private:
			CharStream& stream;

			struct TokenData{
				Token::Kind kind;

				union Value{
					size_t index;
					bool boolean;
					// might be undef
				} value;

				explicit TokenData(Token::Kind token_kind, size_t index_val) : kind(token_kind), value(index_val) {};
				explicit TokenData(Token::Kind token_kind, bool bool_val) : kind(token_kind), value(bool_val) {};
				explicit TokenData(Token::Kind token_kind) : kind(token_kind) {};
			};

			std::vector<TokenData> tokens{};
			std::vector<std::string> token_value_strings{};


			friend class TokenizerReader;
	};


	struct TokenID{ uint32_t id; };


	class TokenizerReader{
		public:
			TokenizerReader(Tokenizer& _tokenizer) : tokenizer(_tokenizer) {};
			~TokenizerReader() = default;


			EVO_NODISCARD auto getStringValue(TokenID id) const noexcept -> const std::string&;


			auto print() const noexcept -> void;

		private:
			Tokenizer& tokenizer;
	};

	
};
