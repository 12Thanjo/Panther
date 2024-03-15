#include "SemanticAnalyzer.h"

#include "frontend/SourceManager.h"

namespace panther{
	

	auto SemanticAnalyzer::semantic_analysis() noexcept -> bool {
		this->enter_scope();

		for(AST::Node::ID global_stmt : this->source.global_stmts){
			const AST::Node& node = this->source.getNode(global_stmt);

			if(this->analyze_stmt(node) == false){
				return false;
			}
		}

		this->leave_scope();

		return true;
	};



	auto SemanticAnalyzer::analyze_stmt(const AST::Node& node) noexcept -> bool {
		switch(node.kind){
			break; case AST::Kind::VarDecl: return this->analyze_var(this->source.getVarDecl(node));
			break; case AST::Kind::Func: return this->analyze_func(this->source.getFunc(node));
		};

		EVO_FATAL_BREAK("This AST Kind is not handled");
	};





	auto SemanticAnalyzer::analyze_var(const AST::VarDecl& var_decl) noexcept -> bool {
		// check ident is unused
		const Token::ID ident_tok_id = this->source.getNode(var_decl.ident).token;
		const Token& ident = this->source.getToken(ident_tok_id);

		if(this->has_in_scope(ident.value.string)){
			this->already_defined(ident);
			return false;
		}


		///////////////////////////////////
		// type checking

		const AST::Type& type = this->source.getType(var_decl.type);
		const Token& type_token = this->source.getToken(type.token);


		SourceManager& src_manager = this->source.getSourceManager();

		const object::Type::ID var_type_id = src_manager.getTypeID(
			object::Type{
				.base_type = src_manager.getBaseTypeID(type_token.kind),
			}
		);

		
		if(type_token.kind == Token::TypeVoid){
			this->source.error("Variable cannot be of type Void", type.token);
			return false;

		}else{
			const AST::Node& expr_node = this->source.getNode(var_decl.expr);

			if(expr_node.kind != AST::Kind::Uninit){
				const std::optional<object::Type::ID> expr_type_id = this->get_type_of_expr(expr_node);

				if(expr_type_id.has_value() == false){ return false; }

				if(var_type_id != *expr_type_id){
					this->source.error(
						"Variable cannot be assigned a value of a different type", 
						var_decl.expr,

						std::vector<Message::Info>{
							{std::string("Variable is of type:   ") + src_manager.printType(var_type_id)},
							{std::string("Expression is of type: ") + src_manager.printType(*expr_type_id)}
						}
					);

					return false;
				}
			}

		}


		///////////////////////////////////
		// create object

		object::Expr var_value = var_decl.expr;

		const AST::Node& value_node = this->source.getNode(var_decl.expr);

		// get ident value / pointer
		if(value_node.kind == AST::Kind::Ident){
			const Token& value_ident = this->source.getIdent(value_node);
			std::string_view value_ident_str = value_ident.value.string;


			for(const Scope& scope : this->scopes){
				if(scope.vars.contains(value_ident_str)){
					const object::Var::ID value_var_id = scope.vars.at(value_ident_str);

					if(this->is_global_scope()){
						const object::Var& value_var = this->source.getVar(value_var_id);

						// check if value var is uninit
						if(value_var.value.kind == object::Expr::Kind::ASTNode && this->source.getNode(value_var.value.ast_node).kind == AST::Kind::Uninit){
							const Token& value_var_token = this->source.getToken(value_var.ident);
							const Location value_var_location = Location(value_var_token.line_start, value_var_token.collumn_start, value_var_token.collumn_end);

							this->source.warning(
								"declaring global variable with value of another global variable that's has the value of \"uninit\"", value_ident,
								std::vector<Message::Info>{ {std::format("global variable \"{}\" defined here", value_ident_str), value_var_location}, }
							);
						}

						var_value = value_var.value;

					}else{
						var_value = value_var_id;
					}
					
					break;
				}
			}
		}


		const object::Var::ID var_id = this->source.createVar(ident_tok_id, var_type_id, var_value);

		this->add_var_to_scope(ident.value.string, var_id);

		if(this->is_global_scope()){
			this->source.objects.global_vars.emplace_back(var_id);
		}else{
			this->current_func->stmts.emplace_back(var_id);
		}


		return true;
	};





