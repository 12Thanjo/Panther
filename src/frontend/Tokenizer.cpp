#include "./Tokenizer.h"


// for std::countl_one
#include <bit>

#include <charconv>

namespace panther{


	auto Tokenizer::tokenize() noexcept -> bool {
		while(this->char_stream.eof() == false && this->source.hasErrored() == false){
			this->line_start = this->char_stream.get_line();
			this->collumn_start = this->char_stream.get_collumn();

			if(this->tokenize_whitespace()){ continue; }
			if(this->tokenize_comment()){ continue; }
			if(this->tokenize_identifier()){ continue; }
			if(this->tokenize_operators()){ continue; }
			if(this->tokenize_punctuation()){ continue; }
			if(this->tokenize_number_literal()){ continue; }
			if(this->tokenize_string_literal()){ continue; }

			// unrecognized character
			this->error_unrecognized_character();
			return false;
		};

		return !this->source.hasErrored();
	};



	auto Tokenizer::tokenize_whitespace() noexcept -> bool {
		if(evo::isWhitespace(this->char_stream.peek())){
			this->char_stream.skip(1);
			return true;
		}
		return false;
	};


	auto Tokenizer::tokenize_comment() noexcept -> bool {
		if(this->char_stream.ammount_left() < 2 || this->char_stream.peek() != '/'){
			return false;
		}

		if(this->char_stream.peek(1) == '/'){
			// line comment

			this->char_stream.skip(2);

			while(this->char_stream.eof() == false && this->char_stream.peek() != '\n' && this->char_stream.peek() != '\r'){
				this->char_stream.skip(1);
			};

			return true;

		}else if(this->char_stream.peek(1) == '*'){
			// multi-line comment

			// TODO: remove and use this->line_start, etc.
			const uint32_t comment_beginning_line = this->char_stream.get_line();
			const uint32_t comment_beginning_collumn = this->char_stream.get_collumn();

			this->char_stream.skip(2);

			unsigned num_closes_needed = 1;
			while(num_closes_needed > 0){
				if(this->char_stream.ammount_left() < 2){
					this->source.error(
						"Unterminated multi-line comment",
						Location{comment_beginning_line, comment_beginning_line, comment_beginning_collumn, comment_beginning_collumn + 1},
						std::vector<Message::Info>{{"Expected a \"*/\" before the end of the file"}}
					);
					return true;
				}


				if(this->char_stream.peek() == '/' && this->char_stream.peek(1) == '*'){
					this->char_stream.skip(2);
					num_closes_needed += 1;

				}else if(this->char_stream.peek() == '*' && this->char_stream.peek(1) == '/'){
					this->char_stream.skip(2);
					num_closes_needed -= 1;

				}else{
					this->char_stream.skip(1);
				}
			};

			return true;
		}

		return false;
	};



