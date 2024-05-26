#include "./Parser.h"


namespace panther{


	auto Parser::parse() noexcept -> bool {
		while(this->eof() == false){
			const Result stmt_res = this->parse_stmt();

			switch(stmt_res.code()){
				case Result::Success: {
					this->source.global_stmts.push_back(stmt_res.value());
					continue;
				} break;

				case Result::WrongType: {
					this->source.error("Unknown beginning to statement or unknown statement type", this->peek());
					return false;
				} break;

				case Result::Error: return false;
			};
		};

		return this->source.hasErrored() == false;
	};



	auto Parser::parse_stmt() noexcept -> Result {
		Result result;

		result = this->parse_var_decl();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_func();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_struct();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_conditional();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_return();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_assignment();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_unreachable();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_alias();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		// meant for things like function calls (make sure to check in semantic ananlysis that there actually are side effects)
		result = this->parse_expr();
		if(result.code() == Result::Success){
			// ;
			if(this->expect_token(Token::get(";"), "at end of expression statement") == false){ return Result::Error; }
			
			return result;

		}else if(result.code() == Result::Error){
			return Result::Error;
		}


		return Result::WrongType;
	};


	// TODO: add checking for EOF
	auto Parser::parse_var_decl() noexcept -> Result {
		const Token& first_token = this->get(this->peek());

		bool is_def = false;

		// keyword var
		if(first_token.kind == Token::KeywordVar){
			this->skip(1);

		}else if(first_token.kind == Token::KeywordDef){
			this->skip(1);
			is_def = true;

		}else{
			return Result::WrongType;
		}


		// ident
		const Result ident = this->parse_ident();
		if(this->check_result_fail(ident, "identifier in variable declaration")){ return Result::Error; }

		// attributes
		auto attributes = std::vector<Token::ID>();
		while(this->get(this->peek()).kind == Token::Attribute){
			attributes.emplace_back(this->next());
		};


		// type
		auto type = std::optional<AST::Node::ID>();
		if(this->get(this->peek()).kind == Token::get(":")){
			this->skip(1);

			const Result type_result = this->parse_type();
			if(this->check_result_fail(type_result, "type in variable declaration after \":\"")){ return Result::Error; }

			type = type_result.value();
		}

		// no value given
		if(this->get(this->peek()).kind == Token::get(";")){
			this->skip(1);
			
			return this->create_node(
				this->source.var_decls, AST::Kind::VarDecl,
				is_def, ident.value(), std::move(attributes), type, std::nullopt
			);			
		}


		// =
		if(this->expect_token(Token::get("="), "in variable declaration") == false){ return Result::Error; }

		// expr
		const Result expr = this->parse_expr();
		if(this->check_result_fail(expr, "expr in variable declaration")){ return Result::Error; }

		// ;
		if(this->expect_token(Token::get(";"), "at end of variable declaration") == false){ return Result::Error; }


		return this->create_node(
			this->source.var_decls, AST::Kind::VarDecl,
			is_def, ident.value(), std::move(attributes), type, expr.value()
		);
	};


