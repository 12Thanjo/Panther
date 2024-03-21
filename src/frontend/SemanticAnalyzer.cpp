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
		if(this->current_func != nullptr && this->current_func->returns){
			// TODO: better messaging
			this->source.error("Code after return statement", node);
			return false;
		}

		switch(node.kind){
			break; case AST::Kind::VarDecl: return this->analyze_var(this->source.getVarDecl(node));
			break; case AST::Kind::Func: return this->analyze_func(this->source.getFunc(node));
			break; case AST::Kind::Return: return this->analyze_return(this->source.getReturn(node));
			break; case AST::Kind::Infix: return this->analyze_infix(this->source.getInfix(node));
			break; case AST::Kind::FuncCall: return this->analyze_func_call(this->source.getFuncCall(node));
			break;

			case AST::Kind::Literal: {
				this->source.error("A literal expression cannot be a statement", node);
				return false;
			} break;

			case AST::Kind::Ident: {
				this->source.error("An identifier expression cannot be a statement", node);
				return false;
			} break;

			case AST::Kind::Uninit: {
				this->source.error("An uninit expression cannot be a statement", node);
				return false;
			} break;
		};

		EVO_FATAL_BREAK("This AST Kind is not handled (semantic analysis of stmt)");
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

		const ExprValueType expr_value_type = this->get_expr_value_type(var_decl.expr);
		if(expr_value_type != ExprValueType::Ephemeral){
			// TODO: better messaging
			this->source.error("Variables must be assigned with a ephemeral value", var_decl.expr);
			return false;
		}



		const AST::Type& type = this->source.getType(var_decl.type);
		const Token& type_token = this->source.getToken(type.token);


		SourceManager& src_manager = this->source.getSourceManager();

		const PIR::Type::ID var_type_id = src_manager.getTypeID(
			PIR::Type( src_manager.getBaseTypeID(type_token.kind) )
		);

		
		if(type_token.kind == Token::TypeVoid){
			this->source.error("Variable cannot be of type Void", type.token);
			return false;

		}else{
			const AST::Node& expr_node = this->source.getNode(var_decl.expr);

			if(expr_node.kind != AST::Kind::Uninit){
				const std::optional<PIR::Type::ID> expr_type_id = this->analyze_and_get_type_of_expr(expr_node);

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
		// get / check value

		auto var_value = [&]() noexcept {
			if(this->is_global_scope()){
				const AST::Node& value_node = this->source.getNode(var_decl.expr);

				// get ident value / pointer
				if(value_node.kind == AST::Kind::Ident){
					const Token& value_ident = this->source.getIdent(value_node);
					std::string_view value_ident_str = value_ident.value.string;

					// find the var
					for(const Scope& scope : this->scopes){
						if(scope.vars.contains(value_ident_str)){
							const PIR::Var::ID value_var_id = scope.vars.at(value_ident_str);

							const PIR::Var& value_var = this->source.getVar(value_var_id);

							// check if value var is uninit
							if(
								value_var.value.kind == PIR::Expr::Kind::ASTNode &&
								this->source.getNode(value_var.value.ast_node).kind == AST::Kind::Uninit
							){
								const Token& value_var_token = this->source.getToken(value_var.ident);
								const Location value_var_location = Location(
									value_var_token.line_start, value_var_token.collumn_start, value_var_token.collumn_end
								);

								this->source.warning(
									"declaring global variable with value of another global variable that's has the value of \"uninit\"", value_ident,
									std::vector<Message::Info>{
										{std::format("global variable \"{}\" defined here", value_ident_str), value_var_location},
									}
								);
							}

							return value_var.value;					
						}
					}
				}


				// literal
				return PIR::Expr(var_decl.expr);

			}else{
				return this->get_expr_value(var_decl.expr);
			}
		}();


		

		///////////////////////////////////
		// create object

		const PIR::Var::ID var_id = this->source.createVar(ident_tok_id, var_type_id, var_value);

		this->add_var_to_scope(ident.value.string, var_id);

		if(this->is_global_scope()){
			this->source.pir.global_vars.emplace_back(var_id);
		}else{
			this->current_func->stmts.emplace_back(var_id);
		}



		///////////////////////////////////
		// done

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

		// check attributes
		bool is_export = false;
		bool is_entry = false;
		for(Token::ID attribute : func.attributes){
			const Token& token = this->source.getToken(attribute);
			std::string_view token_str = token.value.string;

			if(token_str == "export"){
				if(this->is_valid_export_name(ident.value.string) == false){
					this->source.error(std::format("Function with attribute \"#export\" cannot be named \"{}\"", ident.value.string), ident);
					return false;
				}

				is_export = true;

			}else if(token_str == "entry"){
				is_entry = true;

			}else{
				this->source.error(std::format("Uknown attribute \"#{}\"", token_str), token);
				return false;
			}
		}


		// check typing
		const AST::Type& return_type = this->source.getType(func.return_type);
		const Token& return_type_token = this->source.getToken(return_type.token);

		// get return type
		std::optional<PIR::Type::ID> return_type_id = std::nullopt;
		if(return_type_token.kind != Token::TypeVoid){
			SourceManager& src_manager = this->source.getSourceManager();

			return_type_id = src_manager.getTypeID(
				PIR::Type( src_manager.getBaseTypeID(return_type_token.kind) )
			);
		}




		///////////////////////////////////
		// create base type

		auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);

		base_type.call_operators.emplace_back(std::vector<PIR::Type::ID>{}, return_type_id);

		const PIR::BaseType::ID base_type_id = this->source.getSourceManager().createBaseType(std::move(base_type));





		///////////////////////////////////
		// create object

		const PIR::Func::ID func_id = this->source.createFunc(ident_tok_id, base_type_id, return_type_id, is_export);

		this->add_func_to_scope(ident.value.string, func_id);

		this->current_func = &this->source.pir.funcs[func_id.id];



		if(is_entry){
			SourceManager& src_manager = this->source.getSourceManager();

			// check is valid return type
			if(return_type_id.has_value() == false || *return_type_id != src_manager.getTypeInt()){
				this->source.error("Function with attribute \"#entry\" must return type Int", ident);
				return false;
			}

			// check there isn't already an entry function defined
			if(src_manager.hasEntry()){
				this->source.error("Already has entry function", ident);
				return false;
			}

			// create entry
			src_manager.addEntry(this->source.getID(), func_id);
		}




		///////////////////////////////////
		// analyze block

		const AST::Block& block = this->source.getBlock(func.block);
		if(this->analyze_block(block) == false){
			return false;
		}

		if(return_type_id.has_value() && this->current_func->returns == false){
			this->source.error("Function with return type does not return", ident);
			return false;
		}

		this->current_func = nullptr;



		///////////////////////////////////
		// done

		return true;
	};



	auto SemanticAnalyzer::analyze_return(const AST::Return& return_stmt) noexcept -> bool {
		if(this->current_func == nullptr){
			this->source.error("Return statements can only be inside functions", return_stmt.keyword);
			return false;
		}


		std::optional<PIR::Expr> return_value = std::nullopt;

		if(return_stmt.value.has_value()){
			// return expr;

			if(this->current_func->return_type.has_value() == false){
				this->source.error("Return statement has value when function's return type is \"Void\"", return_stmt.keyword);
				return false;	
			}

			const AST::Node& expr_node = this->source.getNode(*return_stmt.value);

			const std::optional<PIR::Type::ID> expr_type_id = this->analyze_and_get_type_of_expr(expr_node);
			if(expr_type_id.has_value() == false){ return false; }

			if(*expr_type_id != *this->current_func->return_type){
				SourceManager& src_manager = this->source.getSourceManager();

				this->source.error(
					"Return value type and function return type do not match", 
					expr_node,

					std::vector<Message::Info>{
						{std::string("Function return is type: ") + src_manager.printType(*this->current_func->return_type)},
						{std::string("Return value is of type: ") + src_manager.printType(*expr_type_id)}
					}
				);

				return false;
			}

			return_value = this->get_expr_value(*return_stmt.value);


		}else{
			// return;
			
			if(this->current_func->return_type.has_value()){
				this->source.error("Return statement has no value when function's return type is not \"Void\"", return_stmt.keyword);
				return false;
			}
		}



		const PIR::Return::ID ret_id = this->source.createReturn(return_value);
		this->current_func->stmts.emplace_back(ret_id);

		this->current_func->returns = true;

		return true;
	};




	auto SemanticAnalyzer::analyze_infix(const AST::Infix& infix) noexcept -> bool {
		// for now since there are no other infix operations yet
		return this->analyze_assignment(infix);
	};


	auto SemanticAnalyzer::analyze_assignment(const AST::Infix& infix) noexcept -> bool {
		// check if ident exists
		const Token& ident = this->source.getIdent(infix.lhs);
		if(this->has_in_scope(ident.value.string) == false){
			this->source.error("Attemted to assign a value to a variable that does not exist", ident);
			return false;
		}


		///////////////////////////////////
		// check that the ident is a variable, and get it

		std::optional<PIR::Var::ID> var_id;

		for(Scope& scope : this->scopes){
			if(scope.vars.contains(ident.value.string)){
				// is a var
				var_id = scope.vars.at(ident.value.string);
				break;

			}else if(scope.funcs.contains(ident.value.string)){
				// is a function
				this->source.error("Cannot assign a value to a function", ident);
				return false;
			}
		}


		const PIR::Var& var = this->source.getVar(*var_id);


		///////////////////////////////////
		// check typing

		const ExprValueType expr_value_type = this->get_expr_value_type(infix.rhs);
		if(expr_value_type != ExprValueType::Ephemeral){
			// TODO: better messaging
			this->source.error("Variables must be assigned with an ephemeral value", infix.rhs);
			return false;
		}


		const std::optional<PIR::Type::ID> expr_type = this->analyze_and_get_type_of_expr(this->source.getNode(infix.rhs));
		if(expr_type.has_value() == false){ return false; }

		if(*expr_type != var.type){
			SourceManager& src_manager = this->source.getSourceManager();

			this->source.error(
				"Attempted to assign a value to a variable of a different type",
				infix.rhs,

				std::vector<Message::Info>{
					{std::string("Variable is of type:   ") + src_manager.printType(var.type)},
					{std::string("Expression is of type: ") + src_manager.printType(*expr_type)}
				}
			);
		}


		///////////////////////////////////
		// create object

		const PIR::Assignment::ID assignment_id = this->source.createAssignment(*var_id, infix.op, this->get_expr_value(infix.rhs));
		this->current_func->stmts.emplace_back(assignment_id);

		return true;
	};




	auto SemanticAnalyzer::analyze_func_call(const AST::FuncCall& func_call) noexcept -> bool {
		// analyze and get type of ident
		const std::optional<PIR::Type::ID> target_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(func_call.target));
		if(target_type_id.has_value() == false){ return false; }

		const SourceManager& src_manager = this->source.getSourceManager();

		// check that it's a function type
		const PIR::Type& type = src_manager.getType(*target_type_id);
		const PIR::BaseType& base_type = src_manager.getBaseType(type.base_type);
		if(base_type.call_operators.empty()){
			// TODO: better messaging?
			this->source.error(
				"cannot be called like a function", func_call.target,
				std::vector<Message::Info>{ std::format("Type \"{}\" does not have a call operator", src_manager.printType(*target_type_id)) }
			);
			return false;
		}

		///////////////////////////////////
		// get_func

		const PIR::Func::ID func_id = [&]() noexcept {
			for(Scope& scope : this->scopes){
				auto find = scope.funcs.find(this->source.getIdent(func_call.target).value.string);
				if(find != scope.funcs.end()){
					return find->second;
				}
			}

			EVO_FATAL_BREAK("Unknown func ident");
		}();


		///////////////////////////////////
		// create object

		const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(func_id);
		this->current_func->stmts.emplace_back(func_call_id);

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




	auto SemanticAnalyzer::analyze_and_get_type_of_expr(const AST::Node& node) const noexcept -> std::optional<PIR::Type::ID> {
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
					PIR::Type( src_manager.getBaseTypeID(base_type) )
				);
			} break;


			case AST::Kind::Ident: {
				const Token& ident = this->source.getIdent(node);
				std::string_view ident_str = ident.value.string;

				for(const Scope& scope : this->scopes){
					if(scope.vars.contains(ident_str)){
						const PIR::Var& var = this->source.getVar(scope.vars.at(ident_str));
						return var.type;

					}else if(scope.funcs.contains(ident_str)){
						const PIR::Func& func = this->source.getFunc(scope.funcs.at(ident_str));
						return src_manager.getTypeID(PIR::Type(func.base_type));
					}
				}


				this->source.error(std::format("Identifier \"{}\" is undefined", ident_str), node);
				return std::nullopt;
			} break;


			case AST::Kind::FuncCall: {
				const AST::FuncCall& func_call = this->source.getFuncCall(node);

				// get target type
				const std::optional<PIR::Type::ID> target_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(func_call.target));
				if(target_type_id.has_value() == false){ return std::nullopt; }


				// check that it's a function type
				const PIR::Type& type = src_manager.getType(*target_type_id);
				const PIR::BaseType& base_type = src_manager.getBaseType(type.base_type);
				if(base_type.call_operators.empty()){
					// TODO: better messaging?
					this->source.error(
						"cannot be called like a function", func_call.target,
						std::vector<Message::Info>{ std::format("Type \"{}\" does not have a call operator", src_manager.printType(*target_type_id)) }
					);
					return std::nullopt;
				}


				const std::optional<PIR::Type::ID> return_type = base_type.call_operators[0].return_type;
				if(return_type.has_value() == false){
					this->source.error("This function does not return a value", func_call.target);
					return std::nullopt;
				}

				return *return_type;

			} break;


			case AST::Kind::Prefix: {
				const AST::Prefix& prefix = this->source.getPrefix(node);

				switch(this->source.getToken(prefix.op).kind){
					case Token::KeywordCopy: {
						const ExprValueType expr_value_type = this->get_expr_value_type(prefix.rhs);
						if(expr_value_type != ExprValueType::Concrete){
							this->source.error("right-hand-side of copy expression must be a concrete expression", prefix.rhs);
							return std::nullopt;
						}

						return this->analyze_and_get_type_of_expr(this->source.getNode(prefix.rhs));
					} break;

					default: EVO_FATAL_BREAK("Unknown prefix operator - SemanticAnalyzer::analyze_and_get_type_of_expr");
				};

			} break;



			case AST::Kind::Uninit: {
				EVO_FATAL_BREAK("[uninit] exprs should not be analyzed with this function - SemanticAnalyzer::analyze_and_get_type_of_expr");
			} break;
		};

		EVO_FATAL_BREAK("Unknown expr type - SemanticAnalyzer::analyze_and_get_type_of_expr");
	};




	auto SemanticAnalyzer::get_expr_value(AST::Node::ID node_id) const noexcept -> PIR::Expr {
		evo::debugAssert(this->is_global_scope() == false, "SemanticAnalyzer::get_expr_value() is not for use in global variables");


		const AST::Node& value_node = this->source.getNode(node_id);


		switch(value_node.kind){
			case AST::Kind::Ident: {
				const Token& value_ident = this->source.getIdent(value_node);
				std::string_view value_ident_str = value_ident.value.string;

				// find the var
				for(const Scope& scope : this->scopes){
					if(scope.vars.contains(value_ident_str)){
						return PIR::Expr(scope.vars.at(value_ident_str));
					}
				}

				EVO_FATAL_BREAK("Didn't find value_ident");
			} break;


			case AST::Kind::FuncCall: {
				const AST::FuncCall& func_call = this->source.getFuncCall(node_id);


				const PIR::Func::ID func_id = [&]() noexcept {
					for(const Scope& scope : this->scopes){
						auto find = scope.funcs.find(this->source.getIdent(func_call.target).value.string);
						if(find != scope.funcs.end()){
							return find->second;
						}
					}

					EVO_FATAL_BREAK("Unknown func ident");
				}();


				const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(func_id);
				return PIR::Expr(func_call_id);
			} break;


			case AST::Kind::Prefix: {
				const AST::Prefix& prefix = this->source.getPrefix(node_id);

				const PIR::Prefix::ID prefix_id = this->source.createPrefix(prefix.op, this->get_expr_value(prefix.rhs));
				return PIR::Expr(prefix_id);
			} break;


			default: {
				return PIR::Expr(node_id);
			} break;
		};

	};



	auto SemanticAnalyzer::get_expr_value_type(AST::Node::ID node_id) const noexcept -> ExprValueType {
		const AST::Node& value_node = this->source.getNode(node_id);

		switch(value_node.kind){
			break; case AST::Kind::Prefix: return ExprValueType::Ephemeral;
			break; case AST::Kind::FuncCall: return ExprValueType::Ephemeral;

			break; case AST::Kind::Ident: return ExprValueType::Concrete;
			break; case AST::Kind::Literal: return ExprValueType::Ephemeral;
			break; case AST::Kind::Uninit: return ExprValueType::Ephemeral;
		};

		EVO_FATAL_BREAK("Unknown AST::Node::Kind - SemanticAnalyzer::get_expr_value_type()");
	};







	// TODO: check for exported functions in all files (through saving in SourceManager)
	auto SemanticAnalyzer::is_valid_export_name(std::string_view name) const noexcept -> bool {
		if(name == "main"){ return false; }

		return true;
	};



	//////////////////////////////////////////////////////////////////////
	// scope

	auto SemanticAnalyzer::enter_scope() noexcept -> void {
		this->scopes.emplace_back();
	};

	auto SemanticAnalyzer::leave_scope() noexcept -> void {
		this->scopes.pop_back();
	};


	auto SemanticAnalyzer::add_var_to_scope(std::string_view str, PIR::Var::ID id) noexcept -> void {
		this->scopes.back().vars.emplace(str, id);
	};

	auto SemanticAnalyzer::add_func_to_scope(std::string_view str, PIR::Func::ID id) noexcept -> void {
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
					const PIR::Var& var = this->source.getVar( scope.vars.at(ident_str) );
					return this->source.getToken(var.ident);
				}

				if(scope.funcs.contains(ident_str)){
					const PIR::Func& func = this->source.getFunc( scope.funcs.at(ident_str) );
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
