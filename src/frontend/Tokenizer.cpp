#include "frontend/Tokenizer.h"


namespace panther{

	auto Tokenizer::tokenize() noexcept -> bool {
		while(this->stream.is_eof() == false && this->stream.get_source_manager().errored() == false){
			if(this->tokenize_whitespace()){ continue; }
			if(this->tokenize_comment()){ continue; }
			if(this->tokenize_ident()){ continue; }
			if(this->tokenize_punctuation()){ continue; }
			if(this->tokenize_string_literal()){ continue; }


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

			const uint32_t comment_beginning_line = this->stream.get_line();
			const uint32_t comment_beginning_collumn = this->stream.get_collumn();

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
		auto kind = Token::Kind::None;


		if(evo::isLetter(this->stream.peek()) || this->stream.peek() == '_'){
			kind = Token::Kind::Ident;

		}else if(this->stream.ammount_left() >= 2 && (evo::isLetter(this->stream.peek(1)) || this->stream.peek(1) == '_')){
			if(this->stream.peek() == '@'){
				kind = Token::Kind::Intrinsic;
				this->stream.skip(1);

			}else if(this->stream.peek() == '#'){
				kind = Token::Kind::Attribute;
				this->stream.skip(1);
			}
		}


		if(kind == Token::Kind::None){
			return false;
		}


		auto ident_name = std::string{this->stream.next()};

		while(this->stream.is_eof() == false && (evo::isAlphaNumeric(this->stream.peek()) || this->stream.peek() == '_')){
			ident_name += this->stream.next();
		};


		if(kind == Token::Kind::Ident){
			     if(ident_name == "true"){ this->tokens.emplace_back(Token::Kind::LiteralBool, true); }
			else if(ident_name == "false"){ this->tokens.emplace_back(Token::Kind::LiteralBool, false); }

			else if(ident_name == "var")    { this->tokens.emplace_back(Token::Kind::KeywordVar);    }
			else if(ident_name == "def")    { this->tokens.emplace_back(Token::Kind::KeywordDef);    }
			else if(ident_name == "func")   { this->tokens.emplace_back(Token::Kind::KeywordFunc);   }
			else if(ident_name == "struct") { this->tokens.emplace_back(Token::Kind::KeywordStruct); }
			else if(ident_name == "enum")   { this->tokens.emplace_back(Token::Kind::KeywordEnum);   }
			else if(ident_name == "union")  { this->tokens.emplace_back(Token::Kind::KeywordUnion);  }


			else{
				const size_t string_index = this->token_value_strings.size();
				this->tokens.emplace_back(Token::Kind::Ident, string_index);
				this->token_value_strings.emplace_back(std::move(ident_name));
			}

		}else{
			const size_t string_index = this->token_value_strings.size();
			this->tokens.emplace_back(kind, string_index);
			this->token_value_strings.emplace_back(std::move(ident_name));
		}


		return true;

	};



	auto Tokenizer::tokenize_punctuation() noexcept -> bool {
		bool found_punctuation = false;

		switch(this->stream.peek()){
			case ';':
			case ':':
			case ',':

			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
				found_punctuation = true;
		};

		if(found_punctuation == false){ return false; }

		char punctuation_str[2] = {this->stream.next(), '\0'};
		this->tokens.emplace_back(Token::get(punctuation_str));

		return true;
	};




	auto Tokenizer::tokenize_string_literal() noexcept -> bool {
		if(this->stream.peek() != '"' && this->stream.peek() != '\''){ return false; }

		const uint32_t string_beginning_line = this->stream.get_line();
		const uint32_t string_beginning_collumn = this->stream.get_collumn();

		const char delimiter = this->stream.next();

		auto contents = std::string{};

		do{
			bool unexpected_eof = false;

			if(this->stream.is_eof()){
				unexpected_eof = true;

			}else if(this->stream.peek() == '\\'){
				if(this->stream.ammount_left() < 2){
					unexpected_eof = true;

				}else{
					switch(this->stream.peek(1)){
						break; case 'a': contents += '\a'; this->stream.skip(2);
						break; case 'b': contents += '\b'; this->stream.skip(2);
						break; case 't': contents += '\t'; this->stream.skip(2);
						break; case 'n': contents += '\n'; this->stream.skip(2);
						break; case 'v': contents += '\v'; this->stream.skip(2);
						break; case 'f': contents += '\f'; this->stream.skip(2);
						break; case 'r': contents += '\r'; this->stream.skip(2);

						break; case '\'': contents += '\''; this->stream.skip(2);
						break; case '"': contents += '"'; this->stream.skip(2);
						break; case '\\': contents += '\\'; this->stream.skip(2);
						break; default: {
							this->stream.error(
								std::format("Unknown string escape code (\\{})", this->stream.peek(1)),
								this->stream.get_line(), this->stream.get_collumn()
							);

							return true;
						}
					};
				}

			}else{
				contents += this->stream.next();
			}

			// needed because some code above may have called next() or skip()
			if(this->stream.is_eof()){
				unexpected_eof = true;
			}


			if(unexpected_eof){
				const char* string_type_name = [&](){
					if(delimiter == '"'){ return "string"; }
					if(delimiter == '\''){ return "char"; }
				}();

				this->stream.error(
					std::format("Unexpected end of file in {} literal", string_type_name),
					this->stream.get_line(), this->stream.get_collumn()
				);
				this->stream.error_info(
					std::format("{} literal begins here", string_type_name),
					string_beginning_line, string_beginning_collumn
				);

				return true;
			}



		}while(this->stream.peek() != delimiter);

		this->stream.skip(1);


		const size_t string_index = this->token_value_strings.size();
		if(delimiter == '\''){
			this->tokens.emplace_back(Token::Kind::LiteralChar, string_index);
		}else{
			this->tokens.emplace_back(Token::Kind::LiteralString, string_index);
		}
		this->token_value_strings.emplace_back(std::move(contents));

		return true;
	};









	//////////////////////////////////////////////////////////////////////
	// Tokenizer reader

	auto TokenizerReader::getStringValue(TokenID id) const noexcept -> const std::string& {
		const Tokenizer::TokenData& token = this->tokenizer.tokens[id.id];

		evo::debugAssert(
			token.kind == Token::Kind::Ident
			|| token.kind == Token::Kind::Intrinsic
			|| token.kind == Token::Kind::Attribute
			|| token.kind == Token::Kind::LiteralString
			|| token.kind == Token::Kind::LiteralChar
			, std::format("cannot get string value of Token Kind ({})", Token::print_kind(token.kind))
		);

		return this->tokenizer.token_value_strings[token.value.index];
	};






	auto TokenizerReader::print() const noexcept -> void {
		for(auto& token : this->tokenizer.tokens){
			switch(token.kind){
				case Token::Kind::Ident:
				case Token::Kind::Attribute:
				case Token::Kind::Intrinsic:
				case Token::Kind::LiteralString: 
				case Token::Kind::LiteralChar:
				{
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