	auto Tokenizer::tokenize_identifier() noexcept -> bool {
		auto kind = Token::Kind::None;

		char peeked_char = this->char_stream.peek();
		if(evo::isLetter(peeked_char) || peeked_char == '_'){
			kind = Token::Kind::Ident;

		}else if(this->char_stream.ammount_left() >= 2 && (evo::isLetter(this->char_stream.peek(1)) || this->char_stream.peek(1) == '_')){
			if(this->char_stream.peek() == '@'){
				kind = Token::Kind::Intrinsic;
				this->char_stream.skip(1);

			}else if(this->char_stream.peek() == '#'){
				kind = Token::Kind::Attribute;
				this->char_stream.skip(1);
			}
		}

		if(kind == Token::Kind::None){
			return false;
		}



		const char* string_start_ptr = this->char_stream.get_raw_ptr();

		std::string_view::size_type token_length = 0;

		do{
			this->char_stream.skip(1);
			token_length += 1;

			if(this->char_stream.eof()){ break; }

			peeked_char = this->char_stream.peek();
		}while( (evo::isAlphaNumeric(peeked_char) || peeked_char == '_'));

		auto ident_name = std::string_view(string_start_ptr, token_length);



		// TODO: hashmap
		if(kind == Token::Kind::Ident){
			
			///////////////////////////////////
			// literals

			     if(ident_name == "true")  { this->create_token(Token::Kind::LiteralBool, true);  }
			else if(ident_name == "false") { this->create_token(Token::Kind::LiteralBool, false); }


			///////////////////////////////////
			// types

			else if(ident_name == "Void") { this->create_token(Token::Kind::TypeVoid); }

			else if(ident_name == "Int")    { this->create_token(Token::Kind::TypeInt); }
			else if(ident_name == "UInt")   { this->create_token(Token::Kind::TypeUInt); }
			else if(ident_name == "Bool")   { this->create_token(Token::Kind::TypeBool); }
			else if(ident_name == "String") { this->create_token(Token::Kind::TypeString); }


			///////////////////////////////////
			// keywords

			else if(ident_name == "var")    { this->create_token(Token::Kind::KeywordVar); }
			else if(ident_name == "def")    { this->create_token(Token::Kind::KeywordDef); }
			else if(ident_name == "func")   { this->create_token(Token::Kind::KeywordFunc); }
			else if(ident_name == "struct") { this->create_token(Token::Kind::KeywordStruct); }

			else if(ident_name == "Type") { this->create_token(Token::Kind::KeywordType); }

			else if(ident_name == "return")      { this->create_token(Token::Kind::KeywordReturn); }
			else if(ident_name == "unreachable") { this->create_token(Token::Kind::KeywordUnreachable); }
			else if(ident_name == "if")          { this->create_token(Token::Kind::KeywordIf); }
			else if(ident_name == "else")        { this->create_token(Token::Kind::KeywordElse); }

			else if(ident_name == "copy")   { this->create_token(Token::Kind::KeywordCopy); }
			else if(ident_name == "uninit") { this->create_token(Token::Kind::KeywordUninit); }
			else if(ident_name == "addr")   { this->create_token(Token::Kind::KeywordAddr); }
			else if(ident_name == "and")    { this->create_token(Token::Kind::KeywordAnd); }
			else if(ident_name == "or")     { this->create_token(Token::Kind::KeywordOr); }

			else if(ident_name == "read")  { this->create_token(Token::Kind::KeywordRead); }
			else if(ident_name == "write") { this->create_token(Token::Kind::KeywordWrite); }
			else if(ident_name == "in")    { this->create_token(Token::Kind::KeywordIn); }

			else if(ident_name == "alias")    { this->create_token(Token::Kind::KeywordAlias); }


			///////////////////////////////////
			// else

			else{ this->create_token(Token::Ident, ident_name); }

		}else{
			this->create_token(kind, ident_name);
		}
		

		return true;
	};



	auto Tokenizer::tokenize_punctuation() noexcept -> bool {
		const char peeked_char = this->char_stream.peek();
		Token::Kind tok_kind = Token::None;

		switch(peeked_char){
			break; case '(': tok_kind = Token::OpenParen;
			break; case ')': tok_kind = Token::CloseParen;
			break; case '[': tok_kind = Token::OpenBracket;
			break; case ']': tok_kind = Token::CloseBracket;
			break; case '{': tok_kind = Token::OpenBrace;
			break; case '}': tok_kind = Token::CloseBrace;

			break; case ',': tok_kind = Token::Comma;
			break; case ';': tok_kind = Token::SemiColon;
			break; case ':': tok_kind = Token::Colon;

			break; case '|': tok_kind = Token::Pipe;
		};

		if(tok_kind == Token::None){ return false; }

		this->char_stream.skip(1);

		this->create_token(tok_kind);

		return true;
	};


