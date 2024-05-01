#include "SemanticAnalyzer.h"

#include "frontend/SourceManager.h"

namespace panther{
	

	auto SemanticAnalyzer::semantic_analysis_declarations() noexcept -> bool {
		this->enter_scope(nullptr);

		for(AST::Node::ID global_stmt : this->source.global_stmts){
			const AST::Node& node = this->source.getNode(global_stmt);

			switch(node.kind){
				case AST::Kind::VarDecl: {
					if(this->analyze_var(this->source.getVarDecl(node)) == false){ return false; }
				} break;
			};
		}

		for(AST::Node::ID global_stmt : this->source.global_stmts){
			const AST::Node& node = this->source.getNode(global_stmt);

			if(node.kind != AST::Kind::VarDecl){
				if(this->analyze_stmt(node) == false){
					return false;
				}
			}
		}

		return true;
	};


	auto SemanticAnalyzer::semantic_analysis() noexcept -> bool {
		for(const GlobalFunc& global_func : this->global_funcs){
			PIR::Func& pir_func = this->source.pir.funcs[global_func.pir_id.id];
			if(this->analyze_func_block(pir_func, global_func.ast) == false){
				return false;
			}
		}

		this->leave_scope();
		return true;
	};



	auto SemanticAnalyzer::analyze_stmt(const AST::Node& node) noexcept -> bool {
		if(this->in_func_scope() && this->scope_is_terminated()){
			// TODO: better messaging
			this->source.error("Unreachable code", node);
			return false;
		}

		switch(node.kind){
			break; case AST::Kind::VarDecl: return this->analyze_var(this->source.getVarDecl(node));
			break; case AST::Kind::Func: return this->analyze_func(this->source.getFunc(node));
			break; case AST::Kind::Conditional: return this->analyze_conditional(this->source.getConditional(node));
			break; case AST::Kind::Return: return this->analyze_return(this->source.getReturn(node));
			break; case AST::Kind::Infix: return this->analyze_infix(this->source.getInfix(node));
			break; case AST::Kind::FuncCall: return this->analyze_func_call(this->source.getFuncCall(node));
			break; case AST::Kind::Unreachable: return this->analyze_unreachable(this->source.getUnreachable(node));
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

		EVO_FATAL_BREAK("unknown ast kind");
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
		// checking attributes

		bool is_pub = false;
		bool is_export = false;
		for(Token::ID attribute : var_decl.attributes){
			const Token& token = this->source.getToken(attribute);
			std::string_view token_str = token.value.string;

			if(token_str == "pub"){
				if(this->is_global_scope()){
					is_pub = true;
				}else{
					// TODO: maybe this should be an error instead?
					this->source.warning("Only variables at global scope can be marked with the attribute #pub - ignoring", token);
				}

			}else if(token_str == "export"){
				if(this->is_global_scope() == false){
					this->source.error("Only variables at global scope can be marked with the attribute #export - ignoring", token);
					return false;
				}

				if(this->is_valid_export_name(ident.value.string) == false){
					this->source.error(std::format("Variables with attribute \"#export\" cannot be named \"{}\"", ident.value.string), ident);
					return false;
				}

				if(this->src_manager.hasExport(ident.value.string)){
					// TODO: better messaging
					this->source.error(
						std::format("Already exported a identifier named \"{}\"", ident.value.string), ident,
						std::vector<Message::Info>{ Message::Info("Location information for first export location not supported yet") }
					);
					return false;
				}

				this->src_manager.addExport(ident.value.string);
				is_export = true;

			}else{
				// TODO: better messaging
				this->source.error(std::format("Unknown attribute \"#{}\"", token_str), token);
				return false;
			}
		}


		///////////////////////////////////
		// type checking

		const evo::Result<ExprValueType> expr_value_type = this->get_expr_value_type(var_decl.expr);
		if(expr_value_type.isError()){ return false; }
		if(expr_value_type.value() != ExprValueType::Ephemeral && expr_value_type.value() != ExprValueType::Import){
			// TODO: better messaging
			this->source.error("Variables must be assigned with an ephemeral value", var_decl.expr);
			return false;
		}
		const AST::Node& expr_node = this->source.getNode(var_decl.expr);

		auto var_type_id = std::optional<PIR::Type::ID>();
		if(var_decl.type.has_value()){
			const evo::Result<PIR::Type::VoidableID> var_type_id_result = this->get_type_id(*var_decl.type);
			if(var_type_id_result.isError()){ return false; }

			if(var_type_id_result.value().isVoid()){
				this->source.error("Variable cannot be of type Void", *var_decl.type);
				return false;
			}

			var_type_id = var_type_id_result.value().typeID();


			if(expr_node.kind != AST::Kind::Uninit){
				const evo::Result<PIR::Type::ID> expr_type_id = this->analyze_and_get_type_of_expr(expr_node);
				if(expr_type_id.isError()){ return false; }

				if(*var_type_id != expr_type_id.value()){
					const PIR::Type& var_type = this->src_manager.getType(*var_type_id);
					const PIR::Type& expr_type = this->src_manager.getType(expr_type_id.value());

					if(this->is_implicitly_convertable_to(expr_type, var_type, expr_node) == false){
						this->source.error(
							"Variable cannot be assigned a value of a different type, and cannot be implicitly converted", 
							expr_node,

							std::vector<Message::Info>{
								{std::string("Variable is of type:   ") + this->src_manager.printType(*var_type_id)},
								{std::string("Expression is of type: ") + this->src_manager.printType(expr_type_id.value())}
							}
						);
						return false;
					}

				}
			}

		}else{
			// type inference

			if(expr_node.kind == AST::Kind::Uninit){
				this->source.error("The type of [uninit] cannot be inferenced", expr_node);
				return false;
			}

			const evo::Result<PIR::Type::ID> expr_type_id = this->analyze_and_get_type_of_expr(expr_node);
			if(expr_type_id.isError()){ return false; }

			if(expr_type_id.value() == SourceManager::getTypeString()){
				this->source.error("String literal values (outside of calls to `@import()`) are not supported yet", expr_node);
				return false;

			}

			var_type_id = expr_type_id.value();
		}



		///////////////////////////////////
		// get / check value

		const evo::Result<PIR::Expr> var_value = [&]() noexcept {
			if(this->is_global_scope()){
				return this->get_const_expr_value(var_decl.expr);

			}else{
				return this->get_expr_value(var_decl.expr);
			}
		}();

		if(var_value.isError()){ return false; }


		// handle imports differently
		if(var_type_id == SourceManager::getTypeImport()){
			evo::debugAssert(var_value.value().kind == PIR::Expr::Kind::Import, "should be import");

			if(var_decl.isDef == false){
				this->source.error("import variables must be marked [def] not [var]", var_decl.ident);
				return false;
			}

			this->add_import_to_scope(ident.value.string, Import(var_value.value().import, var_decl.ident));

			if(is_pub){
				this->source.addPublicImport(ident.value.string, var_value.value().import);
			}

			return true;
		}



		// check for uninit
		if(var_value.value().kind == PIR::Expr::Kind::ASTNode){
			const AST::Node& var_value_node = this->source.getNode(var_value.value().astNode);

			if(var_value_node.kind == AST::Kind::Uninit){
				if(this->is_global_scope()){
					this->source.error("Global variables cannot be initialized with the value \"uninit\"", var_decl.expr);
					return false;
				}

				if(var_decl.isDef){
					this->source.warning(
						"Declared a def variable with the value \"uninit\"", var_decl.expr,
						std::vector<Message::Info>{ {"Any use of this variable would be undefined behavior"} }
					);
				}
			}
		}
		

		///////////////////////////////////
		// create object

		const PIR::Var::ID var_id = this->source.createVar(ident_tok_id, *var_type_id, var_value.value(), var_decl.isDef, is_export);

		this->add_var_to_scope(ident.value.string, var_id);

		if(this->is_global_scope()){
			this->source.pir.global_vars.emplace_back(var_id);
		}else{
			this->get_stmts_entry().emplace_back(var_id);
		}


		if(is_pub){
			this->source.addPublicVar(ident.value.string, var_id);
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


		///////////////////////////////////
		// params

		auto params = std::vector<PIR::Param::ID>();
		auto param_type_ids = std::vector<PIR::BaseType::Operator::Param>();

		// this is just for params
		this->enter_scope(nullptr);

		const AST::FuncParams& ast_func_params = this->source.getFuncParams(func.params);
		for(const AST::FuncParams::Param& param : ast_func_params.params){
			const Token::ID param_ident_tok_id = this->source.getNode(param.ident).token;
			const Token& param_ident = this->source.getToken(param_ident_tok_id);

			if(this->has_in_scope(param_ident.value.string)){
				this->already_defined(param_ident);
				return false;
			}


			// get param type
			const evo::Result<PIR::Type::VoidableID> param_type_id = this->get_type_id(param.type);
			if(param_type_id.isError()){ return false; }

			if(param_type_id.value().isVoid()){
				this->source.error("Function parameters cannot be of type Void", param.type);
				return false;
			}


			param_type_ids.emplace_back(param_type_id.value().typeID(), param.kind);

			const PIR::Param::ID param_id = this->source.createParam(param_ident_tok_id, param_type_id.value().typeID(), param.kind);
			this->add_param_to_scope(param_ident.value.string, param_id);
			params.emplace_back(param_id);
		}


		this->leave_scope();


		///////////////////////////////////
		// attributes

		bool is_export = false;
		bool is_entry = false;
		bool is_pub = false;
		for(Token::ID attribute : func.attributes){
			const Token& token = this->source.getToken(attribute);
			std::string_view token_str = token.value.string;

			if(token_str == "export"){
				if(this->is_valid_export_name(ident.value.string) == false){
					this->source.error(std::format("Function with attribute \"#export\" cannot be named \"{}\"", ident.value.string), ident);
					return false;
				}

				if(params.empty() == false){
					this->source.error("Function with attribute \"#export\" cannot have arguments yet", ident);
					return false;
				}

				if(this->src_manager.hasExport(ident.value.string)){
					// TODO: better messaging
					this->source.error(
						std::format("Already exported an identifier named \"{}\"", ident.value.string), ident,
						std::vector<Message::Info>{ Message::Info("Location information for first export location not supported yet") }
					);
					return false;
				}

				this->src_manager.addExport(ident.value.string);
				is_export = true;

			}else if(token_str == "entry"){
				is_entry = true;

			}else if(token_str == "pub"){
				if(this->is_global_scope()){
					is_pub = true;
				}else{
					// TODO: maybe this should be an error instead?
					this->source.warning("Only functions at global scope can be marked with the attribute #pub - ignoring", token);
				}

			}else{
				// TODO: better messaging
				this->source.error(std::format("Unknown attribute \"#{}\"", token_str), token);
				return false;
			}
		}


		///////////////////////////////////
		// return type

		const evo::Result<PIR::Type::VoidableID> return_type_id = this->get_type_id(func.returnType);
		if(return_type_id.isError()){ return false; }


		///////////////////////////////////
		// create base type

		auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
		base_type.callOperator = PIR::BaseType::Operator(std::move(param_type_ids), return_type_id.value());

		const PIR::BaseType::ID base_type_id = this->src_manager.createBaseType(std::move(base_type));


		///////////////////////////////////
		// check for overload reuse

		for(const Scope& scope : this->scopes){
			using ConstFuncScopeListIter = std::unordered_map<std::string_view, std::vector<PIR::Func::ID>>::const_iterator;
			ConstFuncScopeListIter func_scope_list_iter = scope.funcs.find(ident.value.string);
			if(func_scope_list_iter == scope.funcs.end()){ continue; }

			for(PIR::Func::ID existing_func_id : func_scope_list_iter->second){
				const PIR::Func& existing_func = this->source.getFunc(existing_func_id);

				if(existing_func.baseType == base_type_id){
					this->source.error(
						"Function with same prototype already defined", ident,
						std::vector<Message::Info>{ Message::Info("First defined here:", this->source.getToken(existing_func.ident).location) }
					);
					return false;
				}
			}
		}


		///////////////////////////////////
		// create object

		const PIR::Func::ID func_id = this->source.createFunc(ident_tok_id, base_type_id, std::move(params), return_type_id.value(), is_export);

		this->add_func_to_scope(ident.value.string, func_id);




		if(is_entry){
			// check is valid return type
			if(return_type_id.value().isVoid() || return_type_id.value().typeID() != this->src_manager.getTypeInt()){
				this->source.error("Function with attribute \"#entry\" must return type Int", ident);
				return false;
			}

			// check there isn't already an entry function defined
			if(this->src_manager.hasEntry()){
				this->source.error("Already has entry function", ident);
				return false;
			}

			// create entry
			this->src_manager.addEntry(this->source.getID(), func_id);
		}


		///////////////////////////////////
		// analyze block

		if(this->is_global_scope() == false){
			PIR::Func& pir_func = this->source.pir.funcs[func_id.id];
			if(this->analyze_func_block(pir_func, func) == false){
				return false;
			};

		}else{
			this->global_funcs.emplace_back(func_id, func);

			if(is_pub){
				this->source.addPublicFunc(ident.value.string, func_id);
			}
		}


		///////////////////////////////////
		// done

		return true;
	};



	auto SemanticAnalyzer::analyze_func_block(PIR::Func& pir_func, const AST::Func& ast_func) noexcept -> bool {
		this->enter_func_scope(pir_func);

			this->enter_scope(&pir_func.stmts);
				this->enter_scope_level();
					this->add_scope_level_scope();

					// add the params into the scope
					for(PIR::Param::ID param_id : pir_func.params){
						const PIR::Param& param = this->source.getParam(param_id);
						this->add_param_to_scope(this->source.getToken(param.ident).value.string, param_id);
					}

					// analyze the statements in the block
					const AST::Block& block = this->source.getBlock(ast_func.block);
					if(this->analyze_block(block) == false){ return false; }

				this->leave_scope_level();

			// check if function is terminated
			if(pir_func.returnType.isVoid()){
				if(this->scope_is_terminated()){
					pir_func.stmts.setTerminated();
				}
			}else{
				if(this->scope_is_terminated() == false){
					this->source.error("Function with return type does not return on all control paths", pir_func.ident);
					return false;
				}
			}

			this->leave_scope();
		this->leave_func_scope();


		for(PIR::Param::ID param_id : pir_func.params){
			const PIR::Param& param = this->source.getParam(param_id);

			if(param.kind == AST::FuncParams::Param::Kind::Write && param.mayHaveBeenEdited == false){
				this->source.warning("write parameter was not written to in any control path", param.ident);
			}
		}



		return true;
	};






	auto SemanticAnalyzer::analyze_conditional(const AST::Conditional& cond) noexcept -> bool {
		this->enter_scope_level();

		const bool analyze_conditional_result = analyze_conditional_recursive(cond);

		this->leave_scope_level();

		return analyze_conditional_result;
	};


	auto SemanticAnalyzer::analyze_conditional_recursive(const AST::Conditional& sub_cond) noexcept -> bool {
		if(this->in_func_scope() == false){
			this->source.error("Conditional statements can only be inside functions", sub_cond.ifTok);
			return false;
		}


		///////////////////////////////////
		// condition

		const AST::Node& cond_expr = this->source.getNode(sub_cond.ifExpr);

		const evo::Result<PIR::Type::ID> cond_type_id = this->analyze_and_get_type_of_expr(cond_expr);
		if(cond_type_id.isError()){ return false; }

		if(cond_type_id.value() != this->src_manager.getTypeBool()){
			this->source.error(
				"Conditional expression must be a boolean", cond_expr,
				std::vector<Message::Info>{
					{std::string("Conditional expression is of type: ") + this->src_manager.printType(cond_type_id.value())}
				}
			);
			return false;
		}


		///////////////////////////////////
		// setup stmt blocks

		auto then_stmts = PIR::StmtBlock();
		auto else_stmts = PIR::StmtBlock();


		///////////////////////////////////
		// then block

		this->add_scope_level_scope();
		const AST::Block& then_block = this->source.getBlock(sub_cond.thenBlock);
		if(this->analyze_block(then_block, then_stmts) == false){ return false; }


		///////////////////////////////////
		// else block

		if(sub_cond.elseBlock.has_value()){
			const AST::Node& else_block_node = this->source.getNode(*sub_cond.elseBlock);

			if(else_block_node.kind == AST::Kind::Block){
				this->add_scope_level_scope();

				const AST::Block& else_block = this->source.getBlock(else_block_node);
				if(this->analyze_block(else_block, else_stmts) == false){ return false; }

			}else if(else_block_node.kind == AST::Kind::Conditional){
				this->enter_scope(&else_stmts);

				const AST::Conditional& else_block = this->source.getConditional(else_block_node);
				if(this->analyze_conditional_recursive(else_block) == false){ return false; }		

				this->leave_scope();

			}else{
				EVO_FATAL_BREAK("Unkonwn else block kind");
			}

		}else{
			this->add_scope_level_scope();
		}


		///////////////////////////////////
		// create object

		const evo::Result<PIR::Expr> if_expr_value = this->get_expr_value(sub_cond.ifExpr);
		if(if_expr_value.isError()){ return false; }

		const PIR::Conditional::ID cond_id = this->source.createConditional(if_expr_value.value(), std::move(then_stmts), std::move(else_stmts));
		this->get_stmts_entry().emplace_back(cond_id);



		///////////////////////////////////
		// done

		return true;
	};







	auto SemanticAnalyzer::analyze_return(const AST::Return& return_stmt) noexcept -> bool {
		if(this->in_func_scope() == false){
			this->source.error("Return statements can only be inside functions", return_stmt.keyword);
			return false;
		}

		const PIR::Type::VoidableID func_return_type_id = this->get_current_func().func.returnType;

		std::optional<PIR::Expr> return_value = std::nullopt;

		if(return_stmt.value.has_value()){
			// "return expr;"

			if(this->get_current_func().func.returnType.isVoid()){
				this->source.error("Return statement has value when function's return type is \"Void\"", return_stmt.keyword);
				return false;	
			}

			const AST::Node& expr_node = this->source.getNode(*return_stmt.value);

			const evo::Result<PIR::Type::ID> expr_type_id = this->analyze_and_get_type_of_expr(expr_node);
			if(expr_type_id.isError()){ return false; }


			if(expr_type_id.value() != func_return_type_id.typeID()){
				const PIR::Type& func_return_type = this->src_manager.getType(func_return_type_id.typeID());
				const PIR::Type& expr_type = this->src_manager.getType(expr_type_id.value());

				if(this->is_implicitly_convertable_to(expr_type, func_return_type, expr_node) == false){
					this->source.error(
						"Return value type and function return type do not match", 
						expr_node,

						std::vector<Message::Info>{
							{std::string("Function return is type: ") + this->src_manager.printType(func_return_type_id.typeID())},
							{std::string("Return value is of type: ") + this->src_manager.printType(expr_type_id.value())}
						}
					);

					return false;
				}
			}

			const evo::Result<PIR::Expr> return_value_expr = this->get_expr_value(*return_stmt.value);
			if(return_value_expr.isError()){ return false; }
			return_value.emplace(return_value_expr.value());

		}else{
			// "return;"
			
			if(func_return_type_id.isVoid() == false){
				this->source.error("Return statement has no value but the function's return type is not \"Void\"", return_stmt.keyword);
				return false;
			}
		}



		const PIR::Return::ID ret_id = this->source.createReturn(return_value);
		this->get_stmts_entry().emplace_back(ret_id);
		this->get_stmts_entry().setTerminated();
		if(this->is_in_func_base_scope()){
			this->get_current_func().func.terminatesInBaseScope = true;
		}

		this->set_scope_terminated();
		this->add_scope_level_terminated();


		return true;
	};




	auto SemanticAnalyzer::analyze_infix(const AST::Infix& infix) noexcept -> bool {
		// for now since there are no other infix operations yet
		return this->analyze_assignment(infix);
	};



	auto SemanticAnalyzer::analyze_assignment(const AST::Infix& infix) noexcept -> bool {
		// check if at global scope
		if(this->is_global_scope()){
			this->source.error("Assignment statements are not allowed at global scope", infix.op);
			return false;
		}


		///////////////////////////////////
		// checking of lhs

		const evo::Result<ExprValueType> dst_value_type = this->get_expr_value_type(infix.lhs);
		if(dst_value_type.isError()){ return false; }
		if(dst_value_type.value() != ExprValueType::Concrete){
			this->source.error("Only concrete values may be assigned to", infix.lhs);
			return false;
		}

		const evo::Result<PIR::Type::ID> dst_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(infix.lhs));
		if(dst_type_id.isError()){ return false; }

		if(this->is_expr_mutable(infix.lhs) == false){
			this->source.error("Only mutable values may be assigned to", infix.lhs);
			return false;
		}

		const evo::Result<PIR::Expr> lhs_value = this->get_expr_value(infix.lhs);
		if(lhs_value.isError()){ return false; }

		if(lhs_value.value().kind == PIR::Expr::Kind::Param){
			PIR::Param& param = this->source.getParam(lhs_value.value().param);
			param.mayHaveBeenEdited = true;
		}


		///////////////////////////////////
		// checking of rhs

		const evo::Result<ExprValueType> expr_value_type = this->get_expr_value_type(infix.rhs);
		if(expr_value_type.isError()){ return false; }
		if(expr_value_type.value() != ExprValueType::Ephemeral){
			this->source.error("Only ephemeral values may be assignment values", infix.rhs);
			return false;
		}

		const AST::Node& rhs_node = this->source.getNode(infix.rhs);
		const evo::Result<PIR::Type::ID> expr_type_id = this->analyze_and_get_type_of_expr(rhs_node);
		if(expr_type_id.isError()){ return false; }

		const evo::Result<PIR::Expr> rhs_value = this->get_expr_value(infix.rhs);
		if(rhs_value.isError()){ return false; }


		///////////////////////////////////
		// type checking

		if(dst_type_id.value() != expr_type_id.value()){
			const PIR::Type& dst_type = this->src_manager.getType(dst_type_id.value());
			const PIR::Type& expr_type = this->src_manager.getType(expr_type_id.value());

			if(this->is_implicitly_convertable_to(expr_type, dst_type, rhs_node) == false){
				this->source.error(
					"The types of the left-hand-side and right-hand-side of an assignment statement do not match, and cannot be implicitly converted",
					infix.op,

					std::vector<Message::Info>{
						{std::string("left-hand-side is of type:  ") + this->src_manager.printType(dst_type_id.value())},
						{std::string("right-hand-side is of type: ") + this->src_manager.printType(expr_type_id.value())}
					}
				);
				return false;
			}
		}


		///////////////////////////////////
		// create object

		const PIR::Assignment::ID assignment_id = this->source.createAssignment(lhs_value.value(), infix.op, rhs_value.value());
		this->get_stmts_entry().emplace_back(assignment_id);


		return true;
	};




	auto SemanticAnalyzer::analyze_func_call(const AST::FuncCall& func_call) noexcept -> bool {
		// analyze and get type of ident
		const evo::Result<PIR::Type::ID> target_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(func_call.target), &func_call);
		if(target_type_id.isError()){ return false; }

		// check if discarding return value
		const PIR::Type& target_type = this->src_manager.getType(target_type_id.value());
		const PIR::BaseType& target_base_type = this->src_manager.getBaseType(target_type.baseType);
		if(target_base_type.callOperator->returnType.isVoid() == false){
			this->source.error("Discarding return value of function call", func_call.target);
			return false;
		}

		if(this->check_func_call(func_call, target_type_id.value()) == false){ return false; }

		const std::vector<PIR::Expr> args = this->get_func_call_args(func_call);
		switch(this->source.getNode(func_call.target).kind){
			case AST::Kind::Ident: {
				const Token& ident_tok = this->source.getIdent(func_call.target);

				// get func
				const evo::Result<PIR::Func::ID> func_id = this->lookup_func_in_scope(ident_tok.value.string, func_call);
				if(func_id.isError()){ return false; }


				// create object
				const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(func_id.value(), std::move(args));
				this->get_stmts_entry().emplace_back(func_call_id);

				return true;
			} break;
			
			case AST::Kind::Intrinsic: {
				const Token& intrinsic_tok = this->source.getIntrinsic(func_call.target);

				const evo::ArrayProxy<PIR::Intrinsic> intrinsics = this->src_manager.getIntrinsics();
				for(size_t i = 0; i < intrinsics.size(); i+=1){
					const PIR::Intrinsic& intrinsic = intrinsics[i];

					if(intrinsic.ident == intrinsic_tok.value.string){
						// create object
						const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(PIR::Intrinsic::ID(uint32_t(i)), std::move(args));
						this->get_stmts_entry().emplace_back(func_call_id);
						
						return true;
					}
				}


				EVO_FATAL_BREAK("Unknown intrinsic func");
			} break;


			case AST::Kind::Infix: {
				const AST::Infix& infix = this->source.getInfix(func_call.target);

				switch(this->source.getToken(infix.op).kind){
					case Token::get("."): {
						const evo::Result<PIR::Expr> value_of_lhs = this->get_expr_value(infix.lhs);
						evo::debugAssert(value_of_lhs.isSuccess(), "uncaught error");
						evo::debugAssert(value_of_lhs.value().kind == PIR::Expr::Kind::Import, "incorrect expr kind gotten");

						const Source& import_source = this->src_manager.getSource(value_of_lhs.value().import);
						const Token& rhs_ident = this->source.getIdent(infix.rhs);

						const evo::Result<PIR::Func::ID> imported_func_id = this->lookup_func_in_import(rhs_ident.value.string, import_source, func_call);
						if(imported_func_id.isError()){ return false; }
						
						// create object
						const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(imported_func_id.value(), std::move(args));
						this->get_stmts_entry().emplace_back(func_call_id);

						return true;
					} break;
				};

				EVO_FATAL_BREAK("Unknown or unsupported infix type");
			} break;
		};
		
		EVO_FATAL_BREAK("Unknown or unsupported func call target");
	};



