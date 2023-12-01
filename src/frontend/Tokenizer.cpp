#include "frontend/Tokenizer.h"


namespace panther{

	auto Tokenizer::tokenize() noexcept -> bool {
		while(this->stream.is_eof() == false && this->stream.get_source_manager().errored() == false){
			if(this->tokenize_whitespace()){ continue; }
			if(this->tokenize_comment()){ continue; }
			if(this->tokenize_ident()){ continue; }


			// TODO: error
			this->stream.error("Unrecognized character", this->stream.get_line(), this->stream.get_collumn());

			return false;
		};

		return !this->stream.get_source_manager().errored();
	};



	auto Tokenizer::tokenize_whitespace() noexcept -> bool {
		if(evo::isWhitespace(this->stream.peek())){
			this->stream.skip(1);
			return true;
		}

		return false;
	};


	auto Tokenizer::tokenize_comment() noexcept -> bool {
		if(this->stream.ammount_left() < 2 || this->stream.peek() != '/'){
			return false;
		}

		if(this->stream.peek(1) == '/'){
			// line comment

			this->stream.skip(2);

			while(this->stream.is_eof() == false && this->stream.peek() != '\n' && this->stream.peek() != '\r'){
				this->stream.skip(1);
			};

			return true;

		}else if(this->stream.peek(1) == '*'){
			// multi-line comment

			uint32_t comment_beginning_line = this->stream.get_line();
			uint32_t comment_beginning_collumn = this->stream.get_collumn();

			this->stream.skip(2);

			unsigned num_closes_needed = 1;
			while(num_closes_needed > 0){
				if(this->stream.ammount_left() < 2){
					this->stream.error("Unexpected end of file in multi-line comment", this->stream.get_line(), this->stream.get_collumn());
					this->stream.error_info("Multi-line comment begins here", comment_beginning_line, comment_beginning_collumn);
					return true;
				}


				if(this->stream.peek() == '/' && this->stream.peek(1) == '*'){
					this->stream.skip(2);
					num_closes_needed += 1;

				}else if(this->stream.peek() == '*' && this->stream.peek(1) == '/'){
					this->stream.skip(2);
					num_closes_needed -= 1;

				}else{
					this->stream.skip(1);
				}
			};

			return true;
		}

		return false;
	};



	auto Tokenizer::tokenize_ident() noexcept -> bool {
		if(evo::isLetter(this->stream.peek()) || this->stream.peek() == '_'){
			auto ident_name = std::string{this->stream.next()};

			while(this->stream.is_eof() == false && (evo::isAlphaNumeric(this->stream.peek()) || this->stream.peek() == '_')){
				ident_name += this->stream.next();
			};

			const size_t string_index = this->token_value_strings.size();
			this->tokens.emplace_back(Token::Kind::Ident, string_index);
			this->token_value_strings.emplace_back(std::move(ident_name));



			return true;
		}

		return false;
	};





	//////////////////////////////////////////////////////////////////////
	// Tokenizer reader


	auto TokenizerReader::print() noexcept -> void {
		for(auto& token : this->tokenizer.tokens){
			switch(token.kind){
				case Token::Kind::Ident: {
					const std::string& token_value_string = this->tokenizer.token_value_strings[token.value.index];
					evo::logInfo(std::format("{} ({})", Token::print_kind(token.kind), token_value_string));
				} break;

				default: {
					evo::logInfo(std::format("{}", Token::print_kind(token.kind)));
				} break;
			};

		}
	};

	
};
