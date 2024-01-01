#include "frontend/Parser.h"


namespace panther{
	
	auto Parser::parse() noexcept -> bool {
		while(this->reader.is_eof() == false && this->reader.get_source_manager().errored() == false){
			const Result stmt_result = this->parse_stmt();

			switch(stmt_result.code()){
				case Result::Success: {
					this->top_level_statements.emplace_back(stmt_result.value());
					continue;
				} break;

				case Result::WrongType: {
					this->error("Failed to parse statement - could not find any matching statement type", this->reader.peek());
					this->error_info("Above is pointing to the beginning of the statement");
					return false;
				} break;

				case Result::Error: {
					return false;
				} break;

			};

		};

		return this->reader.get_source_manager().errored() == false;
	};




	auto Parser::parse_stmt() noexcept -> Result {
		// var_decl
		{
			const auto result = this->parse_var_decl();
			switch(result.code()){
				case Result::Success: return result;
				case Result::WrongType: break;
				case Result::Error: return Result::Error;
			};
		}


		// func_def
		{
			const auto result = this->parse_func_def();
			switch(result.code()){
				case Result::Success: return result;
				case Result::WrongType: break;
				case Result::Error: return Result::Error;
			};
		}

		// multiple_assignment
		{
			const auto result = this->parse_multiple_assignment();
			switch(result.code()){
				case Result::Success: return result;
				case Result::WrongType: break;
				case Result::Error: return Result::Error;
			};
		}

		// assignment
		{
			const auto result = this->parse_assignment();
			switch(result.code()){
				case Result::Success: return result;
				case Result::WrongType: break;
				case Result::Error: return Result::Error;
			};
		}

		// conditional
		{
			const auto result = this->parse_conditional();
			switch(result.code()){
				case Result::Success: return result;
				case Result::WrongType: break;
				case Result::Error: return Result::Error;
			};
		}

		// TODO: iteration
		// TODO: try_catch

		// while
		{
			const auto result = this->parse_while();
			switch(result.code()){
				case Result::Success: return result;
				case Result::WrongType: break;
				case Result::Error: return Result::Error;
			};
		}

		// TODO: do_while
		// TODO: typedef_stmt
		// TODO: alias_stmt

		// return / throw
		{
			const auto result = this->parse_return();
			switch(result.code()){
				case Result::Success: return result;
				case Result::WrongType: break;
				case Result::Error: return Result::Error;
			};
		}


		// TODO: defer_stmt
		// TODO: struct_def

		// block
		{
			const auto result = this->parse_block();
			switch(result.code()){
				case Result::Success: return result;
				case Result::WrongType: break;
				case Result::Error: return Result::Error;
			};
		}


		// meant for things like function calls (make sure to check in semantic ananlysis that there actually are side effects)
		// expr
		{
			const auto result = this->parse_expr();
			switch(result.code()){
				case Result::Success: {
					if(this->reader.getKind(this->reader.next()) != Token::get(";")){
						this->expected_but_got("semicolon at end of expression statement", this->reader.peek(-1));
						return Result::Error;
					}

					return result;
				} break;

				case Result::WrongType: break;
				case Result::Error: return Result::Error;
			};
		}

		
		return Result::WrongType;
	};






	auto Parser::parse_var_decl() noexcept -> Result {
		const TokenID first_token = this->reader.peek();

		// check public
		bool is_public = false;
		if(this->reader.getKind(first_token) == Token::Kind::KeywordPub){
			is_public = true;
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->unexpected_eof("variable declaration");
				return Result::Error;
			}
		}


		// check static
		bool is_static = false;
		if(this->reader.getKind(this->reader.peek()) == Token::Kind::KeywordStatic){
			is_static = true;
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->unexpected_eof("variable declaration");
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
			this->unexpected_eof("variable declaration");
			return Result::Error;
		}


		// identifier
		const Result ident_result = this->parse_ident();
		if(ident_result.code() == Result::WrongType){
			this->reader.go_back(first_token);
			return Result::WrongType;
		}

		if(this->reader.is_eof()){
			this->unexpected_eof("variable declaration");
			return Result::Error;
		}


		// : type (maybe)
		auto type = std::optional<AST::NodeID>{};
		if(this->reader.getKind(this->reader.peek()) == Token::get(":")){
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->unexpected_eof("variable declaration");
				return Result::Error;
			}

			const Result type_result = this->parse_type();

			switch(type_result.code()){
				case Result::Success: {
					type = type_result.value();
				} break;

				case Result::WrongType: {
					this->expected_but_got("type in variable declaration");
					return Result::Error;
				} break;

				case Result::Error: {
					return Result::Error;
				} break;
			};
			