	auto SemanticAnalyzer::analyze_unreachable(const Token& unreachable) noexcept -> bool {
		if(this->in_func_scope() == false){
			// TODO: different / better messaging? Should check if in global scope instead?
			this->source.error("\"unreachable\" statements are not outside of a function", unreachable);
			return false;
		}

		this->add_scope_level_terminated();
		this->set_scope_terminated();

		this->get_stmts_entry().emplace_back(PIR::Stmt::getUnreachable());
		this->get_stmts_entry().setTerminated();
		if(this->is_in_func_base_scope()){
			this->get_current_func().func.terminatesInBaseScope = true;
		}

		return true;
	};





	auto SemanticAnalyzer::analyze_block(const AST::Block& block, PIR::StmtBlock& stmts_entry) noexcept -> bool {
		this->enter_scope(&stmts_entry);

		if(this->analyze_block(block) == false){ return false; }

		this->leave_scope();

		return true;
	};


	auto SemanticAnalyzer::analyze_block(const AST::Block& block) noexcept -> bool {
		for(AST::Node::ID node_id : block.nodes){
			if(this->analyze_stmt(this->source.getNode(node_id)) == false){
				return false;
			}
		}

		return true;
	};






	auto SemanticAnalyzer::check_func_call(const AST::FuncCall& func_call, PIR::Type::ID type_id) const noexcept -> bool {
		const PIR::Type& type = this->src_manager.getType(type_id);

		if(type.qualifiers.empty() == false){
			this->source.error(
				"cannot be called like a function", func_call.target,
				std::vector<Message::Info>{ std::format("Type \"{}\" does not have a call operator", this->src_manager.printType(type_id)) }
			);
			return false;
		}

		const PIR::BaseType& base_type = this->src_manager.getBaseType(type.baseType);
		if(base_type.callOperator.has_value() == false){
			// TODO: better messaging?
			this->source.error(
				"cannot be called like a function", func_call.target,
				std::vector<Message::Info>{ std::format("Type \"{}\" does not have a call operator", this->src_manager.printType(type_id)) }
			);
			return false;
		}


		if(base_type.callOperator->params.size() != func_call.args.size()){
			// TODO: better messaging
			this->source.error("Function call number of arguments do not match function", func_call.target);
			return false;
		}


		for(size_t i = 0; i < base_type.callOperator->params.size(); i+=1){
			const PIR::BaseType::Operator::Param& param = base_type.callOperator->params[i];
			const AST::Node::ID arg_id = func_call.args[i];
			const AST::Node& arg_node = this->source.getNode(arg_id);

			// check types match
			const evo::Result<PIR::Type::ID> arg_type_id = this->analyze_and_get_type_of_expr(arg_node);
			if(arg_type_id.isError()){ return false; }

			if(param.type != arg_type_id.value()){
				const PIR::Type& param_type = this->src_manager.getType(param.type);
				const PIR::Type& arg_type = this->src_manager.getType(arg_type_id.value());

				if(this->is_implicitly_convertable_to(arg_type, param_type, arg_node) == false){
					// TODO: better messaging
					this->source.error(
						"Function call arguments do not match function", func_call.target,
						std::vector<Message::Info>{
							Message::Info(std::format("In argument: {}", i)),
							Message::Info(std::format("Type of parameter: {}", this->src_manager.printType(param.type))),
							Message::Info(std::format("Type of argument:  {}", this->src_manager.printType(arg_type_id.value()))),
						}
					);
					return false;
				}
			}

			// check param kind accepts arg value type
			const evo::Result<ExprValueType> arg_value_type = this->get_expr_value_type(arg_id);
			if(arg_value_type.isError()){ return false; }

			using ParamKind = AST::FuncParams::Param::Kind;
			switch(param.kind){
				case ParamKind::Read: {
					// accepts any value type
				} break;

				case ParamKind::Write: {
					if(arg_value_type.value() != ExprValueType::Concrete){
						this->source.error("write parameters require concrete expression values", arg_id);
						return false;
					}

					if(this->is_expr_mutable(arg_id) == false){
						this->source.error("write parameters require mutable expression values", arg_id);
						return false;
					}
				} break;

				case ParamKind::In: {
					if(arg_value_type.value() != ExprValueType::Ephemeral){
						this->source.error("write parameters require ephemeral expression values", arg_id);
						return false;
					}
				} break;
			};


			// if param is write and arg is a param, mark it as edited
			if(arg_node.kind == AST::Kind::Ident && param.kind == ParamKind::Write){
				std::string_view param_ident_str = this->source.getIdent(arg_node).value.string;

				for(const Scope& scope : this->scopes){
 					if(scope.params.contains(param_ident_str)){
						PIR::Param& pir_param = this->source.getParam(scope.params.at(param_ident_str));
						pir_param.mayHaveBeenEdited = true;
					}
				}
			}
		}

		return true;
	};