	auto SemanticAnalyzer::analyze_func(const AST::Func& func) noexcept -> bool {
		const Token::ID ident_tok_id = this->source.getNode(func.ident).token;
		const Token& ident = this->source.getToken(ident_tok_id);

		// check function is in global scope
		if(this->is_global_scope() == false){
			this->source.error("Functions can only be defined at global scope", ident);
			return false;
		}


		// check ident is unused
		if(this->has_in_scope(ident.value.string)){
			this->already_defined(ident);
			return false;
		}


		// check typing
		const AST::Type& return_type = this->source.getType(func.return_type);
		const Token& return_type_token = this->source.getToken(return_type.token);

		if(return_type_token.kind != Token::TypeVoid){
			this->source.error("Function return types can only be Void at the moment", return_type_token);
			return false;
		}




		// create object
		const object::Func::ID func_id = this->source.createFunc(ident_tok_id);

		this->add_func_to_scope(ident.value.string, func_id);

		this->current_func = &this->source.objects.funcs[func_id.id];

		// analyze block
		const AST::Block& block = this->source.getBlock(func.block);
		if(this->analyze_block(block) == false){
			return false;
		}


		return true;
	};



	auto SemanticAnalyzer::analyze_block(const AST::Block& block) noexcept -> bool {
		this->enter_scope();

		for(AST::Node::ID node_id : block.nodes){
			if(this->analyze_stmt(this->source.getNode(node_id)) == false){
				return false;
			}
		}

		this->leave_scope();

		return true;
	};




	auto SemanticAnalyzer::get_type_of_expr(const AST::Node& node) const noexcept -> std::optional<object::Type::ID> {
		SourceManager& src_manager = this->source.getSourceManager();


		switch(node.kind){
			case AST::Kind::Literal: {
				const Token& literal_value = this->source.getLiteral(node);

				const Token::Kind base_type = [&]() noexcept {
					switch(literal_value.kind){
						break; case Token::LiteralInt: return Token::TypeInt;
						break; case Token::LiteralBool: return Token::TypeBool;
					};

					EVO_FATAL_BREAK("Unkonwn literal type");
				}(); 



				return src_manager.getTypeID(
					object::Type{
						.base_type = src_manager.getBaseTypeID(base_type),
					}
				);
			} break;


			case AST::Kind::Ident: {
				const Token& ident = this->source.getIdent(node);
				std::string_view ident_str = ident.value.string;

				for(const Scope& scope : this->scopes){
					if(scope.vars.contains(ident_str)){
						const object::Var& var = this->source.getVar( scope.vars.at(ident_str) );
						return var.type;

					}else if(scope.funcs.contains(ident_str)){
						this->source.error("At this time, functions cannot used be expressions", node);
						return std::nullopt;
					}
				}


				this->source.error(std::format("Identifier \"{}\" is undefined", ident_str), node);
				return std::nullopt;
			} break;



			case AST::Kind::Uninit: {
				EVO_FATAL_BREAK("[uinit] exprs should not be analyzed with this function");
			} break;
		};

		EVO_FATAL_BREAK("Unknwon expr type");
	};



	//////////////////////////////////////////////////////////////////////
	// scope

	auto SemanticAnalyzer::enter_scope() noexcept -> void {
		this->scopes.emplace_back();
	};

	auto SemanticAnalyzer::leave_scope() noexcept -> void {
		this->scopes.pop_back();
	};


	auto SemanticAnalyzer::add_var_to_scope(std::string_view str, object::Var::ID id) noexcept -> void {
		this->scopes.back().vars.emplace(str, id);
	};

	auto SemanticAnalyzer::add_func_to_scope(std::string_view str, object::Func::ID id) noexcept -> void {
		this->scopes.back().funcs.emplace(str, id);
	};



	auto SemanticAnalyzer::has_in_scope(std::string_view ident) const noexcept -> bool {
		for(const Scope& scope : this->scopes){
			if(scope.vars.contains(ident)){ return true; }
			if(scope.funcs.contains(ident)){ return true; }
		}

		return false;
	};


	auto SemanticAnalyzer::already_defined(const Token& ident) const noexcept -> void {
		const std::string_view ident_str = ident.value.string;

		const Token& token = [&]() noexcept {
			for(const Scope& scope : this->scopes){
				if(scope.vars.contains(ident_str)){
					const object::Var& var = this->source.getVar( scope.vars.at(ident_str) );
					return this->source.getToken(var.ident);
				}

				if(scope.funcs.contains(ident_str)){
					const object::Func& func = this->source.getFunc( scope.funcs.at(ident_str) );
					return this->source.getToken(func.ident);
				}
			}

			EVO_FATAL_BREAK("Didn't find ident");
		}();





		this->source.error(
			std::format("Identifier \"{}\" already defined", ident.value.string),
			ident,
			std::vector<Message::Info>{
				{"First defined here:", Location(token.line_start, token.collumn_start, token.collumn_end)}
			}
		);
	};

	
};
