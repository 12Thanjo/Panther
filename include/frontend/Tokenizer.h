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
			EVO_NODISCARD auto tokenize_operators() noexcept -> bool;
			EVO_NODISCARD auto tokenize_number_literal() noexcept -> bool;
			EVO_NODISCARD auto tokenize_string_literal() noexcept -> bool;



		
		private:
			CharStream& stream;

			struct TokenData{
				Token::Kind kind;
				uint32_t line;
				uint32_t collumn;

				union Value{
					size_t index;

					bool boolean;

					uint64_t integer;
					float128_t floating_point;

					// might be undef
				} value;


				explicit TokenData(Token::Kind token_kind, uint32_t _line, uint32_t _collumn , size_t index_val)
					: kind(token_kind), line(_line), collumn(_collumn), value(index_val) {};

				explicit TokenData(Token::Kind token_kind, uint32_t _line, uint32_t _collumn , bool bool_val)
					: kind(token_kind), line(_line), collumn(_collumn), value(bool_val) {};

				explicit TokenData(Token::Kind token_kind, uint32_t _line, uint32_t _collumn , float128_t float_val)
					: kind(token_kind), line(_line), collumn(_collumn) { this->value.floating_point = float_val; };

				explicit TokenData(Token::Kind token_kind, uint32_t _line, uint32_t _collumn )
					: kind(token_kind), line(_line), collumn(_collumn) {};
			};

			auto add_token(Token::Kind token_kind, size_t index_val) noexcept -> void;
			auto add_token(Token::Kind token_kind, bool bool_val) noexcept -> void;
			auto add_token(Token::Kind token_kind, float128_t float_val) noexcept -> void;
			auto add_token(Token::Kind token_kind) noexcept -> void;


			std::vector<TokenData> tokens{};
			std::vector<std::string> token_value_strings{};


			friend class TokenizerReader;
	};



	//////////////////////////////////////////////////////////////////////
	// tokenizer reader


	struct TokenID{ uint32_t id; };


	class TokenizerReader{
		public:
			TokenizerReader(Tokenizer& _tokenizer) : tokenizer(_tokenizer) {};
			~TokenizerReader() = default;


			EVO_NODISCARD auto peek(int32_t offset = 0) noexcept -> TokenID;
			EVO_NODISCARD auto next() noexcept -> TokenID;
			EVO_NODISCARD auto skip(uint32_t ammount) noexcept -> void;
			EVO_NODISCARD auto go_back(TokenID id) noexcept -> void;



			EVO_NODISCARD inline auto is_eof() const noexcept -> bool { return this->cursor >= int64_t(this->tokenizer.tokens.size()); };
			EVO_NODISCARD inline auto ammount_left() const noexcept -> size_t { return this->tokenizer.tokens.size() - size_t(this->cursor); };



			EVO_NODISCARD auto getKind(TokenID id) const noexcept -> Token::Kind;
			EVO_NODISCARD auto getLine(TokenID id) const noexcept -> uint32_t;
			EVO_NODISCARD auto getCollumn(TokenID id) const noexcept -> uint32_t;

			EVO_NODISCARD auto getStringValue(TokenID id) const noexcept -> const std::string&;
			EVO_NODISCARD auto getBoolValue(TokenID id) const noexcept -> bool;
			EVO_NODISCARD auto getIntegerValue(TokenID id) const noexcept -> uint64_t;


			auto error(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void;
			auto error_info(const std::string& message) const noexcept -> void;
			auto error_info(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void;
			auto fatal(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void;

			EVO_NODISCARD auto get_source_manager() const noexcept -> SourceManager&;


			auto print_to_console() const noexcept -> void;


		private:
			Tokenizer& tokenizer;

			int64_t cursor = 0;
	};

	
};
