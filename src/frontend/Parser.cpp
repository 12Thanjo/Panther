#include "frontend/Parser.h"


namespace panther{
	
	auto Parser::parse() noexcept -> bool {
		while(this->reader.is_eof() == false && this->reader.get_source_manager().errored() == false){
			// var_decl
			{
				const auto result = this->parse_var_decl();
				switch(result.code()){
					case Result::Success: {
						this->top_level_statements.emplace_back(result.value());
						continue;
					}

					case Result::WrongType: break;

					case Result::Error: continue;

					case Result::UnreportedError:{
						this->fatal("Failed to parse Variable Declaration", this->reader.peek());
						continue;
					}
				};
			}


			// func_def
			// assignment_stmt
			// multiple_assignment
			// conditional
			// iteration
			// try_catch
			// while_stmt
			// do_while_stmt
			// typedef_stmt
			// alias_stmt
			// return_stmt
			// defer_stmt
			// struct_def
			// expr ; // meant for things like function calls (make sure to check in semantic ananlysis that there actually are side effects)
		
			this->error("Invalid beginning to statement", this->reader.peek());
		};


		return this->reader.get_source_manager().errored() == false;
	};





	auto Parser::parse_var_decl() noexcept -> Result {
		const TokenID first_token = this->reader.peek();

		// check public
		bool is_public = false;
		if(this->reader.getKind(first_token) == Token::Kind::KeywordPub){
			is_public = true;
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->error("Unexpected end of file in variable declaration", this->reader.peek(-1)); 
				return Result::Error;
			}
		}


