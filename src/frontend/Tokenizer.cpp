#include "frontend/Tokenizer.h"


#include <cstdlib>


namespace panther{

	auto Tokenizer::tokenize() noexcept -> bool {
		while(this->stream.is_eof() == false && this->stream.get_source_manager().errored() == false){
			this->line_start = this->stream.get_line();
			this->collumn_start = this->stream.get_collumn();

			if(this->tokenize_whitespace()){ continue; }
			if(this->tokenize_comment()){ continue; }
			if(this->tokenize_ident()){ continue; }
			if(this->tokenize_punctuation()){ continue; }
			if(this->tokenize_operators()){ continue; }
			if(this->tokenize_number_literal()){ continue; }
			if(this->tokenize_string_literal()){ continue; }


			this->stream.error(std::format("Unrecognized character ({})", this->stream.peek()), this->stream.get_line(), this->stream.get_collumn());

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
					this->stream.error("Unexpected end of file in multi-line comment", comment_beginning_line, comment_beginning_collumn, this->stream.get_line(), this->stream.get_collumn());
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

		const uint32_t ident_beginning_line = this->stream.get_line();
		const uint32_t ident_beginning_collumn = this->stream.get_collumn();


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

			///////////////////////////////////
			// literals

			     if(ident_name == "true")  { this->add_token(Token::Kind::LiteralBool, true);  }
			else if(ident_name == "false") { this->add_token(Token::Kind::LiteralBool, false); }


			///////////////////////////////////
			// types

			else if(ident_name == "Void") { this->add_token(Token::Kind::TypeVoid); }
			else if(ident_name == "Bool") { this->add_token(Token::Kind::TypeBool); }
			else if(ident_name == "String") { this->add_token(Token::Kind::TypeString); }
			else if(ident_name == "Char") { this->add_token(Token::Kind::TypeChar); }

			else if(ident_name == "Int")   { this->add_token(Token::Kind::TypeInt);   }
			else if(ident_name == "UInt")  { this->add_token(Token::Kind::TypeUInt);  }

			else if(ident_name == "F16")  { this->add_token(Token::Kind::TypeF16);  }
			else if(ident_name == "F32")  { this->add_token(Token::Kind::TypeF32);  }
			else if(ident_name == "F64")  { this->add_token(Token::Kind::TypeF64);  }
			else if(ident_name == "F128") { this->add_token(Token::Kind::TypeF128); }

			else if(ident_name == "USize") { this->add_token(Token::Kind::TypeUSize); }
			else if(ident_name == "Rawptr") { this->add_token(Token::Kind::TypeRawptr); }
			else if(ident_name == "Bool32") { this->add_token(Token::Kind::TypeBool32); }

			else if(ident_name == "CInt") { this->add_token(Token::Kind::TypeCInt); }
			else if(ident_name == "CUInt") { this->add_token(Token::Kind::TypeCUInt); }


			///////////////////////////////////
			// keywords

			else if(ident_name == "var")     { this->add_token(Token::Kind::KeywordVar);     }
			else if(ident_name == "def")     { this->add_token(Token::Kind::KeywordDef);     }
			else if(ident_name == "typedef") { this->add_token(Token::Kind::KeywordTypedef); }
			else if(ident_name == "alias")   { this->add_token(Token::Kind::KeywordAlias);   }

			else if(ident_name == "pub")    { this->add_token(Token::Kind::KeywordPub);    }
			else if(ident_name == "static") { this->add_token(Token::Kind::KeywordStatic); }


			else if(ident_name == "func")     { this->add_token(Token::Kind::KeywordFunc);     }
			else if(ident_name == "operator") { this->add_token(Token::Kind::KeywordOperator); }
			else if(ident_name == "struct")   { this->add_token(Token::Kind::KeywordStruct);   }
			else if(ident_name == "enum")     { this->add_token(Token::Kind::KeywordEnum);     }
			else if(ident_name == "union")    { this->add_token(Token::Kind::KeywordUnion);    }
			else if(ident_name == "flags")    { this->add_token(Token::Kind::KeywordFlags);    }

			else if(ident_name == "if")      { this->add_token(Token::Kind::KeywordIf);      }
			else if(ident_name == "else")    { this->add_token(Token::Kind::KeywordElse);    }
			else if(ident_name == "do")      { this->add_token(Token::Kind::KeywordDo);      }
			else if(ident_name == "while")   { this->add_token(Token::Kind::KeywordWhile);   }
			else if(ident_name == "switch")  { this->add_token(Token::Kind::KeywordSwitch);  }
			else if(ident_name == "case")    { this->add_token(Token::Kind::KeywordCase);    }
			else if(ident_name == "default") { this->add_token(Token::Kind::KeywordDefault); }

			else if(ident_name == "copy")   { this->add_token(Token::Kind::KeywordCopy);   }
			else if(ident_name == "move")   { this->add_token(Token::Kind::KeywordMove);   }
			else if(ident_name == "addr")   { this->add_token(Token::Kind::KeywordAddr);   }
			else if(ident_name == "as")     { this->add_token(Token::Kind::KeywordAs);     }
			else if(ident_name == "cast")   { this->add_token(Token::Kind::KeywordCast);   }

			else if(ident_name == "read")  { this->add_token(Token::Kind::KeywordRead);  }
			else if(ident_name == "write") { this->add_token(Token::Kind::KeywordWrite); }
			else if(ident_name == "in")    { this->add_token(Token::Kind::KeywordIn);    }

			else if(ident_name == "return") { this->add_token(Token::Kind::KeywordReturn); }
			else if(ident_name == "throw")  { this->add_token(Token::Kind::KeywordThrow);  }
			else if(ident_name == "defer")  { this->add_token(Token::Kind::KeywordDefer);  }
			else if(ident_name == "break")  { this->add_token(Token::Kind::KeywordBreak);  }
			else if(ident_name == "continue")  { this->add_token(Token::Kind::KeywordContinue);  }

			else if(ident_name == "try")   { this->add_token(Token::Kind::KeywordTry);   }
			else if(ident_name == "catch") { this->add_token(Token::Kind::KeywordCatch); }

			else if(ident_name == "this")   { this->add_token(Token::Kind::KeywordThis);       }
			else if(ident_name == "_")      { this->add_token(Token::Kind::KeywordUnderscore); }
			else if(ident_name == "uninit") { this->add_token(Token::Kind::KeywordUninit);     }
			else if(ident_name == "null")   { this->add_token(Token::Kind::KeywordNull);       }


			else{

				Token::Kind token_kind = Token::Kind::Ident;

				//////////////////////////////////////////////////////////////////////
				// arbitrary integer bit widths

				if(ident_name.size() >= 2 && ident_name[0] == 'I'){
					token_kind = Token::Kind::TypeIntN;

					for(int i = 1; i < ident_name.size(); i+=1){
						if(evo::isNumber(ident_name[i]) == false){
							token_kind = Token::Kind::Ident;
							break;
						}
					}

				}else if(ident_name.size() >= 3 && ident_name[0] == 'U' && ident_name[1] == 'I'){
					token_kind = Token::Kind::TypeUIntN;

					for(int i = 2; i < ident_name.size(); i+=1){
						if(evo::isNumber(ident_name[i]) == false){
							token_kind = Token::Kind::Ident;
							break;
						}
					}
				}



				if(token_kind != Token::Kind::Ident){
					const char* int_width_string_start = ident_name.data();

					if(token_kind == Token::Kind::TypeIntN){
						int_width_string_start += 1;
					}else if(token_kind == Token::Kind::TypeUIntN){
						int_width_string_start += 2;
					}

					const uint32_t integer_width = std::strtoul(int_width_string_start, nullptr, 10);

					if(integer_width == ULONG_MAX && errno == ERANGE){
						this->stream.error(
							"Integer bit-width is too large. Maximum is 2^23",
							ident_beginning_line, ident_beginning_collumn
						);
						return true;

					}else if(integer_width == 0){
						for(int i = int(int_width_string_start - ident_name.data()); i < ident_name.size(); i+=1){
							if(ident_name[i] != '0'){
								this->stream.fatal("Tried to parse invalid integer string for integer bit-width", ident_beginning_line, ident_beginning_collumn);
								return true;
							}
						}


						this->stream.error("Integer bit-width must be at least 1", ident_beginning_line, ident_beginning_collumn);
						return true;

					}else if(integer_width > std::pow(2, 23)){
						this->stream.error("Integer bit-width is too large. Maximum is 2^23", ident_beginning_line, ident_beginning_collumn);
						return true;
					}


					this->add_token(token_kind, uint64_t(integer_width));


				}else{
					// general ident

					const size_t string_index = this->token_value_strings.size();
					this->add_token(token_kind, string_index);
					this->token_value_strings.emplace_back(std::move(ident_name));
				}

			}

		}else{
			const size_t string_index = this->token_value_strings.size();
			this->add_token(kind, string_index);
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
		this->add_token(Token::get(punctuation_str));

		return true;
	};


	auto Tokenizer::tokenize_operators() noexcept -> bool {
		auto is_op = [&](std::string_view op) noexcept -> bool {
			if(this->stream.ammount_left() < op.size()){ return false; }

			for(int i = 0; i < op.size(); i+=1){
				if(this->stream.peek(i) != op[i]){
					return false;
				}
			}

			return true;
		};



		auto set_op = [&](std::string_view op){
			this->stream.skip(unsigned(op.size()));
			this->add_token(Token::get(op.data()));
		};


		// length 3
		if(is_op("...")){ set_op("..."); return true; }


		// length 2
		else  if(is_op(".^")){ set_op(".^"); return true; }
		else if(is_op(".?")){ set_op(".?"); return true; }

		else if(is_op("+=")){ set_op("+="); return true; }
		else if(is_op("-=")){ set_op("-="); return true; }
		else if(is_op("*=")){ set_op("*="); return true; }
		else if(is_op("/=")){ set_op("/="); return true; }
		else if(is_op("%=")){ set_op("%="); return true; }

		else if(is_op("&&")){ set_op("&&"); return true; }
		else if(is_op("||")){ set_op("||"); return true; }

		else if(is_op("<=")){ set_op("<="); return true; }
		else if(is_op(">=")){ set_op(">="); return true; }
		else if(is_op("==")){ set_op("=="); return true; }
		else if(is_op("!=")){ set_op("!="); return true; }

		else if(is_op("<<")){ set_op("<<"); return true; }
		else if(is_op(">>")){ set_op(">>"); return true; }


		else if(is_op("->")){ set_op("->"); return true; }



		// length 1

		else if(is_op("=")){ set_op("="); return true; }
		else if(is_op("?")){ set_op("?"); return true; }

		else if(is_op("^")){ set_op("^"); return true; }
		else if(is_op(".")){ set_op("."); return true; }

		else if(is_op("+")){ set_op("+"); return true; }
		else if(is_op("-")){ set_op("-"); return true; }
		else if(is_op("*")){ set_op("*"); return true; }
		else if(is_op("/")){ set_op("/"); return true; }
		else if(is_op("%")){ set_op("%"); return true; }


		else if(is_op("<")){ set_op("<"); return true; }
		else if(is_op(">")){ set_op(">"); return true; }



		else if(is_op("&")){ set_op("&"); return true; }
		else if(is_op("|")){ set_op("|"); return true; }
		else if(is_op("~")){ set_op("~"); return true; }

		
		return false;
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

				this->stream.error_info("the literal integer prefix for base-8 is \"0o\"");
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
			const float128_t floating_point_number = float128_t(exponent_number);

			if(has_decimal){
				const static float128_t max_float_exp = std::log10(std::numeric_limits<float128_t>::max()) + 1;

				if(floating_point_number > max_float_exp){
					this->stream.error(
						"Literal number exponent too large to fit into an F128.",
						number_beginning_line, number_beginning_collumn
					);
					return true;
				}

			}else{
				const static float128_t max_int_exp = std::log10(std::numeric_limits<uint64_t>::max()) + 1;

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
			const float128_t parsed_number = std::strtold(number_string.data(), &str_end);

			if(parsed_number == HUGE_VALL){
				this->stream.error(
					"Literal floating-point too large to fit into an F128",
					number_beginning_line, number_beginning_collumn
				);
			}else if(parsed_number == 0.0L && str_end == number_string.data()){
				this->stream.fatal("Tried to parse invalid floating-point string", number_beginning_line, number_beginning_collumn);
				return true;
			}

			if(parsed_number == 0.0 && std::numeric_limits<float128_t>::max() / parsed_number < std::pow(10, exponent_number)){
				this->stream.error(
					"Literal number exponent too large to fit into an F128. This limitation will be removed when the compiler is self hosted.",
					number_beginning_line, number_beginning_collumn
				);
				return true;
			}


			float128_t output_number = parsed_number;
			     if(exponent_number == 0){ output_number = 0; }
			else if(exponent_number != 1){ output_number *= std::pow(10, exponent_number); }

			this->add_token(Token::Kind::LiteralFloat, output_number);


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


			if(parsed_number != 0 && std::numeric_limits<uint64_t>::max() / parsed_number < std::pow(10, exponent_number)){
				this->stream.error(
					"Literal number exponent too large to fit into a UI64. This limitation will be removed when the compiler is self hosted.",
					number_beginning_line, number_beginning_collumn
				);
				return true;
			}



			uint64_t output_number = parsed_number;
			     if(exponent_number == 0){ output_number = 0; }
			else if(exponent_number != 1){ output_number *= uint64_t(std::pow(10, exponent_number)); }

			this->add_token(Token::Kind::LiteralInt, output_number);
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
					string_beginning_line, string_beginning_collumn, this->stream.get_line(), this->stream.get_collumn()
				);

				return true;
			}



		}while(this->stream.peek() != delimiter);

