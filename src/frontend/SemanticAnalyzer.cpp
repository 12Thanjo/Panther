#include "SemanticAnalyzer.h"

#include "frontend/SourceManager.h"

namespace panther{
	

	auto SemanticAnalyzer::semantic_analysis() noexcept -> bool {
		this->enter_scope(nullptr);

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
		// type checking

		const ExprValueType expr_value_type = this->get_expr_value_type(var_decl.expr);
		if(expr_value_type != ExprValueType::Ephemeral){
			// TODO: better messaging
			this->source.error("Variables must be assigned with a ephemeral value", var_decl.expr);
			return false;
		}


		SourceManager& src_manager = this->source.getSourceManager();

		const evo::Result<PIR::Type::VoidableID> var_type_id = this->get_type_id(var_decl.type);
		if(var_type_id.isError()){ return false; }

		if(var_type_id.value().isVoid()){
			this->source.error("Variable cannot be of type Void", var_decl.type);
			return false;
		}


		
		const AST::Node& expr_node = this->source.getNode(var_decl.expr);
		if(expr_node.kind != AST::Kind::Uninit){
			const evo::Result<PIR::Type::ID> expr_type_id = this->analyze_and_get_type_of_expr(expr_node);
			if(expr_type_id.isError()){ return false; }

			if(var_type_id.value().typeID() != expr_type_id.value()){
				const PIR::Type& var_type = src_manager.getType(var_type_id.value().typeID());
				const PIR::Type& expr_type = src_manager.getType(expr_type_id.value());

				if(expr_type.isImplicitlyConvertableTo(var_type) == false){
					this->source.error(
						"Variable cannot be assigned a value of a different type, and cannot be implicitly converted", 
						var_decl.expr,

						std::vector<Message::Info>{
							{std::string("Variable is of type:   ") + src_manager.printType(var_type_id.value().typeID())},
							{std::string("Expression is of type: ") + src_manager.printType(expr_type_id.value())}
						}
					);
					return false;
				}

			}
		}



		///////////////////////////////////
		// get / check value

		const evo::Result<PIR::Expr> var_value = [&]() noexcept {
			if(this->is_global_scope()){
				return this->get_const_expr_value(var_decl.expr);

			}else{
				return evo::Result<PIR::Expr>(this->get_expr_value(var_decl.expr));
			}
		}();

		if(var_value.isError()){ return false; }



		if(var_value.value().kind == PIR::Expr::Kind::ASTNode){
			const AST::Node& var_value_node = this->source.getNode(var_value.value().ast_node);

			if(var_value_node.kind == AST::Kind::Uninit){
				if(this->is_global_scope()){
					this->source.error("Global variables cannot be initialized with the value \"uninit\"", var_decl.expr);
					return false;
				}

				if(var_decl.is_def){
					this->source.warning(
						"Declared a def variable with the value \"uninit\"", var_decl.expr,
						std::vector<Message::Info>{ {"Any use of this variable would be undefined behavior"} }
					);
				}
			}
		}
		

		///////////////////////////////////
		// create object

		const PIR::Var::ID var_id = this->source.createVar(ident_tok_id, var_type_id.value().typeID(), var_value.value(), var_decl.is_def);

		this->add_var_to_scope(ident.value.string, var_id);

		if(this->is_global_scope()){
			this->source.pir.global_vars.emplace_back(var_id);
		}else{
			this->get_stmts_entry().emplace_back(var_id);
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


		///////////////////////////////////
		// return type

		const evo::Result<PIR::Type::VoidableID> return_type_id = this->get_type_id(func.return_type);
		if(return_type_id.isError()){ return false; }


		///////////////////////////////////
		// create base type

		auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
		base_type.call_operator = PIR::BaseType::Operator(std::move(param_type_ids)	, return_type_id.value());

		const PIR::BaseType::ID base_type_id = this->source.getSourceManager().createBaseType(std::move(base_type));


		///////////////////////////////////
		// create object

		const PIR::Func::ID func_id = this->source.createFunc(ident_tok_id, base_type_id, std::move(params), return_type_id.value(), is_export);

		this->add_func_to_scope(ident.value.string, func_id);




		if(is_entry){
			SourceManager& src_manager = this->source.getSourceManager();

			// check is valid return type
			if(return_type_id.value().isVoid() || return_type_id.value().typeID() != src_manager.getTypeInt()){
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


		PIR::Func& pir_func = this->source.pir.funcs[func_id.id];
		if(this->analyze_func_block(pir_func, func) == false){
			return false;
		};


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
			if(pir_func.return_type.isVoid()){
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

		return true;
	};






	auto SemanticAnalyzer::analyze_conditional(const AST::Conditional& cond) noexcept -> bool {
		this->enter_scope_level();

		const bool analyze_conditional_result = analyze_conditional_recursive(cond);

		this->leave_scope_level();

		return analyze_conditional_result;
	};


	auto SemanticAnalyzer::analyze_conditional_recursive(const AST::Conditional& sub_cond) noexcept -> bool {
		SourceManager& src_manager = this->source.getSourceManager();

		if(this->in_func_scope() == false){
			this->source.error("Conditional statements can only be inside functions", sub_cond.if_tok);
			return false;
		}


		///////////////////////////////////
		// condition

		const AST::Node& cond_expr = this->source.getNode(sub_cond.if_expr);

		const evo::Result<PIR::Type::ID> cond_type_id = this->analyze_and_get_type_of_expr(cond_expr);
		if(cond_type_id.isError()){ return false; }

		if(cond_type_id.value() != src_manager.getTypeBool()){
			this->source.error(
				"Conditional expression must be a boolean", cond_expr,
				std::vector<Message::Info>{
					{std::string("Conditional expression is of type: ") + src_manager.printType(cond_type_id.value())}
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
		const AST::Block& then_block = this->source.getBlock(sub_cond.then_block);
		if(this->analyze_block(then_block, then_stmts) == false){ return false; }


		///////////////////////////////////
		// else block

		if(sub_cond.else_block.has_value()){
			const AST::Node& else_block_node = this->source.getNode(*sub_cond.else_block);

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

		const PIR::Conditional::ID cond_id = this->source.createConditional(
			this->get_expr_value(sub_cond.if_expr), std::move(then_stmts), std::move(else_stmts)
		);
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

		const PIR::Type::VoidableID func_return_type_id = this->get_current_func().func.return_type;

		std::optional<PIR::Expr> return_value = std::nullopt;

		if(return_stmt.value.has_value()){
			// "return expr;"

			if(this->get_current_func().func.return_type.isVoid()){
				this->source.error("Return statement has value when function's return type is \"Void\"", return_stmt.keyword);
				return false;	
			}

			const AST::Node& expr_node = this->source.getNode(*return_stmt.value);

			const evo::Result<PIR::Type::ID> expr_type_id = this->analyze_and_get_type_of_expr(expr_node);
			if(expr_type_id.isError()){ return false; }


			if(expr_type_id.value() != func_return_type_id.typeID()){
				SourceManager& src_manager = this->source.getSourceManager();

				const PIR::Type& func_return_type = src_manager.getType(func_return_type_id.typeID());
				const PIR::Type& expr_type = src_manager.getType(expr_type_id.value());

				if(expr_type.isImplicitlyConvertableTo(func_return_type) == false){
					this->source.error(
						"Return value type and function return type do not match", 
						expr_node,

						std::vector<Message::Info>{
							{std::string("Function return is type: ") + src_manager.printType(func_return_type_id.typeID())},
							{std::string("Return value is of type: ") + src_manager.printType(expr_type_id.value())}
						}
					);

					return false;
				}
			}

			return_value = this->get_expr_value(*return_stmt.value);


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
			this->get_current_func().func.terminates_in_base_scope = true;
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

		const ExprValueType dst_value_type = this->get_expr_value_type(infix.lhs);
		if(dst_value_type != ExprValueType::Concrete){
			this->source.error("Only concrete values may be assigned to", infix.lhs);
			return false;
		}

		const evo::Result<PIR::Type::ID> dst_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(infix.lhs));
		if(dst_type_id.isError()){ return false; }

		if(this->is_expr_mutable(infix.lhs) == false){
			this->source.error("Only mutable values may be assigned to", infix.lhs);
			return false;
		}


		///////////////////////////////////
		// checking of rhs

		const ExprValueType expr_value_type = this->get_expr_value_type(infix.rhs);
		if(expr_value_type != ExprValueType::Ephemeral){
			this->source.error("Only ephemeral values may be assignment values", infix.rhs);
			return false;
		}

		const evo::Result<PIR::Type::ID> expr_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(infix.rhs));
		if(expr_type_id.isError()){ return false; }


		///////////////////////////////////
		// type checking

		if(dst_type_id.value() != expr_type_id.value()){
			SourceManager& src_manager = this->source.getSourceManager();

			const PIR::Type& dst_type = src_manager.getType(dst_type_id.value());
			const PIR::Type& expr_type = src_manager.getType(expr_type_id.value());

			if(expr_type.isImplicitlyConvertableTo(dst_type) == false){
				this->source.error(
					"The types of the left-hand-side and right-hand-side of an assignment statement do not match, and cannot be implicitly converted",
					infix.rhs,

					std::vector<Message::Info>{
						{std::string("left-hand-side is of type:  ") + src_manager.printType(dst_type_id.value())},
						{std::string("right-hand-side is of type: ") + src_manager.printType(expr_type_id.value())}
					}
				);
			}
		}


		///////////////////////////////////
		// create object

		const PIR::Assignment::ID assignment_id = this->source.createAssignment(
			this->get_expr_value(infix.lhs), infix.op, this->get_expr_value(infix.rhs)
		);
		this->get_stmts_entry().emplace_back(assignment_id);


		return true;
	};




	auto SemanticAnalyzer::analyze_func_call(const AST::FuncCall& func_call) noexcept -> bool {
		// analyze and get type of ident
		const evo::Result<PIR::Type::ID> target_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(func_call.target));
		if(target_type_id.isError()){ return false; }

		const SourceManager& src_manager = this->source.getSourceManager();

		if(this->check_func_call(func_call, target_type_id.value()) == false){ return false; }


		const std::vector<PIR::Expr> args = this->get_func_call_args(func_call);


		switch(this->source.getNode(func_call.target).kind){
			case AST::Kind::Ident: {
				const Token& ident_tok = this->source.getIdent(func_call.target);

				// get_func
				const PIR::Func::ID func_id = [&]() noexcept {
					for(Scope& scope : this->scopes){
						auto find = scope.funcs.find(ident_tok.value.string);
						if(find != scope.funcs.end()){
							return find->second;
						}
					}

					EVO_FATAL_BREAK("Unknown func ident");
				}();


				// create object
				const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(func_id, std::move(args));
				this->get_stmts_entry().emplace_back(func_call_id);

				return true;
			} break;
			
			case AST::Kind::Intrinsic: {
				const Token& intrinsic_tok = this->source.getIntrinsic(func_call.target);

				const std::vector<PIR::Intrinsic>& intrinsics = src_manager.getIntrinsics();

				for(size_t i = 0; i < intrinsics.size(); i+=1){
					const PIR::Intrinsic& intrinsic = intrinsics[i];

					if(intrinsic.ident == intrinsic_tok.value.string){
						// create object
						const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(PIR::IntrinsicID(uint32_t(i)) , std::move(args));
						this->get_stmts_entry().emplace_back(func_call_id);
						
						return true;
					}
				}


				EVO_FATAL_BREAK("Unknown intrinsic func");
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
			this->get_current_func().func.terminates_in_base_scope = true;
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
		const SourceManager& src_manager = this->source.getSourceManager();

		const PIR::Type& type = src_manager.getType(type_id);

		if(type.qualifiers.empty() == false){
			this->source.error(
				"cannot be called like a function", func_call.target,
				std::vector<Message::Info>{ std::format("Type \"{}\" does not have a call operator", src_manager.printType(type_id)) }
			);
			return false;
		}

		const PIR::BaseType& base_type = src_manager.getBaseType(type.base_type);
		if(base_type.call_operator.has_value() == false){
			// TODO: better messaging?
			this->source.error(
				"cannot be called like a function", func_call.target,
				std::vector<Message::Info>{ std::format("Type \"{}\" does not have a call operator", src_manager.printType(type_id)) }
			);
			return false;
		}


		if(base_type.call_operator->params.size() != func_call.args.size()){
			// TODO: better messaging
			this->source.error("Function call number of arguments do not match function", func_call.target);
			return false;
		}


		for(size_t i = 0; i < base_type.call_operator->params.size(); i+=1){
			const PIR::BaseType::Operator::Param& param = base_type.call_operator->params[i];
			const AST::Node::ID arg_id = func_call.args[i];
			const AST::Node& arg = this->source.getNode(arg_id);

			// check types match
			const evo::Result<PIR::Type::ID> arg_type_id = this->analyze_and_get_type_of_expr(arg);
			if(arg_type_id.isError()){ return false; }

			if(param.type != arg_type_id.value()){
				const PIR::Type& param_type = src_manager.getType(param.type);
				const PIR::Type& arg_type = src_manager.getType(arg_type_id.value());

				if(param_type.isImplicitlyConvertableTo(arg_type) == false){
					// TODO: better messaging
					this->source.error("Function call arguments do not match function", func_call.target);
					return false;
				}
			}

			// check param kind accepts arg value type
			const ExprValueType arg_value_type = this->get_expr_value_type(arg_id);

			using ParamKind = AST::FuncParams::Param::Kind;
			switch(param.kind){
				case ParamKind::Read: {
					// accepts any value type
				} break;

				case ParamKind::Write: {
					if(arg_value_type != ExprValueType::Concrete){
						this->source.error("write parameters require concrete expression values", arg_id);
						return false;
					}

					if(this->is_expr_mutable(arg_id) == false){
						this->source.error("write parameters require mutable expression values", arg_id);
						return false;
					}
				} break;

				case ParamKind::In: {
					if(arg_value_type != ExprValueType::Ephemeral){
						this->source.error("write parameters require ephemeral expression values", arg_id);
						return false;
					}
				} break;
			};
		}

		return true;
	};


	auto SemanticAnalyzer::get_func_call_args(const AST::FuncCall& func_call) const noexcept -> std::vector<PIR::Expr> {
		auto args = std::vector<PIR::Expr>();

		for(AST::Node::ID arg_id : func_call.args){
			args.emplace_back(this->get_expr_value(arg_id));
		}

		return args;
	};







	auto SemanticAnalyzer::analyze_and_get_type_of_expr(const AST::Node& node) const noexcept -> evo::Result<PIR::Type::ID> {
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

					}else if(scope.params.contains(ident_str)){
						const PIR::Param& param = this->source.getParam(scope.params.at(ident_str));
						return param.type;
					}
				}


				this->source.error(std::format("Identifier \"{}\" is undefined", ident_str), node);
				return evo::ResultError;
			} break;


			case AST::Kind::Intrinsic: {
				const Token& intrinsic_tok = this->source.getIntrinsic(node);

				for(const PIR::Intrinsic& intrinsic : src_manager.getIntrinsics()){
					if(intrinsic.ident == intrinsic_tok.value.string){
						return src_manager.getTypeID(PIR::Type(intrinsic.base_type));
					}
				}


				this->source.error(std::format("Intrinsic \"@{}\" does not exist", intrinsic_tok.value.string), intrinsic_tok);
				return evo::ResultError;
			} break;


			case AST::Kind::FuncCall: {
				const AST::FuncCall& func_call = this->source.getFuncCall(node);

				// get target type
				const evo::Result<PIR::Type::ID> target_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(func_call.target));
				if(target_type_id.isError()){ return evo::ResultError; }


				// check that it's a function type
				const PIR::Type& type = src_manager.getType(target_type_id.value());
				const PIR::BaseType& base_type = src_manager.getBaseType(type.base_type);

				if(this->check_func_call(func_call, target_type_id.value()) == false){ return evo::ResultError; }

				const PIR::Type::VoidableID return_type = base_type.call_operator->return_type;
				if(return_type.isVoid()){
					this->source.error("This function does not return a value", func_call.target);
					return evo::ResultError;
				}

				return return_type.typeID();
			} break;


			case AST::Kind::Prefix: {
				const AST::Prefix& prefix = this->source.getPrefix(node);

				switch(this->source.getToken(prefix.op).kind){
					case Token::KeywordCopy: {
						const ExprValueType expr_value_type = this->get_expr_value_type(prefix.rhs);
						if(expr_value_type != ExprValueType::Concrete){
							this->source.error("right-hand-side of copy expression must be a concrete expression", prefix.rhs);
							return evo::ResultError;
						}

						return this->analyze_and_get_type_of_expr(this->source.getNode(prefix.rhs));
					} break;


					case Token::KeywordAddr: {
						// check value type
						const ExprValueType expr_value_type = this->get_expr_value_type(prefix.rhs);
						if(expr_value_type != ExprValueType::Concrete){
							this->source.error("right-hand-side of addr expression must be a concrete expression", prefix.rhs);
							return evo::ResultError;
						}

						// get type of rhs
						const evo::Result<PIR::Type::ID> type_of_rhs = this->analyze_and_get_type_of_expr(this->source.getNode(prefix.rhs));
						if(type_of_rhs.isError()){ return evo::ResultError; }

						const PIR::Type& rhs_type = this->source.getSourceManager().getType(type_of_rhs.value());
						PIR::Type rhs_type_copy = rhs_type;


						// make the type pointer of the type of rhs
						rhs_type_copy.qualifiers.emplace_back(true, !this->is_expr_mutable(prefix.rhs));

						return this->source.getSourceManager().getTypeID(rhs_type_copy);
					} break;

					default: EVO_FATAL_BREAK("Unknown prefix operator");
				};

			} break;


			case AST::Kind::Postfix: {
				const AST::Postfix& postfix = this->source.getPostfix(node);

				switch(this->source.getToken(postfix.op).kind){
					case Token::get(".^"): {
						// get type of lhs
						const evo::Result<PIR::Type::ID> type_of_lhs = this->analyze_and_get_type_of_expr(this->source.getNode(postfix.lhs));
						if(type_of_lhs.isError()){ return evo::ResultError; }

						// check that type of lhs is pointer
						const PIR::Type& lhs_type = this->source.getSourceManager().getType(type_of_lhs.value());
						if(lhs_type.qualifiers.empty() || lhs_type.qualifiers.back().is_ptr == false){
							this->source.error(
								"left-hand-side of dereference expression must be of a pointer type", postfix.op,
								std::vector<Message::Info>{
									{std::string("expression is of type: ") + src_manager.printType(type_of_lhs.value())},
								}
							);
							return evo::ResultError;
						}

						// get dereferenced type
						PIR::Type lhs_type_copy = lhs_type;
						lhs_type_copy.qualifiers.pop_back();

						return this->source.getSourceManager().getTypeID(lhs_type_copy);
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
				return evo::ResultError;
			}

			return PIR::Type::VoidableID::Void();
		}



		std::vector<AST::Type::Qualifier> type_qualifiers = type.qualifiers;

		{
			// checking const-ness of type levels
			bool type_qualifiers_has_a_const = false;
			bool has_warned = false;
			for(auto i = type_qualifiers.rbegin(); i != type_qualifiers.rend(); ++i){
				if(type_qualifiers_has_a_const){
					if(i->is_const == false && has_warned == false){
						has_warned = true;
						this->source.warning("If one type level is const, all previous levels will automatically be made const as well", node_id);
					}

					i->is_const = true;

				}else if(i->is_const){
					type_qualifiers_has_a_const = true;
				}
			}
		}
		

		SourceManager& src_manager = this->source.getSourceManager();

		return PIR::Type::VoidableID(
			src_manager.getTypeID(
				PIR::Type(src_manager.getBaseTypeID(type_token.kind), type_qualifiers)
			)
		);
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

					if(scope.params.contains(value_ident_str)){
						return PIR::Expr(scope.params.at(value_ident_str));
					}
				}

				EVO_FATAL_BREAK("Didn't find value_ident");
			} break;


			case AST::Kind::FuncCall: {
				const AST::FuncCall& func_call = this->source.getFuncCall(node_id);
				const std::string_view ident = this->source.getIdent(func_call.target).value.string;

				const PIR::Func::ID func_id = [&]() noexcept {
					for(const Scope& scope : this->scopes){
						auto find = scope.funcs.find(ident);
						if(find != scope.funcs.end()){
							return find->second;
						}
					}

					EVO_FATAL_BREAK("Unknown func ident");
				}();

				const std::vector<PIR::Expr> args = this->get_func_call_args(func_call);

				const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(func_id, std::move(args));
				return PIR::Expr(func_call_id);
			} break;


			case AST::Kind::Prefix: {
				const AST::Prefix& prefix = this->source.getPrefix(node_id);

				const PIR::Prefix::ID prefix_id = this->source.createPrefix(prefix.op, this->get_expr_value(prefix.rhs));
				return PIR::Expr(prefix_id);
			} break;


			case AST::Kind::Postfix: {
				const AST::Postfix& postfix = this->source.getPostfix(node_id);

				// get deref type
				const evo::Result<PIR::Type::ID> ptr_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(postfix.lhs));
				evo::debugAssert(ptr_type_id.isSuccess(), "Failed to get deref type - should have caught error earlier");

				const PIR::Type& ptr_type = this->source.getSourceManager().getType(ptr_type_id.value());
				PIR::Type ptr_type_copy = ptr_type;
				ptr_type_copy.qualifiers.pop_back();
				const PIR::Type::ID deref_type_id = this->source.getSourceManager().getTypeID(ptr_type_copy);

				const PIR::Deref::ID deref_id = this->source.createDeref(this->get_expr_value(postfix.lhs), deref_type_id);
				return PIR::Expr(deref_id);
			} break;



			case AST::Kind::Literal: {
				return PIR::Expr(node_id);
			} break;

			case AST::Kind::Uninit: {
				return PIR::Expr(node_id);
			} break;


			default: EVO_FATAL_BREAK("Unknown node value kind");
		};

	};



	auto SemanticAnalyzer::get_const_expr_value(AST::Node::ID node_id) const noexcept -> evo::Result<PIR::Expr> {
		const evo::Result<PIR::Expr> recursive_value = this->get_const_expr_value_recursive(node_id);
		if(recursive_value.isError()){ return evo::ResultError; }

		if(recursive_value.value().kind == PIR::Expr::Kind::Var){
			const PIR::Var& value_var = this->source.getVar(recursive_value.value().var);
			return value_var.value;
		}

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
						return PIR::Expr(scope.vars.at(value_ident_str));
					}

					if(scope.params.contains(value_ident_str)){
						return PIR::Expr(scope.params.at(value_ident_str));
					}
				}