			if(this->reader.is_eof()){
				this->unexpected_eof("variable declaration");
				return Result::Error;
			}
		}



		// =
		if(this->reader.getKind(this->reader.peek()) != Token::get("=")){
			if(type.has_value()){
				this->expected_but_got("\"=\" in variable declaration");

			}else{
				this->expected_but_got("either \": Type =\" or \"=\" in variable declaration");
			}

			return Result::Error;

		}else{
			this->reader.skip(1);
		}

		if(this->reader.is_eof()){
			this->unexpected_eof("variable declaration");
			return Result::Error;
		}


		// expression
		const Result expr_result = this->parse_expr();
		switch(expr_result.code()){
			case Result::Success: break;

			case Result::WrongType: {
				this->expected_but_got("expression in variable declaration");
				return Result::Error;
			} break;

			case Result::Error: {
				return Result::Error;
			} break;
		};

		// if(this->reader.is_eof()){
		// 	this->error("Unexpected end of file in variable declaration", this->reader.peek(-1)); 
		// 	return Result::Error;
		// }


		// ;
		if(this->reader.getKind(this->reader.peek()) != Token::get(";")){
			this->expected_but_got("\";\" in variable declaration");

		}else{
			this->reader.skip(1);
		}


		// create and return
		return this->create_node(
			this->var_decls, AST::Kind::VarDecl,
			is_public, is_static, decl_type, ident_result.value(), type, expr_result.value()
		);
	};




	// TODO: EOF checking
	auto Parser::parse_assignment() noexcept -> Result {
		const TokenID first_token = this->reader.peek();

		const Result lhs_result = this->parse_expr();
		switch(lhs_result.code()){
			case Result::Success: break;
			case Result::WrongType: return Result::WrongType;
			case Result::Error: return Result::Error;
		};


		const TokenID assignemnt_op = this->reader.next();
		switch(this->reader.getKind(assignemnt_op)){
			case Token::get("="):
			case Token::get("+="):
			case Token::get("-="):
			case Token::get("*="):
			case Token::get("/="):
			case Token::get("%="):
				break;
			default:
				this->reader.go_back(first_token);
				return Result::WrongType;
		};


		const Result rhs_result = this->parse_expr();
		switch(rhs_result.code()){
			case Result::Success: break;
			case Result::WrongType: {
				this->expected_but_got("value expression for assignment statement");
				return Result::Error;
			} break;
			case Result::Error: return Result::Error;
		};


		// ;
		if(this->reader.getKind(this->reader.next()) != Token::get(";")){
			this->expected_but_got("semicolon at end of assignment statement");
			return Result::Error;
		}


		// create and return
		return this->create_node(this->infixes, AST::Kind::Infix, lhs_result.value(), assignemnt_op, rhs_result.value());
	};


	// TODO: check EOF
	auto Parser::parse_multiple_assignment() noexcept -> Result {
		const TokenID first_token = this->reader.peek();

		// check public
		bool is_public = false;
		if(this->reader.getKind(first_token) == Token::Kind::KeywordPub){
			is_public = true;
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->unexpected_eof("multiple assignment");
				return Result::Error;
			}
		}


		// check static
		bool is_static = false;
		if(this->reader.getKind(this->reader.peek()) == Token::Kind::KeywordStatic){
			is_static = true;
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->unexpected_eof("multiple assignment");
				return Result::Error;
			}
		}


		// decl type
		using DeclType = AST::MultipleAssignment::DeclType;
		DeclType decl_type;
		switch(this->reader.getKind(this->reader.peek())){
			break; case Token::KeywordVar: decl_type = DeclType::Var; this->reader.skip(1);
			break; case Token::KeywordDef: decl_type = DeclType::Def; this->reader.skip(1);
			break; default: decl_type = DeclType::None;
		};



		if(this->reader.getKind(this->reader.next()) != Token::get("[")){
			this->reader.go_back(first_token);
			return Result::WrongType;
		}


		auto assignment_values = std::vector<AST::NodeID>{};

		while(true){
			if(this->reader.getKind(this->reader.peek()) == Token::get("]")){
				this->reader.skip(1);	
				break;
			}

			const Result expr_result = this->parse_expr();
			switch(expr_result.code()){
				case Result::Success: {
					assignment_values.emplace_back(expr_result.value());
				} break;

				case Result::WrongType: {
					this->expected_but_got("expression in multiple assignment block");
					return Result::Error;
				} break;

				case Result::Error: return Result::Error;
			};


			const TokenID after_param_peek = this->reader.next();
			if(this->reader.getKind(after_param_peek) != Token::get(",")){
				if(this->reader.getKind(after_param_peek) == Token::get("]")){
					break;
					
				}else{
					this->expected_but_got("\",\" at end of multiple assignment value \"]\" at end of multiple assignment block", this->reader.peek(-1));
					return Result::Error;
				}
			}
		};


		// =
		if(this->reader.getKind(this->reader.next()) != Token::get("=")){
			this->expected_but_got("\"=\" in multiple assignment");
			return Result::Error;
		}


		// expr
		const Result expr_result = this->parse_expr();
		switch(expr_result.code()){
			case Result::Success: break;
			case Result::WrongType: {
				this->expected_but_got("expression on right-hand side of multiple assignment");
				return Result::Error;
			} break;
			case Result::Error: return Result::Error;
		};


		// ;
		if(this->reader.getKind(this->reader.next()) != Token::get(";")){
			this->expected_but_got("\";\" at end of multiple assignment");
			return Result::Error;
		}


		return this->create_node(this->multiple_assignments, AST::Kind::MultipleAssignment,
			is_public,
			is_static,
			decl_type,
			std::move(assignment_values),
			expr_result.value()
		);
	};





	// TODO: EOF checking
	auto Parser::parse_func_def() noexcept -> Result {
		const TokenID first_token = this->reader.peek();

		// check public
		bool is_public = false;
		if(this->reader.getKind(first_token) == Token::Kind::KeywordPub){
			is_public = true;
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->unexpected_eof("function definition");
				return Result::Error;
			}
		}


		// check static
		bool is_static = false;
		if(this->reader.getKind(this->reader.peek()) == Token::Kind::KeywordStatic){
			is_static = true;
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->unexpected_eof("function definition");
				return Result::Error;
			}
		}


		// func
		if(this->reader.getKind(this->reader.next()) != Token::KeywordFunc){
			this->reader.go_back(first_token);
			return Result::WrongType;
		}

		// identifier
		const Result ident_result = this->parse_ident();
		if(ident_result.code() == Result::WrongType){
			this->expected_but_got("identifer in function definition");
			return Result::Error;
		}

		// =
		if(this->reader.getKind(this->reader.next()) != Token::get("=")){
			this->expected_but_got("\"=\" in function definition", this->reader.peek(-1));
			return Result::Error;
		}




		// func params
		const Result func_params_result = this->parse_func_params();
		if(func_params_result.code() == Result::WrongType){
			this->expected_but_got("\"(\" in function definition", this->reader.peek(-1));
			return Result::Error;
		}else if(func_params_result.code() == Result::Error){
			return Result::Error;
		}


		

		// capture
		// auto captures_optional = std::optional<std::vector<AST::FuncDef::Capture>>{};
		// if(this->reader.getKind(this->reader.peek()) == Token::get("[")){
		// 	const TokenID open_capture = this->reader.next();


		// 	auto captures = std::vector<AST::FuncDef::Capture>{};


		// 	while(true){
		// 		if(this->reader.getKind(this->reader.peek()) == Token::get("]")){
		// 			this->reader.skip(1);
		// 			break;
		// 		}


		// 		const Result capture_ident_result = this->parse_ident();
		// 		if(capture_ident_result.code() == Result::WrongType){
		// 			this->expected_but_got("identifier in function capture");
		// 			return Result::Error;
		// 		}


		// 		AST::FuncDef::Capture::Kind param_kind = AST::FuncDef::Capture::Kind::Read;
		// 		switch(this->reader.getKind(this->reader.peek())){
		// 			case Token::KeywordRead: {
		// 				this->reader.skip(1);
		// 			} break;

		// 			case Token::KeywordWrite: {
		// 				param_kind = AST::FuncDef::Capture::Kind::Write;
		// 				this->reader.skip(1);
		// 			} break;

		// 			case Token::KeywordIn: {
		// 				param_kind = AST::FuncDef::Capture::Kind::In;
		// 				this->reader.skip(1);
		// 			} break;
		// 		};



		// 		captures.emplace_back(capture_ident_result.value(), param_kind);

		// 		// ,
		// 		const TokenID after_param_peek = this->reader.next();
		// 		if(this->reader.getKind(after_param_peek) != Token::get(",")){
		// 			if(this->reader.getKind(after_param_peek) == Token::get("]")){
		// 				break;
						
		// 			}else{
		// 				this->expected_but_got("\",\" at end of function capture or \"]\" at end of function captures block", this->reader.peek(-1));
		// 				return Result::Error;
		// 			}
		// 		}


		// 	};

		// 	captures_optional = std::move(captures);
		// }


		// attributes
		const Result attributes_result = this->parse_attributes();


		// ->
		if(this->reader.getKind(this->reader.next()) != Token::get("->")){
			this->expected_but_got("\"->\" in function definition", this->reader.peek(-1));
			return Result::Error;
		}


		// returns
		const Result returns_result = this->parse_func_returns();
		if(returns_result.code() == Result::Error){
			return Result::Error;
		}




		// errors
		const Result errors_result = this->parse_func_errors();
		if(errors_result.code() == Result::Error){
			return Result::Error;
		}





		const Result block_result = this->parse_block();
		switch(block_result.code()){
			case Result::Success: break;

			case Result::WrongType: {
				this->expected_but_got("{ } enclosed block after function definition");
				return Result::Error;
			} break;

			case Result::Error: return Result::Error;
		};
		



		// create and return
		return this->create_node(this->func_defs, AST::Kind::FuncDef,
			is_public,
			is_static,
			ident_result.value(),
			func_params_result.value(),
			// std::move(captures_optional),
			attributes_result.value(),
			returns_result.value(),
			errors_result.value(),
			block_result.value()
		);
	};



	// TODO: check for EOF
	auto Parser::parse_func_params() noexcept -> Result {
		// (
		if(this->reader.getKind(this->reader.peek()) != Token::get("(")){
			return Result::WrongType;
		}else{
			this->reader.skip(1);
		}

		auto params = std::vector<AST::FuncParams::Param>{};

		while(true){
			// )
			if(this->reader.getKind(this->reader.peek()) == Token::get(")")){
				this->reader.skip(1);
				break;
			}


			const Result ident_result = this->parse_ident();
			if(ident_result.code() == Result::WrongType){
				this->expected_but_got("identifier in function parameter");
				return Result::Error;
			}


			// :
			if(this->reader.getKind(this->reader.next()) != Token::get(":")){
				this->expected_but_got("\":\" in function parameter", this->reader.peek(-1));
				return Result::Error;
			}


			const Result type_result = this->parse_type();
			if(type_result.code() == Result::WrongType){
				this->expected_but_got("type in function parameter");
				return Result::Error;
			}


			AST::FuncParams::Param::Kind param_kind = AST::FuncParams::Param::Kind::Read;
			switch(this->reader.getKind(this->reader.peek())){
				case Token::KeywordRead: {
					this->reader.skip(1);
				} break;

				case Token::KeywordWrite: {
					param_kind = AST::FuncParams::Param::Kind::Write;
					this->reader.skip(1);
				} break;

				case Token::KeywordIn: {
					param_kind = AST::FuncParams::Param::Kind::In;
					this->reader.skip(1);
				} break;
			};


			auto default_value = std::optional<AST::NodeID>{};
			if(this->reader.getKind(this->reader.peek()) == Token::get("=")){
				this->reader.skip(1);

				const Result expr_result = this->parse_expr();
				switch(expr_result.code()){
					case Result::Success: {
						default_value = expr_result.value();
					} break;

					case Result::WrongType: {
						this->expected_but_got("expression in default value for function parameter");
						return Result::Error;
					} break;

					case Result::Error: return Result::Error;
				};

			}



			params.emplace_back(ident_result.value(), type_result.value(), param_kind, default_value);

			// ,
			const TokenID after_param_peek = this->reader.next();
			if(this->reader.getKind(after_param_peek) != Token::get(",")){
				if(this->reader.getKind(after_param_peek) == Token::get(")")){
					break;
					
				}else{
					this->expected_but_got("\",\" at end of function parameter or \")\" at end of function parameters block", this->reader.peek(-1));
					return Result::Error;
				}
			}

		};





		// create and return
		return this->create_node(this->func_params, AST::Kind::FuncParams, std::move(params));
	};



	// TODO: check for EOF
	auto Parser::parse_func_returns() noexcept -> Result {
		auto func_returns = std::vector<AST::FuncOutputs::Value>{};
		if(this->reader.getKind(this->reader.peek()) == Token::get("(")){
			this->reader.skip(1);

			while(true){
				if(this->reader.getKind(this->reader.peek()) == Token::get(")")){
					this->reader.skip(1);
					break;
				}


				const Result return_ident_result = this->parse_ident();
				if(return_ident_result.code() == Result::WrongType){
					this->expected_but_got("identifier in function return value");
					return Result::Error;
				}


				// :
				if(this->reader.getKind(this->reader.next()) != Token::get(":")){
					this->expected_but_got("\":\" in function definition", this->reader.peek(-1));
					return Result::Error;
				}



				const Result return_type_result = this->parse_type();
				if(return_type_result.code() == Result::WrongType){
					this->expected_but_got("type in function return value");
					return Result::Error;
				}

				func_returns.emplace_back(return_ident_result.value(), return_type_result.value());


				const TokenID after_return_peek = this->reader.next();
				if(this->reader.getKind(after_return_peek) != Token::get(",")){
					if(this->reader.getKind(after_return_peek) == Token::get(")")){
						break;
						
					}else{
						this->expected_but_got("\",\" at end of function return value or \")\" at end of function values block", this->reader.peek(-1));
						return Result::Error;
					}
				}


			};



		}else{
			const Result return_type = this->parse_type();
			switch(return_type.code()){
				case Result::Success: {
					func_returns.emplace_back(std::nullopt, return_type.value());
				} break;

				case Result::WrongType: {
					this->expected_but_got("function return type");
					return Result::Error;
				} break;

				case Result::Error: {
					return Result::Error;
				} break;
			};
		}


		return this->create_node(this->func_outputs, AST::Kind::FuncOutputs, std::move(func_returns));
	};




	// TODO: check for EOF
	auto Parser::parse_func_errors() noexcept -> Result {
		auto errors = std::vector<AST::FuncOutputs::Value>{};

		if(this->reader.getKind(this->reader.peek()) == Token::get("<")){
			this->reader.skip(1);

			const TokenID start_of_errors = this->reader.peek();
			const Result single_error_type_result = this->parse_type();

			bool is_single_return = false;
			switch(single_error_type_result.code()){
				case Result::Success: {
					if(this->reader.getKind(this->reader.next()) != Token::get(">")){
						this->reader.go_back(start_of_errors);
					}else{
						is_single_return = true;
						errors.emplace_back(std::nullopt, single_error_type_result.value());
					}
				} break;

				case Result::WrongType: {
					this->reader.go_back(start_of_errors);
				} break;

				case Result::Error: {
					return Result::Error;
				} break;
			};



			while(is_single_return == false){
				if(this->reader.getKind(this->reader.peek()) == Token::get(">")){
					this->reader.skip(1);
					break;
				}


				const Result return_ident_result = this->parse_ident();
				if(return_ident_result.code() == Result::WrongType){
					this->expected_but_got("identifier in function errors value");
					return Result::Error;
				}


				// :
				if(this->reader.getKind(this->reader.next()) != Token::get(":")){
					this->expected_but_got("\":\" in function definition", this->reader.peek(-1));
					return Result::Error;
				}



				const Result return_type_result = this->parse_type();
				if(return_type_result.code() == Result::WrongType){
					this->expected_but_got("type in function errors value");
					return Result::Error;
				}

				errors.emplace_back(return_ident_result.value(), return_type_result.value());


				const TokenID after_error_peek = this->reader.next();
				if(this->reader.getKind(after_error_peek) != Token::get(",")){
					if(this->reader.getKind(after_error_peek) == Token::get(">")){
						break;
						
					}else{
						this->expected_but_got("\",\" at end of function errors value or \">\" at end of function values block", this->reader.peek(-1));
						return Result::Error;
					}
				}

			};
		}



		return this->create_node(this->func_outputs, AST::Kind::FuncOutputs, std::move(errors));
	};




	// TODO: check for EOF
	auto Parser::parse_block() noexcept -> Result {
		if(this->reader.getKind(this->reader.peek()) != Token::get("{")){
			return Result::WrongType;
		}

		this->reader.skip(1);

		auto stmts = std::vector<AST::NodeID>{};

		while(true){
			// }
			if(this->reader.getKind(this->reader.peek()) == Token::get("}")){
				this->reader.skip(1);
				break; 
			}
			
			const Result stmt_result = this->parse_stmt();
			switch(stmt_result.code()){
				case Result::Success: {
					stmts.push_back(stmt_result.value());
				} break;

				case Result::WrongType: {
					this->error("Failed to parse statement - could not find any matching statement type", this->reader.peek());
					this->error_info("Above is pointing to the beginning of the statement");
					return Result::Error;
				} break;

				case Result::Error: {
					return Result::Error;
				} break;
			};
		};


		return this->create_node(this->blocks, AST::Kind::Block, std::move(stmts));
	};




	// TODO: check for EOF
	auto Parser::parse_conditional() noexcept -> Result {
		if(this->reader.getKind(this->reader.peek()) != Token::KeywordIf){
			return Result::WrongType;
		}

		this->reader.skip(1);

		// (
		if(this->reader.getKind(this->reader.next()) != Token::get("(")){
			this->expected_but_got("\"(\" in conditional");
			return Result::WrongType;
		}


		const Result if_stmt_result = this->parse_expr();
		switch(if_stmt_result.code()){
			case Result::Success: break;
			case Result::WrongType: {
				this->expected_but_got("expression in conditional");
				return Result::Error;
			} break;
			case Result::Error: return Result::Error;
		};


		// )
		if(this->reader.getKind(this->reader.next()) != Token::get(")")){
			this->expected_but_got("\")\" in conditional");
			return Result::WrongType;
		}


		const Result then_stmt_result = this->parse_block();
		switch(then_stmt_result.code()){
			case Result::Success: break;
			case Result::WrongType: {
				this->expected_but_got("block in conditional");
				return Result::Error;
			} break;
			case Result::Error: return Result::Error;
		};


		auto else_stmt = std::optional<AST::NodeID>{};
		if(this->reader.getKind(this->reader.peek()) == Token::KeywordElse){
			this->reader.skip(1);

			if(this->reader.getKind(this->reader.peek()) == Token::KeywordIf){
				const Result else_stmt_result = this->parse_conditional();
				switch(else_stmt_result.code()){
					case Result::Success: break;
					case Result::WrongType: {
						this->expected_but_got("conditional");
						return Result::Error;
					} break;
					case Result::Error: return Result::Error;
				};

				else_stmt = else_stmt_result.value();

			}else{
				const Result else_stmt_result = this->parse_block();
				switch(else_stmt_result.code()){
					case Result::Success: break;
					case Result::WrongType: {
						this->expected_but_got("conditional");
						return Result::Error;
					} break;
					case Result::Error: return Result::Error;
				};

				else_stmt = else_stmt_result.value();
			}
		}
				
		
		return this->create_node(this->conditionals, AST::Kind::Conditional, if_stmt_result.value(), then_stmt_result.value(), else_stmt);
	};



	auto Parser::parse_while() noexcept -> Result {
		if(this->reader.getKind(this->reader.peek()) != Token::KeywordWhile){
			return Result::WrongType;
		}

		this->reader.skip(1);


		// (
		if(this->reader.getKind(this->reader.next()) != Token::get("(")){
			this->expected_but_got("\"(\" in while loop condition");
			return Result::Error;
		}


		const Result condition_result = this->parse_expr();
		switch(condition_result.code()){
			case Result::Success: break;
			case Result::WrongType: {
				this->expected_but_got("expression in while loop condition");
				return Result::Error;
			} break;
			case Result::Error: return Result::Error;
		};


		// )
		if(this->reader.getKind(this->reader.next()) != Token::get(")")){
			this->expected_but_got("\")\" in while loop condition");
			return Result::Error;
		}


		const Result block_stmt_result = this->parse_block();
		switch(block_stmt_result.code()){
			case Result::Success: break;
			case Result::WrongType: {
				this->expected_but_got("block in while loop");
				return Result::Error;
			} break;
			case Result::Error: return Result::Error;
		};


		return this->create_node(this->while_loops, AST::Kind::WhileLoop, false, condition_result.value(), block_stmt_result.value());
	};



	auto Parser::parse_return() noexcept -> Result {
		const TokenID first_token = this->reader.peek();
		const Token::Kind token_kind = this->reader.getKind(first_token);

		if(token_kind != Token::KeywordReturn && token_kind != Token::KeywordThrow){
			return Result::WrongType;
		}

		const bool is_throw = token_kind == Token::KeywordThrow ? true : false;

		this->reader.skip(1);

		AST::Return::Kind kind;
		auto expr = std::optional<AST::NodeID>{};

		if(this->reader.getKind(this->reader.peek()) == Token::get(";")){
			kind = AST::Return::Kind::Nothing;
			this->reader.skip(1);

		}else if(this->reader.getKind(this->reader.peek()) == Token::get("...")){
			kind = AST::Return::Kind::Ellipsis;
			this->reader.skip(1);

			if(this->reader.getKind(this->reader.next()) != Token::get(";")){
				this->expected_but_got(std::format("\";\" at end of {} statement", is_throw ? "throw" : "error"));
				return Result::Error;
			}

		}else{
			kind = AST::Return::Kind::Expr;

			const Result return_expr = this->parse_expr();
			switch(return_expr.code()){
				case Result::Success: break;
				case Result::WrongType: {
					this->expected_but_got(std::format("expression in {} statement", is_throw ? "throw" : "error"));
					return Result::Error;
				} break;
				case Result::Error: return Result::Error;
			};

			expr = return_expr.value();

			if(this->reader.getKind(this->reader.next()) != Token::get(";")){
				this->expected_but_got(std::format("\";\" at end of {} statement", is_throw ? "throw" : "error"));
				return Result::Error;
			}
		}

		
		return this->create_node(this->returns, AST::Kind::Return, is_throw, kind, expr);
	};




	auto Parser::parse_ident() noexcept -> Result {
		const TokenID first_token = this->reader.peek();

		if(this->reader.getKind(first_token) == Token::Ident){
			this->reader.skip(1);

		}else{
			return Result::WrongType;
		}


		return this->create_node(this->idents, AST::Kind::Ident, first_token);
	};


	auto Parser::parse_intrinsic() noexcept -> Result {
		const TokenID first_token = this->reader.peek();

		if(this->reader.getKind(first_token) == Token::Intrinsic){
			this->reader.skip(1);

		}else{
			return Result::WrongType;
		}


		return this->create_node(this->intrinsics, AST::Kind::Intrinsic, first_token);
	};



	auto Parser::parse_attributes() noexcept -> Result {
		auto attributes_list = std::vector<TokenID>{};

		while(this->reader.getKind(this->reader.peek()) == Token::Attribute){
			attributes_list.emplace_back(this->reader.next());
		};


		return this->create_node(this->attributes, AST::Kind::Attributes);
	};


	// TODO: check EOF
	auto Parser::parse_type() noexcept -> Result {
		// array type
		const Result arr_type_result = this->parse_arr_type();		
		switch(arr_type_result.code()){
			case Result::Success: return arr_type_result;

			case Result::WrongType: break;

			case Result::Error: return Result::Error;
		};


		// function type
		const Result func_type_result = this->parse_func_type();		
		switch(func_type_result.code()){
			case Result::Success: return func_type_result;

			case Result::WrongType: break;

			case Result::Error: return Result::Error;
		};


		///////////////////////////////////
		// basic types

		auto type_name = std::optional<TokenID>{};


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
				type_name = this->reader.next();
			}
		};


		// ident
		if(type_name.has_value() == false && this->reader.getKind(this->reader.peek()) == Token::Ident){
			type_name = this->reader.next();
		}


		if(type_name.has_value() == false){
			return Result::WrongType;
		}


		std::vector<AST::Type::Qualifier> qualifiers = this->parse_type_qualifiers();


		return this->create_node(
			this->types, AST::Kind::Type,
			AST::Type::Kind::Basic, *type_name, std::move(qualifiers)
		);
	};



	// TODO: check for EOF
	auto Parser::parse_arr_type() noexcept -> Result {
		if(this->reader.getKind(this->reader.peek()) != Token::get("[")){
			return Result::WrongType;
		}

		// skip [
		this->reader.skip(1);

		const Result arr_type = this->parse_type();
		switch(arr_type.code()){
			case Result::Success: break;

			case Result::WrongType: {
				this->expected_but_got("type in array type", this->reader.peek(-1));
				return Result::Error;
			} break;

			case Result::Error: return Result::Error;
		};


		// :
		if(this->reader.getKind(this->reader.next()) != Token::get(":")){
			this->expected_but_got("\":\" in array type", this->reader.peek(-1));
			return Result::Error;
		}


		AST::NodeID arr_length;
		if(this->reader.getKind(this->reader.peek()) == Token::KeywordUnderscore){
			this->reader.skip(1);

			arr_length = this->create_node(AST::Kind::Underscore);

		}else{
			const Result expr_result = this->parse_expr();
			switch(expr_result.code()){
				case Result::Success: break;

				case Result::WrongType: {
					this->expected_but_got("length expr in array type", this->reader.peek(-1));
					return Result::Error;
				} break;

				case Result::Error: return Result::Error;
			};

			arr_length = expr_result.value();
		}


		// ]
		if(this->reader.getKind(this->reader.next()) != Token::get("]")){
			this->expected_but_got("\"]\" in array type", this->reader.peek(-1));
			return Result::Error;
		}

		std::vector<AST::Type::Qualifier> qualifiers = this->parse_type_qualifiers();


		return this->create_node(
			this->types, AST::Kind::Type,
			AST::Type::Kind::Array, arr_type.value(), arr_length, std::move(qualifiers)
		);
	};




	// TODO: check for EOF
	auto Parser::parse_func_type() noexcept -> Result {
		if(this->reader.getKind(this->reader.peek()) != Token::KeywordFunc){
			return Result::WrongType;
		}

		// skip func
		this->reader.skip(1);

		// func params
		const Result func_params_result = this->parse_func_params();
		switch(func_params_result.code()){
			case Result::Success: break;

			case Result::WrongType: {
				this->expected_but_got("function paramaters block in function type", this->reader.peek(-1));
				return Result::Error;
			} break;

			case Result::Error: return Result::Error;
		};


		// attributes
		const Result attributes_result = this->parse_attributes();
		switch(attributes_result.code()){
			case Result::Success: break;

			case Result::WrongType: break;

			case Result::Error: return Result::Error;
		};


		// ->
		if(this->reader.getKind(this->reader.next()) != Token::get("->")){
			this->expected_but_got("\"->\" in function type");
			return Result::Error;
		}




		// returns
		const Result returns_result = this->parse_func_returns();
		if(returns_result.code() == Result::Error){
			return Result::Error;
		}


		// errors
		const Result errors_result = this->parse_func_errors();
		if(errors_result.code() == Result::Error){
			return Result::Error;
		}


		// qualifiers
		std::vector<AST::Type::Qualifier> qualifiers = this->parse_type_qualifiers();



		return this->create_node(this->types, AST::Kind::Type,
			AST::Type::Kind::Func,
			func_params_result.value(),
			attributes_result.value(),
			returns_result.value(),
			errors_result.value(),
			std::move(qualifiers)
		);
	};



	// TODO: check for EOF
	auto Parser::parse_type_qualifiers() noexcept -> std::vector<AST::Type::Qualifier> {
		auto qualifiers = std::vector<AST::Type::Qualifier>{};

		while(true){
			if(this->reader.getKind(this->reader.peek()) == Token::get("^")){
				// is pointer
				this->reader.skip(1);
				AST::Type::Qualifier& qualifer = qualifiers.emplace_back(true, false, false);

				// is const
				if(this->reader.getKind(this->reader.peek()) == Token::get("|")){
					this->reader.skip(1);
					qualifer.is_const = true;
				}

				// is optional
				if(this->reader.getKind(this->reader.peek()) == Token::get("?")){
					this->reader.skip(1);
					qualifer.is_optional = true;
				}


			}else if(this->reader.getKind(this->reader.peek()) == Token::get("?")){
				// is optional
				this->reader.skip(1);
				qualifiers.emplace_back(false, false, true);

			}else{
				break;
			}
		};

		return qualifiers;
	};





	///////////////////////////////////
	// expressions


	auto Parser::parse_expr() noexcept -> Result {
		if(this->reader.is_eof()){
			return Result::WrongType;
		}


		if(this->reader.getKind(this->reader.peek()) == Token::KeywordUninit){
			this->reader.skip(1);

			return this->create_node(AST::Kind::Uninit);
		}

		const Result infix_expr = this->parse_infix_expr();
		if(infix_expr.code() == Result::Success){
			return infix_expr;
		}else if(infix_expr.code() == Result::Error){
			return Result::Error;
		}

		return Result::WrongType;
	};




	constexpr int MAX_PREC = 7;
	EVO_NODISCARD static constexpr auto get_infix_op_precedence(Token::Kind kind) noexcept -> int {
		switch(kind){
			case Token::get("||"): return 1;
			case Token::get("&&"): return 1;

			case Token::get("|"): return 2;
			case Token::get("&"): return 2;

			case Token::get("=="): return 3;
			case Token::get("!="): return 3;
			case Token::get("<"): return 3;
			case Token::get("<="): return 3;
			case Token::get(">"): return 3;
			case Token::get(">="): return 3;

			case Token::get("<<"): return 4;
			case Token::get(">>"): return 4;

			case Token::get("+"): return 5;
			case Token::get("-"): return 5;

			case Token::get("*"): return 6;
			case Token::get("/"): return 6;
			case Token::get("%"): return 6;

			case Token::KeywordAs: return 7;
			case Token::KeywordCast: return 7;
		};

		return -1;
	};



	auto Parser::parse_infix_expr() noexcept -> Result {
		const Result lhs_result = this->parse_prefix_expr();
		switch(lhs_result.code()){
			case Result::Success: break;
			case Result::WrongType: return Result::WrongType;
			case Result::Error: return Result::Error;
		};

		return this->parse_infix_expr_impl(lhs_result.value(), 1);
	};



	auto Parser::parse_infix_expr_impl(AST::NodeID lhs, int prec_level) noexcept -> Result {
		const TokenID peeked_op = this->reader.peek();
		const Token::Kind peeked_kind = this->reader.getKind(peeked_op);

		const int next_op_prec = get_infix_op_precedence(peeked_kind);

		// if not an infix operator or is same or lower precedence
		// 		next_op_prec == -1 if its not an infix op
		//   	< to maintain operator precedence
		// 		<= to prevent `a + b + c` from being parsed as `a + (b + c)`
		if(next_op_prec <= prec_level){

			// TODO: fix the cast operator thinking multiply is pointer


			return lhs;
		}

		// skip operator
		this->reader.skip(1);


		const Result next_term = this->parse_prefix_expr();
		switch(next_term.code()){
			case Result::Success: break;
			case Result::WrongType: return Result::WrongType;
			case Result::Error: return Result::Error;
		};


		const Result rhs_result = this->parse_infix_expr_impl(next_term.value(), next_op_prec);
		switch(rhs_result.code()){
			case Result::Success: break;
			case Result::WrongType: return Result::WrongType;
			case Result::Error: return Result::Error;
		};



		return this->create_node(this->infixes, AST::Kind::Infix, 
			lhs, peeked_op, rhs_result.value()
		);
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
			this->expected_but_got(std::format("expression on right-hand side of ({}) operator", Token::print_kind(this->reader.getKind(op_token))));
			return Result::Error;
		}


		return this->create_node(this->prefixes, AST::Kind::Prefix, op_token, rhs_result.value());
	};


	auto Parser::parse_postfix_expr() noexcept -> Result {
		// nothing at the moment
		return this->parse_accessor_expr();		
	};



	// TODO: check for EOF
	auto Parser::parse_accessor_expr() noexcept -> Result {
		Result output = this->parse_paren_expr();
		switch(output.code()){
			case Result::Success: break;
			case Result::WrongType: return Result::WrongType;
			case Result::Error: return Result::Error;
		};



		while(true){
			if(this->reader.getKind(this->reader.peek()) == Token::get(".")){
				const TokenID accessor_op_token = this->reader.next();

				const Result rhs_result = this->parse_ident();
				switch(rhs_result.code()){
					case Result::Success: break;

					case Result::WrongType: {
						this->expected_but_got("expression on right-hand side of accessor operator");
						return Result::Error;
					} break;

					case Result::Error: return Result::Error;
				};


				output = this->create_node(this->infixes, AST::Kind::Infix, output.value(), accessor_op_token, rhs_result.value());

				continue;

			}else if(this->reader.getKind(this->reader.peek()) == Token::get(".?") || this->reader.getKind(this->reader.peek()) == Token::get(".^")){
				const TokenID op_token = this->reader.next();

				output = this->create_node(this->postfixes, AST::Kind::Postfix, output.value(), op_token);
				continue;
				
			}else if(this->reader.getKind(this->reader.peek()) == Token::get("[")){
				this->reader.skip(1);

				const Result expr_result = this->parse_expr();
				switch(expr_result.code()){
					case Result::Success: break;

					case Result::WrongType: {
						this->expected_but_got("expression inside index operator");
						return Result::Error;
					} break;

					case Result::Error: return Result::Error;
				};


				if(this->reader.getKind(this->reader.peek()) != Token::get("]")){
					this->expected_but_got("closing \"]\" at end of index operator");
					return Result::Error;

				}else{
					this->reader.skip(1);
				}


				output = this->create_node(this->index_ops, AST::Kind::IndexOp, output.value(), expr_result.value());
				continue;

			}else if(this->reader.getKind(this->reader.peek()) == Token::get("(")){
				this->reader.skip(1);


				auto arguments = std::vector<AST::NodeID>{};

				while(true){
					if(this->reader.getKind(this->reader.peek()) == Token::get(")")){
						this->reader.skip(1);	
						break;
					}


					const Result expr_result = this->parse_expr();
					switch(expr_result.code()){
						case Result::Success: {
							arguments.emplace_back(expr_result.value());
						} break;

						case Result::WrongType: {
							this->expected_but_got("expression inside function call operator");
							return Result::Error;
						} break;

						case Result::Error: return Result::Error;
					};


					const TokenID after_param_peek = this->reader.next();
					if(this->reader.getKind(after_param_peek) != Token::get(",")){
						if(this->reader.getKind(after_param_peek) == Token::get(")")){
							break;
							
						}else{
							this->expected_but_got("\",\" at end of function call argument or \")\" at end of function call", this->reader.peek(-1));
							return Result::Error;
						}
					}
				};


				output = this->create_node(this->func_calls, AST::Kind::FuncCall, output.value(), std::move(arguments));
				continue;

			}else{
				break;
			}
		};



		return output;
	};




	// TODO: check for EOF
	auto Parser::parse_paren_expr() noexcept -> Result {
		if(this->reader.getKind(this->reader.peek()) != Token::get("(")){
			return this->parse_term();
		}

		const TokenID open_location = this->reader.next();

		const Result expr_result = this->parse_expr();
		switch(expr_result.code()){
			case Result::Success: break;
			case Result::WrongType: return expr_result;
			case Result::Error: return expr_result;
		};


		if(this->reader.getKind(this->reader.peek()) != Token::get(")")){
			this->expected_but_got("either closing parenthesis around expression or continuation of expression");
			this->error_info("parenthesis opened here", open_location);
			return Result::Error;
		}

		// skip ')'
		this->reader.skip(1);

		return expr_result;
	};





	// TODO: check for EOF
	auto Parser::parse_term() noexcept -> Result {
		// literals
		const Result literal_result = this->parse_literal();
		if(literal_result.code() == Result::Success){ return literal_result; }

		// types
		const Result type_result = this->parse_type();
		if(type_result.code() == Result::Success){ return type_result; }

		// idents
		const Result ident_result = this->parse_ident();
		if(ident_result.code() == Result::Success){ return ident_result; }

		// intrinsics
		const Result intrinsic_result = this->parse_intrinsic();
		if(intrinsic_result.code() == Result::Success){ return intrinsic_result; }



		const TokenID first_token = this->reader.next();
		switch(this->reader.getKind(first_token)){
			case Token::KeywordNull: {
				return this->create_node(AST::Kind::Null);
			} break;

			case Token::KeywordThis: {
				return this->create_node(AST::Kind::This);
			} break;
		};

		this->reader.go_back(first_token);

		return Result::WrongType;
	};




	// TODO: check for EOF
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


		return this->create_node(this->literals, AST::Kind::Literal, token);
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


	auto Parser::unexpected_eof(const std::string& location) noexcept -> void {
		this->error(std::format("Unexpected end of file in {}", location), this->reader.peek(-1)); 
	};

	auto Parser::expected_but_got(const std::string& expected, TokenID token) noexcept -> void {
		this->error(std::format("Expected {} - got ({}) instead", expected, Token::print_kind( this->reader.getKind(token) )), token);
	};


};