		this->stream.skip(1);


		const size_t string_index = this->token_value_strings.size();
		if(delimiter == '\''){
			this->add_token(Token::Kind::LiteralChar, string_index);
		}else{
			this->add_token(Token::Kind::LiteralString, string_index);
		}
		this->token_value_strings.emplace_back(std::move(contents));

		return true;
	};






	auto Tokenizer::add_token(Token::Kind token_kind, size_t index_val) noexcept -> void {
		this->tokens.emplace_back(token_kind, this->line_start, this->collumn_start, this->stream.get_line(), this->stream.get_collumn() - 1, index_val);
	};

	auto Tokenizer::add_token(Token::Kind token_kind, bool bool_val) noexcept -> void {
		this->tokens.emplace_back(token_kind, this->line_start, this->collumn_start, this->stream.get_line(), this->stream.get_collumn() - 1, bool_val);
	};

	auto Tokenizer::add_token(Token::Kind token_kind, float128_t float_val) noexcept -> void {
		this->tokens.emplace_back(token_kind, this->line_start, this->collumn_start, this->stream.get_line(), this->stream.get_collumn() - 1, float_val);
	};

	auto Tokenizer::add_token(Token::Kind token_kind) noexcept -> void {
		this->tokens.emplace_back(token_kind, this->line_start, this->collumn_start, this->stream.get_line(), this->stream.get_collumn() - 1);
	};







	//////////////////////////////////////////////////////////////////////
	// Tokenizer reader

	auto TokenizerReader::peek(int32_t offset) noexcept -> TokenID {
		evo::debugAssert(offset > 0 || (offset * -1) <= this->cursor, "TokenizerReader peek+offset is less than 0");

		const uint32_t target_location = uint32_t(this->cursor + offset);

		evo::debugAssert(size_t(target_location) < this->tokenizer.tokens.size(), "TokenizerReader peek+offset is larger than src data");


		return TokenID{target_location};
	};



	auto TokenizerReader::next() noexcept -> TokenID {
		evo::debugAssert(this->cursor < int64_t(this->tokenizer.tokens.size()), "TokenizerReader cannot get next token - already at EOF");
		
		this->cursor += 1;

		return TokenID{uint32_t(this->cursor - 1)};
	};

	auto TokenizerReader::skip(uint32_t ammount) noexcept -> void {
		evo::debugAssert(this->ammount_left() >= ammount, std::format("TokenizerReader cannot skip {} chars - goes past EOF", ammount));
		
		this->cursor += ammount;
	};


	auto TokenizerReader::go_back(TokenID id) noexcept -> void {
		evo::debugAssert(id.id <= uint32_t(this->cursor), std::format("{} should be only used to go backwards, not forwards", __FUNCTION__));

		this->cursor = int64_t(id.id);
	};



	

	auto TokenizerReader::getKind(TokenID id) const noexcept -> Token::Kind {
		return this->tokenizer.tokens[id.id].kind;
	};

	auto TokenizerReader::getLine(TokenID id) const noexcept -> uint32_t {
		return this->tokenizer.tokens[id.id].line;
	};

	auto TokenizerReader::getCollumn(TokenID id) const noexcept -> uint32_t {
		return this->tokenizer.tokens[id.id].collumn;
	};

	auto TokenizerReader::getLineStart(TokenID id) const noexcept -> uint32_t {
		return this->tokenizer.tokens[id.id].line_start;
	};

	auto TokenizerReader::getCollumnStart(TokenID id) const noexcept -> uint32_t {
		return this->tokenizer.tokens[id.id].collumn_start;
	};



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
			token.kind == Token::Kind::LiteralInt
			|| token.kind == Token::Kind::TypeIntN
			|| token.kind == Token::Kind::TypeUIntN
			, std::format("cannot get integer value of Token Kind ({})", Token::print_kind(token.kind))
		);

		return token.value.integer;
	};


	auto TokenizerReader::getFloatingPointValue(TokenID id) const noexcept -> float128_t {
		const Tokenizer::TokenData& token = this->tokenizer.tokens[id.id];
		
		evo::debugAssert(
			token.kind == Token::Kind::LiteralFloat, 
			std::format("cannot get floating-point value of Token Kind ({})", Token::print_kind(token.kind))
		);

		return token.value.floating_point;
	};





	///////////////////////////////////
	// messaging

	auto TokenizerReader::error(const std::string& message, uint32_t line_start, uint32_t collumn_start, uint32_t line, uint32_t collumn) const noexcept -> void {
		this->tokenizer.stream.error(message, line_start, collumn_start, line, collumn);
	};
	auto TokenizerReader::error(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void {
		this->tokenizer.stream.error(message, line, collumn);
	};
	auto TokenizerReader::error_info(const std::string& message) const noexcept -> void {
		this->tokenizer.stream.error_info(message);
	};
	auto TokenizerReader::error_info(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void {
		this->tokenizer.stream.error_info(message, line, collumn);
	};
	auto TokenizerReader::fatal(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void {
		this->tokenizer.stream.fatal(message, line, collumn);
	};
	auto TokenizerReader::fatal(const std::string& message, uint32_t line_start, uint32_t collumn_start, uint32_t line, uint32_t collumn) const noexcept -> void {
		this->tokenizer.stream.fatal(message, line_start, collumn_start, line, collumn);
	};




	auto TokenizerReader::get_source_manager() const noexcept -> SourceManager& {
		return this->tokenizer.stream.get_source_manager();
	};




	///////////////////////////////////
	// debug printing

	auto TokenizerReader::print_to_console() const noexcept -> void {
		for(auto& token : this->tokenizer.tokens){
			switch(token.kind){
				case Token::Kind::Ident:
				case Token::Kind::Attribute:
				case Token::Kind::Intrinsic:
				case Token::Kind::LiteralString: 
				case Token::Kind::LiteralChar:
				{
					const std::string& token_value_string = this->tokenizer.token_value_strings[token.value.index];
					this->printer.info(std::format("{} ({})", Token::print_kind(token.kind), token_value_string) + '\n');
				} break;


				case Token::Kind::LiteralBool: {
					const char* token_value = [&](){
						if(token.value.boolean){
							return "true";
						}else{
							return "false";
						}
					}();


					this->printer.info(std::format("LiteralBool ({})", token_value) + '\n');
				} break;


				case Token::Kind::LiteralInt: {
					this->printer.info(std::format("LiteralInt ({})", token.value.integer) + '\n');
				} break;

				case Token::Kind::LiteralFloat: {
					this->printer.info(std::format("LiteralFloat ({})", token.value.floating_point) + '\n');
				} break;


				case Token::Kind::TypeIntN: {
					this->printer.info(std::format("I{}", token.value.integer) + '\n');
				} break;

				case Token::Kind::TypeUIntN: {
					this->printer.info(std::format("UI{}", token.value.integer) + '\n');
				} break;


				default: {
					this->printer.info(std::format("{}", Token::print_kind(token.kind)) + '\n');
				} break;
			};

		}
	};





	
};
