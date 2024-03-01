#pragma once


#include <Evo.h>

#include "./CharStream.h"
#include "Source.h"

namespace panther{


	class Tokenizer{
		public:
			Tokenizer(Source& src) : source(src), char_stream(src.getData()) {};
			~Tokenizer() = default;

			EVO_NODISCARD auto tokenize() noexcept -> bool;

		private:
			EVO_NODISCARD auto tokenize_whitespace() noexcept -> bool;
			EVO_NODISCARD auto tokenize_comment() noexcept -> bool;
			EVO_NODISCARD auto tokenize_identifier() noexcept -> bool;
			EVO_NODISCARD auto tokenize_punctuation() noexcept -> bool;
			EVO_NODISCARD auto tokenize_operators() noexcept -> bool;
			EVO_NODISCARD auto tokenize_number_literal() noexcept -> bool;

			auto create_token(Token::Kind kind) noexcept -> void;
			template<typename T> auto create_token(Token::Kind kind, T val) noexcept -> void;

			EVO_NODISCARD auto error_unrecognized_character() noexcept -> void;
	
		private:
			Source& source;
			CharStream char_stream;
	};


};