	auto Tokenizer::tokenize_operators() noexcept -> bool {
		auto is_op = [&](std::string_view op) noexcept -> bool {
			if(this->char_stream.ammount_left() < op.size()){ return false; }

			for(int i = 0; i < op.size(); i+=1){
				if(this->char_stream.peek(i) != op[i]){
					return false;
				}
			}

			return true;
		};



		auto set_op = [&](std::string_view op){
			this->char_stream.skip(ptrdiff_t(op.size()));
			this->create_token(Token::get(op.data()));
		};



		// length 2
		if(is_op("==")){ set_op("=="); return true; }
		if(is_op("!=")){ set_op("!="); return true; }
		if(is_op("<=")){ set_op("<="); return true; }
		if(is_op(">=")){ set_op(">="); return true; }

		if(is_op("->")){ set_op("->"); return true; }
		if(is_op(".&")){ set_op(".&"); return true; }

		if(is_op("+@")){ set_op("+@"); return true; }
		if(is_op("-@")){ set_op("-@"); return true; }
		if(is_op("*@")){ set_op("*@"); return true; }


		// length 1
		if(is_op("=")){ set_op("="); return true; }

		if(is_op("!")){ set_op("!"); return true; }

		if(is_op("<")){ set_op("<"); return true; }
		if(is_op(">")){ set_op(">"); return true; }

		if(is_op("+")){ set_op("+"); return true; }
		if(is_op("-")){ set_op("-"); return true; }
		if(is_op("*")){ set_op("*"); return true; }
		if(is_op("/")){ set_op("/"); return true; }

		if(is_op("&")){ set_op("&"); return true; }

		if(is_op(".")){ set_op("."); return true; }


		return false;
	};


