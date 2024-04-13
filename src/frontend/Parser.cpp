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

		result = this->parse_conditional();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_return();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_assignment();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_unreachable();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		// meant for things like function calls (make sure to check in semantic ananlysis that there actually are side effects)
		result = this->parse_expr();
		if(result.code() == Result::Success){
			// ;
			if(this->expect_token(Token::get(";"), "at end of variable declaration") == false){ return Result::Error; }
			
			return result;

		}else if(result.code() == Result::Error){
			return Result::Error;
		}


		return Result::WrongType;
	};



	auto Parser::parse_var_decl() noexcept -> Result {
		const Token& first_token = this->get(this->peek());

		// keyword var
		if(first_token.kind == Token::KeywordVar){
			this->skip(1);
		}else{
			return Result::WrongType;
		}


		// ident
		const Result ident = this->parse_ident();
		if(this->check_result_fail(ident, "identifier in variable declaration")){ return Result::Error; }

		// :
		if(this->expect_token(Token::get(":"), "in variable declaration") == false){ return Result::Error; }

		// type
		const Result type = this->parse_type();
		if(this->check_result_fail(type, "type in variable declaration")){ return Result::Error; }

		// =
		if(this->expect_token(Token::get("="), "in variable declaration") == false){ return Result::Error; }

		// expr
		const Result expr = this->parse_expr();
		if(this->check_result_fail(expr, "expr in variable declaration")){ return Result::Error; }

		// ;
		if(this->expect_token(Token::get(";"), "at end of variable declaration") == false){ return Result::Error; }


		return this->create_node(
			this->source.var_decls, AST::Kind::VarDecl,
			ident.value(), type.value(), expr.value()
		);
	};



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

		// (
		if(this->expect_token(Token::get("("), "in function declaration") == false){ return Result::Error; }

		// )
		if(this->expect_token(Token::get(")"), "in function declaration") == false){ return Result::Error; }

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
			ident.value(), return_type.value(), block.value(), std::move(attributes)
		);
	};




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



	auto Parser::parse_type() noexcept -> Result {
		switch(this->get(this->peek()).kind){
			case Token::TypeVoid:
			case Token::TypeInt:
			case Token::TypeBool:
			// case Token::Ident: 
				break;

			default:
				return Result::WrongType;
		};


		const Token::ID base_type = this->next();


		auto qualifiers = std::vector<AST::Type::Qualifier>();
		while(this->get(this->peek()).kind == Token::get("^")){
			qualifiers.emplace_back(true);
			this->skip(1);
		};


		return this->create_node(
			this->source.types, AST::Kind::Type,
			base_type, std::move(qualifiers)
		);
	};



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



	auto Parser::parse_infix_expr() noexcept -> Result {
		const Result lhs_result = this->parse_prefix_expr();
		if(lhs_result.code() == Result::WrongType || lhs_result.code() == Result::Error){ return lhs_result; }

		return this->parse_infix_expr_impl(lhs_result.value(), 1);
	};



	EVO_NODISCARD static constexpr auto get_infix_op_precedence(Token::Kind kind) noexcept -> int {
		switch(kind){
			// This warns at the moment, but the fix will come with operators
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

		const Result next_term = this->parse_prefix_expr();
		if(next_term.code() == Result::WrongType || next_term.code() == Result::Error){ return next_term; }

		const Result rhs_result = this->parse_infix_expr_impl(next_term.value(), next_op_prec);
		if(next_term.code() == Result::WrongType || next_term.code() == Result::Error){ return next_term; }


		return this->create_node(
			this->source.infixes, AST::Kind::Infix,
			lhs, peeked_op, rhs_result.value()
		);
	};



	auto Parser::parse_prefix_expr() noexcept -> Result {
		// get prefix operation
		const Token::ID op_token = this->peek();
		switch(this->get(op_token).kind){
			case Token::KeywordCopy:
			case Token::KeywordAddr:
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
			this->expected_but_got(std::format("expression on right-hand size of [{}] operator", Token::printKind(this->get(op_token).kind)));
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
		return this->parse_accessor_expr();
	};



	auto Parser::parse_accessor_expr() noexcept -> Result {
		Result output = this->parse_paren_expr();
		if(output.code() == Result::WrongType || output.code() == Result::Error){ return output; }


		while(true){
			if(this->get(this->peek()).kind == Token::get(".^")){
				const Token::ID op_token = this->next();

				output = this->create_node(this->source.postfixes, AST::Kind::Postfix,
					output.value(), op_token
				);

			}else if(this->get(this->peek()).kind == Token::get("(")){
				this->skip(1);

				// )
				if(this->expect_token(Token::get(")"), "at end of function call") == false){ return Result::Error; }

				output = this->create_node(this->source.func_calls, AST::Kind::FuncCall,
					output.value()
				);

			}else{
				break;
			}
		};

		return output;
	};



	auto Parser::parse_paren_expr() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::get("(")){
			return this->parse_term();
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



	auto Parser::parse_term() noexcept -> Result {
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