	auto SemanticAnalyzer::get_func_call_args(const AST::FuncCall& func_call) const noexcept -> std::vector<PIR::Expr> {
		auto args = std::vector<PIR::Expr>();

		for(AST::Node::ID arg_id : func_call.args){
			const evo::Result<PIR::Expr> expr = this->is_global_scope() ? this->get_const_expr_value(arg_id) : this->get_expr_value(arg_id);
			evo::debugAssert(expr.isSuccess(), "uncaught error");

			args.emplace_back(expr.value());
		}

		return args;
	};






	auto SemanticAnalyzer::analyze_and_get_type_of_expr(const AST::Node& node, const AST::FuncCall* lookup_func_call) const noexcept 
	-> evo::Result<PIR::Type::ID> {
		switch(node.kind){
			case AST::Kind::Literal: {
				const Token& literal_value = this->source.getLiteral(node);

				if(literal_value.kind == Token::LiteralFloat){
					this->source.error("Literal floats are not supported yet", node);
					return evo::resultError;
				}
				if(literal_value.kind == Token::LiteralChar){
					this->source.error("Literal chars are not supported yet", node);
					return evo::resultError;
				}


				const Token::Kind base_type = [&]() noexcept {
					switch(literal_value.kind){
						break; case Token::LiteralInt: return Token::TypeInt;
						break; case Token::LiteralBool: return Token::TypeBool;
						break; case Token::LiteralString: return Token::TypeString;
					};

					EVO_FATAL_BREAK("Unkonwn literal type");
				}();


				return this->src_manager.getOrCreateTypeID(
					PIR::Type( this->src_manager.getBaseTypeID(base_type) )
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
						if(lookup_func_call == nullptr){
							this->source.error("Functions as values are not supported yet", node);
							return evo::resultError;
						}

						const evo::Result<PIR::Func::ID> lookup_func_id = this->lookup_func_in_scope(ident_str, *lookup_func_call);
						if(lookup_func_id.isError()){ return evo::resultError; }

						const PIR::Func& lookup_func = this->source.getFunc(lookup_func_id.value());
						return this->src_manager.getOrCreateTypeID(PIR::Type(lookup_func.baseType));

					}else if(scope.params.contains(ident_str)){
						const PIR::Param& param = this->source.getParam(scope.params.at(ident_str));
						return param.type;

					}else if(scope.imports.contains(ident_str)){
						return this->src_manager.getTypeImport();
					}
				}


				this->source.error(std::format("Identifier \"{}\" is undefined", ident_str), node);
				return evo::resultError;
			} break;


			case AST::Kind::Intrinsic: {
				const Token& intrinsic_tok = this->source.getIntrinsic(node);

				for(const PIR::Intrinsic& intrinsic : this->src_manager.getIntrinsics()){
					if(intrinsic.ident == intrinsic_tok.value.string){
						return this->src_manager.getOrCreateTypeID(PIR::Type(intrinsic.baseType));
					}
				}


				this->source.error(std::format("Intrinsic \"@{}\" does not exist", intrinsic_tok.value.string), intrinsic_tok);
				return evo::resultError;
			} break;


			case AST::Kind::FuncCall: {
				const AST::FuncCall& func_call = this->source.getFuncCall(node);

				// get target type
				const evo::Result<PIR::Type::ID> target_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(func_call.target), &func_call);
				if(target_type_id.isError()){ return evo::resultError; }


				// check that it's a function type
				const PIR::Type& type = this->src_manager.getType(target_type_id.value());
				const PIR::BaseType& base_type = this->src_manager.getBaseType(type.baseType);

				const PIR::Type::VoidableID return_type = base_type.callOperator->returnType;
				if(return_type.isVoid()){
					this->source.error("Function does not return a value", func_call.target);
					return evo::resultError;
				}

				return return_type.typeID();
			} break;