				EVO_FATAL_BREAK("Unkown ident");
			} break;

			case AST::Kind::FuncCall: {
				this->source.error("At this time, constant values cannot be function calls", node);
				return evo::ResultError;
			} break;

			case AST::Kind::Prefix: {
				const AST::Prefix& prefix = this->source.getPrefix(node);

				switch(this->source.getToken(prefix.op).kind){
					case Token::KeywordCopy: {					
						return this->get_const_expr_value_recursive(prefix.rhs);
					} break;

					case Token::KeywordAddr: {
						const evo::Result<PIR::Expr> rhs_value = this->get_const_expr_value_recursive(prefix.rhs);
						if(rhs_value.isError()){ return evo::ResultError; }

						const PIR::Prefix::ID prefix_id = this->source.createPrefix(prefix.op, rhs_value.value());
						return PIR::Expr(prefix_id);
					} break;

					default: EVO_FATAL_BREAK("Unknown prefix kind");
				};
			} break;

			case AST::Kind::Postfix: {
				this->source.error("At this time, constant values cannot be postfix operations", node);
				return evo::ResultError;
			} break;

			case AST::Kind::Literal: {
				return PIR::Expr(node_id);
			} break;

			case AST::Kind::Uninit: {
				this->source.error("Constant values cannot be \"uninit\"", node);
				return evo::ResultError;
			} break;

			default: EVO_FATAL_BREAK("Unknown node value kind");
		};

	};






	auto SemanticAnalyzer::get_expr_value_type(AST::Node::ID node_id) const noexcept -> ExprValueType {
		const AST::Node& value_node = this->source.getNode(node_id);

		switch(value_node.kind){
			break; case AST::Kind::Prefix: return ExprValueType::Ephemeral;
			break; case AST::Kind::FuncCall: return ExprValueType::Ephemeral;

			break; case AST::Kind::Postfix: {
				const AST::Postfix& postfix = this->source.getPostfix(value_node);

				switch(this->source.getToken(postfix.op).kind){
					break; case Token::get(".^"): return ExprValueType::Concrete;

					default: EVO_FATAL_BREAK("Unknown postfix kind");
				};
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
			break; case AST::Kind::Prefix: EVO_FATAL_BREAK("Not concreted");
			break; case AST::Kind::FuncCall: EVO_FATAL_BREAK("Not concreted");

			break; case AST::Kind::Postfix: {
				const AST::Postfix& postfix = this->source.getPostfix(value_node);

				switch(this->source.getToken(postfix.op).kind){
					case Token::get(".^"): {
						const evo::Result<PIR::Type::ID> lhs_type_id = this->analyze_and_get_type_of_expr(this->source.getNode(postfix.lhs));
						evo::debugAssert(lhs_type_id.isSuccess(), "Should have caught this error already");

						const PIR::Type& lhs_type = this->source.getSourceManager().getType(lhs_type_id.value());
						evo::debugAssert(lhs_type.qualifiers.back().is_ptr, "Should have already been checked that this type is a pointer");

						return !lhs_type.qualifiers.back().is_const;

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
						return !var.is_def;
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
				}

				EVO_FATAL_BREAK("Didn't find value_ident");
			} break;

			break; case AST::Kind::Intrinsic: return false;

			break; case AST::Kind::Literal: EVO_FATAL_BREAK("Not concreted");
			break; case AST::Kind::Uninit: EVO_FATAL_BREAK("Not concreted");
		};

		EVO_FATAL_BREAK("Unknown AST node kind")
	};







	// TODO: check for exported functions in all files (through saving in SourceManager)
	auto SemanticAnalyzer::is_valid_export_name(std::string_view name) const noexcept -> bool {
		if(name == "main"){ return false; }
		if(name == "puts"){ return false; }

		return true;
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

				if(scope.params.contains(ident_str)){
					const PIR::Param& param = this->source.getParam( scope.params.at(ident_str) );
					return this->source.getToken(param.ident);
				}
			}

			EVO_FATAL_BREAK("Didn't find ident");
		}();


		this->source.error(
			std::format("Identifier \"{}\" already defined", ident.value.string), ident,
			std::vector<Message::Info>{
				{"First defined here:", token.location}
			}
		);
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
		this->scopes.back().funcs.emplace(str, id);
	};

	auto SemanticAnalyzer::add_param_to_scope(std::string_view str, PIR::Param::ID id) noexcept -> void {
		this->scopes.back().params.emplace(str, id);
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
		}

		return false;
	};


	auto SemanticAnalyzer::is_in_func_base_scope() const noexcept -> bool {
		return this->scopes.back().stmts_entry == &this->get_current_func().func.stmts;
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
