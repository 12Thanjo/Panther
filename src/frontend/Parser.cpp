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

		result = this->parse_return();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_assignment();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_func_call();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

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



	auto Parser::parse_return() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::KeywordReturn){
			return Result::WrongType;
		}

		const Token::ID keyword = this->next();

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
			keyword, value
		);
	};



	auto Parser::parse_assignment() noexcept -> Result {
		const Token::ID start = this->peek();

		// ident
		const Result ident = this->parse_ident();
		if(ident.code() == Result::Error){
			return Result::Error;

		}else if(ident.code() == Result::WrongType){
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
			ident.value(), op, expr.value()
		);
	};




	auto Parser::parse_func_call() noexcept -> Result {

		return Result::WrongType;
	};





	auto Parser::parse_expr() noexcept -> Result {
		Result result = this->parse_literal();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_ident();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }

		result = this->parse_uninit();
		if(result.code() == Result::Success || result.code() == Result::Error){ return result; }


		return Result::WrongType;
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



		return this->create_node(
			this->source.types, AST::Kind::Type,
			base_type
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


	auto Parser::parse_uninit() noexcept -> Result {
		if(this->get(this->peek()).kind != Token::KeywordUninit){
			return Result::WrongType;
		}

		return this->create_token_node(AST::Kind::Uninit, this->next());	
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


	auto Parser::expected_but_got(evo::CStrProxy msg, Token::ID token_id) const noexcept -> void {
		const Token& token = this->get(token_id);
		this->source.error(std::format("Expected {} - got [{}] instead.", msg.data(), Token::printKind(token.kind)), token_id);
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
