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


		
		private:
			CharStream& stream;

			struct TokenData{
				Token::Kind kind;

				union Value{
					size_t index;
					
				} value;

				TokenData(Token::Kind token_kind, size_t index_val) : kind(token_kind), value(index_val) {};
			};

			std::vector<TokenData> tokens{};
			std::vector<std::string> token_value_strings{};


			friend class TokenizerReader;
	};



	class TokenizerReader{
		public:
			TokenizerReader(Tokenizer& _tokenizer) : tokenizer(_tokenizer) {};
			~TokenizerReader() = default;

			auto print() noexcept -> void;

	
		private:
			Tokenizer& tokenizer;
	};

	
};