	auto Tokenizer::tokenize_number_literal() noexcept -> bool {
		if(evo::isNumber(this->char_stream.peek()) == false){ return false; }

		const uint32_t number_beginning_line = this->char_stream.get_line();
		const uint32_t number_beginning_collumn = this->char_stream.get_collumn();


		int base = 10;


		///////////////////////////////////
		// get number prefix

		if(this->char_stream.peek() == '0' && this->char_stream.ammount_left() >= 2){
			const char second_peek = this->char_stream.peek(1);
			if(second_peek == 'x'){
				base = 16;
				this->char_stream.skip(2);

			}else if(second_peek == 'b'){
				base = 2;
				this->char_stream.skip(2);

			}else if(second_peek == 'o'){
				base = 8;
				this->char_stream.skip(2);

			}else if(evo::isNumber(second_peek)){
				this->source.error(
					"Leading zeros in literal numbers are not supported",
					this->char_stream.get_line(), this->char_stream.get_collumn(),
					std::vector<Message::Info>{{"the literal integer prefix for base-8 is \"0o\""}}
				);

				return true;
			}
		}




		///////////////////////////////////
		// get number

		auto number_string = std::string{};

		bool has_decimal_point = false;

		while(this->char_stream.eof() == false){
			const char peeked_char = this->char_stream.peek();


			if(peeked_char == '_'){
				this->char_stream.skip(1);
				continue;

			}else if(peeked_char == '.'){
				if(has_decimal_point){
					this->source.error(
						"Cannot have multiple decimal points in a floating-point literal",
						this->char_stream.get_line(), this->char_stream.get_collumn()
					);
					return true;
				}

				if(base == 2){
					this->source.error(
						"Base-2 floating-point literals are not supported",
						number_beginning_line, number_beginning_collumn
					);
					return true;
				}else if(base == 8){
					this->source.error(
						"Base-8 floating-point literals are not supported",
						number_beginning_line, number_beginning_collumn
					);
					return true;
				}

				has_decimal_point = true;
				number_string += '.';

				this->char_stream.skip(1);
				continue;
			}



			if(base == 2){
				if(peeked_char == '0' || peeked_char == '1'){
					number_string += this->char_stream.next();

				}else if(evo::isHexNumber(peeked_char)){
					this->source.error(
						"Base-2 numbers should only have digits 0 and 1",
						this->char_stream.get_line(), this->char_stream.get_collumn()
					);
					return true;

				}else{
					break;
				}

			}else if(base == 8){
				if(evo::isOctalNumber(peeked_char)){
					number_string += this->char_stream.next();

				}else if(evo::isHexNumber(peeked_char)){
					this->source.error(
						"Base-8 numbers should only have digits 0-7",
						this->char_stream.get_line(), this->char_stream.get_collumn()
					);
					return true;

				}else{
					break;
				}

			}else if(base == 10){
				if(evo::isNumber(peeked_char)){
					number_string += this->char_stream.next();

				}else if(peeked_char == 'e' || peeked_char == 'E'){
					break;

				}else if(evo::isHexNumber(peeked_char)){
					this->source.error(
						"Base-10 numbers should only have digits 0-9",
						this->char_stream.get_line(), this->char_stream.get_collumn()
					);
					return true;

				}else{
					break;
				}

			}else{
				// base-16
				if(evo::isHexNumber(peeked_char)){
					number_string += this->char_stream.next();

				}else{
					break;
				}
			}

		};


		///////////////////////////////////
		// get exponent (if it exsits)

		auto exponent_string = std::string{};
		if(this->char_stream.ammount_left() >= 2  && (this->char_stream.peek() == 'e' || this->char_stream.peek() == 'E')){
			this->char_stream.skip(1);

			if(this->char_stream.peek() == '-' || this->char_stream.peek() == '+'){
				exponent_string += this->char_stream.next();
			}

			while(this->char_stream.eof() == false){
				const char peeked_char = this->char_stream.peek();

				if(evo::isNumber(peeked_char)){
					exponent_string += this->char_stream.next();

				}else if(evo::isHexNumber(peeked_char)){
					this->source.error(
						"Literal number exponents should only have digits 0-9",
						this->char_stream.get_line(), this->char_stream.get_collumn(
					));
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
				this->source.error(
					"Literal number exponent too large to fit into a I64. This limitation will be removed when the compiler is self hosted.",
					number_beginning_line, number_beginning_collumn
				);
				return true;

			}else if(exponent_number == 0){
				for(const char& character : exponent_string){
					if(character != '0'){
						this->source.fatal("Tried to parse invalid integer string for exponent", number_beginning_line, number_beginning_collumn);
						return true;
					}
				}
			}
		}


		///////////////////////////////////
		// check exponent isn't too large

		if(exponent_number != 0 && exponent_number != 1){
			const float64_t floating_point_number = float64_t(exponent_number);

			if(has_decimal_point){
				const static float64_t max_float_exp = std::log10(std::numeric_limits<float64_t>::max()) + 1;

				if(floating_point_number > max_float_exp){
					this->source.error(
						"Literal number exponent too large to fit into an F64.",
						number_beginning_line, number_beginning_collumn
					);
					return true;
				}

			}else{
				const static float64_t max_int_exp = std::log10(std::numeric_limits<uint64_t>::max()) + 1;

				if(floating_point_number > max_int_exp){
					this->source.error(
						"Literal number exponent too large to fit into a UI64. This limitation will be removed when the compiler is self hosted.",
						number_beginning_line, number_beginning_collumn
					);
					return true;
				}
			}
		}



		///////////////////////////////////
		// parse / save number (with some checking)

		if(has_decimal_point){
			if(base == 16){
				number_string = "0x" + number_string;
			}

			char* str_end;
			const float64_t parsed_number = std::strtod(number_string.data(), &str_end);

			if(parsed_number == HUGE_VALL){
				this->source.error(
					"Literal floating-point too large to fit into an F64",
					number_beginning_line, number_beginning_collumn
				);
			}else if(parsed_number == 0.0L && str_end == number_string.data()){
				this->source.fatal("Tried to parse invalid floating-point string", number_beginning_line, number_beginning_collumn);
				return true;
			}

			if(parsed_number == 0.0 && std::numeric_limits<float64_t>::max() / parsed_number < std::pow(10, exponent_number)){
				this->source.error(
					"Literal number exponent too large to fit into an F64. This limitation will be removed when the compiler is self hosted.",
					number_beginning_line, number_beginning_collumn
				);
				return true;
			}


			float64_t output_number = parsed_number;
			     if(exponent_number == 0){ output_number = 0; }
			else if(exponent_number != 1){ output_number *= std::pow(10, exponent_number); }

			this->create_token(Token::Kind::LiteralFloat, output_number);


		}else{
			const uint64_t parsed_number = std::strtoull(number_string.data(), nullptr, base);

			if(parsed_number == ULLONG_MAX && errno == ERANGE){
				this->source.error(
					"Literal integer too large to fit into a UI64. This limitation will be removed when the compiler is self hosted.",
					number_beginning_line, number_beginning_collumn
				);
				return true;

			}else if(parsed_number == 0){
				for(const char& character : number_string){
					if(character != '0'){
						this->source.fatal("Tried to parse invalid integer string", number_beginning_line, number_beginning_collumn);
						return true;
					}
				}
			}


			if(parsed_number != 0 && std::numeric_limits<uint64_t>::max() / parsed_number < std::pow(10, exponent_number)){
				this->source.error(
					"Literal number exponent too large to fit into a UI64. This limitation will be removed when the compiler is self hosted.",
					number_beginning_line, number_beginning_collumn
				);
				return true;
			}



			uint64_t output_number = parsed_number;
			     if(exponent_number == 0){ output_number = 0; }
			else if(exponent_number != 1){ output_number *= uint64_t(std::pow(10, exponent_number)); }

			this->create_token(Token::Kind::LiteralInt, output_number);
		}

		return true;
	};



	auto Tokenizer::tokenize_string_literal() noexcept -> bool {
		if(this->char_stream.peek() != '"' && this->char_stream.peek() != '\''){ return false; }

		const char delimiter = this->char_stream.next();

		auto literal_value = std::string();

		while(this->char_stream.peek() != delimiter){
			bool unexpected_eof = false;

			if(this->char_stream.eof()){
				unexpected_eof = true;

			}else if(this->char_stream.peek() == '\\'){
				switch(this->char_stream.peek(1)){
					break; case '0': literal_value += '\0';
					break; case 'a': literal_value += '\a';
					break; case 'b': literal_value += '\b';
					break; case 't': literal_value += '\t';
					break; case 'n': literal_value += '\n';
					break; case 'v': literal_value += '\v';
					break; case 'f': literal_value += '\f';
					break; case 'r': literal_value += '\r';

					break; case '\'': literal_value += '\'';
					break; case '"':  literal_value += '"';
					break; case '\\': literal_value += '\\';

					break; default: {
						this->source.error(
							std::format("Unknown string escape code '\\{}'", this->char_stream.peek(1)),
							Location(
								this->char_stream.get_line(), this->char_stream.get_line(),
								this->char_stream.get_collumn(), this->char_stream.get_collumn() + 1
							)
						);

						return true;
					}
				};

				this->char_stream.skip(2);

			}else{
				literal_value += this->char_stream.next();
			}

			// needed because some code above may have called next() or skip()
			if(this->char_stream.eof()){
				unexpected_eof = true;
			}

			if(unexpected_eof){
				const char* string_type_name = [&]() noexcept {
					if(delimiter == '"'){ return "string"; }
					if(delimiter == '\''){ return "character"; }
					evo::unreachable();
				}();

				evo::breakpoint();

				this->source.error(
					std::format("Unterminated {} literal", string_type_name),
					this->line_start, this->collumn_start,

					std::vector<Message::Info>{
						Message::Info(std::format("Expected a {} before the end of the file", delimiter)),
					}
				);
				
				return true;	
			}

		};


		this->char_stream.skip(1);

		const std::string* string_literal_value = this->source.string_literal_values.emplace_back(
			new std::string(std::move(literal_value))
		);


		if(delimiter == '\''){
			this->create_token(Token::LiteralChar, std::string_view(*string_literal_value));
		}else{
			this->create_token(Token::LiteralString, std::string_view(*string_literal_value));
		}


		return true;
	};







	auto Tokenizer::create_token(Token::Kind kind) noexcept -> void {
		this->source.tokens.emplace_back(
			kind, Location(this->line_start, this->char_stream.get_line(), this->collumn_start, this->char_stream.get_collumn() - 1)
		);
	};


	template<typename T>
	auto Tokenizer::create_token(Token::Kind kind, T value) noexcept -> void {
		this->source.tokens.emplace_back(
			kind, Location(this->line_start, this->char_stream.get_line(), this->collumn_start, this->char_stream.get_collumn() - 1), value
		);
	};





	//////////////////////////////////////////////////////////////////////
	// unrecognized character


	EVO_NODISCARD static constexpr auto hex_from_4_bits(char num) noexcept -> char {
		switch(num){
			case 0: return '0';
			case 1: return '1';
			case 2: return '2';
			case 3: return '3';
			case 4: return '4';
			case 5: return '5';
			case 6: return '6';
			case 7: return '7';
			case 8: return '8';
			case 9: return '9';
			case 10: return 'A';
			case 11: return 'B';
			case 12: return 'C';
			case 13: return 'D';
			case 14: return 'E';
			case 15: return 'F';
			default: EVO_FATAL_BREAK("Not valid num (must be 4 bits)");
		};
	};


	inline auto Tokenizer::error_unrecognized_character() noexcept -> void {
		const char peeked_char = this->char_stream.peek();

		if(peeked_char >= 0){
			this->source.error(
				std::format("Unrecognized character \"{}\" (charcode: {})", evo::printCharName(peeked_char), int(peeked_char)),
				this->char_stream.get_line(), this->char_stream.get_collumn()
			);

		}else{
			// detect utf-8
			// https://en.wikipedia.org/wiki/UTF-8

			auto utf8_str = evo::StaticString<4>();

			// TODO: does this need to be std::bit_cast?
			const ptrdiff_t num_chars_of_utf8 = ptrdiff_t(std::countl_one( static_cast<unsigned char>(this->char_stream.peek()) ));

			if(num_chars_of_utf8 > 4){
				this->source.error(
					"Unrecognized character (non-standard utf-8 character)",
					this->char_stream.get_line(), this->char_stream.get_collumn()
				);
				return;
			}

			// TODO: make sure this is safe to do (not past EOF)
			for(ptrdiff_t i = 0; i < num_chars_of_utf8; i+=1){
				utf8_str.push_back(this->char_stream.peek(i));
			}

			auto utf8_charcodes_str = evo::StaticString<8>("U+");
			switch(num_chars_of_utf8){
				case 2: {
					utf8_charcodes_str.push_back('0');

					char charcode = utf8_str[0] >> 2;
					charcode &= 0b0111;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));

					charcode = utf8_str[0] & 0b11;
					charcode <<= 2;
					charcode |= (utf8_str[1] >> 4) & 0b0011;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));

					charcode = utf8_str[1] & 0b1111;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));
				} break;

				case 3: {
					char charcode = utf8_str[0] & 0b1111;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));

					charcode = utf8_str[1] >> 2;
					charcode &= 0b1111;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));

					charcode = utf8_str[1] & 0b11;
					charcode <<= 2;
					charcode |= (utf8_str[2] >> 4) & 0b0011;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));

					charcode = utf8_str[2] & 0b1111;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));
				} break;


				case 4: {
					char charcode = utf8_str[0] >> 2;
					charcode &= 0b1;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));

					charcode = utf8_str[0] & 0b11;
					charcode <<= 2;
					charcode |= (utf8_str[1] >> 4) & 0b0011;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));

					charcode = utf8_str[1] & 0b1111;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));

					charcode = utf8_str[2] >> 2;
					charcode &= 0b1111;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));

					charcode = utf8_str[2] & 0b11;
					charcode <<= 2;
					charcode |= (utf8_str[3] >> 4) & 0b0011;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));

					charcode = utf8_str[3] & 0b1111;
					utf8_charcodes_str.push_back(hex_from_4_bits(charcode));
				} break;
			};


			this->source.error(
				std::format("Unrecognized character \"{}\" (UTF-8 code: {})", utf8_str, utf8_charcodes_str),
				this->char_stream.get_line(), this->char_stream.get_collumn()
			);
		}
	};

	
};
