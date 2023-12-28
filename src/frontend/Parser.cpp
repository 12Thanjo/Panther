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

				case Result::UnreportedError: {
					this->fatal("Encountered unknown and unreported error while parsing a statement", this->reader.peek());
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

				case Result::UnreportedError:{
					this->fatal("Failed to parse variable declaration", this->reader.peek());
					return Result::Error;
				}
			};
		}


		// func_def
		{
			const auto result = this->parse_func_def();
			switch(result.code()){
				case Result::Success: return result;
				case Result::WrongType: break;
				case Result::Error: return Result::Error;

				case Result::UnreportedError:{
					this->fatal("Failed to parse function definition", this->reader.peek());
					return Result::Error;
				}
			};
		}


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
		// block
		// expr ; // meant for things like function calls (make sure to check in semantic ananlysis that there actually are side effects)
		
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
			this->expected_but_got("identifier in variable declaration");
			return Result::Error;
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

				case Result::UnreportedError: {
					this->error("Invalid type in variable declaration", this->reader.peek());
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
			this->expected_but_got("\";\" in variable declaration");

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
				this->expected_but_got("{ } enclosed block after function defintion");
				return Result::Error;
			} break;

			case Result::Error: return Result::Error;

			case Result::UnreportedError: {
				this->fatal("Encoutered error while trying to parse { } enclosed block after function definition", this->reader.peek(-1));
				return Result::Error;
			} break;
		};
		



		// create and return
		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t func_def_index = uint32_t(this->func_defs.size());

		this->nodes.emplace_back(AST::Kind::FuncDef, func_def_index);
		this->func_defs.emplace_back(
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

		return AST::NodeID{node_index};
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

					case Result::UnreportedError: {
						this->error("Failed to parse expression in default value for function parameter", this->reader.peek());
						return Result::Error;	
					}
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
		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t func_params_index = uint32_t(this->func_params.size());

		this->nodes.emplace_back(AST::Kind::FuncParams, func_params_index);
		this->func_params.emplace_back(std::move(params));

		return AST::NodeID{node_index};
	};



	// TODO: check for EOF
	auto Parser::parse_func_returns() noexcept -> Result {
		auto returns = std::vector<AST::FuncOutputs::Value>{};
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

				returns.emplace_back(return_ident_result.value(), return_type_result.value());


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
					returns.emplace_back(std::nullopt, return_type.value());
				} break;

				case Result::WrongType: {
					this->expected_but_got("function return type");
					return Result::Error;
				} break;

				case Result::Error: {
					return Result::Error;
				} break;

				case Result::UnreportedError: {
					this->fatal("Failed to parse function return type", this->reader.peek(-1));
				} break;
			};
		}



		// create and return
		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t func_output_index = uint32_t(this->func_outputs.size());

		this->nodes.emplace_back(AST::Kind::FuncOutputs, func_output_index);
		this->func_outputs.emplace_back(std::move(returns));

		return AST::NodeID{node_index};
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

				case Result::UnreportedError: {
					this->error("Failed to parse type of function error", this->reader.peek(-1));
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



		// create and return
		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t func_output_index = uint32_t(this->func_outputs.size());

		this->nodes.emplace_back(AST::Kind::FuncOutputs, func_output_index);
		this->func_outputs.emplace_back(std::move(errors));

		return AST::NodeID{node_index};
	};




	// TODO: check for EOF
	auto Parser::parse_block() noexcept -> Result {
		if(this->reader.getKind(this->reader.next()) != Token::get("{")){
			return Result::WrongType;
		}

		// TODO: parse statements
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

				case Result::UnreportedError: {
					this->fatal("Encountered unknown and unreported error while parsing a statement", this->reader.peek());
				} break;
			};
		};



		// create and return
		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t blocks_index = uint32_t(this->blocks.size());

		this->nodes.emplace_back(AST::Kind::Block, blocks_index);
		this->blocks.emplace_back(std::move(stmts));

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


	auto Parser::parse_intrinsic() noexcept -> Result {
		const TokenID first_token = this->reader.peek();

		if(this->reader.getKind(first_token) == Token::Intrinsic){
			this->reader.skip(1);

		}else{
			return Result::WrongType;
		}


		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t intrinsic_index = uint32_t(this->intrinsics.size());

		this->nodes.emplace_back(AST::Kind::Intrinsic, intrinsic_index);
		this->intrinsics.emplace_back(first_token);


		return AST::NodeID{node_index};
	};



	auto Parser::parse_attributes() noexcept -> Result {
		auto attributes_list = std::vector<TokenID>{};

		while(this->reader.getKind(this->reader.peek()) == Token::Attribute){
			attributes_list.emplace_back(this->reader.next());
		};


		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t attribute_index = uint32_t(this->attributes.size());

		this->nodes.emplace_back(AST::Kind::Attributes, attribute_index);
		this->attributes.emplace_back(std::move(attributes_list));


		return AST::NodeID{node_index};
	};


	// TODO: check EOF
	auto Parser::parse_type() noexcept -> Result {
		// array type
		const Result arr_type_result = this->parse_arr_type();		
		switch(arr_type_result.code()){
			case Result::Success: return arr_type_result;

			case Result::WrongType: break;

			case Result::Error: return Result::Error;

			case Result::UnreportedError: {
				this->error("Failed to parse array type", this->reader.peek(-1));
				return Result::Error;
			} break;
		};


		// function type
		const Result func_type_result = this->parse_func_type();		
		switch(func_type_result.code()){
			case Result::Success: return func_type_result;

			case Result::WrongType: break;

			case Result::Error: return Result::Error;

			case Result::UnreportedError: {
				this->error("Failed to parse function type", this->reader.peek(-1));
				return Result::Error;
			} break;
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



		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t type_index = uint32_t(this->types.size());

		this->nodes.emplace_back(AST::Kind::Type, type_index);
		this->types.emplace_back(AST::Type::Kind::Basic, *type_name, std::move(qualifiers));

		return AST::NodeID{node_index};
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

			case Result::UnreportedError: {
				this->error("Failed to parse array element type", this->reader.peek(-1));
				return Result::Error;
			} break;
		};


		// :
		if(this->reader.getKind(this->reader.next()) != Token::get(":")){
			this->expected_but_got("\":\" in array type", this->reader.peek(-1));
			return Result::Error;
		}


		AST::NodeID arr_length;
		if(this->reader.getKind(this->reader.peek()) == Token::KeywordUnderscore){
			this->reader.skip(1);

			const uint32_t node_index = uint32_t(this->nodes.size());
			this->nodes.emplace_back(AST::Kind::Underscore);

			arr_length = AST::NodeID{node_index};

		}else{
			const Result expr_result = this->parse_expr();
			switch(expr_result.code()){
				case Result::Success: break;

				case Result::WrongType: {
					this->expected_but_got("length expr in array type", this->reader.peek(-1));
					return Result::Error;
				} break;

				case Result::Error: return Result::Error;

				case Result::UnreportedError: {
					this->error("Failed to parse array element length expr", this->reader.peek(-1));
					return Result::Error;
				} break;
			};

			arr_length = expr_result.value();
		}


		// ]
		if(this->reader.getKind(this->reader.next()) != Token::get("]")){
			this->expected_but_got("\"]\" in array type", this->reader.peek(-1));
			return Result::Error;
		}

		std::vector<AST::Type::Qualifier> qualifiers = this->parse_type_qualifiers();


		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t type_index = uint32_t(this->types.size());

		this->nodes.emplace_back(AST::Kind::Type, type_index);
		this->types.emplace_back(AST::Type::Kind::Array, arr_type.value(), arr_length, std::move(qualifiers));

		return AST::NodeID{node_index};
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

			case Result::UnreportedError: {
				this->error("Failed to parse function parameters block in function type", this->reader.peek(-1));
				return Result::Error;
			} break;
		};


		// attributes
		const Result attributes_result = this->parse_attributes();
		switch(attributes_result.code()){
			case Result::Success: break;

			case Result::WrongType: break;

			case Result::Error: return Result::Error;

			case Result::UnreportedError: {
				this->error("Failed to parse function parameters block in function type", this->reader.peek(-1));
				return Result::Error;
			} break;
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



		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t type_index = uint32_t(this->types.size());

		this->nodes.emplace_back(AST::Kind::Type, type_index);
		this->types.emplace_back(
			AST::Type::Kind::Func,
			func_params_result.value(),
			attributes_result.value(),
			returns_result.value(),
			errors_result.value(),
			std::move(qualifiers)
		);

		return AST::NodeID{node_index};
	};



	// TODO: check for EOF
	auto Parser::parse_type_qualifiers() noexcept -> std::vector<AST::Type::Qualifier> {
		auto qualifiers = std::vector<AST::Type::Qualifier>{};

		while(true){
			if(this->reader.getKind(this->reader.peek()) == Token::get("*")){
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




	constexpr int MAX_PREC = 10;
	EVO_NODISCARD static constexpr auto get_infix_op_precedence(Token::Kind kind) noexcept -> int {
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

			case Token::KeywordAs: return 10;
			case Token::KeywordCast: return 10;
		};

		return -1;
	};



	auto Parser::parse_infix_expr() noexcept -> Result {
		const Result lhs_result = this->parse_prefix_expr();
		switch(lhs_result.code()){
			case Result::Success: break;
			case Result::WrongType: return Result::WrongType;
			case Result::Error: return Result::Error;
			case Result::UnreportedError: return lhs_result; // don't want to report error here (although I'm not sure this will ever happen)
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
			return lhs;
		}

		// skip operator
		this->reader.skip(1);


		const Result next_term = this->parse_prefix_expr();
		switch(next_term.code()){
			case Result::Success: break;
			case Result::WrongType: return Result::WrongType;
			case Result::Error: return Result::Error;
			case Result::UnreportedError: return next_term; // don't want to report error here (although I'm not sure this will ever happen)
		};


		const Result rhs_result = this->parse_infix_expr_impl(next_term.value(), next_op_prec);
		switch(rhs_result.code()){
			case Result::Success: break;
			case Result::WrongType: return Result::WrongType;
			case Result::Error: return Result::Error;
			case Result::UnreportedError: return Result::UnreportedError; // don't want to report error here (although I'm not sure this will ever happen)
		};



		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t infix_index = uint32_t(this->infixes.size());

		this->nodes.emplace_back(AST::Kind::Infix, infix_index);
		this->infixes.emplace_back(lhs, peeked_op, rhs_result.value());

		// make sure operator chaining works
		return this->parse_infix_expr_impl(AST::NodeID{node_index}, prec_level);
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



		const uint32_t node_index = uint32_t(this->nodes.size());
		const uint32_t prefix_index = uint32_t(this->prefixes.size());

		this->nodes.emplace_back(AST::Kind::Prefix, prefix_index);
		this->prefixes.emplace_back(op_token, rhs_result.value());

		return AST::NodeID{node_index};
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
			case Result::UnreportedError: return output; // don't want to report error here (although I'm not sure this will ever happen)
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

					case Result::UnreportedError: {
						this->error("Failed to parse expression on right-hand side of accessor operator", this->reader.peek(-1));
						return Result::Error;
					} break;
				};


				const uint32_t node_index = uint32_t(this->nodes.size());
				const uint32_t infix_index = uint32_t(this->infixes.size());

				this->nodes.emplace_back(AST::Kind::Infix, infix_index);
				this->infixes.emplace_back(output.value(), accessor_op_token, rhs_result.value());

				output = AST::NodeID{node_index};

				continue;

			}else if(this->reader.getKind(this->reader.peek()) == Token::get(".?") || this->reader.getKind(this->reader.peek()) == Token::get(".*")){
				const TokenID op_token = this->reader.next();


				const uint32_t node_index = uint32_t(this->nodes.size());
				const uint32_t postfix_index = uint32_t(this->postfixes.size());

				this->nodes.emplace_back(AST::Kind::Postfix, postfix_index);
				this->postfixes.emplace_back(output.value(), op_token);

				output = AST::NodeID{node_index};

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

					case Result::UnreportedError: {
						this->error("Failed to parse expression inside index operator", this->reader.peek(-1));
						return Result::Error;
					} break;
				};


				if(this->reader.getKind(this->reader.peek()) != Token::get("]")){
					this->expected_but_got("closing \"]\" at end of index operator");
					return Result::Error;

				}else{
					this->reader.skip(1);
				}



				const uint32_t node_index = uint32_t(this->nodes.size());
				const uint32_t index_op_index = uint32_t(this->index_ops.size());

				this->nodes.emplace_back(AST::Kind::IndexOp, index_op_index);
				this->index_ops.emplace_back(output.value(), expr_result.value());

				output = AST::NodeID{node_index};



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
			case Result::UnreportedError: return expr_result; // don't want to report error here (although I'm not sure this will ever happen)
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





	// TODO: finish terms
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


	auto Parser::unexpected_eof(const std::string& location) noexcept -> void {
		this->error(std::format("Unexpected end of file in {}", location), this->reader.peek(-1)); 
	};

	auto Parser::expected_but_got(const std::string& expected, TokenID token) noexcept -> void {
		this->error(std::format("Expected {} - got ({}) instead", expected, Token::print_kind( this->reader.getKind(token) )), token);
	};





};