		// check static
		bool is_static = false;
		if(this->reader.getKind(this->reader.peek()) == Token::Kind::KeywordStatic){
			is_static = true;
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->error("Unexpected end of file in variable declaration", this->reader.peek(-1)); 
				return Result::Error;
			}
		}


		// decl type
		AST::VarDecl::DeclType decl_type;
		const TokenID decl_type_token = this->reader.next();

		if(this->reader.getKind(decl_type_token) == Token::KeywordVar){
			decl_type = AST::VarDecl::DeclType::Var;

		}else if(this->reader.getKind(decl_type_token) == Token::KeywordDef){
			decl_type = AST::VarDecl::DeclType::Def;

		}else{
			this->reader.go_back(first_token);
			return Result::WrongType;
		}

		if(this->reader.is_eof()){
			this->error("Unexpected end of file in variable declaration", this->reader.peek(-1)); 
			return Result::Error;
		}


		// identifier
		const Result ident_result = this->parse_ident();
		if(ident_result.code() == Result::WrongType){
			this->error(
				std::format(
					"Expected identifer in variable declaration - got ({}) instead", Token::print_kind( this->reader.getKind(this->reader.peek()) )
				),
				this->reader.peek()
			);
			return Result::Error;
		}

		if(this->reader.is_eof()){
			this->error("Unexpected end of file in variable declaration", this->reader.peek(-1)); 
			return Result::Error;
		}


		// : type (maybe)
		auto type = std::optional<AST::NodeID>{};
		if(this->reader.getKind(this->reader.peek()) == Token::get(":")){
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->error("Unexpected end of file in variable declaration", this->reader.peek(-1)); 
				return Result::Error;
			}

			const Result type_result = this->parse_type();

			switch(type_result.code()){
				case Result::Success: {
					type = type_result.value();
				} break;

				case Result::WrongType: {
					this->error(
						std::format(
							"Expected type in variable declaration - got ({}) instead", Token::print_kind( this->reader.getKind(this->reader.peek()) )
						),
						this->reader.peek()
					);
					return Result::Error;
				} break;

				case Result::Error: {
					return Result::Error;
				} break;

				case Result::UnreportedError: {
					this->error("Invalid type in variable declaration", this->reader.peek());
					return Result::Error;
				} break;
			};
			
			if(this->reader.is_eof()){
				this->error("Unexpected end of file in variable declaration", this->reader.peek(-1)); 
				return Result::Error;
			}
		}



		// =
		if(this->reader.getKind(this->reader.peek()) != Token::get("=")){
			if(type.has_value()){
				this->error(
					std::format(
						"Expected \"=\" in variable declaration - got ({}) instead", Token::print_kind( this->reader.getKind(this->reader.peek()) )
					),
					this->reader.peek()
				);

			}else{
				this->error(
					std::format(
						"Expected either \": Type =\" or \"=\" in variable declaration - got ({}) instead", Token::print_kind( this->reader.getKind(this->reader.peek()) )
					),
					this->reader.peek()
				);
			}

			return Result::Error;

		}else{
			this->reader.skip(1);
		}

		if(this->reader.is_eof()){
			this->error("Unexpected end of file in variable declaration", this->reader.peek(-1)); 
			return Result::Error;
		}


		// expression
		const Result expr_result = this->parse_expr();
		switch(expr_result.code()){
			case Result::Success: break;

			case Result::WrongType: {
				this->error(
					std::format(
						"Expected expression in variable declaration - got ({}) instead", Token::print_kind( this->reader.getKind(this->reader.peek()) )
					),
					this->reader.peek()
				);
				return Result::Error;
			} break;

			case Result::Error: {
				return Result::Error;
			} break;

			case Result::UnreportedError: {
				this->error("Failed to parse expression in variable declaration", this->reader.peek());
				return Result::Error;
			} break;
		};

		// if(this->reader.is_eof()){
		// 	this->error("Unexpected end of file in variable declaration", this->reader.peek(-1)); 
		// 	return Result::Error;
		// }


		// ;
		if(this->reader.getKind(this->reader.peek()) != Token::get(";")){
			this->error(
				std::format(
					"Expected \";\" at end of variable declaration - got ({}) instead", Token::print_kind( this->reader.getKind(this->reader.peek()) )
				),
				this->reader.peek()
			);

		}else{
			this->reader.skip(1);
		}


		// create and return
		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t var_decl_index = uint32_t(this->var_decls.size());

		this->nodes.emplace_back(AST::Kind::VarDecl, var_decl_index);
		this->var_decls.emplace_back(is_public, is_static, decl_type, ident_result.value(), type, expr_result.value());

		return AST::NodeID{node_index};
	};



	auto Parser::parse_ident() noexcept -> Result {
		const TokenID first_token = this->reader.peek();

		if(this->reader.getKind(first_token) == Token::Ident){
			this->reader.skip(1);

		}else{
			return Result::WrongType;
		}


		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t ident_index = uint32_t(this->idents.size());

		this->nodes.emplace_back(AST::Kind::Ident, ident_index);
		this->idents.emplace_back(first_token);


		return AST::NodeID{node_index};
	};



	// TODO: finish parsing all types
	auto Parser::parse_type() noexcept -> Result {
		// IDENT
		// builtin_types
		// type *
		// type CONST *
		// type ?
		// type CONST // should be marked only once
		// [ type : expr ] // array
		// [ type : UNDERSCORE ] // array (infer length)
		// FUNC ( func_params ) ->   type                                     
		// FUNC ( func_params ) -> ( func_return_types )                      
		// FUNC ( func_params ) ->   type                < func_return_types >
		// FUNC ( func_params ) -> ( func_return_types ) < func_return_types >

		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t type_index = uint32_t(this->types.size());



		// ident
		if(this->reader.getKind(this->reader.peek()) == Token::Ident){
			this->nodes.emplace_back(AST::Kind::Type, type_index);
			this->types.emplace_back(AST::Type::Kind::Ident, AST::Ident{this->reader.next()});

			return AST::NodeID{node_index};
		}


		// builtin types
		switch(this->reader.getKind(this->reader.peek())){
			case Token::TypeVoid:
			case Token::TypeBool:
			case Token::TypeString:
			case Token::TypeChar:

			case Token::TypeInt:
			case Token::TypeIntN:
			case Token::TypeUInt:
			case Token::TypeUIntN:

			case Token::TypeF16:
			case Token::TypeF32:
			case Token::TypeF64:
			case Token::TypeF128:

			case Token::TypeUSize:
			case Token::TypeRawptr:
			case Token::TypeBool32:

			case Token::TypeCInt:
			case Token::TypeCUInt: {
				this->nodes.emplace_back(AST::Kind::Type, type_index);
				this->types.emplace_back(AST::Type::Kind::Builtin, this->reader.next());

				return AST::NodeID{node_index};
			}
		};


		return Result::WrongType;
	};




	///////////////////////////////////
	// expressions


	auto Parser::parse_expr() noexcept -> Result {
		if(this->reader.is_eof()){
			return Result::WrongType;
		}


		if(this->reader.getKind(this->reader.peek()) == Token::KeywordUninit){
			this->reader.skip(1);

			const uint32_t node_index = uint32_t(this->nodes.size());
			this->nodes.emplace_back(AST::Kind::Uninit);

			return AST::NodeID{node_index};
		}

		const Result infix_expr = this->parse_infix_expr();
		if(infix_expr.code() == Result::Success){
			return infix_expr;
		}else if(infix_expr.code() == Result::Error){
			return Result::Error;
		}

		return Result::WrongType;
	};




	constexpr int MAX_PREC = 9;
	EVO_NODISCARD static constexpr auto get_infix_op_precidence(Token::Kind kind) noexcept -> int {
		switch(kind){
			case Token::get("||"): return 1;

			case Token::get("&&"): return 2;

			case Token::get("|"): return 3;

			case Token::get("&"): return 4;

			case Token::get("=="): return 5;
			case Token::get("!="): return 5;

			case Token::get("<"): return 6;
			case Token::get("<="): return 6;
			case Token::get(">"): return 6;
			case Token::get(">="): return 6;

			case Token::get("<<"): return 7;
			case Token::get(">>"): return 7;

			case Token::get("+"): return 8;
			case Token::get("-"): return 8;

			case Token::get("*"): return 9;
			case Token::get("/"): return 9;
			case Token::get("%"): return 9;
		};

		return -1;
	};


	auto Parser::parse_infix_expr(int prec_level) noexcept -> Result {
		if(prec_level > MAX_PREC){
			return this->parse_prefix_expr();
		}


		const Result lhs_result = this->parse_infix_expr(prec_level + 1);
		switch(lhs_result.code()){
			case Result::Success: break;
			case Result::WrongType: return Result::WrongType;
			case Result::Error: return Result::Error;
			case Result::UnreportedError: return lhs_result; // don't want to report error here (although I'm not sure this will ever happen)
		};

		if(lhs_result.code() == Result::WrongType){ return lhs_result; }


		const TokenID op_token = this->reader.peek();
		if(get_infix_op_precidence(this->reader.getKind(op_token)) != prec_level){
			return lhs_result;
		}

		// skip op
		this->reader.skip(1);


		if(this->reader.is_eof()){
			this->error("Unexpected end of file in expression", this->reader.peek(-1));
			return Result::Error;
		}


		// get rhs
		const Result rhs_result = this->parse_infix_expr(prec_level);
		switch(rhs_result.code()){
			case Result::Success: break;
			case Result::WrongType: return Result::WrongType;
			case Result::Error: return Result::Error;
			case Result::UnreportedError: {
				this->error(
					std::format("Expected expression on right-hand side of ({}) operator", Token::print_kind(this->reader.getKind(op_token))), this->reader.peek()
				);
				return Result::Error;
			}
		};



		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t infix_index = uint32_t(this->infixes.size());

		this->nodes.emplace_back(AST::Kind::Infix, infix_index);
		this->infixes.emplace_back(lhs_result.value(), op_token, rhs_result.value());

		return AST::NodeID{node_index};
	};




	auto Parser::parse_prefix_expr() noexcept -> Result {
		// get prefix operation
		const TokenID op_token = this->reader.peek();
		bool is_postfix = false;
		switch(this->reader.getKind(op_token)){
			case Token::KeywordCopy:
			case Token::KeywordMove:
			case Token::KeywordAddr:
			case Token::get("-"):
				is_postfix = true;
		};

		if(is_postfix == false){ return this->parse_postfix_expr(); }

		// skip op
		this->reader.skip(1);


		// get rhs
		const Result rhs_result = this->parse_prefix_expr();
		if(rhs_result.code() == Result::WrongType){
			this->error(
				std::format("Expected expression on right-hand side of ({}) operator", Token::print_kind(this->reader.getKind(op_token))), this->reader.peek()
			);
			return Result::Error;
		}



		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t prefix_index = uint32_t(this->prefixes.size());

		this->nodes.emplace_back(AST::Kind::Prefix, prefix_index);
		this->prefixes.emplace_back(op_token, rhs_result.value());

		return AST::NodeID{node_index};
	};


	auto Parser::parse_postfix_expr() noexcept -> Result {
		// nothing at the moment
		return this->parse_paren_expr();		
	};

	auto Parser::parse_paren_expr() noexcept -> Result {
		if(this->reader.getKind(this->reader.peek()) != Token::get("(")){
			return this->parse_term();
		}

		// skip '('
		this->reader.skip(1);

		const Result expr_result = this->parse_expr();
		switch(expr_result.code()){
			case Result::Success: break;
			case Result::WrongType: return expr_result;
			case Result::Error: return expr_result;
			case Result::UnreportedError: return expr_result; // don't want to report error here (although I'm not sure this will ever happen)
		};


		if(this->reader.getKind(this->reader.peek()) != Token::get(")")){
			this->error("Expected closing parenthesis around expression", this->reader.peek());
			return Result::Error;
		}

		// skip ')'
		this->reader.skip(1);

		return expr_result;
	};





	// TODO: finish terms
	auto Parser::parse_term() noexcept -> Result {
		// literals
		const Result literal_result = this->parse_literal();
		if(literal_result.code() == Result::Success){ return literal_result; }

		// idents
		const Result ident_result = this->parse_ident();
		if(ident_result.code() == Result::Success){ return ident_result; }

		// types
		const Result type_result = this->parse_type();
		if(type_result.code() == Result::Success){ return type_result; }


		const TokenID first_token = this->reader.next();
		switch(this->reader.getKind(first_token)){
			case Token::KeywordNull: {
				const uint32_t node_index = uint32_t(this->nodes.size());
				this->nodes.emplace_back(AST::Kind::Null);
				return AST::NodeID{node_index};
			} break;

			case Token::KeywordThis: {
				const uint32_t node_index = uint32_t(this->nodes.size());
				this->nodes.emplace_back(AST::Kind::This);
				return AST::NodeID{node_index};
			} break;
		};

		this->reader.go_back(first_token);

		return Result::WrongType;
	};





	auto Parser::parse_literal() noexcept -> Result {
		const TokenID token = this->reader.next();

		bool is_literal = false;

		switch(this->reader.getKind(token)){
			case Token::LiteralBool:
			case Token::LiteralString:
			case Token::LiteralChar:
			case Token::LiteralInt:
			case Token::LiteralFloat:
				is_literal = true;
		};


		if(is_literal == false){
			this->reader.go_back(token);
			return Result::WrongType;
		}


		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t literal_index = uint32_t(this->literals.size());

		this->nodes.emplace_back(AST::Kind::Literal, literal_index);
		this->literals.emplace_back(token);

		return AST::NodeID{node_index};
	};






	///////////////////////////////////
	// errors etc.


	auto Parser::error(const std::string& message, TokenID token) const noexcept -> void {
		this->reader.error(message, this->reader.getLineStart(token), this->reader.getCollumnStart(token), this->reader.getLine(token), this->reader.getCollumn(token));
	};
	auto Parser::error_info(const std::string& message) const noexcept -> void {
		this->reader.error_info(message);
	};
	auto Parser::error_info(const std::string& message, TokenID token) const noexcept -> void {
		this->reader.error_info(message, this->reader.getLine(token), this->reader.getCollumn(token));
	};
	auto Parser::fatal(const std::string& message, TokenID token) const noexcept -> void {
		this->reader.fatal(message, this->reader.getLineStart(token), this->reader.getCollumnStart(token), this->reader.getLine(token), this->reader.getCollumn(token));
	};





};