			case AST::Kind::Prefix: {
				const AST::Prefix& prefix = this->source.getPrefix(node);

				switch(this->source.getToken(prefix.op).kind){
					case Token::KeywordCopy: {
						const evo::Result<ExprValueType> expr_value_type = this->get_expr_value_type(prefix.rhs);
						if(expr_value_type.isError()){ return evo::resultError; }
						if(expr_value_type.value() != ExprValueType::Concrete){
							this->source.error("Only concrete expressions can be copied", prefix.rhs);
							return evo::resultError;
						}

						return this->analyze_and_get_type_of_expr(this->source.getNode(prefix.rhs));
					} break;


					case Token::KeywordAddr: {
						// check value type
						const evo::Result<ExprValueType> expr_value_type = this->get_expr_value_type(prefix.rhs);
						if(expr_value_type.isError()){ return evo::resultError; }
						if(expr_value_type.value() != ExprValueType::Concrete){
							this->source.error("Can only take the address of a concrete expression", prefix.rhs);
							return evo::resultError;
						}


						// check that it's not an intrinsic
						if(this->source.getNode(prefix.rhs).kind == AST::Kind::Intrinsic){
							this->source.error("Cannot take the address of an intrinsic", prefix.rhs);
							return evo::resultError;
						}


						// get type of rhs
						const evo::Result<PIR::Type::ID> type_of_rhs = this->analyze_and_get_type_of_expr(this->source.getNode(prefix.rhs));
						if(type_of_rhs.isError()){ return evo::resultError; }

						const PIR::Type& rhs_type = this->src_manager.getType(type_of_rhs.value());
						PIR::Type rhs_type_copy = rhs_type;


						// make the type pointer of the type of rhs
						rhs_type_copy.qualifiers.emplace_back(true, !this->is_expr_mutable(prefix.rhs));

						return this->src_manager.getOrCreateTypeID(rhs_type_copy);
					} break;

					case Token::get("-"): {
						const evo::Result<PIR::Type::ID> type_of_rhs = this->analyze_and_get_type_of_expr(this->source.getNode(prefix.rhs));
						if(type_of_rhs.isError()){ return evo::resultError; }

						const PIR::Type& rhs_type = this->src_manager.getType(type_of_rhs.value());
						const PIR::BaseType& rhs_base_type = this->src_manager.getBaseType(rhs_type.baseType);

						if(rhs_base_type.negateOperators.empty()){
							// TODO: better messaging
							this->source.error(
								"This type does not have a negate ([-]) operator", prefix.rhs,
								std::vector<Message::Info>{
									Message::Info(std::format("Type of right-hand-side: {}", this->src_manager.printType(type_of_rhs.value()))),
								}
							);
							return evo::resultError;
						}

						const PIR::Intrinsic::ID intrinsic_id = rhs_base_type.negateOperators[0].intrinsic;
						const PIR::BaseType::ID intrinsic_base_type_id = this->src_manager.getIntrinsic(intrinsic_id).baseType;
						return this->src_manager.getBaseType(intrinsic_base_type_id).callOperator->returnType.typeID();
					} break;

					default: EVO_FATAL_BREAK("Unknown prefix operator");
				};

			} break;


			case AST::Kind::Infix: {
				const AST::Infix& infix = this->source.getInfix(node);
				const Token::Kind infix_op_kind = this->source.getToken(infix.op).kind;

				switch(infix_op_kind){
					case Token::get("."): {
						const evo::Result<PIR::Type::ID> type_id_of_lhs = this->analyze_and_get_type_of_expr(this->source.getNode(infix.lhs));
						if(type_id_of_lhs.isError()){ return evo::resultError; }

						if(type_id_of_lhs.value() != SourceManager::getTypeImport()){
							this->source.error("Expression does not have valid accessor operator", infix.lhs);
							return evo::resultError;
						}

						const evo::Result<PIR::Expr> value_of_lhs = this->get_expr_value(infix.lhs);
						if(value_of_lhs.isError()){ return evo::resultError; }
						evo::debugAssert(value_of_lhs.value().kind == PIR::Expr::Kind::Import, "incorrect expr kind gotten");

						const Source& import_source = this->src_manager.getSource(value_of_lhs.value().import);
						const Token& rhs_ident = this->source.getIdent(infix.rhs);

						if(import_source.pir.pub_funcs.contains(rhs_ident.value.string)){
							const std::vector<PIR::Func::ID>& imported_func_list = import_source.pir.pub_funcs.at(rhs_ident.value.string);

							if(imported_func_list.size() == 1){
								const PIR::Func& imported_func = Source::getFunc(imported_func_list[0]);

								return this->src_manager.getOrCreateTypeID(
									PIR::Type(imported_func.baseType)
								);
							}

							if(lookup_func_call != nullptr){
								const evo::Result<PIR::Func::ID> imported_func_id = this->lookup_func_in_import(
									rhs_ident.value.string, import_source, *lookup_func_call
								);
								if(imported_func_id.isError()){ return evo::resultError; }

								const PIR::Func& imported_func = Source::getFunc(imported_func_id.value());

								return this->src_manager.getOrCreateTypeID(
									PIR::Type(imported_func.baseType)
								);
							}

							this->source.error("Cannot get overloaded function", infix.rhs);
							return evo::resultError;

						}else if(import_source.pir.pub_vars.contains(rhs_ident.value.string)){
							const PIR::Var::ID imported_var_id = import_source.pir.pub_vars.at(rhs_ident.value.string);
							const PIR::Var& imported_var = Source::getVar(imported_var_id);

							return imported_var.type;

						}else if(import_source.pir.pub_imports.contains(rhs_ident.value.string)){
							return this->src_manager.getTypeImport();
						}

						// TODO: better messaging
						this->source.error(std::format("import does not have public member \"{}\"", rhs_ident.value.string), infix.rhs);
						return evo::resultError;
					} break;

					case Token::get("+"): case Token::get("+@"):
					case Token::get("-"): case Token::get("-@"):
					case Token::get("*"): case Token::get("*@"):
					case Token::get("/"): {

						///////////////////////////////////
						// lhs

						const AST::Node& lhs_node = this->source.getNode(infix.lhs);
						const evo::Result<PIR::Type::ID> type_id_of_lhs = this->analyze_and_get_type_of_expr(lhs_node);
						if(type_id_of_lhs.isError()){ return evo::resultError; }

						const PIR::Type& type_of_lhs = this->src_manager.getType(type_id_of_lhs.value());

						if(type_of_lhs.qualifiers.empty() == false){
							this->source.error(
								std::format("Types with qualifiers do not support the [{}] operator", Token::printKind(infix_op_kind)), infix.lhs
							);
							return evo::resultError;
						}

						const PIR::BaseType& base_type_of_lhs = this->src_manager.getBaseType(type_of_lhs.baseType);


						const bool has_operator = [&]() noexcept {
							switch(infix_op_kind){
								case Token::get("+"): return base_type_of_lhs.addOperators.empty() == false;
								case Token::get("+@"): return base_type_of_lhs.addWrapOperators.empty() == false;
								case Token::get("-"): return base_type_of_lhs.subOperators.empty() == false;
								case Token::get("-@"): return base_type_of_lhs.subWrapOperators.empty() == false;
								case Token::get("*"): return base_type_of_lhs.mulOperators.empty() == false;
								case Token::get("*@"): return base_type_of_lhs.mulWrapOperators.empty() == false;
								case Token::get("/"): return base_type_of_lhs.divOperators.empty() == false;
							};

							EVO_FATAL_BREAK("Unknown intrinsic kind");
						}();

						if(has_operator == false){
							this->source.error(
								std::format("This type does not have a [{}] operator", Token::printKind(infix_op_kind)), infix.lhs,
								std::vector<Message::Info>{ 
									Message::Info(std::format("Type of left-hand-side: {}", this->src_manager.printType(type_id_of_lhs.value()))),
								}
							);
							return evo::resultError;
						}


						///////////////////////////////////
						// rhs

						const AST::Node& rhs_node = this->source.getNode(infix.rhs);
						const evo::Result<PIR::Type::ID> type_id_of_rhs = this->analyze_and_get_type_of_expr(rhs_node);
						if(type_id_of_rhs.isError()){ return evo::resultError; }

						const PIR::Type& type_of_rhs = this->src_manager.getType(type_id_of_rhs.value());

						if(type_of_rhs.qualifiers.empty() == false){
							this->source.error(
								std::format("Types with qualifiers do not support the [{}] operator", Token::printKind(infix_op_kind)), infix.lhs
							);
							return evo::resultError;
						}

						const PIR::BaseType& base_type_of_rhs = this->src_manager.getBaseType(type_of_rhs.baseType);



						///////////////////////////////////
						// op checking

						evo::debugAssert(base_type_of_lhs.kind != PIR::BaseType::Kind::Import, "should have been caught already");
						evo::debugAssert(base_type_of_lhs.kind != PIR::BaseType::Kind::Function, "should have been caught already");

						if(base_type_of_lhs.kind == PIR::BaseType::Kind::Builtin){
							const PIR::BaseType* base_type_to_use = &base_type_of_lhs;

							if(type_id_of_lhs.value() != type_id_of_rhs.value()){
								if(this->is_implicitly_convertable_to(type_of_lhs, type_of_rhs, lhs_node)){
									base_type_to_use = &base_type_of_rhs;
									// do conversion (when needed/implemented)

								}else if(this->is_implicitly_convertable_to(type_of_rhs, type_of_lhs, rhs_node)){
									// do conversion (when needed/implemented)
									
								}else{
									// TODO: better messaging
									this->source.error(
										std::format("No matching [{}] operator found", Token::printKind(infix_op_kind)), infix.lhs
									);
									return evo::resultError;
								}
							}


							const PIR::Intrinsic::ID intrinsic_id = [&]() noexcept {
								switch(infix_op_kind){
									case Token::get("+"): return base_type_to_use->addOperators[0].intrinsic;
									case Token::get("+@"): return base_type_to_use->addWrapOperators[0].intrinsic;
									case Token::get("-"): return base_type_to_use->subOperators[0].intrinsic;
									case Token::get("-@"): return base_type_to_use->subWrapOperators[0].intrinsic;
									case Token::get("*"): return base_type_to_use->mulOperators[0].intrinsic;
									case Token::get("*@"): return base_type_to_use->mulWrapOperators[0].intrinsic;
									case Token::get("/"): return base_type_to_use->divOperators[0].intrinsic;
								};

								EVO_FATAL_BREAK("Unknown intrinsic kind");
							}();

							const PIR::BaseType::ID intrinsic_base_type_id = this->src_manager.getIntrinsic(intrinsic_id).baseType;
							return this->src_manager.getBaseType(intrinsic_base_type_id).callOperator->returnType.typeID();
						}

						EVO_FATAL_BREAK("Unknown base type kind");
					} break;
				};

				EVO_FATAL_BREAK("Unknown infix kind");
			} break;