	// TODO: add checking for EOF
	auto Parser::parse_func() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::KeywordFunc){
			return Result::WrongType;
		}

		this->skip(1);


		// ident
		const Result ident = this->parse_ident();
		if(this->check_result_fail(ident, "identifier in function declaration")){ return Result::Error; }

		// =
		if(this->expect_token(Token::get("="), "in function declaration") == false){ return Result::Error; }
	
		// template pack
		const Result template_pack = this->parse_template_pack();
		if(template_pack.code() == Result::Error){ return Result::Error; }

		const std::optional<AST::Node::ID> template_pack_value = [&]() noexcept {
			if(template_pack.code() == Result::Success){
				return std::optional<AST::Node::ID>(template_pack.value());
			}else{
				return std::optional<AST::Node::ID>(std::nullopt);
			}
		}();


		const Result func_params = this->parse_func_params();
		if(this->check_result_fail(func_params, "parameter block in function declaration")){ return Result::Error; }


		// attributes
		auto attributes = std::vector<Token::ID>();
		while(this->get(this->peek()).kind == Token::Attribute){
			attributes.emplace_back(this->next());
		};


		// ->
		if(this->expect_token(Token::get("->"), "in function declaration") == false){ return Result::Error; }

		// return type
		const Result return_type = this->parse_type();
		if(this->check_result_fail(return_type, "return type in function declaration")){ return Result::Error; }

		
		const Result block = this->parse_block();
		if(this->check_result_fail(block, "statement block in function declaration")){ return Result::Error; }


		return this->create_node(
			this->source.funcs, AST::Kind::Func,
			ident.value(), template_pack_value, func_params.value(), std::move(attributes), return_type.value(), block.value()
		);
	};


	// TODO: add checking for EOF
	auto Parser::parse_struct() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::KeywordStruct){ return Result::WrongType; }
		this->skip(1);

		// ident
		const Result ident = this->parse_ident();
		if(this->check_result_fail(ident, "ident in struct declaration")){ return Result::Error; }

		// =
		if(this->expect_token(Token::get("="), "in struct declaration") == false){ return Result::Error; }

		// template pack
		const Result template_pack = this->parse_template_pack();
		if(template_pack.code() == Result::Error){ return Result::Error; }

		const std::optional<AST::Node::ID> template_pack_value = [&]() noexcept {
			if(template_pack.code() == Result::Success){
				return std::optional<AST::Node::ID>(template_pack.value());
			}else{
				return std::optional<AST::Node::ID>(std::nullopt);
			}
		}();

		// attributes
		auto attributes = std::vector<Token::ID>();
		while(this->get(this->peek()).kind == Token::Attribute){
			attributes.emplace_back(this->next());
		};

		const Result block = this->parse_block();
		if(this->check_result_fail(block, "statement block in struct declaration")){ return Result::Error; }

		return this->create_node(
			this->source.structs, AST::Kind::Struct,
			ident.value(), template_pack_value, std::move(attributes), block.value()
		);
	};


	// TODO: add checking for EOF
	auto Parser::parse_template_pack() noexcept -> Result {
		// |
		if(this->get(this->peek()).kind != Token::get("<{")){ return Result::WrongType; }
		const Token::ID start_location = this->next();


		auto templates = std::vector<AST::TemplatePack::Template>();

		while(true){
			if(this->get(this->peek()).kind == Token::get("}>")){
				this->skip(1);
				break;
			}

			// ident
			const Result ident_result = this->parse_ident();
			if(this->check_result_fail(ident_result, "identifier in template parameter")){ return Result::Error; }

			// :
			if(this->expect_token(Token::get(":"), "in template parameter") == false){ return Result::Error; }


			// type
			if(this->get(this->peek()).kind == Token::KeywordType){
				templates.emplace_back(ident_result.value(), this->next());

			}else{
				const Result type_result = this->parse_type();
				if(this->check_result_fail(type_result, "type in template parameter")){ return Result::Error; }


				templates.emplace_back(ident_result.value(), type_result.value());
			}


			

			// check if ending or should continue
			const Token::ID after_param_peek = this->next();
			const Token& after_param_peek_tok = this->get(after_param_peek);
			if(after_param_peek_tok.kind != Token::get(",")){
				if(after_param_peek_tok.kind != Token::get("}>")){
					this->expected_but_got("\",\" at end of template parameter or \"}>\" at end of template pack block", after_param_peek);
					return Result::Error;
				}

				break;
			}
		};


		return this->create_node(
			this->source.template_packs, AST::Kind::TemplatePack,
			start_location, std::move(templates)
		);
	};


	// TODO: add checking for EOF
	auto Parser::parse_func_params() noexcept -> Result {
		// (
		if(this->get(this->peek()).kind != Token::get("(")){ return Result::WrongType; }
		const Token::ID start_location = this->next();


		auto params = std::vector<AST::FuncParams::Param>();

		while(true){
			if(this->get(this->peek()).kind == Token::get(")")){
				this->skip(1);
				break;
			}

			// ident
			const Result ident_result = this->parse_ident();
			if(this->check_result_fail(ident_result, "identifier in function parameter")){ return Result::Error; }

			// :
			if(this->expect_token(Token::get(":"), "in function parameter") == false){ return Result::Error; }

			// type
			const Result type_result = this->parse_type();
			if(this->check_result_fail(type_result, "type in function parameter")){ return Result::Error; }

			// kind
			using ParamKind = AST::FuncParams::Param::Kind;
			const ParamKind param_kind = [&]() noexcept {
				switch(this->get(this->peek()).kind){
					case Token::KeywordRead: {
						this->skip(1);
						return ParamKind::Read;
					} break;

					case Token::KeywordWrite: {
						this->skip(1);
						return ParamKind::Write;
					} break;

					case Token::KeywordIn: {
						this->skip(1);
						return ParamKind::In;
					} break;

					default: return ParamKind::Read;
				};
			}();

			if(param_kind == ParamKind::In){
				this->source.error("The parameter qualifier \"in\" is not supported yet", this->peek(-1));
				return Result::Error;
			}



			// create param
			params.emplace_back(ident_result.value(), type_result.value(), param_kind);

			// check if ending or should continue
			const Token::ID after_param_peek = this->next();
			const Token& after_param_peek_tok = this->get(after_param_peek);
			if(after_param_peek_tok.kind != Token::get(",")){
				if(after_param_peek_tok.kind != Token::get(")")){
					this->expected_but_got("\",\" at end of function parameter or \")\" at end of function parameters block", after_param_peek);
					return Result::Error;
				}

				break;
			}
		};


		return this->create_node(
			this->source.func_params, AST::Kind::FuncParams,
			start_location, std::move(params)
		);
	};



	// TODO: add checking for EOF
	auto Parser::parse_conditional() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::KeywordIf){
			return Result::WrongType;
		}

		const Token::ID keyword_if_tok = this->next();

		// (
		if(this->expect_token(Token::get("("), "in function declaration") == false){ return Result::Error; }

		// conditional
		const Result cond_expr = this->parse_expr();
		if(this->check_result_fail(cond_expr, "expression in conditional")){ return Result::Error; }

		// )
		if(this->expect_token(Token::get(")"), "in function declaration") == false){ return Result::Error; }

		// then block
		const Result then_block = this->parse_block();
		if(this->check_result_fail(then_block, "statement block in conditional")){ return Result::Error; }


		auto else_stmt = std::optional<AST::Node::ID>();
		if(this->get(this->peek()).kind == Token::KeywordElse){
			this->skip(1);

			const Result else_stmt_result = [&]() noexcept {
				if(this->get(this->peek()).kind == Token::KeywordIf){
					return this->parse_conditional();
				}else{
					return this->parse_block();
				}
			}();

			// TODO: better message
			if(this->check_result_fail(else_stmt_result, "conditional")){ return Result::Error; }

			else_stmt = else_stmt_result.value();
		}



		return this->create_node(
			this->source.conditionals, AST::Kind::Conditional,
			keyword_if_tok, cond_expr.value(), then_block.value(), else_stmt
		);
	};




	// TODO: add checking for EOF
	auto Parser::parse_return() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::KeywordReturn){
			return Result::WrongType;
		}

		const Token::ID keyword_ret_tok = this->next();

		std::optional<AST::Node::ID> value = std::nullopt;


		if(this->get(this->peek()).kind == Token::get(";")){
			this->skip(1);
		}else{
			const Result value_result = this->parse_expr();
			// TODO: better messaging
			if(this->check_result_fail(value_result, "expression in return statement")){ return Result::Error; }

			value = value_result.value();
			
			// ;
			if(this->expect_token(Token::get(";"), "at end of return statement") == false){ return Result::Error; }
		}



		return this->create_node(
			this->source.returns, AST::Kind::Return,
			keyword_ret_tok, value
		);
	};


	// TODO: add checking for EOF
	auto Parser::parse_assignment() noexcept -> Result {
		const Token::ID start = this->peek();

		// ident
		const Result lhs = this->parse_expr();
		if(lhs.code() == Result::Error){
			return Result::Error;

		}else if(lhs.code() == Result::WrongType){
			return Result::WrongType;
		}


		// op
		const Token::ID op = this->next();

		switch(this->get(op).kind){
			case Token::get("="): break;

			default:
				this->go_back(start);
				return Result::WrongType;
		};


		// expr
		const Result expr = this->parse_expr();
		if(this->check_result_fail(expr, "Expression value in assignment")){ return Result::Error; }

		// ;
		if(this->expect_token(Token::get(";"), "at end of assignment") == false){ return Result::Error; }


		return this->create_node(
			this->source.infixes, AST::Kind::Infix,
			lhs.value(), op, expr.value()
		);
	};



	auto Parser::parse_unreachable() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::KeywordUnreachable){ return Result::WrongType; }

		const Token::ID tok = this->next();

		// ;
		if(this->expect_token(Token::get(";"), "at end of \"unreachable\" statement") == false){ return Result::Error; }

		return this->create_token_node(AST::Kind::Unreachable, tok);
	};


	auto Parser::parse_alias() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::KeywordAlias){ return Result::WrongType; };
		this->skip(1);

		// ident
		const Result ident = this->parse_ident();
		if(this->check_result_fail(ident, "identifier in alias")){ return Result::Error; }

		// attributes
		auto attributes = std::vector<Token::ID>();
		while(this->get(this->peek()).kind == Token::Attribute){
			attributes.emplace_back(this->next());
		};

		// =
		if(this->expect_token(Token::get("="), "in alias") == false){ return Result::Error; }
		
		const Result type = this->parse_type();
		if(this->check_result_fail(type, "type in alias")){ return Result::Error; }		

		// ;
		if(this->expect_token(Token::get(";"), "at end of assignment") == false){ return Result::Error; }

		return this->create_node(
			this->source.aliases, AST::Kind::Alias,
			ident.value(), std::move(attributes), type.value()
		);
	};





	// TODO: add checking for EOF
	auto Parser::parse_type() noexcept -> Result {
		bool is_builtin = true;
		switch(this->get(this->peek()).kind){
			case Token::TypeVoid:
			case Token::TypeInt:
			case Token::TypeUInt:
			case Token::TypeBool:
			case Token::TypeString:
			case Token::TypeISize:
			case Token::TypeUSize:
				break;

			case Token::Ident:
				is_builtin = false;
				break;

			default:
				return Result::WrongType;
		};


		const evo::Result<AST::Type::Base> base_type = [&]() noexcept {
			if(is_builtin){
				return evo::Result<AST::Type::Base>(AST::Type::Base(this->next()));
			}else{
				const Result accessor_expr = this->parse_term(true);
				if(accessor_expr.code() != Result::Success){ return evo::Result<AST::Type::Base>(evo::resultError); }
				return evo::Result<AST::Type::Base>( AST::Type::Base{ .node = accessor_expr.value() } );
			}
		}();

		if(base_type.isError()){ return Result::Error; }





		auto qualifiers = std::vector<AST::Type::Qualifier>();
		while(this->get(this->peek()).kind == Token::get("&")){
			this->skip(1);

			bool is_const = false;
			if(this->get(this->peek()).kind == Token::get("|")){
				is_const = true;
				this->skip(1);
			}

			qualifiers.emplace_back(true, is_const);
		};


		return this->create_node(
			this->source.types, AST::Kind::Type,
			is_builtin, base_type.value(), std::move(qualifiers)
		);
	};


	// TODO: add checking for EOF
	auto Parser::parse_block() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::get("{")){
			return Result::WrongType;
		}

		this->skip(1);

		auto statements = std::vector<AST::Node::ID>();



		while(true){
			if(this->get(this->peek()).kind == Token::get("}")){
				this->skip(1);
				break;
			}

			const Result stmt = this->parse_stmt();
			if(this->check_result_fail(stmt, "statement in statement block")){ return Result::Error; }

			statements.push_back(stmt.value());
		};



		return this->create_node(
			this->source.blocks, AST::Kind::Block,
			std::move(statements)
		);
	};




	auto Parser::parse_uninit() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::KeywordUninit){
			return Result::WrongType;
		}

		return this->create_token_node(AST::Kind::Uninit, this->next());	
	};

	
	auto Parser::parse_expr() noexcept -> Result {
		Result result = this->parse_uninit();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }


		return this->parse_infix_expr();
	};


	// TODO: add checking for EOF
	auto Parser::parse_infix_expr() noexcept -> Result {
		const Result lhs_result = this->parse_prefix_expr();
		if(lhs_result.code() == Result::WrongType || lhs_result.code() == Result::Error){ return lhs_result; }

		return this->parse_infix_expr_impl(lhs_result.value(), 0);
	};



	EVO_NODISCARD static constexpr auto get_infix_op_precedence(Token::Kind kind) noexcept -> int {
		switch(kind){
			case Token::KeywordAnd:  return 1;
			case Token::KeywordOr:   return 1;

			case Token::get("=="):   return 3;
			case Token::get("!="):   return 3;
			case Token::get("<"):    return 3;
			case Token::get("<="):   return 3;
			case Token::get(">"):    return 3;
			case Token::get(">="):   return 3;

			case Token::get("+"):    return 5;
			case Token::get("+@"):   return 5;
			case Token::get("-"):    return 5;
			case Token::get("-@"):   return 5;

			case Token::get("*"):    return 6;
			case Token::get("*@"):   return 6;
			case Token::get("/"):    return 6;

			case Token::KeywordAs:	 return 7;
		};

		return -1;
	};

	auto Parser::parse_infix_expr_impl(AST::Node::ID lhs, int prec_level) noexcept -> Result {
		const Token::ID peeked_op = this->peek();
		const Token::Kind peeked_kind = this->get(peeked_op).kind;

		const int next_op_prec = get_infix_op_precedence(peeked_kind);

		// if not an infix operator or is same or lower precedence
		// 		next_op_prec == -1 if its not an infix op
		//   	< to maintain operator precedence
		// 		<= to prevent `a + b + c` from being parsed as `a + (b + c)`
		if(next_op_prec <= prec_level){ return lhs; }

		// skip operator
		this->skip(1);

		const Result rhs_result = [&]() noexcept {
			if(peeked_kind == Token::KeywordAs){
				return this->parse_type();
			}


			const Result next_term = this->parse_prefix_expr();
			if(next_term.code() == Result::WrongType || next_term.code() == Result::Error){ return next_term; }

			return this->parse_infix_expr_impl(next_term.value(), next_op_prec);
		}();

		if(rhs_result.code() == Result::WrongType || rhs_result.code() == Result::Error){ return rhs_result; }


		const AST::Node::ID created_infix_expr = this->create_node(
			this->source.infixes, AST::Kind::Infix,
			lhs, peeked_op, rhs_result.value()
		);

		return this->parse_infix_expr_impl(created_infix_expr, prec_level);
	};



	auto Parser::parse_prefix_expr() noexcept -> Result {
		// get prefix operation
		const Token::ID op_token = this->peek();
		switch(this->get(op_token).kind){
			case Token::KeywordCopy:
			case Token::KeywordAddr:
			case Token::get("-"):
			case Token::get("!"):
				break;

			default:
				return this->parse_postfix_expr();
		};

		this->skip(1);

		// get rhs
		const Result rhs = this->parse_postfix_expr();
		if(rhs.code() == Result::Error){
			return Result::Error;

		}else if(rhs.code() == Result::WrongType){
			this->expected_but_got(std::format("valid expression on right-hand size of [{}] operator", Token::printKind(this->get(op_token).kind)));
			return Result::Error;
		}


		return this->create_node(
			this->source.prefixes, AST::Kind::Prefix,
			op_token, rhs.value()
		);
	};


	// TODO: remove me?
	auto Parser::parse_postfix_expr() noexcept -> Result {
		// nothing at the moment
		return this->parse_term();
	};


	// TODO: check for EOF
	auto Parser::parse_term(bool is_type_term) noexcept -> Result {
		Result output = this->parse_paren_expr();
		if(output.code() == Result::WrongType || output.code() == Result::Error){ return output; }


		while(true){
			const Token::Kind peeked_kind = this->get(this->peek()).kind;



			if(peeked_kind == Token::get(".")){
				const Token::ID accessor_op_token = this->next();

				const Result rhs_result = this->parse_ident();
				if(this->check_result_fail(rhs_result, "identifier on right-hand side of accessor operator (\".\")")){ return Result::Error; }

				output = this->create_node(this->source.infixes, AST::Kind::Infix,
					output.value(), accessor_op_token, rhs_result.value()
				);

				continue;
				
			}else if(peeked_kind == Token::get(".&") && !is_type_term){
				const Token::ID op_token = this->next();

				output = this->create_node(this->source.postfixes, AST::Kind::Postfix,
					output.value(), op_token
				);

				continue;

			}else if(peeked_kind == Token::get("(") && !is_type_term){
				this->skip(1);

				auto arguments = std::vector<AST::Node::ID>();

				while(true){
					if(this->get(this->peek()).kind == Token::get(")")){
						this->skip(1);
						break;
					}

					const Result expr_result = this->parse_expr();
					if(this->check_result_fail(expr_result, "expression inside function call")){
						return Result::Error;
					}

					arguments.emplace_back(expr_result.value());

					const Token::ID after_param_peek_tok = this->next();
					const Token& after_param_peek = this->get(after_param_peek_tok);
					if(after_param_peek.kind != Token::get(",")){
						if(after_param_peek.kind != Token::get(")")){
							this->expected_but_got("\",\" at end of function call argument or \")\" at end of function call", after_param_peek_tok);
							return Result::Error;
						}

						break;
					}
				};

				output = this->create_node(this->source.func_calls, AST::Kind::FuncCall,
					output.value(), std::move(arguments)
				);

			}else if(peeked_kind == Token::get("<{")){
				this->skip(1);

				auto template_args = std::vector<AST::Node::ID>();
				while(true){
					if(this->get(this->peek()).kind == Token::get("}>")){
						this->skip(1);
						break;
					}


					const Result type_result = this->parse_type();
					switch(type_result.code()){
						case Result::Success: {
							template_args.emplace_back(type_result.value());
						} break;

						case Result::WrongType: {
							const Result expr_result = this->parse_expr();
							if(this->check_result_fail(expr_result, "expression inside template")){ return Result::Error; }

							template_args.emplace_back(expr_result.value());
						} break;

						case Result::Error: {
							return Result::Error;
						} break;
					};


					const Token::ID after_param_peek_tok = this->next();
					const Token& after_param_peek = this->get(after_param_peek_tok);
					if(after_param_peek.kind != Token::get(",")){
						if(after_param_peek.kind != Token::get("}>")){
							this->expected_but_got("\",\" at end of template argument or \"}>\" at end of template pack", after_param_peek_tok);
							return Result::Error;
						}

						break;
					}
				};


				output = this->create_node(this->source.templated_exprs, AST::Kind::TemplatedExpr,
					output.value(), std::move(template_args)
				);

				continue;

			}else if(peeked_kind == Token::get("{") && !is_type_term){
				this->skip(1);

				auto members = std::vector<AST::Initializer::Member>();
				while(true){
					if(this->get(this->peek()).kind == Token::get("}")){
						this->skip(1);
						break;
					}

					const Result ident = this->parse_ident();
					if(this->check_result_fail(ident, "identifier in member initializer")){ return Result::Error; }

					// =
					if(this->expect_token(Token::get("="), "in member initializer") == false){ return Result::Error; }

					const Result expr = this->parse_expr();
					if(this->check_result_fail(expr, "expression in member initializer")){ return Result::Error; }

					members.emplace_back(ident.value(), expr.value());

					const Token::ID after_param_peek_tok = this->next();
					const Token& after_param_peek = this->get(after_param_peek_tok);
					if(after_param_peek.kind != Token::get(",")){
						if(after_param_peek.kind != Token::get("}")){
							this->expected_but_got("\",\" at end of member initalizer or \"}\" at end of struct initializer", after_param_peek_tok);
							return Result::Error;
						}

						break;
					}
				};


				output = this->create_node(this->source.initializers, AST::Kind::Initializer,
					output.value(), std::move(members)
				);

			}else{
				break;
			}
		};

		return output;
	};


	// TODO: add checking for EOF
	auto Parser::parse_paren_expr() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::get("(")){
			return this->parse_atom();
		}

		const Token::ID open_location = this->next();

		const Result expr_result = this->parse_expr();
		if(expr_result.code() == Result::WrongType || expr_result.code() == Result::Error){ return expr_result; }


		if(this->get(this->peek()).kind != Token::get(")")){
			const Token& open_location_token = this->source.getToken(open_location);

			this->expected_but_got("either closing parenthesis around expression or continuation of expression", 
				std::vector<Message::Info>{ {"parenthesis opened here", open_location_token.location}, }
			);
			return Result::Error;
		}

		// skip ')'
		this->skip(1);

		return expr_result;
	};



	auto Parser::parse_atom() noexcept -> Result {
		Result result = this->parse_literal();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_ident();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_intrinsic();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }


		return Result::WrongType;
	};





	auto Parser::parse_literal() noexcept -> Result {
		switch(this->get(this->peek()).kind){
			case Token::LiteralBool:
			case Token::LiteralInt:
			case Token::LiteralFloat:
			case Token::LiteralString:
				break;

			default:
				return Result::WrongType;
		};

		return this->create_token_node(AST::Kind::Literal, this->next());
	};



	auto Parser::parse_ident() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::Ident){
			return Result::WrongType;
		}

		return this->create_token_node(AST::Kind::Ident, this->next());
	};

	auto Parser::parse_intrinsic() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::Intrinsic){
			return Result::WrongType;
		}

		return this->create_token_node(AST::Kind::Intrinsic, this->next());
	};






	//////////////////////////////////////////////////////////////////////
	// messaging

	auto Parser::expect_token(Token::Kind kind, evo::CStrProxy location) noexcept -> bool {
		if(this->get(this->next()).kind != kind){
			this->expected_but_got(std::format("\"{}\" {}", Token::printKind(kind), location), this->peek(-1));
			return false;
		}

		return true;
	};


	auto Parser::expected_but_got(evo::CStrProxy msg, Token::ID token_id, std::vector<Message::Info>&& infos) const noexcept -> void {
		const Token& token = this->get(token_id);
		this->source.error(std::format("Expected {} - got [{}] instead.", msg.data(), Token::printKind(token.kind)), token_id, std::move(infos));
	};




	auto Parser::check_result_fail(const Result& result, evo::CStrProxy msg) const noexcept -> bool {
		if(result.code() == Result::Error){ return true; }
		else if(result.code() == Result::WrongType){
			this->expected_but_got(msg);
			return true;
		}

		return false;
	};


	
};
