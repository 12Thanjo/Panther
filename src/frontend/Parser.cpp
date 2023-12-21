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
		
			// TODO: remove
			this->fatal("Failed to parse", this->reader.peek());
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
				this->error("Unexpected end of file in Variable declaration", this->reader.peek()); 
				return Result::Error;
			}
		}


		// check static
		bool is_static = false;
		if(this->reader.getKind(this->reader.peek()) == Token::Kind::KeywordStatic){
			is_static = true;
			this->reader.skip(1);

			if(this->reader.is_eof()){
				this->error("Unexpected end of file in variable declaration", this->reader.peek()); 
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


		// : type (maybe)
		auto type = std::optional<AST::NodeID>{};
		if(this->reader.getKind(this->reader.peek()) == Token::get(":")){
			this->reader.skip(1);

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

	auto Parser:: parse_expr() noexcept -> Result {
		
		if(this->reader.getKind(this->reader.peek()) == Token::KeywordUninit){
			this->reader.skip(1);

			const uint32_t node_index = uint32_t(this->nodes.size());
			this->nodes.emplace_back(AST::Kind::Uninit);

			return AST::NodeID{node_index};
		}

		return Result::WrongType;
	};




	///////////////////////////////////
	// errors etc.


	auto Parser::error(const std::string& message, TokenID token) const noexcept -> void {
		this->reader.error(message, this->reader.getLine(token), this->reader.getCollumn(token));
	};
	auto Parser::error_info(const std::string& message) const noexcept -> void {
		this->reader.error_info(message);
	};
	auto Parser::error_info(const std::string& message, TokenID token) const noexcept -> void {
		this->reader.error_info(message, this->reader.getLine(token), this->reader.getCollumn(token));
	};
	auto Parser::fatal(const std::string& message, TokenID token) const noexcept -> void {
		this->reader.fatal(message, this->reader.getLine(token), this->reader.getCollumn(token));
	};





};
