#include "frontend/Tokenizer.h"


#include <cstdlib>


namespace panther{

	auto Tokenizer::tokenize() noexcept -> bool {
		while(this->stream.is_eof() == false && this->stream.get_source_manager().errored() == false){
			if(this->tokenize_whitespace()){ continue; }
			if(this->tokenize_comment()){ continue; }
			if(this->tokenize_ident()){ continue; }
			if(this->tokenize_punctuation()){ continue; }
			if(this->tokenize_number_literal()){ continue; }
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



	auto Tokenizer::tokenize_number_literal() noexcept -> bool {
		if(evo::isNumber(this->stream.peek()) == false){ return false; }

		const uint32_t number_beginning_line = this->stream.get_line();
		const uint32_t number_beginning_collumn = this->stream.get_collumn();


		int base = 10;


		///////////////////////////////////
		// get number prefix

		if(this->stream.peek() == '0' && this->stream.ammount_left() >= 2){
			const char second_peek = this->stream.peek(1);
			if(second_peek == 'x'){
				base = 16;
				this->stream.skip(2);

			}else if(second_peek == 'b'){
				base = 2;
				this->stream.skip(2);

			}else if(second_peek == 'o'){
				base = 8;
				this->stream.skip(2);

			}else if(evo::isNumber(second_peek)){
				this->stream.error(
					"Leading zeros in literal numbers are not supported",
					this->stream.get_line(), this->stream.get_collumn()
				);

				this->stream.error_info("Note: the literal integer prefix for base-8 is \"0o\"");
				return true;
			}
		}




		///////////////////////////////////
		// get number

		auto number_string = std::string{};

		bool has_decimal = false;

		while(this->stream.is_eof() == false){
			const char peeked_char = this->stream.peek();


			if(peeked_char == '_'){
				this->stream.skip(1);
				continue;

			}else if(peeked_char == '.'){
				if(has_decimal){
					this->stream.error("Cannot have multiple decimal points in a floating-point literal", this->stream.get_line(), this->stream.get_collumn());
					return true;
				}

				if(base == 2){
					this->stream.error("Base-2 floating-point literals are not supported", number_beginning_line, number_beginning_collumn);
					return true;
				}else if(base == 8){
					this->stream.error("Base-8 floating-point literals are not supported", number_beginning_line, number_beginning_collumn);
					return true;
				}

				has_decimal = true;
				number_string += '.';

				this->stream.skip(1);
				continue;
			}



			if(base == 2){
				if(peeked_char == '0' || peeked_char == '1'){
					number_string += this->stream.next();

				}else if(evo::isHexNumber(peeked_char)){
					this->stream.error("Base-2 numbers should only have digits 0 and 1", this->stream.get_line(), this->stream.get_collumn());
					return true;

				}else{
					break;
				}

			}else if(base == 8){
				if(evo::isOctalNumber(peeked_char)){
					number_string += this->stream.next();

				}else if(evo::isHexNumber(peeked_char)){
					this->stream.error("Base-8 numbers should only have digits 0-7", this->stream.get_line(), this->stream.get_collumn());
					return true;

				}else{
					break;
				}

			}else if(base == 10){
				if(evo::isNumber(peeked_char)){
					number_string += this->stream.next();

				}else if(peeked_char == 'e' || peeked_char == 'E'){
					break;

				}else if(evo::isHexNumber(peeked_char)){
					this->stream.error("Base-10 numbers should only have digits 0-9", this->stream.get_line(), this->stream.get_collumn());
					return true;

				}else{
					break;
				}

			}else{
				// base-16
				if(evo::isHexNumber(peeked_char)){
					number_string += this->stream.next();

				}else{
					break;
				}
			}

		};


		///////////////////////////////////
		// get exponent (if it exsits)

		auto exponent_string = std::string{};
		if(this->stream.ammount_left() >= 2  && (this->stream.peek() == 'e' || this->stream.peek() == 'E')){
			this->stream.skip(1);

			if(this->stream.peek() == '-' || this->stream.peek() == '+'){
				exponent_string += this->stream.next();
			}

			while(this->stream.is_eof() == false){
				const char peeked_char = this->stream.peek();

				if(evo::isNumber(peeked_char)){
					exponent_string += this->stream.next();

				}else if(evo::isHexNumber(peeked_char)){
					this->stream.error("Literal number exponents should only have digits 0-9", this->stream.get_line(), this->stream.get_collumn());
					return true;

				}else{
					break;
				}
			};
		}



		///////////////////////////////////
		// parse exponent (if it exists)

		int64_t exponent_number = 1;

		if(exponent_string.size() != 0){
			exponent_number = std::strtoll(exponent_string.data(), nullptr, base);

			if(exponent_number == ULLONG_MAX && errno == ERANGE){
				this->stream.error(
					"Literal number exponent too large to fit into a I64. This limitation will be removed when the compiler is self hosted.",
					number_beginning_line, number_beginning_collumn
				);
				return true;

			}else if(exponent_number == 0){
				for(const char& character : exponent_string){
					if(character != '0'){
						this->stream.fatal("Tried to parse invalid integer string for exponent", number_beginning_line, number_beginning_collumn);
						return true;
					}
				}
			}
		}


		///////////////////////////////////
		// check exponent isn't too large

		if(exponent_number != 0 && exponent_number != 1){
			const long double floating_point_number = long double(exponent_number);

			if(has_decimal){
				const static long double max_float_exp = std::log10(std::numeric_limits<long double>::max()) + 1;

				if(floating_point_number > max_float_exp){
					this->stream.error(
						"Literal number exponent too large to fit into an F128.",
						number_beginning_line, number_beginning_collumn
					);
					return true;
				}

			}else{
				const static long double max_int_exp = std::log10(std::numeric_limits<uint64_t>::max()) + 1;

				if(floating_point_number > max_int_exp){
					this->stream.error(
						"Literal number exponent too large to fit into a UI64. This limitation will be removed when the compiler is self hosted.",
						number_beginning_line, number_beginning_collumn
					);
					return true;
				}
			}
		}



		///////////////////////////////////
		// parse / save number (with some checking)

		if(has_decimal){
			if(base == 16){
				number_string = "0x" + number_string;
			}

			char* str_end;
			const long double parsed_number = std::strtold(number_string.data(), &str_end);

			if(parsed_number == HUGE_VALL){
				this->stream.error(
					"Literal floating-point too large to fit into an F128",
					number_beginning_line, number_beginning_collumn
				);
			}else if(parsed_number == 0.0L && str_end == number_string.data()){
				this->stream.fatal("Tried to parse invalid floating-point string", number_beginning_line, number_beginning_collumn);
				return true;
			}



			if(std::numeric_limits<long double>::max() / parsed_number < std::pow(10, exponent_number)){
				this->stream.error(
					"Literal number exponent too large to fit into an F128. This limitation will be removed when the compiler is self hosted.",
					number_beginning_line, number_beginning_collumn
				);
				return true;
			}


			long double output_number = parsed_number;
			if(exponent_number != 0 && exponent_number != 1){
				output_number *= std::pow(10, exponent_number);
			}else if(exponent_number == 0){
				output_number = 0;
			}

			this->tokens.emplace_back(Token::Kind::LiteralFloat, output_number);


		}else{
			const uint64_t parsed_number = std::strtoull(number_string.data(), nullptr, base);

			if(parsed_number == ULLONG_MAX && errno == ERANGE){
				this->stream.error(
					"Literal integer too large to fit into a UI64. This limitation will be removed when the compiler is self hosted.",
					number_beginning_line, number_beginning_collumn
				);
				return true;

			}else if(parsed_number == 0){
				for(const char& character : number_string){
					if(character != '0'){
						this->stream.fatal("Tried to parse invalid integer string", number_beginning_line, number_beginning_collumn);
						return true;
					}
				}
			}


			if(std::numeric_limits<uint64_t>::max() / parsed_number < std::pow(10, exponent_number)){
				this->stream.error(
					"Literal number exponent too large to fit into a UI64. This limitation will be removed when the compiler is self hosted.",
					number_beginning_line, number_beginning_collumn
				);
				return true;
			}



			uint64_t output_number = parsed_number;
			if(exponent_number != 0 && exponent_number != 1){
				output_number *= uint64_t(std::pow(10, exponent_number));
			}else if(exponent_number == 0){
				output_number = 0;
			}

			this->tokens.emplace_back(Token::Kind::LiteralInt, output_number);
		}


		

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
					evo::unreachable();
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


	auto TokenizerReader::getBoolValue(TokenID id) const noexcept -> bool {
		const Tokenizer::TokenData& token = this->tokenizer.tokens[id.id];
		
		evo::debugAssert(
			token.kind == Token::Kind::LiteralBool, 
			std::format("cannot get bool value of Token Kind ({})", Token::print_kind(token.kind))
		);

		return token.value.boolean;
	};


	auto TokenizerReader::getIntegerValue(TokenID id) const noexcept -> uint64_t {
		const Tokenizer::TokenData& token = this->tokenizer.tokens[id.id];
		
		evo::debugAssert(
			token.kind == Token::Kind::LiteralInt, 
			std::format("cannot get integer value of Token Kind ({})", Token::print_kind(token.kind))
		);

		return token.value.integer;
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


				case Token::Kind::LiteralBool: {
					const char* token_value = [&](){
						if(token.value.boolean){
							return "true";
						}else{
							return "false";
						}
					}();


					evo::logInfo(std::format("LiteralBool ({})", token_value));
				} break;


				case Token::Kind::LiteralInt: {
					evo::logInfo(std::format("LiteralInt ({})", token.value.integer));
				} break;

				case Token::Kind::LiteralFloat: {
					evo::logInfo(std::format("LiteralFloat ({})", token.value.floating_point));
				} break;


				default: {
					evo::logInfo(std::format("{}", Token::print_kind(token.kind)));
				} break;
			};

		}
	};

	
};