			case AST::Kind::Postfix: {
				const AST::Postfix& postfix = this->source.getPostfix(node);

				switch(this->source.getToken(postfix.op).kind){
					case Token::get(".^"): {
						// get type of lhs
						const evo::Result<PIR::Type::ID> type_of_lhs = this->analyze_and_get_type_of_expr(this->source.getNode(postfix.lhs));
						if(type_of_lhs.isError()){ return evo::resultError; }

						// check that type of lhs is pointer
						const PIR::Type& lhs_type = this->src_manager.getType(type_of_lhs.value());
						if(lhs_type.qualifiers.empty() || lhs_type.qualifiers.back().isPtr == false){
							this->source.error(
								"left-hand-side of dereference expression must be of a pointer type", postfix.op,
								std::vector<Message::Info>{
									{std::string("expression is of type: ") + this->src_manager.printType(type_of_lhs.value())},
								}
							);
							return evo::resultError;
						}

						// get dereferenced type
						PIR::Type lhs_type_copy = lhs_type;
						lhs_type_copy.qualifiers.pop_back();

						return this->src_manager.getOrCreateTypeID(lhs_type_copy);
					} break;

					default: EVO_FATAL_BREAK("Unknown postfix operator");
				};

			} break;




			case AST::Kind::Uninit: {
				EVO_FATAL_BREAK("[uninit] exprs should not be analyzed with this function");
			} break;
		};

		EVO_FATAL_BREAK("Unknown expr type");
	};



	auto SemanticAnalyzer::get_type_id(AST::Node::ID node_id) const noexcept -> evo::Result<PIR::Type::VoidableID> {
		const AST::Type& type = this->source.getType(node_id);
		const Token& type_token = this->source.getToken(type.token);

		if(type_token.kind == Token::TypeVoid){
			if(type.qualifiers.empty() == false){
				this->source.error("Void type cannot have qualifiers", node_id);
				return evo::resultError;
			}

			return PIR::Type::VoidableID::Void();
		}

		if(type_token.kind == Token::TypeString){
			this->source.error("String type is not supported yet", node_id);
			return evo::resultError;
		}



		std::vector<AST::Type::Qualifier> type_qualifiers = type.qualifiers;

		{
			// checking const-ness of type levels
			bool type_qualifiers_has_a_const = false;
			bool has_warned = false;
			for(auto i = type_qualifiers.rbegin(); i != type_qualifiers.rend(); ++i){
				if(type_qualifiers_has_a_const){
					if(i->isConst == false && has_warned == false){
						has_warned = true;
						this->source.warning("If one type level is const, all previous levels will automatically be made const as well", node_id);
					}

					i->isConst = true;

				}else if(i->isConst){
					type_qualifiers_has_a_const = true;
				}
			}
		}
		

		return PIR::Type::VoidableID(
			this->src_manager.getOrCreateTypeID(
				PIR::Type(this->src_manager.getBaseTypeID(type_token.kind), type_qualifiers)
			)
		);
	};



	auto SemanticAnalyzer::is_implicitly_convertable_to(const PIR::Type& from, const PIR::Type& to, const AST::Node& from_expr) const noexcept -> bool {
		if(from.isImplicitlyConvertableTo(to)){ return true; }

		///////////////////////////////////
		// literal conversion

		if(from_expr.kind != AST::Kind::Literal){ return false; }

		if(from.qualifiers.empty() == false || to.qualifiers.empty() == false){ return false; }

		// const PIR::BaseType& from_base = this->src_manager.getBaseType(from.baseType);
		// const PIR::BaseType& to_base = this->src_manager.getBaseType(to.baseType);

		const bool from_is_integral = 
			from.baseType == this->src_manager.getBaseTypeID(Token::TypeInt) || 
			from.baseType == this->src_manager.getBaseTypeID(Token::TypeUInt);

		const bool to_is_integral = 
			to.baseType == this->src_manager.getBaseTypeID(Token::TypeInt) || 
			to.baseType == this->src_manager.getBaseTypeID(Token::TypeUInt);

		if(from_is_integral && to_is_integral){ return true; }

		return false;
	};




	auto SemanticAnalyzer::get_expr_value(AST::Node::ID node_id) const noexcept -> evo::Result<PIR::Expr> {
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

					if(scope.params.contains(value_ident_str)){
						return PIR::Expr(scope.params.at(value_ident_str));
					}

					if(scope.imports.contains(value_ident_str)){
						const Source::ID import_source_id = scope.imports.at(value_ident_str).source_id;
						return PIR::Expr(import_source_id);
					}
				}

				EVO_FATAL_BREAK("Didn't find value_ident");
			} break;


			case AST::Kind::FuncCall: {
				const AST::FuncCall& func_call = this->source.getFuncCall(value_node);
				const AST::Node& target_node = this->source.getNode(func_call.target);

				const std::vector<PIR::Expr> args = this->get_func_call_args(func_call);

				if(target_node.kind == AST::Kind::Ident){
					const std::string_view ident = this->source.getIdent(func_call.target).value.string;

					// get func
					const evo::Result<PIR::Func::ID> func_id = this->lookup_func_in_scope(ident, func_call);
					if(func_id.isError()){ return evo::resultError; }


					const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(func_id.value(), std::move(args));
					return PIR::Expr(func_call_id);

				}else if(target_node.kind == AST::Kind::Intrinsic){
					// get the intrinsic id
					const PIR::Intrinsic::ID intrinsic_id = [&]() noexcept {
						const Token& intrinsic_tok = this->source.getIntrinsic(func_call.target);
						const evo::ArrayProxy<PIR::Intrinsic> intrinsics = this->src_manager.getIntrinsics();

						for(size_t i = 0; i < intrinsics.size(); i+=1){
							const PIR::Intrinsic& intrinsic = intrinsics[i];

							if(intrinsic.ident == intrinsic_tok.value.string){
								return PIR::Intrinsic::ID(uint32_t(i));
							}
						}

						EVO_FATAL_BREAK("Unknown intrinsic");
					}();


					// imports
					if(intrinsic_id == SourceManager::getIntrinsicID(PIR::Intrinsic::Kind::import)){
						const evo::Result<Source::ID> import_source_id = this->get_import_source_id(args[0], func_call.target);
						if(import_source_id.isError()){ return evo::resultError; }

						return PIR::Expr(import_source_id.value());
					}

					// function calls normally
					const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(intrinsic_id, std::move(args));	
					return PIR::Expr(func_call_id);

				}else if(target_node.kind == AST::Kind::Infix){
					const AST::Infix& infix = this->source.getInfix(func_call.target);

					switch(this->source.getToken(infix.op).kind){
						case Token::get("."): {
							const evo::Result<PIR::Expr> value_of_lhs = this->get_expr_value(infix.lhs);
							if(value_of_lhs.isError()){ return evo::resultError; }
							evo::debugAssert(value_of_lhs.value().kind == PIR::Expr::Kind::Import, "incorrect expr kind gotten");

							const Source& import_source = this->src_manager.getSource(value_of_lhs.value().import);
							const Token& rhs_ident = this->source.getIdent(infix.rhs);

							// get func
							const evo::Result<PIR::Func::ID> imported_func_id = this->lookup_func_in_import(
								rhs_ident.value.string, import_source, func_call
							);
							if(imported_func_id.isError()){ return evo::resultError; }

							
							// create object
							const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(imported_func_id.value(), std::move(args));
							return PIR::Expr(func_call_id);
						} break;

					};

					EVO_FATAL_BREAK("Unknown or unsupported infix type");
				}

				EVO_FATAL_BREAK("Unknown func target kind");

			} break;


			case AST::Kind::Prefix: {
				const AST::Prefix& prefix = this->source.getPrefix(value_node);

				const evo::Result<PIR::Expr> rhs_value = this->get_expr_value(prefix.rhs);
				if(rhs_value.isError()){ return evo::resultError; }

				if(this->source.getToken(prefix.op).kind == Token::get("-")){
					const evo::Result<PIR::Type::ID> type_of_rhs = this->analyze_and_get_type_of_expr(this->source.getNode(prefix.rhs));
					if(type_of_rhs.isError()){ return evo::resultError; }

					const PIR::Type& rhs_type = this->src_manager.getType(type_of_rhs.value());
					const PIR::BaseType& rhs_base_type = this->src_manager.getBaseType(rhs_type.baseType);

					if(rhs_base_type.negateOperators.empty()){
						// TODO: better messaging
						this->source.error(
							"This type does not have a negate ([-]) operator", prefix.rhs,
							std::vector<Message::Info>{
								Message::Info(std::format("Type of right-hand-side: {}", this->src_manager.printType(type_of_rhs.value()))),
							}
						);
						return evo::resultError;
					}

					const PIR::Intrinsic::ID intrinsic_id = rhs_base_type.negateOperators[0].intrinsic;
					const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(intrinsic_id, std::vector<PIR::Expr>{rhs_value.value()});
					return PIR::Expr(func_call_id);
				}
				

				const PIR::Prefix::ID prefix_id = this->source.createPrefix(prefix.op, rhs_value.value());

				if(rhs_value.value().kind == PIR::Expr::Kind::Param && this->source.getToken(prefix.op).kind == Token::KeywordAddr){
					this->source.getParam(rhs_value.value().param).mayHaveBeenEdited = true;
				}

				return PIR::Expr(prefix_id);
			} break;


			case AST::Kind::Infix: {
				const AST::Infix& infix = this->source.getInfix(value_node);
				const Token::Kind infix_op_kind = this->source.getToken(infix.op).kind;

				switch(infix_op_kind){
					case Token::get("."): {
						const evo::Result<PIR::Expr> value_of_lhs = this->get_expr_value(infix.lhs);
						if(value_of_lhs.isError()){ return evo::resultError; }
						evo::debugAssert(value_of_lhs.value().kind == PIR::Expr::Kind::Import, "incorrect expr kind gotten");

						const Source& import_source = this->src_manager.getSource(value_of_lhs.value().import);
						const Token& rhs_ident = this->source.getIdent(infix.rhs);


						if(import_source.pir.pub_vars.contains(rhs_ident.value.string)){
							const PIR::Var::ID imported_var_id = import_source.pir.pub_vars.at(rhs_ident.value.string);
							return PIR::Expr(imported_var_id);

						}else if(import_source.pir.pub_funcs.contains(rhs_ident.value.string)){
							this->source.error("Functions as values are not supported yet", infix.rhs);
							return evo::resultError;
							
						}else if(import_source.pir.pub_imports.contains(rhs_ident.value.string)){
							const Source::ID imported_source_id = import_source.pir.pub_imports.at(rhs_ident.value.string);
							return PIR::Expr(imported_source_id);
						}

						EVO_FATAL_BREAK("should have already caught that it's non-existant");
					} break;


					case Token::get("+"): case Token::get("+@"):
					case Token::get("-"): case Token::get("-@"):
					case Token::get("*"): case Token::get("*@"):
					case Token::get("/"): {
						///////////////////////////////////
						// lhs

						const AST::Node& lhs_node = this->source.getNode(infix.lhs);
						const evo::Result<PIR::Type::ID> type_id_of_lhs = this->analyze_and_get_type_of_expr(lhs_node);
						if(type_id_of_lhs.isError()){ return evo::resultError; }

						const PIR::Type& type_of_lhs = this->src_manager.getType(type_id_of_lhs.value());
						evo::debugAssert(type_of_lhs.qualifiers.empty(), "uncaught qualifiers");

						const PIR::BaseType& base_type_of_lhs = this->src_manager.getBaseType(type_of_lhs.baseType);


						///////////////////////////////////
						// rhs

						const AST::Node& rhs_node = this->source.getNode(infix.rhs);
						const evo::Result<PIR::Type::ID> type_id_of_rhs = this->analyze_and_get_type_of_expr(rhs_node);
						if(type_id_of_rhs.isError()){ return evo::resultError; }

						const PIR::Type& type_of_rhs = this->src_manager.getType(type_id_of_rhs.value());
						evo::debugAssert(type_of_rhs.qualifiers.empty(), "uncaught qualifiers");

						const PIR::BaseType& base_type_of_rhs = this->src_manager.getBaseType(type_of_rhs.baseType);


						///////////////////////////////////
						// create func call

						evo::debugAssert(base_type_of_lhs.kind != PIR::BaseType::Kind::Import, "uncaught import");
						evo::debugAssert(base_type_of_lhs.kind != PIR::BaseType::Kind::Function, "uncaught function");

						if(base_type_of_lhs.kind == PIR::BaseType::Kind::Builtin){
							// evo::debugAssert(type_id_of_lhs.value() == type_id_of_rhs.value(), "uncaught type mismatch");

							const PIR::BaseType& base_type_to_use = [&]() noexcept {
								if(type_id_of_lhs.value() == type_id_of_rhs.value()){
									return base_type_of_lhs;

								}else if(this->is_implicitly_convertable_to(type_of_lhs, type_of_rhs, lhs_node)){
									return base_type_of_rhs;

								}else{
									evo::debugAssert(this->is_implicitly_convertable_to(type_of_rhs, type_of_lhs, rhs_node), "uncaught invalid op");
									return base_type_of_lhs;
								}
							}();

							const PIR::Intrinsic::ID intrinsic_id = [&]() noexcept {
								switch(infix_op_kind){
									case Token::get("+"): return base_type_to_use.addOperators[0].intrinsic;
									case Token::get("+@"): return base_type_to_use.addWrapOperators[0].intrinsic;
									case Token::get("-"): return base_type_to_use.subOperators[0].intrinsic;
									case Token::get("-@"): return base_type_to_use.subWrapOperators[0].intrinsic;
									case Token::get("*"): return base_type_to_use.mulOperators[0].intrinsic;
									case Token::get("*@"): return base_type_to_use.mulWrapOperators[0].intrinsic;
									case Token::get("/"): return base_type_to_use.divOperators[0].intrinsic;
								};

								EVO_FATAL_BREAK("Unknown intrinsic kind");
							}();

							const evo::Result<PIR::Expr> lhs_expr = this->get_expr_value(infix.lhs);
							if(lhs_expr.isError()){ return evo::resultError; }

							const evo::Result<PIR::Expr> rhs_expr = this->get_expr_value(infix.rhs);
							if(lhs_expr.isError()){ return evo::resultError; }

							const PIR::FuncCall::ID func_call_id = 
								this->source.createFuncCall(intrinsic_id, std::vector<PIR::Expr>{lhs_expr.value(), rhs_expr.value()});
							return PIR::Expr(func_call_id);
						}

					} break;

				};

				EVO_FATAL_BREAK("Unknown or unsupported infix type");
			} break;


			case AST::Kind::Postfix: {
				const AST::Postfix& postfix = this->source.getPostfix(value_node);

				switch(this->source.getToken(postfix.op).kind){
					case Token::get(".^"): {
						// get deref type
						const evo::Result<PIR::Type::ID> ptr_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(postfix.lhs));
						evo::debugAssert(ptr_type_id.isSuccess(), "Failed to get deref type - should have caught error earlier");

						const PIR::Type& ptr_type = this->src_manager.getType(ptr_type_id.value());
						PIR::Type ptr_type_copy = ptr_type;
						ptr_type_copy.qualifiers.pop_back();
						const PIR::Type::ID deref_type_id = this->src_manager.getOrCreateTypeID(ptr_type_copy);

						const evo::Result<PIR::Expr> lhs_expr_value = this->get_expr_value(postfix.lhs);
						if(lhs_expr_value.isError()){ return evo::resultError; }

						const PIR::Deref::ID deref_id = this->source.createDeref(lhs_expr_value.value(), deref_type_id);
						return PIR::Expr(deref_id);
					} break;
				};

				EVO_FATAL_BREAK("Unknown or unsupported postfix type");
			} break;



			case AST::Kind::Literal: {
				return PIR::Expr(node_id);
			} break;

			case AST::Kind::Uninit: {
				return PIR::Expr(node_id);
			} break;


		};

		EVO_FATAL_BREAK("Unknown node value kind");
	};



	auto SemanticAnalyzer::get_const_expr_value(AST::Node::ID node_id) const noexcept -> evo::Result<PIR::Expr> {
		const evo::Result<PIR::Expr> recursive_value = this->get_const_expr_value_recursive(node_id);
		if(recursive_value.isError()){ return evo::resultError; }

		// if(recursive_value.value().kind == PIR::Expr::Kind::Var){
		// 	const PIR::Var& value_var = this->source.getVar(recursive_value.value().var);
		// 	return value_var.value;
		// }

		return recursive_value;
	};



	auto SemanticAnalyzer::get_const_expr_value_recursive(AST::Node::ID node_id) const noexcept -> evo::Result<PIR::Expr> {
		const AST::Node& node = this->source.getNode(node_id);

		switch(node.kind){
			case AST::Kind::Ident: {
				const Token& value_ident = this->source.getIdent(node);
				std::string_view value_ident_str = value_ident.value.string;

				// find the var
				for(const Scope& scope : this->scopes){
					if(scope.vars.contains(value_ident_str)){
						// return PIR::Expr(scope.vars.at(value_ident_str));

						this->source.error("At this time, constant-evaluated values cannot be the value of a variable", node);
						return evo::resultError;
					}

					if(scope.params.contains(value_ident_str)){
						// return PIR::Expr(scope.params.at(value_ident_str));

						this->source.error("At this time, constant-evaluated values cannot be the value of a parameter", node);
						return evo::resultError;
					}

					if(scope.imports.contains(value_ident_str)){
						// return PIR::Expr(scope.imports.at(value_ident_str).source_id);

						this->source.error("At this time, constant-evaluated values cannot be the value of an import", node);
						return evo::resultError;
					}
				}

				EVO_FATAL_BREAK("Unkown ident");

			} break;

			case AST::Kind::FuncCall: {
				const AST::FuncCall& func_call = this->source.getFuncCall(node);
				const AST::Node& target_node = this->source.getNode(func_call.target);

				if(target_node.kind != AST::Kind::Intrinsic){
					this->source.error(
						"At this time, constant-evaluated values cannot be function calls", node,
						std::vector<Message::Info>{ Message::Info("calls to intrinsic functions are allowed") }
					);
					return evo::resultError;
				}

				const evo::Result<PIR::Type::ID> target_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(func_call.target));
				if(target_type_id.isError()){ return evo::resultError; }


				const std::vector<PIR::Expr> args = this->get_func_call_args(func_call);

				// get the intrinsic id
				const PIR::Intrinsic::ID intrinsic_id = [&]() noexcept {
					const Token& intrinsic_tok = this->source.getIntrinsic(func_call.target);
					const evo::ArrayProxy<PIR::Intrinsic> intrinsics = this->src_manager.getIntrinsics();

					for(size_t i = 0; i < intrinsics.size(); i+=1){
						const PIR::Intrinsic& intrinsic = intrinsics[i];

						if(intrinsic.ident == intrinsic_tok.value.string){
							return PIR::Intrinsic::ID(uint32_t(i));
						}
					}

					EVO_FATAL_BREAK("Unknown intrinsic");
				}();

				// imports
				if(intrinsic_id == SourceManager::getIntrinsicID(PIR::Intrinsic::Kind::import)){
					const evo::Result<Source::ID> import_source_id = this->get_import_source_id(args[0], func_call.target);
					if(import_source_id.isError()){ return evo::resultError; }

					return PIR::Expr(import_source_id.value());
				}

				// function calls normally
				const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(intrinsic_id, std::move(args));	
				return PIR::Expr(func_call_id);
			} break;

			case AST::Kind::Prefix: {
				const AST::Prefix& prefix = this->source.getPrefix(node);

				switch(this->source.getToken(prefix.op).kind){
					case Token::KeywordCopy: {					
						return this->get_const_expr_value_recursive(prefix.rhs);
					} break;

					case Token::KeywordAddr: {
						const evo::Result<PIR::Expr> rhs_value = this->get_const_expr_value_recursive(prefix.rhs);
						if(rhs_value.isError()){ return evo::resultError; }

						const PIR::Prefix::ID prefix_id = this->source.createPrefix(prefix.op, rhs_value.value());
						return PIR::Expr(prefix_id);
					} break;

				};

				EVO_FATAL_BREAK("Unknown prefix kind");
			} break;

			case AST::Kind::Postfix: {
				this->source.error("At this time, constant values cannot be postfix operations", node);
				return evo::resultError;
			} break;

			case AST::Kind::Literal: {
				return PIR::Expr(node_id);
			} break;

			case AST::Kind::Uninit: {
				this->source.error("Constant values cannot be \"uninit\"", node);
				return evo::resultError;
			} break;

			default: EVO_FATAL_BREAK("Unknown node value kind");
		};

	};


	auto SemanticAnalyzer::get_import_source_id(const PIR::Expr& import_path, AST::Node::ID expr_node) const noexcept -> evo::Result<Source::ID> {
		const std::string_view import_path_str = this->source.getLiteral(import_path.astNode).value.string;
		const std::filesystem::path source_file_path = this->source.getLocation();
		const evo::Expected<Source::ID, SourceManager::GetSourceIDError> imported_source_id_result =
			this->src_manager.getSourceID(source_file_path, import_path_str);

		if(imported_source_id_result.has_value() == false){
			switch(imported_source_id_result.error()){
				case SourceManager::GetSourceIDError::EmptyPath: {
					this->source.error("Empty path is an invalid lookup location", expr_node);
					return evo::resultError;
				} break;

				case SourceManager::GetSourceIDError::SameAsCaller: {
					// TODO: better messaging
					this->source.error("Cannot import self", expr_node);
					return evo::resultError;
				} break;

				case SourceManager::GetSourceIDError::NotOneOfSources: {
					this->source.error(std::format("File \"{}\" is not one of the files being compiled", import_path_str), expr_node);
					return evo::resultError;
				} break;

				case SourceManager::GetSourceIDError::DoesntExist: {
					this->source.error(std::format("Couldn't find file \"{}\"", import_path_str), expr_node);
					return evo::resultError;
				} break;
			};

			EVO_FATAL_BREAK("Unkonwn or unsupported error code");
		}

		return imported_source_id_result.value();
	};






	auto SemanticAnalyzer::get_expr_value_type(AST::Node::ID node_id) const noexcept -> evo::Result<ExprValueType> {
		const AST::Node& value_node = this->source.getNode(node_id);

		switch(value_node.kind){
			break; case AST::Kind::FuncCall: return ExprValueType::Ephemeral;

			break; case AST::Kind::Prefix: return ExprValueType::Ephemeral;

			break; case AST::Kind::Infix: {
				const AST::Infix& infix = this->source.getInfix(value_node);

				switch(this->source.getToken(infix.op).kind){
					case Token::get("."): {
						const evo::Result<PIR::Type::ID> lhs_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(infix.lhs));
						if(lhs_type_id.isError()){ return evo::resultError; }

						const evo::Result<PIR::Expr> value_of_lhs = this->is_global_scope() ? 
							this->get_const_expr_value(infix.lhs) : this->get_expr_value(infix.lhs);
						if(value_of_lhs.isError()){ return evo::resultError; } // TODO: make this a debug assert when globals can be variables


						if(lhs_type_id.value() == SourceManager::getTypeImport()){
							evo::debugAssert(value_of_lhs.value().kind == PIR::Expr::Kind::Import, "incorrect expr kind gotten");

							const Source& import_source = this->src_manager.getSource(value_of_lhs.value().import);
							const Token& rhs_ident = this->source.getIdent(infix.rhs);

							if(import_source.pir.pub_funcs.contains(rhs_ident.value.string)){
								return ExprValueType::Concrete;

							}else if(import_source.pir.pub_vars.contains(rhs_ident.value.string)){
								return ExprValueType::Concrete;

							}else if(import_source.pir.pub_imports.contains(rhs_ident.value.string)){
								return ExprValueType::Import;
							}

							this->source.error(std::format("import does not have public member \"{}\"", rhs_ident.value.string), infix.rhs);
							return evo::resultError;
						}

						EVO_FATAL_BREAK("Invalid lhs type");
					} break;

					case Token::get("+"): return ExprValueType::Ephemeral;
					case Token::get("+@"): return ExprValueType::Ephemeral;
					case Token::get("-"): return ExprValueType::Ephemeral;
					case Token::get("-@"): return ExprValueType::Ephemeral;
					case Token::get("*"): return ExprValueType::Ephemeral;
					case Token::get("*@"): return ExprValueType::Ephemeral;
					case Token::get("/"): return ExprValueType::Ephemeral;
				};
				EVO_FATAL_BREAK("Unknown infix kind");
			}break;

			break; case AST::Kind::Postfix: {
				const AST::Postfix& postfix = this->source.getPostfix(value_node);

				switch(this->source.getToken(postfix.op).kind){
					break; case Token::get(".^"): return ExprValueType::Concrete;
				};
				EVO_FATAL_BREAK("Unknown postfix kind");
			} break;

			break; case AST::Kind::Ident: return ExprValueType::Concrete;
			break; case AST::Kind::Intrinsic: return ExprValueType::Concrete;
			break; case AST::Kind::Literal: return ExprValueType::Ephemeral;
			break; case AST::Kind::Uninit: return ExprValueType::Ephemeral;
		};

		EVO_FATAL_BREAK("Unknown AST node kind");
	};


	auto SemanticAnalyzer::is_expr_mutable(AST::Node::ID node_id) const noexcept -> bool {
		const AST::Node& value_node = this->source.getNode(node_id);

		switch(value_node.kind){
			break; case AST::Kind::Prefix: EVO_FATAL_BREAK("Not concrete");
			break; case AST::Kind::FuncCall: EVO_FATAL_BREAK("Not concrete");


			break; case AST::Kind::Infix: {
				const AST::Infix& infix = this->source.getInfix(value_node);

				switch(this->source.getToken(infix.op).kind){
					case Token::get("."): {
						const evo::Result<PIR::Type::ID> lhs_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(infix.lhs));
						evo::debugAssert(lhs_type_id.isSuccess(), "Should have caught this error already");

						const evo::Result<PIR::Expr> value_of_lhs = this->get_expr_value(infix.lhs);
						evo::debugAssert(value_of_lhs.isSuccess(), "Should have caught this error already");

						if(lhs_type_id.value() == SourceManager::getTypeImport()){
							evo::debugAssert(value_of_lhs.value().kind == PIR::Expr::Kind::Import, "incorrect expr kind gotten");

							const Source& import_source = this->src_manager.getSource(value_of_lhs.value().import);
							const Token& rhs_ident = this->source.getIdent(infix.rhs);

							if(import_source.pir.pub_funcs.contains(rhs_ident.value.string)){
								return false;

							}else if(import_source.pir.pub_vars.contains(rhs_ident.value.string)){
								const PIR::Var::ID imported_var_id = import_source.pir.pub_vars.at(rhs_ident.value.string);
								const PIR::Var& imported_var = Source::getVar(imported_var_id);

								return !imported_var.isDef;

							}else if(import_source.pir.pub_imports.contains(rhs_ident.value.string)){
								return false;
							}
						}

						EVO_FATAL_BREAK("Invalid lhs type");
					} break;

					default: EVO_FATAL_BREAK("Unknown infix kind");
				};
			} break;


			break; case AST::Kind::Postfix: {
				const AST::Postfix& postfix = this->source.getPostfix(value_node);

				switch(this->source.getToken(postfix.op).kind){
					case Token::get(".^"): {
						const evo::Result<PIR::Type::ID> lhs_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(postfix.lhs));
						evo::debugAssert(lhs_type_id.isSuccess(), "Should have caught this error already");

						const PIR::Type& lhs_type = this->src_manager.getType(lhs_type_id.value());
						evo::debugAssert(lhs_type.qualifiers.back().isPtr, "Should have already been checked that this type is a pointer");

						return !lhs_type.qualifiers.back().isConst;

					} break;

					default: EVO_FATAL_BREAK("Unknown postfix kind");
				};
			} break;

			break; case AST::Kind::Ident: {
				const Token& value_ident = this->source.getIdent(value_node);
				std::string_view value_ident_str = value_ident.value.string;

				// find the var
				for(const Scope& scope : this->scopes){
					if(scope.vars.contains(value_ident_str)){
						const PIR::Var::ID var_id = scope.vars.at(value_ident_str);
						const PIR::Var& var = this->source.getVar(var_id);
						return !var.isDef;
					}

					if(scope.funcs.contains(value_ident_str)){
						return false;
					}

					if(scope.params.contains(value_ident_str)){
						const PIR::Param::ID param_id = scope.params.at(value_ident_str);
						const PIR::Param& param = this->source.getParam(param_id);

						using ParamKind = AST::FuncParams::Param::Kind;
						switch(param.kind){
							case ParamKind::Read: return false;
							case ParamKind::Write: return true;
							case ParamKind::In: return true;
						};
					}

					if(scope.imports.contains(value_ident_str)){
						return false;
					}
				}

				EVO_FATAL_BREAK("Didn't find value_ident");
			} break;

			break; case AST::Kind::Intrinsic: return false;

			break; case AST::Kind::Literal: EVO_FATAL_BREAK("Not concrete");
			break; case AST::Kind::Uninit: EVO_FATAL_BREAK("Not concrete");
		};

		EVO_FATAL_BREAK("Unknown AST node kind")
	};







	// TODO: check for exported functions in all files (through saving in SourceManager)
	auto SemanticAnalyzer::is_valid_export_name(std::string_view name) const noexcept -> bool {
		if(name == "main"){ return false; }
		if(name == "puts"){ return false; }
		if(name == "printf"){ return false; }

		return true;
	};


	auto SemanticAnalyzer::already_defined(const Token& ident) const noexcept -> void {
		const std::string_view ident_str = ident.value.string;

		for(const Scope& scope : this->scopes){
			if(scope.vars.contains(ident_str)){
				const PIR::Var& var = this->source.getVar(scope.vars.at(ident_str));
				const Location location = this->source.getToken(var.ident).location;

				this->source.error(
					std::format("Identifier \"{}\" already defined", ident.value.string), ident,
					std::vector<Message::Info>{ Message::Info("First defined here:", location) }
				);
				return;
			}

			if(scope.funcs.contains(ident_str)){
				// TODO: better messaging
				this->source.error(
					std::format("Identifier \"{}\" already defined", ident.value.string), ident,
					std::vector<Message::Info>{ Message::Info("First defined as a function") }
				);
				return;
			}

			if(scope.params.contains(ident_str)){
				const PIR::Param& param = this->source.getParam(scope.params.at(ident_str));
				const Location location = this->source.getToken(param.ident).location;

				this->source.error(
					std::format("Identifier \"{}\" already defined", ident.value.string), ident,
					std::vector<Message::Info>{ Message::Info("First defined here:", location) }
				);
				return;
			}

			if(scope.imports.contains(ident_str)){
				const Import& import = scope.imports.at(ident_str);
				const Location location = this->source.getIdent(import.ident).location;

				this->source.error(
					std::format("Identifier \"{}\" already defined", ident.value.string), ident,
					std::vector<Message::Info>{ Message::Info("First defined here:", location) }
				);
				return;
			}
		}

		EVO_FATAL_BREAK("Didn't find ident");
	};



	//////////////////////////////////////////////////////////////////////
	// scope

	auto SemanticAnalyzer::enter_scope(PIR::StmtBlock* stmts_entry) noexcept -> void {
		this->scopes.emplace_back(stmts_entry);
	};

	auto SemanticAnalyzer::leave_scope() noexcept -> void {
		this->scopes.pop_back();
	};


	auto SemanticAnalyzer::add_var_to_scope(std::string_view str, PIR::Var::ID id) noexcept -> void {
		this->scopes.back().vars.emplace(str, id);
	};

	auto SemanticAnalyzer::add_func_to_scope(std::string_view str, PIR::Func::ID id) noexcept -> void {
		Scope& current_scope = this->scopes.back();

		using FuncScopeListIter = std::unordered_map<std::string_view, std::vector<PIR::Func::ID>>::iterator;
		FuncScopeListIter func_scope_list_iter = current_scope.funcs.find(str);
		if(func_scope_list_iter != current_scope.funcs.end()){
			// add to existing list
			func_scope_list_iter->second.emplace_back(id);
		}else{
			// create new list
			auto new_func_list = std::vector<PIR::Func::ID>{id};
			current_scope.funcs.emplace(str, std::move(new_func_list));
		}
	};

	auto SemanticAnalyzer::add_param_to_scope(std::string_view str, PIR::Param::ID id) noexcept -> void {
		this->scopes.back().params.emplace(str, id);
	};

	auto SemanticAnalyzer::add_import_to_scope(std::string_view str, Import import) noexcept -> void {
		this->scopes.back().imports.emplace(str, import);
	};


	auto SemanticAnalyzer::set_scope_terminated() noexcept -> void {
		this->scopes.back().is_terminated = true;
	};

	auto SemanticAnalyzer::scope_is_terminated() const noexcept -> bool {
		return this->scopes.back().is_terminated;
	};


	auto SemanticAnalyzer::get_stmts_entry() noexcept -> PIR::StmtBlock& {
		Scope& current_scope = this->scopes.back();

		evo::debugAssert(current_scope.stmts_entry != nullptr, "Cannot get stmts entry as it doesn't exist for this scope");

		return *current_scope.stmts_entry;
	};

	auto SemanticAnalyzer::has_in_scope(std::string_view ident) const noexcept -> bool {
		for(const Scope& scope : this->scopes){
			if(scope.vars.contains(ident)){ return true; }
			if(scope.funcs.contains(ident)){ return true; }
			if(scope.params.contains(ident)){ return true; }
			if(scope.imports.contains(ident)){ return true; }
		}

		return false;
	};


	auto SemanticAnalyzer::is_in_func_base_scope() const noexcept -> bool {
		return this->scopes.back().stmts_entry == &this->get_current_func().func.stmts;
	};


	auto SemanticAnalyzer::lookup_func_in_scope(std::string_view ident, const AST::FuncCall& func_call) const noexcept -> evo::Result<PIR::Func::ID> {
		auto func_list = std::vector<PIR::Func::ID>();
		
		for(const Scope& scope : this->scopes){
			using ConstFuncScopeListIter = std::unordered_map<std::string_view, std::vector<PIR::Func::ID>>::const_iterator;
			ConstFuncScopeListIter func_scope_list_iter = scope.funcs.find(ident);
			if(func_scope_list_iter == scope.funcs.end()){ continue; }

			for(PIR::Func::ID func : func_scope_list_iter->second){
				func_list.emplace_back(func);
			}
		}

		return this->match_function_to_overloads(ident, func_call, func_list);
	};


	auto SemanticAnalyzer::lookup_func_in_import(std::string_view ident, const Source& import, const AST::FuncCall& func_call) const noexcept
	-> evo::Result<PIR::Func::ID> {
		using ConstPubFuncListIter = std::unordered_map<std::string_view, std::vector<PIR::Func::ID>>::const_iterator;
		ConstPubFuncListIter pub_func_list_iter = import.pir.pub_funcs.find(ident);

		if(pub_func_list_iter != import.pir.pub_funcs.end()){
			return this->match_function_to_overloads(ident, func_call, pub_func_list_iter->second);
		}else{
			return evo::resultError;
		}
	};




	auto SemanticAnalyzer::match_function_to_overloads(
		std::string_view ident, const AST::FuncCall& func_call, evo::ArrayProxy<PIR::Func::ID> overload_list
	) const noexcept -> evo::Result<PIR::Func::ID> {
		if(overload_list.empty()){
			this->source.error(std::format("function \"{}\" is undefined", ident), func_call.target);
			return evo::resultError;
		}

		// if only has one function, use SemanticAnalyzer::check_func_call() which is able to give better error messages
		if(overload_list.size() == 1){
			if(this->check_func_call(func_call, this->src_manager.getOrCreateTypeID(PIR::Type(Source::getFunc(overload_list[0]).baseType))) == false){
				return evo::resultError;
			}

			return overload_list[0];
		}


		// find list of candidates
		auto overload_list_candidates = std::vector<PIR::Func::ID>();
		for(PIR::Func::ID overload_list_id : overload_list){
			const PIR::Func& func_candidate = Source::getFunc(overload_list_id);

			const PIR::BaseType& base_type = this->src_manager.getBaseType(func_candidate.baseType);

			if(base_type.callOperator->params.size() != func_call.args.size()){ continue; }

			bool func_is_candidate = true;

			// checking params
			// TODO: redo this section to not redo work
			for(size_t i = 0; i < base_type.callOperator->params.size(); i+=1){
				if(!func_is_candidate){
					break;
				}

				const PIR::BaseType::Operator::Param& param = base_type.callOperator->params[i];
				const AST::Node::ID arg_id = func_call.args[i];
				const AST::Node& arg_node = this->source.getNode(arg_id);

				// check types match
				const evo::Result<PIR::Type::ID> arg_type_id = this->analyze_and_get_type_of_expr(arg_node);
				if(arg_type_id.isError()){
					func_is_candidate = false;
					break;
				}


				if(param.type != arg_type_id.value()){
					const PIR::Type& param_type = this->src_manager.getType(param.type);
					const PIR::Type& arg_type = this->src_manager.getType(arg_type_id.value());

					if(this->is_implicitly_convertable_to(arg_type, param_type, arg_node) == false){
						func_is_candidate = false;
						break;
					}
				}


			
				// check param kind accepts arg value type
				const evo::Result<ExprValueType> arg_value_type = this->get_expr_value_type(arg_id);
				if(arg_value_type.isError()){
					func_is_candidate = false;
					break;
				}

				using ParamKind = AST::FuncParams::Param::Kind;
				switch(param.kind){
					case ParamKind::Read: {
						// accepts any value type
					} break;

					case ParamKind::Write: {
						if(arg_value_type.value() != ExprValueType::Concrete){
							func_is_candidate = false;
							break;
						}

						if(this->is_expr_mutable(arg_id) == false){
							func_is_candidate = false;
							break;
						}
					} break;

					case ParamKind::In: {
						if(arg_value_type.value() != ExprValueType::Ephemeral){
							func_is_candidate = false;
							break;
						}
					} break;
				};

				// if param is write and arg is a param, mark it as edited
				// if(arg_node.kind == AST::Kind::Ident && param.kind == ParamKind::Write){
				// 	std::string_view param_ident_str = this->source.getIdent(arg_node).value.string;

				// 	for(const Scope& scope : this->scopes){
	 		// 			if(scope.params.contains(param_ident_str)){
				// 			PIR::Param& pir_param = this->source.getParam(scope.params.at(param_ident_str));
				// 			pir_param.mayHaveBeenEdited = true;
				// 		}
				// 	}
				// }
			}

			if(func_is_candidate){
				overload_list_candidates.emplace_back(overload_list_id);
			}
		}


		if(overload_list_candidates.empty()){
			// TODO: better messaging
			this->source.error("No matching function overload found", func_call.target);
			return evo::resultError;
		}

		if(overload_list_candidates.size() == 1){
			return overload_list_candidates[0];
		}

		// TODO: better messaging
		// TODO: deal with this better
		this->source.error("multiple function overload candidates found", func_call.target);
		return evo::resultError;
	};




	//////////////////////////////////////////////////////////////////////
	// func scope

	auto SemanticAnalyzer::in_func_scope() const noexcept -> bool {
		return this->func_scopes.empty() == false;
	};

	auto SemanticAnalyzer::enter_func_scope(PIR::Func& func) noexcept -> void {
		this->func_scopes.emplace_back(func);
	};

	auto SemanticAnalyzer::leave_func_scope() noexcept -> void {
		evo::debugAssert(this->in_func_scope(), "Not in a func scope");
		this->func_scopes.pop_back();
	};


	auto SemanticAnalyzer::get_current_func() noexcept -> FuncScope& {
		evo::debugAssert(this->in_func_scope(), "Not in a func scope");
		return this->func_scopes.back();
	};

	auto SemanticAnalyzer::get_current_func() const noexcept -> const FuncScope& {
		evo::debugAssert(this->in_func_scope(), "Not in a func scope");
		return this->func_scopes.back();
	};


	// auto SemanticAnalyzer::set_current_func_terminated() noexcept -> void {
	// 	evo::debugAssert(this->in_func_scope(), "Not in a func scope");
	// 	this->func_scopes.back().is_terminated = true;
	// };

	// auto SemanticAnalyzer::current_func_is_terminated() const noexcept -> bool {
	// 	evo::debugAssert(this->in_func_scope(), "Not in a func scope");
	// 	return this->func_scopes.back().is_terminated;
	// };



	//////////////////////////////////////////////////////////////////////
	// scope level

	auto SemanticAnalyzer::enter_scope_level() noexcept -> void {
		this->scope_levels.emplace_back();
	};

	auto SemanticAnalyzer::leave_scope_level() noexcept -> void {
		const bool scope_level_terminated = [&]() noexcept {
			const ScopeLevel& scope_level = this->scope_levels.back();
			return scope_level.num_scopes == scope_level.num_terminated;
		}();

		this->scope_levels.pop_back();

		if(scope_level_terminated){
			if(this->scope_levels.empty() == false){
				this->add_scope_level_terminated();
			}

			this->set_scope_terminated();
		}
	};

	auto SemanticAnalyzer::add_scope_level_scope() noexcept -> void {
		this->scope_levels.back().num_scopes += 1;
	};

	auto SemanticAnalyzer::add_scope_level_terminated() noexcept -> void {
		this->scope_levels.back().num_terminated += 1;
	};

	
};
