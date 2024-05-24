#include "SemanticAnalyzer.h"

#include "frontend/SourceManager.h"

#include <queue>
#include <unordered_set>

namespace panther{



	//////////////////////////////////////////////////////////////////////
	// semantic analysis passes

	auto SemanticAnalyzer::semantic_analysis_global_idents_and_imports() noexcept -> bool {
		this->scope_managers.emplace_back(std::make_unique<ScopeManager>(this->scope_alloc, this->type_scope_alloc, this->scope_levels_alloc));
		this->scope_managers[0]->enter_scope(nullptr);


		for(AST::Node::ID global_stmt : this->source.global_stmts){
			const AST::Node& node = this->source.getNode(global_stmt);

			switch(node.kind){
				case AST::Kind::VarDecl: {
					if(this->analyze_var(this->source.getVarDecl(node), *this->scope_managers[0]) == false){
						return false;
					}
				} break;

				case AST::Kind::Struct: {
					if(this->analyze_struct(this->source.getStruct(node), *this->scope_managers[0]) == false){
						return false;
					}
				} break;
			};
		}


		return true;
	};



	auto SemanticAnalyzer::semantic_analysis_global_aliases() noexcept -> bool {
		for(AST::Node::ID global_stmt : this->source.global_stmts){
			const AST::Node& node = this->source.getNode(global_stmt);

			switch(node.kind){
				case AST::Kind::Alias: {
					if(this->analyze_alias(this->source.getAlias(node), *this->scope_managers[0]) == false){
						return false;
					}
				} break;
			};
		}

		return true;
	};


	auto SemanticAnalyzer::semantic_analysis_global_types() noexcept -> bool {
		for(AST::Node::ID global_stmt : this->source.global_stmts){
			const AST::Node& node = this->source.getNode(global_stmt);

			if(node.kind == AST::Kind::Func){
				if(this->analyze_func(this->source.getFunc(node), *this->scope_managers[0]) == false){ return false; }
			}
		}

		return true;
	};


	auto SemanticAnalyzer::semantic_analysis_global_values() noexcept -> bool {
		for(const GlobalVar& global_var : this->global_vars){
			if(this->analyze_var_value(this->source.getVar(global_var.pir_id), global_var.ast, *this->scope_managers[0]) == false){ return false; }
		}

		for(const GlobalStruct& global_struct : this->global_structs){
			if(this->analyze_struct_block(this->source.getStruct(global_struct.pir_id), global_struct.ast, *this->scope_managers[0]) == false){ return false; }
		}


		return true;
	};


	auto SemanticAnalyzer::semantic_analysis_runtime() noexcept -> bool {
		this->is_analyzing_runtime = true;

		for(const GlobalFunc& global_func : this->global_funcs){
			if(this->analyze_func_block(this->source.getFunc(global_func.pir_id), global_func.ast, *this->scope_managers[0]) == false){ return false; }
		}

		this->scope_managers[0]->leave_scope();
		return true;
	};



	//////////////////////////////////////////////////////////////////////
	// semantic analysis


	auto SemanticAnalyzer::analyze_stmt(const AST::Node& node, ScopeManager& scope_manager) noexcept -> bool {
		if(scope_manager.in_func_scope() && scope_manager.scope_is_terminated()){
			// TODO: better messaging
			this->source.error("Unreachable code", node);
			return false;
		}

		switch(node.kind){
			break; case AST::Kind::VarDecl: return this->analyze_var(this->source.getVarDecl(node), scope_manager);
			break; case AST::Kind::Func: return this->analyze_func(this->source.getFunc(node), scope_manager);
			break; case AST::Kind::Struct: return this->analyze_struct(this->source.getStruct(node), scope_manager);
			break; case AST::Kind::Conditional: return this->analyze_conditional(this->source.getConditional(node), scope_manager);
			break; case AST::Kind::Return: return this->analyze_return(this->source.getReturn(node), scope_manager);
			break; case AST::Kind::Infix: return this->analyze_infix(this->source.getInfix(node), scope_manager);
			break; case AST::Kind::FuncCall: return this->analyze_func_call(this->source.getFuncCall(node), scope_manager);
			break; case AST::Kind::Unreachable: return this->analyze_unreachable(this->source.getUnreachable(node), scope_manager);
			break; case AST::Kind::Alias: return this->analyze_alias(this->source.getAlias(node), scope_manager);
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

		evo::debugFatalBreak("unknown ast kind");
	};












	auto SemanticAnalyzer::analyze_var(const AST::VarDecl& var_decl, ScopeManager& scope_manager) noexcept -> bool {
		// check ident is unused
		const Token::ID ident_tok_id = this->source.getNode(var_decl.ident).token;
		const Token& ident = this->source.getToken(ident_tok_id);

		if(scope_manager.in_struct_scope()){
			PIR::Struct& current_struct = scope_manager.get_current_struct();
			PIR::BaseType& current_struct_base_type = this->src_manager.getBaseType(current_struct.baseType);
			PIR::BaseType::StructData& struct_data = std::get<PIR::BaseType::StructData>(current_struct_base_type.data);

			for(const PIR::BaseType::StructData::MemberVar& member_var : struct_data.memberVars){
				if(member_var.name == ident.value.string){
					this->source.error(std::format("This struct already has a member named \"{}\"", ident.value.string), ident);
					return false;
				}
			}

		}else{
			if(scope_manager.has_in_scope(ident.value.string)){
				this->already_defined(ident, scope_manager);
				return false;
			}
		}


		///////////////////////////////////
		// checking attributes

		bool is_pub = false;
		bool is_export = false;
		for(Token::ID attribute : var_decl.attributes){
			const Token& token = this->source.getToken(attribute);
			std::string_view token_str = token.value.string;

			if(token_str == "pub"){
				if(scope_manager.is_global_scope()){
					is_pub = true;
				}else{
					this->source.error("Only variables at global scope can be marked with the attribute #pub", token);
					return false;
				}

			}else if(token_str == "export"){
				if(scope_manager.is_global_scope() == false){
					this->source.error("Only variables at global scope can be marked with the attribute #export", token);
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
		// check for import

		const evo::Result<bool> is_import = [&]() noexcept {
			if(var_decl.expr.has_value() == false){ return evo::Result<bool>(false); }

			const ExprValueKind value_kind = scope_manager.is_global_scope() ? ExprValueKind::ConstEval : ExprValueKind::Runtime;
			const evo::Result<ExprInfo> expr_info = this->analyze_expr(*var_decl.expr, scope_manager, value_kind);
			if(expr_info.isError()){ return evo::Result<bool>(evo::resultError); }

			if(expr_info.value().type_id.has_value() == false){ return evo::Result<bool>(false); }
			if(*expr_info.value().type_id != SourceManager::getTypeImport()){ return evo::Result<bool>(false); }

			if(scope_manager.in_struct_scope()){
				this->source.error("Struct members cannot be imports", var_decl.ident);
				return evo::Result<bool>(false);
			}

			if(var_decl.isDef == false){
				this->source.error("Import variables must be marked [def] not [var]", var_decl.ident);
				return evo::Result<bool>(false);
			}

			if(var_decl.type.has_value()){
				this->source.error(
					"Variable cannot be assigned a value of a different type, and the value expression cannot be implicitly converted", var_decl.ident,
					std::vector<Message::Info>{
						Message::Info("Imports are typeless, so type inference should be used"),
					}
				);
				return evo::Result<bool>(false);	
			}

			scope_manager.add_import_to_scope(ident.value.string, ScopeManager::Import(expr_info.value().expr->import, var_decl.ident));

			if(is_pub){
				this->source.addPublicImport(ident.value.string, expr_info.value().expr->import);
			}

			return evo::Result<bool>(true);
		}();

		if(is_import.isError()){ return false; }
		if(is_import.value()){ return true; }




		///////////////////////////////////
		// create object



		if(scope_manager.is_global_scope()){
			const PIR::Var::ID var_id = this->source.createVar(ident_tok_id, SourceManager::getDummyTypeID(), PIR::Expr(), var_decl.isDef, is_export);
			scope_manager.add_var_to_scope(ident.value.string, var_id);

			this->source.pir.global_vars.emplace_back(var_id);
			this->global_vars.emplace_back(var_id, var_decl);

			if(is_pub){
				this->source.addPublicVar(ident.value.string, var_id);
			}

		}else if(scope_manager.in_func_scope()){
			const PIR::Var::ID var_id = this->source.createVar(ident_tok_id, SourceManager::getDummyTypeID(), PIR::Expr(), var_decl.isDef, is_export);
			scope_manager.add_var_to_scope(ident.value.string, var_id);

			scope_manager.get_stmts_entry().emplace_back(var_id);
			if(this->analyze_var_value(this->source.getVar(var_id), var_decl, scope_manager) == false){ return false; }

		}else if(scope_manager.in_struct_scope()){
			if(this->analyze_struct_member(var_decl, scope_manager) == false){ return false; }

		}else{
			evo::debugFatalBreak("Unknown scope type");
		}

		///////////////////////////////////
		// done

		return true;
	};







	auto SemanticAnalyzer::analyze_var_value(PIR::Var& var, const AST::VarDecl& var_decl, ScopeManager& scope_manager) noexcept -> bool {
		if(var_decl.expr.has_value() == false){
			this->source.error("Variables must be given an initial value", var_decl.ident);
			return false;
		}

		const evo::Result<ExprInfo> expr_info = this->analyze_expr(*var_decl.expr, scope_manager);
		if(expr_info.isError()){ return false; }

		if(expr_info.value().value_type != ExprInfo::ValueType::Ephemeral && expr_info.value().value_type != ExprInfo::ValueType::Import){
			// TODO: better messaging
			this->source.error("Variables must be assigned with an ephemeral value", *var_decl.expr);
			return false;
		}


		const AST::Node& expr_node = this->source.getNode(*var_decl.expr);
		if(var_decl.type.has_value()){
			const evo::Result<PIR::Type::VoidableID> var_type_id = this->get_type_id(*var_decl.type, scope_manager);
			if(var_type_id.isError()){ return false; }

			if(var_type_id.value().isVoid()){
				this->source.error("Variable cannot be of type Void", *var_decl.type);
				return false;
			}

			var.type = var_type_id.value().typeID();


			if(expr_node.kind != AST::Kind::Uninit){
				const PIR::Type& var_type = this->src_manager.getType(var.type);
				const PIR::Type& expr_type = this->src_manager.getType(*expr_info.value().type_id);

				if(this->is_implicitly_convertable_to(expr_type, var_type, expr_node) == false){
					this->source.error(
						"Variable cannot be assigned a value of a different type, and the value expression cannot be implicitly converted",
						expr_node,

						std::vector<Message::Info>{
							{std::string("Variable is of type:   ") + this->src_manager.printType(var.type)},
							{std::string("Expression is of type: ") + this->src_manager.printType(*expr_info.value().type_id)}
						}
					);
					return false;
				}
			}

		}else{ // type inference
			if(expr_node.kind == AST::Kind::Uninit){
				this->source.error("The type of [uninit] cannot be inferenced", expr_node);
				return false;
			}

			if(*expr_info.value().type_id == SourceManager::getTypeString()){
				this->source.error("String literal values (outside of calls to `@import()`) are not supported yet", expr_node);
				return false;

			}

			var.type = *expr_info.value().type_id;
		}

		var.value = *expr_info.value().expr;


		///////////////////////////////////
		// check value

		evo::debugAssert(var.type != SourceManager::getTypeImport(), "Imports should already be handled in analyze_var()");


		// check for uninit
		if(var.value.kind == PIR::Expr::Kind::ASTNode){
			const AST::Node& var_value_node = this->source.getNode(var.value.astNode);

			if(var_value_node.kind == AST::Kind::Uninit){
				if(scope_manager.is_global_scope()){
					this->source.error("Global variables cannot be initialized with the value \"uninit\"", *var_decl.expr);
					return false;
				}

				if(var_decl.isDef){
					this->source.warning(
						"Declared a def variable with the value \"uninit\"", *var_decl.expr,
						std::vector<Message::Info>{ {"Any use of this variable would be undefined behavior"} }
					);
				}
			}
		}

		

		///////////////////////////////////
		// done

		return true;
	};




	auto SemanticAnalyzer::analyze_struct_member(const AST::VarDecl& var_decl, ScopeManager& scope_manager) noexcept -> bool {
		if(this->source.getConfig().allowStructMemberTypeInference == false && var_decl.type.has_value() == false){
			this->source.error(
				"Type inference of struct members is not allowed", var_decl.ident,
				{ Message::Info("You can change this with the config option \"allowStructMemberTypeInference\"") }
			);
			return false;
		}


		auto type_id = std::optional<PIR::Type::ID>();
		auto default_value = PIR::Expr();

		if(var_decl.type.has_value()){
			///////////////////////////////////
			// type

			const evo::Result<PIR::Type::VoidableID> var_type_id = this->get_type_id(*var_decl.type, scope_manager);
			if(var_type_id.isError()){ return false; }

			if(var_type_id.value().isVoid()){
				this->source.error("Variable cannot be of type Void", *var_decl.type);
				return false;
			}

			type_id = var_type_id.value().typeID();


			///////////////////////////////////
			// expr

			if(var_decl.expr.has_value()){
				const evo::Result<ExprInfo> expr_info = this->analyze_expr(*var_decl.expr, scope_manager);
				if(expr_info.isError()){ return false; }

				if(expr_info.value().type_id.has_value() == false){
					const PIR::Type& var_type = this->src_manager.getType(*type_id);
					const PIR::Type& expr_type = this->src_manager.getType(*expr_info.value().type_id);

					if(this->is_implicitly_convertable_to(expr_type, var_type, this->source.getNode(*var_decl.expr)) == false){
						this->source.error(
							"Variable cannot be assigned a value of a different type, and the value expression cannot be implicitly converted",
							*var_decl.expr,

							std::vector<Message::Info>{
								{std::string("Variable is of type:   ") + this->src_manager.printType(*type_id)},
								{std::string("Expression is of type: ") + this->src_manager.printType(*expr_info.value().type_id)}
							}
						);
						return false;
					}
				}

				default_value = *expr_info.value().expr;
			}

		}else{ // type inference
			if(var_decl.expr.has_value() == false){
				this->source.error("Cannot get type of struct member since there was neither a type nor a default value given", var_decl.ident);
				return false;
			}

			const evo::Result<ExprInfo> expr_info = this->analyze_expr(*var_decl.expr, scope_manager);
			if(expr_info.isError()){ return false; }

			if(expr_info.value().type_id.has_value() == false){
				this->source.error("The type of [uninit] cannot be inferenced", var_decl.ident);
				return false;
			}

			if(expr_info.value().value_type != ExprInfo::ValueType::Ephemeral){
				this->source.error("Default value of struct member must be ephemeral", var_decl.ident);
				return false;
			}

			type_id = expr_info.value().type_id;
			default_value = *expr_info.value().expr;
		}




		PIR::Struct& current_struct = scope_manager.get_current_struct();

		// look for circular members in type of newly added member
		auto types_seen = std::unordered_set<PIR::Type::ID>();
		types_seen.emplace(this->src_manager.getOrCreateTypeID(PIR::Type(current_struct.baseType)).id);
		auto types_queue = std::queue<PIR::Type::ID>();
		types_queue.push(*type_id);

		while(types_queue.empty() == false){
			const PIR::Type::ID type_id_to_look_at = types_queue.front();
			types_queue.pop();

			const PIR::Type& type_to_look_at = this->src_manager.getType(type_id_to_look_at);
			if(type_to_look_at.qualifiers.empty() == false){ continue; }

			const PIR::BaseType& base_type_to_look_at = this->src_manager.getBaseType(type_to_look_at.baseType);
			if(base_type_to_look_at.kind == PIR::BaseType::Kind::Builtin){ continue; }

			if(types_seen.contains(type_id_to_look_at)){
				// TODO: better messaging
				this->source.error("Detected circular type dependancy", var_decl.ident);
				return false;
			}

			for(const PIR::BaseType::StructData::MemberVar& member : std::get<PIR::BaseType::StructData>(base_type_to_look_at.data).memberVars){
				types_queue.push(member.type);
			}

			types_seen.emplace(type_id_to_look_at);
		};


		// if no circular members found, add the new member to the type
		PIR::BaseType& current_struct_base_type = this->src_manager.getBaseType(current_struct.baseType);
		PIR::BaseType::StructData& struct_data = std::get<PIR::BaseType::StructData>(current_struct_base_type.data);
		struct_data.memberVars.emplace_back(this->source.getIdent(var_decl.ident).value.string, var_decl.isDef, *type_id, default_value);

		return true;
	};










	auto SemanticAnalyzer::analyze_func(const AST::Func& func, ScopeManager& scope_manager) noexcept -> bool {
		const Token::ID ident_tok_id = this->source.getNode(func.ident).token;
		const Token& ident = this->source.getToken(ident_tok_id);

		// check function is in global scope
		if(scope_manager.is_global_scope() == false){
			this->source.error("Functions can only be defined at global scope", ident);
			return false;
		}


		///////////////////////////////////
		// template pack

		if(func.templatePack.has_value()){
			this->source.error("Function declaration with a template pack is not supported yet", ident);
			return false;
		}


		///////////////////////////////////
		// params

		auto params = std::vector<PIR::Param::ID>();
		auto param_type_ids = std::vector<PIR::BaseType::Operator::Param>();

		// this is just for params
		scope_manager.enter_scope(nullptr);

		const AST::FuncParams& ast_func_params = this->source.getFuncParams(func.params);
		for(const AST::FuncParams::Param& param : ast_func_params.params){
			const Token::ID param_ident_tok_id = this->source.getNode(param.ident).token;
			const Token& param_ident = this->source.getToken(param_ident_tok_id);

			if(scope_manager.has_in_scope(param_ident.value.string)){
				this->already_defined(param_ident, scope_manager);
				return false;
			}


			// get param type
			const evo::Result<PIR::Type::VoidableID> param_type_id = this->get_type_id(param.type, scope_manager);
			if(param_type_id.isError()){ return false; }

			if(param_type_id.value().isVoid()){
				this->source.error("Function parameters cannot be of type Void", param.type);
				return false;
			}


			param_type_ids.emplace_back(param_type_id.value().typeID(), param.kind);

			const PIR::Param::ID param_id = this->source.createParam(param_ident_tok_id, param_type_id.value().typeID(), param.kind);
			scope_manager.add_param_to_scope(param_ident.value.string, param_id);
			params.emplace_back(param_id);
		}


		scope_manager.leave_scope();


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
				if(scope_manager.is_global_scope()){
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

		const evo::Result<PIR::Type::VoidableID> return_type_id = this->get_type_id(func.returnType, scope_manager);
		if(return_type_id.isError()){ return false; }


		///////////////////////////////////
		// create base type

		auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
		base_type.callOperator = PIR::BaseType::Operator(std::move(param_type_ids), return_type_id.value());

		const PIR::BaseType::ID base_type_id = this->src_manager.getOrCreateBaseType(std::move(base_type)).id;


		///////////////////////////////////
		// check for overload reuse

		for(size_t scope_index : scope_manager.get_scopes()){
			const ScopeManager::Scope& scope = this->scope_alloc[scope_index];

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

		scope_manager.add_func_to_scope(ident.value.string, func_id);




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

		if(scope_manager.is_global_scope()){
			this->global_funcs.emplace_back(func_id, func);

			if(is_pub){
				this->source.addPublicFunc(ident.value.string, func_id);
			}

		}else{
			PIR::Func& pir_func = this->source.pir.funcs[func_id.id];
			if(this->analyze_func_block(pir_func, func, scope_manager) == false){
				return false;
			};
		}


		///////////////////////////////////
		// done

		return true;
	};



	auto SemanticAnalyzer::analyze_func_block(PIR::Func& pir_func, const AST::Func& ast_func, ScopeManager& scope_manager) noexcept -> bool {
		scope_manager.enter_type_scope(ScopeManager::TypeScope::Kind::Func, pir_func);

			scope_manager.enter_scope(&pir_func.stmts);
				scope_manager.enter_scope_level();
					scope_manager.add_scope_level_scope();

					// add the params into the scope
					for(PIR::Param::ID param_id : pir_func.params){
						const PIR::Param& param = this->source.getParam(param_id);
						scope_manager.add_param_to_scope(this->source.getToken(param.ident).value.string, param_id);
					}

					// analyze the statements in the block
					const AST::Block& block = this->source.getBlock(ast_func.block);
					if(this->analyze_block(block, scope_manager) == false){ return false; }

				scope_manager.leave_scope_level();

			// check if function is terminated
			if(pir_func.returnType.isVoid()){
				if(scope_manager.scope_is_terminated()){
					pir_func.stmts.setTerminated();
				}
			}else{
				if(scope_manager.scope_is_terminated() == false){
					this->source.error("Function with return type does not return on all control paths", pir_func.ident);
					return false;
				}
			}

			scope_manager.leave_scope();
		scope_manager.leave_type_scope();


		for(PIR::Param::ID param_id : pir_func.params){
			const PIR::Param& param = this->source.getParam(param_id);

			if(param.kind == AST::FuncParams::Param::Kind::Write && param.mayHaveBeenEdited == false){
				this->source.warning("write parameter was not written to in any control path", param.ident);
			}
		}



		return true;
	};



	auto SemanticAnalyzer::analyze_struct(const AST::Struct& struct_decl, ScopeManager& scope_manager) noexcept -> bool {
		const Token::ID ident_tok_id = this->source.getNode(struct_decl.ident).token;
		const Token& ident = this->source.getToken(ident_tok_id);

		if(scope_manager.is_global_scope() == false){
			this->source.error("Structs can only be declared in global scope", ident);
			return false;
		}

		if(scope_manager.has_in_scope(ident.value.string)){
			this->already_defined(ident, scope_manager);
			return false;
		}

		///////////////////////////////////
		// attributes

		bool is_pub = false;
		for(Token::ID attribute_id : struct_decl.attributes){
			const Token& attribute = this->source.getToken(attribute_id);

			if(attribute.value.string == "pub"){
				is_pub = true;
			}
		}


		///////////////////////////////////
		// template

		if(struct_decl.templatePack.has_value()){
			ScopeManager& new_scope_manager = *this->scope_managers.emplace_back(std::make_unique<ScopeManager>(scope_manager));
			scope_manager.add_struct_to_scope(ident.value.string, ScopeManager::Scope::StructData(struct_decl, new_scope_manager));
			return true;
		}


		///////////////////////////////////
		// type

		const PIR::BaseType::ID base_type_id = this->src_manager.getOrCreateBaseType(
			PIR::BaseType(PIR::BaseType::Kind::Struct, ident.value.string, &this->source, {})
		).id;
		const PIR::Struct::ID struct_id = this->source.createStruct(ident_tok_id, std::nullopt, base_type_id);

		scope_manager.add_struct_to_scope(ident.value.string, ScopeManager::Scope::StructData(struct_id));

		this->global_structs.emplace_back(struct_id, struct_decl);

		if(is_pub){
			this->source.addPublicStruct(ident.value.string, struct_id);
		}

		return true;
	};


	auto SemanticAnalyzer::analyze_struct_block(PIR::Struct& pir_struct, const AST::Struct& ast_struct, ScopeManager& scope_manager) noexcept -> bool {
		scope_manager.enter_type_scope(ScopeManager::TypeScope::Kind::Struct, pir_struct);
			scope_manager.enter_scope(nullptr);

				const AST::Block& block = this->source.getBlock(ast_struct.block);
				if(this->analyze_block(block, scope_manager) == false){ return false; }

			scope_manager.leave_scope();
		scope_manager.leave_type_scope();

		return true;
	};





	auto SemanticAnalyzer::analyze_conditional(const AST::Conditional& cond, ScopeManager& scope_manager) noexcept -> bool {
		scope_manager.enter_scope_level();

		const bool analyze_conditional_result = analyze_conditional_recursive(cond, scope_manager);

		scope_manager.leave_scope_level();

		return analyze_conditional_result;
	};


	auto SemanticAnalyzer::analyze_conditional_recursive(const AST::Conditional& sub_cond, ScopeManager& scope_manager) noexcept -> bool {
		if(scope_manager.in_func_scope() == false){
			this->source.error("Conditional statements can only be inside functions", sub_cond.ifTok);
			return false;
		}


		///////////////////////////////////
		// condition

		const evo::Result<ExprInfo> cond_info = this->analyze_expr(sub_cond.ifExpr, scope_manager);
		if(cond_info.isError()){ return false; }

		if(cond_info.value().type_id.has_value() == false){
			this->source.error("Conditional expression must be a boolean (cannot be [uninit])", sub_cond.ifExpr);
			return false;
		}

		if(*cond_info.value().type_id != this->src_manager.getTypeBool()){
			this->source.error(
				"Conditional expression must be a boolean", sub_cond.ifExpr,
				std::vector<Message::Info>{
					{std::string("Conditional expression is of type: ") + this->src_manager.printType(*cond_info.value().type_id)}
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

		scope_manager.add_scope_level_scope();
		const AST::Block& then_block = this->source.getBlock(sub_cond.thenBlock);
		if(this->analyze_block(then_block, then_stmts, scope_manager) == false){ return false; }


		///////////////////////////////////
		// else block

		if(sub_cond.elseBlock.has_value()){
			const AST::Node& else_block_node = this->source.getNode(*sub_cond.elseBlock);

			if(else_block_node.kind == AST::Kind::Block){
				scope_manager.add_scope_level_scope();

				const AST::Block& else_block = this->source.getBlock(else_block_node);
				if(this->analyze_block(else_block, else_stmts, scope_manager) == false){ return false; }

			}else if(else_block_node.kind == AST::Kind::Conditional){
				scope_manager.enter_scope(&else_stmts);

				const AST::Conditional& else_block = this->source.getConditional(else_block_node);
				if(this->analyze_conditional_recursive(else_block, scope_manager) == false){ return false; }		

				scope_manager.leave_scope();

			}else{
				evo::debugFatalBreak("Unkonwn else block kind");
			}

		}else{
			scope_manager.add_scope_level_scope();
		}


		///////////////////////////////////
		// create object

		const PIR::Conditional::ID cond_id = this->source.createConditional(*cond_info.value().expr, std::move(then_stmts), std::move(else_stmts));
		scope_manager.get_stmts_entry().emplace_back(cond_id);



		///////////////////////////////////
		// done

		return true;
	};







	auto SemanticAnalyzer::analyze_return(const AST::Return& return_stmt, ScopeManager& scope_manager) noexcept -> bool {
		if(scope_manager.in_func_scope() == false){
			this->source.error("Return statements can only be inside functions", return_stmt.keyword);
			return false;
		}

		const PIR::Type::VoidableID func_return_type_id = scope_manager.get_current_func().returnType;

		auto return_value = PIR::Expr();

		if(return_stmt.value.has_value()){
			// "return expr;"

			if(scope_manager.get_current_func().returnType.isVoid()){
				this->source.error("Return statement has value when function's return type is \"Void\"", return_stmt.keyword);
				return false;	
			}

			const evo::Result<ExprInfo> expr_info = this->analyze_expr(*return_stmt.value, scope_manager);
			if(expr_info.isError()){ return false; }

			if(expr_info.value().type_id.has_value() == false){
				this->source.error("Cannot return [uninit]", *return_stmt.value);
				return false;
			}


			const AST::Node& expr_node = this->source.getNode(*return_stmt.value);

			const PIR::Type& func_return_type = this->src_manager.getType(func_return_type_id.typeID());
			const PIR::Type& expr_type = this->src_manager.getType(*expr_info.value().type_id);

			if(this->is_implicitly_convertable_to(expr_type, func_return_type, expr_node) == false){
				this->source.error(
					"Return value type and function return type do not match", 
					expr_node,

					std::vector<Message::Info>{
						{std::string("Function return is type: ") + this->src_manager.printType(func_return_type_id.typeID())},
						{std::string("Return value is of type: ") + this->src_manager.printType(*expr_info.value().type_id)}
					}
				);

				return false;
			}

			return_value = *expr_info.value().expr;

		}else{
			// "return;"
			
			if(func_return_type_id.isVoid() == false){
				this->source.error("Return statement has no value but the function's return type is not \"Void\"", return_stmt.keyword);
				return false;
			}
		}

		evo::debugAssert(return_value.kind != PIR::Expr::Kind::None, "cannot return PIR::Expr::Kind::None");


		const PIR::Return::ID ret_id = this->source.createReturn(return_value);
		scope_manager.get_stmts_entry().emplace_back(ret_id);
		scope_manager.get_stmts_entry().setTerminated();
		if(scope_manager.is_in_func_base_scope()){
			scope_manager.get_current_func().terminatesInBaseScope = true;
		}

		scope_manager.set_scope_terminated();
		scope_manager.add_scope_level_terminated();


		return true;
	};




	auto SemanticAnalyzer::analyze_infix(const AST::Infix& infix, ScopeManager& scope_manager) noexcept -> bool {
		switch(this->source.getToken(infix.op).kind){
			case Token::get("="): {	
				return this->analyze_assignment(infix, scope_manager);
			} break;

			case Token::get("+"): case Token::get("+@"):
			case Token::get("-"): case Token::get("-@"):
			case Token::get("*"): case Token::get("*@"):
			case Token::get("/"):
			case Token::get("=="): case Token::get("!="):
			case Token::get("<"):  case Token::get("<="):
			case Token::get(">"):  case Token::get(">="):
			case Token::KeywordAnd: case Token::KeywordOr: {
				// TODO: better messaging
				this->source.error("This operator can only be used in an expression, not a statement", infix.op);
				return false;
			} break;
		};

		evo::debugFatalBreak("Unknown or unsupported infix op: {}", int32_t(this->source.getToken(infix.op).kind));
	};



	auto SemanticAnalyzer::analyze_assignment(const AST::Infix& infix, ScopeManager& scope_manager) noexcept -> bool {
		// check if at global scope
		if(scope_manager.is_global_scope()){
			this->source.error("Assignment statements are not allowed at global scope", infix.op);
			return false;
		}


		///////////////////////////////////
		// checking of lhs

		const evo::Result<ExprInfo> lhs_info = this->analyze_expr(infix.lhs, scope_manager);
		if(lhs_info.isError()){ return false; }


		if(lhs_info.value().value_type != ExprInfo::ValueType::ConcreteMutable){
			if(lhs_info.value().value_type == ExprInfo::ValueType::ConcreteConst){
				this->source.error("Only mutable values may be assigned to", infix.lhs);
			}else{
				this->source.error("Only concrete values may be assigned to", infix.lhs);
			}
			return false;
		}

		evo::debugAssert(lhs_info.value().type_id.has_value(), "cannot assign to [uninit]");

		if(lhs_info.value().expr->kind == PIR::Expr::Kind::Param){
			PIR::Param& param = this->source.getParam(lhs_info.value().expr->param);
			param.mayHaveBeenEdited = true;

		}else if(lhs_info.value().expr->kind == PIR::Expr::Kind::Accessor){
			PIR::Accessor& accessor = this->source.getAccessor(lhs_info.value().expr->accessor);
			PIR::Expr* lhs = &accessor.lhs;

			while(lhs->kind == PIR::Expr::Kind::Accessor){
				PIR::Accessor& lhs_accessor = this->source.getAccessor(lhs->accessor);
				lhs = &lhs_accessor.lhs;
			};

			if(lhs->kind == PIR::Expr::Kind::Param){
				PIR::Param& param = this->source.getParam(lhs->param);
				param.mayHaveBeenEdited = true;
			}
		}


		///////////////////////////////////
		// checking of rhs

		const evo::Result<ExprInfo> rhs_info = this->analyze_expr(infix.rhs, scope_manager);
		if(rhs_info.isError()){ return false; }

		if(rhs_info.value().value_type != ExprInfo::ValueType::Ephemeral){
			this->source.error("Only ephemeral values may be assignment values", infix.rhs);
			return false;
		}

		if(rhs_info.value().type_id.has_value() == false){
			// TODO: better messaging
			this->source.error("Cannot assign a value of [uninit]", infix.rhs);
			return false;
		}


		///////////////////////////////////
		// type checking

		const PIR::Type& dst_type = this->src_manager.getType(*lhs_info.value().type_id);
		const PIR::Type& expr_type = this->src_manager.getType(*rhs_info.value().type_id);

		if(this->is_implicitly_convertable_to(expr_type, dst_type, this->source.getNode(infix.rhs)) == false){
			this->source.error(
				"The types of the left-hand-side and right-hand-side of an assignment statement do not match, and cannot be implicitly converted",
				infix.op,

				std::vector<Message::Info>{
					{std::string("left-hand-side is of type:  ") + this->src_manager.printType(*lhs_info.value().type_id)},
					{std::string("right-hand-side is of type: ") + this->src_manager.printType(*rhs_info.value().type_id)}
				}
			);
			return false;
		}


		///////////////////////////////////
		// create object

		const PIR::Assignment::ID assignment_id = this->source.createAssignment(*lhs_info.value().expr, infix.op, *rhs_info.value().expr);
		scope_manager.get_stmts_entry().emplace_back(assignment_id);


		return true;
	};




	auto SemanticAnalyzer::analyze_func_call(const AST::FuncCall& func_call, ScopeManager& scope_manager) noexcept -> bool {
		// analyze and get type of ident
		const evo::Result<ExprInfo> target_info = this->analyze_expr(func_call.target, scope_manager, ExprValueKind::None, &func_call);
		if(target_info.isError()){ return false; }

		evo::debugAssert(target_info.value().type_id.has_value(), "type of function call target is [uninit]");

		// check if discarding return value
		const PIR::Type& target_type = this->src_manager.getType(*target_info.value().type_id);
		const PIR::BaseType& target_base_type = this->src_manager.getBaseType(target_type.baseType);

		if(target_base_type.callOperator.has_value() == false){
			this->source.error(
				"cannot be called like a function", func_call.target,
				std::vector<Message::Info>{
					std::format("Type \"{}\" does not have a call operator", this->src_manager.printType(*target_info.value().type_id))
				}
			);
			return false;
		}

		if(target_base_type.callOperator->returnType.isVoid() == false){
			this->source.error("Discarding return value of function call", func_call.target);
			return false;
		}

		if(this->check_func_call(func_call, *target_info.value().type_id, scope_manager) == false){ return false; }

		const evo::Result<std::vector<PIR::Expr>> args = this->get_func_call_args(func_call, scope_manager);
		if(args.isError()){ return false; }

		switch(this->source.getNode(func_call.target).kind){
			case AST::Kind::Ident: {
				const Token& ident_tok = this->source.getIdent(func_call.target);

				// get func
				const evo::Result<PIR::Func::ID> func_id = this->lookup_func_in_scope(ident_tok.value.string, func_call, scope_manager);
				if(func_id.isError()){ return false; }


				// create object
				const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(func_id.value(), std::move(args.value()));
				scope_manager.get_stmts_entry().emplace_back(func_call_id);

				return true;
			} break;
			
			case AST::Kind::Intrinsic: {
				const Token& intrinsic_tok = this->source.getIntrinsic(func_call.target);

				const evo::ArrayProxy<PIR::Intrinsic> intrinsics = this->src_manager.getIntrinsics();
				for(size_t i = 0; i < intrinsics.size(); i+=1){
					const PIR::Intrinsic& intrinsic = intrinsics[i];

					if(intrinsic.ident == intrinsic_tok.value.string){
						// create object
						const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(PIR::Intrinsic::ID(uint32_t(i)), std::move(args.value()));
						scope_manager.get_stmts_entry().emplace_back(func_call_id);
						
						return true;
					}
				}


				evo::debugFatalBreak("Unknown intrinsic func");
			} break;


			case AST::Kind::Infix: {
				const AST::Infix& infix = this->source.getInfix(func_call.target);

				switch(this->source.getToken(infix.op).kind){
					case Token::get("."): {
						const evo::Result<ExprInfo> lhs_info = this->analyze_expr(infix.lhs, scope_manager);
						evo::debugAssert(lhs_info.isSuccess(), "uncaught error");
						evo::debugAssert(lhs_info.value().expr->kind == PIR::Expr::Kind::Import, "incorrect expr kind gotten");

						const Source& import_source = this->src_manager.getSource(lhs_info.value().expr->import);
						const Token& rhs_ident = this->source.getIdent(infix.rhs);

						const evo::Result<PIR::Func::ID> imported_func_id = this->lookup_func_in_import(rhs_ident.value.string, import_source, func_call, scope_manager);
						if(imported_func_id.isError()){ return false; }
						
						// create object
						const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(imported_func_id.value(), std::move(args.value()));
						scope_manager.get_stmts_entry().emplace_back(func_call_id);

						return true;
					} break;
				};

				evo::debugFatalBreak("Unknown or unsupported infix type");
			} break;
		};
		
		evo::debugFatalBreak("Unknown or unsupported func call target");
	};



	auto SemanticAnalyzer::analyze_unreachable(const Token& unreachable, ScopeManager& scope_manager) noexcept -> bool {
		if(scope_manager.in_func_scope() == false){
			// TODO: different / better messaging? Should check if in global scope instead?
			this->source.error("\"unreachable\" statements are not outside of a function", unreachable);
			return false;
		}

		scope_manager.add_scope_level_terminated();
		scope_manager.set_scope_terminated();

		scope_manager.get_stmts_entry().emplace_back(PIR::Stmt::getUnreachable());
		scope_manager.get_stmts_entry().setTerminated();
		if(scope_manager.is_in_func_base_scope()){
			scope_manager.get_current_func().terminatesInBaseScope = true;
		}

		return true;
	};



	auto SemanticAnalyzer::analyze_alias(const AST::Alias& alias, ScopeManager& scope_manager) noexcept -> bool {
		///////////////////////////////////
		// ident

		const Token& ident_tok = this->source.getIdent(alias.ident);
		const std::string_view ident_str = ident_tok.value.string;
		if(scope_manager.has_in_scope(ident_str)){
			this->already_defined(ident_tok, scope_manager);
			return false;
		}


		///////////////////////////////////
		// attributes

		bool is_pub = false;

		for(Token::ID attribute_tok : alias.attributes){
			const Token& attribute = this->source.getToken(attribute_tok);

			if(attribute.value.string == "pub"){
				is_pub = true;		

			}else{
				// TODO: better messaging
				this->source.error("Unkonwn or unsupported attribute in alias", attribute);
				return false;
			}
		}



		///////////////////////////////////
		// type

		const evo::Result<PIR::Type::VoidableID> type = this->get_type_id(alias.type, scope_manager);
		if(type.isError()){ return false; }


		///////////////////////////////////
		// create

		scope_manager.add_alias_to_scope(ident_str, ScopeManager::Alias(type.value(), alias.ident));

		if(is_pub){
			this->source.pir.pub_aliases.emplace(ident_str, type.value());
		}

		return true;
	};







	auto SemanticAnalyzer::analyze_block(const AST::Block& block, PIR::StmtBlock& stmts_entry, ScopeManager& scope_manager) noexcept -> bool {
		scope_manager.enter_scope(&stmts_entry);

		if(this->analyze_block(block, scope_manager) == false){ return false; }

		scope_manager.leave_scope();

		return true;
	};


	auto SemanticAnalyzer::analyze_block(const AST::Block& block, ScopeManager& scope_manager) noexcept -> bool {
		for(AST::Node::ID node_id : block.nodes){
			if(this->analyze_stmt(this->source.getNode(node_id), scope_manager) == false){
				return false;
			}
		}

		return true;
	};






	auto SemanticAnalyzer::check_func_call(const AST::FuncCall& func_call, PIR::Type::ID func_type_id, ScopeManager& scope_manager) noexcept -> bool {
		const PIR::Type& type = this->src_manager.getType(func_type_id);

		if(type.qualifiers.empty() == false){
			this->source.error(
				"cannot be called like a function", func_call.target,
				std::vector<Message::Info>{ std::format("Type \"{}\" does not have a call operator", this->src_manager.printType(func_type_id)) }
			);
			return false;
		}

		const PIR::BaseType& base_type = this->src_manager.getBaseType(type.baseType);
		if(base_type.callOperator.has_value() == false){
			// TODO: better messaging?
			this->source.error(
				"cannot be called like a function", func_call.target,
				std::vector<Message::Info>{ std::format("Type \"{}\" does not have a call operator", this->src_manager.printType(func_type_id)) }
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
			const evo::Result<ExprInfo> arg_info = this->analyze_expr(arg_id, scope_manager, ExprValueKind::None);
			if(arg_info.isError()){ return false; }

			if(arg_info.value().type_id.has_value() == false){
				this->source.error("An function call argument cannot be [uninit]", arg_node);
				return false;
			}

			const PIR::Type& param_type = this->src_manager.getType(param.type);
			const PIR::Type& arg_type = this->src_manager.getType(*arg_info.value().type_id);

			if(this->is_implicitly_convertable_to(arg_type, param_type, arg_node) == false){
				// TODO: better messaging
				this->source.error(
					"Function call arguments do not match function", func_call.target,
					std::vector<Message::Info>{
						Message::Info(std::format("In argument: {}", i)),
						Message::Info(std::format("Type of parameter: {}", this->src_manager.printType(param.type))),
						Message::Info(std::format("Type of argument:  {}", this->src_manager.printType(*arg_info.value().type_id))),
					}
				);
				return false;
			}


			// check param kind accepts arg value type
			using ParamKind = AST::FuncParams::Param::Kind;
			switch(param.kind){
				case ParamKind::Read: {
					// accepts any value type
				} break;

				case ParamKind::Write: {
					if(arg_info.value().value_type != ExprInfo::ValueType::ConcreteMutable){
						if(arg_info.value().value_type == ExprInfo::ValueType::ConcreteConst){
							this->source.error("write parameters require mutable expression values", arg_id);
						}else{
							this->source.error("write parameters require concrete expression values", arg_id);
						}
						return false;
					}

				} break;

				case ParamKind::In: {
					if(arg_info.value().value_type != ExprInfo::ValueType::Ephemeral){
						this->source.error("write parameters require ephemeral expression values", arg_id);
						return false;
					}
				} break;
			};


			// if param is write and arg is a param, mark it as edited
			if(param.kind == ParamKind::Write){
				if(arg_node.kind == AST::Kind::Ident){
					std::string_view param_ident_str = this->source.getIdent(arg_node).value.string;

					for(size_t scope_index : scope_manager.get_scopes()){
						const ScopeManager::Scope& scope = this->scope_alloc[scope_index];

	 					if(scope.params.contains(param_ident_str)){
							PIR::Param& pir_param = this->source.getParam(scope.params.at(param_ident_str));
							pir_param.mayHaveBeenEdited = true;
						}
					}
				}else if(arg_node.kind == AST::Kind::Infix){
					const AST::Infix& infix = this->source.getInfix(arg_node);
					if(this->source.getToken(infix.op).kind == Token::get(".")){
						const AST::Node* lhs_node = &this->source.getNode(infix.lhs);
						
						while(lhs_node->kind == AST::Kind::Infix){
							const AST::Infix& lhs_infix = this->source.getInfix(*lhs_node);	
							lhs_node = &this->source.getNode(lhs_infix.lhs);
						};

						std::string_view param_ident_str = this->source.getIdent(*lhs_node).value.string;

						for(size_t scope_index : scope_manager.get_scopes()){
							const ScopeManager::Scope& scope = this->scope_alloc[scope_index];

		 					if(scope.params.contains(param_ident_str)){
								PIR::Param& pir_param = this->source.getParam(scope.params.at(param_ident_str));
								pir_param.mayHaveBeenEdited = true;
							}
						}
					}
				}
			}
		}

		return true;
	};


	auto SemanticAnalyzer::get_func_call_args(const AST::FuncCall& func_call, ScopeManager& scope_manager) noexcept -> evo::Result<std::vector<PIR::Expr>> {
		auto args = std::vector<PIR::Expr>();

		for(AST::Node::ID arg_id : func_call.args){
			const evo::Result<ExprInfo> expr_info = this->analyze_expr(arg_id, scope_manager);
			if(expr_info.isError()){ return evo::resultError; }

			args.emplace_back(*expr_info.value().expr);
		}

		return args;
	};



	auto SemanticAnalyzer::analyze_expr(AST::Node::ID node_id, ScopeManager& scope_manager, ExprValueKind value_kind, const AST::FuncCall* lookup_func_call) noexcept 
	-> evo::Result<ExprInfo> {
		const AST::Node& node = this->source.getNode(node_id);

		switch(node.kind){
			case AST::Kind::Prefix:      return this->analyze_prefix_expr(node_id, scope_manager, value_kind);
			case AST::Kind::Infix:       return this->analyze_infix_expr(node_id, scope_manager, value_kind, lookup_func_call);
			case AST::Kind::Postfix:     return this->analyze_postfix_expr(node_id, scope_manager, value_kind);
			case AST::Kind::FuncCall:    return this->analyze_func_call_expr(node_id, scope_manager, value_kind);
			case AST::Kind::Initializer: return this->analyze_initializer_expr(node_id, scope_manager, value_kind);
			case AST::Kind::Ident:       return this->analyze_ident_expr(node_id, scope_manager, value_kind, lookup_func_call);
			case AST::Kind::Literal:     return this->analyze_literal_expr(node_id, value_kind);
			case AST::Kind::Intrinsic:   return this->analyze_intrinsic_expr(node_id, value_kind);
			case AST::Kind::Uninit:      return this->analyze_uninit_expr(node_id, value_kind);
		};


		evo::debugFatalBreak("Unkown AST node kind");
	};



	auto SemanticAnalyzer::analyze_prefix_expr(AST::Node::ID node_id, ScopeManager& scope_manager, ExprValueKind value_kind) noexcept -> evo::Result<ExprInfo> {
		auto type_id = std::optional<PIR::Type::ID>();
		auto expr = std::optional<PIR::Expr>();

		const AST::Node& node = this->source.getNode(node_id);
		const AST::Prefix& prefix = this->source.getPrefix(node);

		const evo::Result<ExprInfo> rhs_info = this->analyze_expr(prefix.rhs, scope_manager, value_kind);
		if(rhs_info.isError()){ return evo::resultError; }


		switch(this->source.getToken(prefix.op).kind){
			case Token::KeywordCopy: {
				if(rhs_info.value().value_type != ExprInfo::ValueType::ConcreteConst && rhs_info.value().value_type != ExprInfo::ValueType::ConcreteMutable){
					this->source.error("Only concrete expressions can be copied", prefix.rhs);
					return evo::resultError;
				}

				type_id = rhs_info.value().type_id;

				if(value_kind != ExprValueKind::None){
					const PIR::Prefix::ID prefix_id = this->source.createPrefix(prefix.op, *rhs_info.value().expr);
					expr = PIR::Expr(prefix_id);
				}
			} break;


			case Token::KeywordAddr: {
				// check value type
				if(rhs_info.value().value_type != ExprInfo::ValueType::ConcreteConst && rhs_info.value().value_type != ExprInfo::ValueType::ConcreteMutable){
					this->source.error("Can only take the address of a concrete expression", prefix.rhs);
					return evo::resultError;
				}


				const AST::Node& rhs_node = this->source.getNode(prefix.rhs);

				// check that it's not an intrinsic
				if(rhs_node.kind == AST::Kind::Intrinsic){
					this->source.error("Cannot take the address of an intrinsic", prefix.rhs);
					return evo::resultError;
				}



				// check that it's not taking the address of a dereference
				if(this->source.getConfig().badPracticeAddrOfDeref && rhs_node.kind == AST::Kind::Postfix){
					const AST::Postfix& rhs_postfix = this->source.getPostfix(rhs_node);
					if(this->source.getToken(rhs_postfix.op).kind == Token::get(".&")){
						this->source.error(
							"[Bad Practice] Should not take the address of ([.&]) a dereference ([.&]) expression", rhs_node,
							{ 
								Message::Info("Remove both the [addr] and the [.&] as it will give you the same value"),
								Message::Info("Note: (addr x.&) == x"),
								Message::Info("You can change this with the config option \"badPracticeAddrOfDeref\""),
							}
						);
						return evo::resultError;
					}
				}


				// get type of rhs
				if(rhs_info.value().type_id.has_value() == false){
					this->source.error("Cannot take the address of the expression [uninit]", rhs_node);
					return evo::resultError;
				}

				const PIR::Type& rhs_type = this->src_manager.getType(*rhs_info.value().type_id);
				PIR::Type rhs_type_copy = rhs_type;


				// make the type pointer of the type of rhs
				const bool is_const = rhs_info.value().value_type == ExprInfo::ValueType::ConcreteConst;
				rhs_type_copy.qualifiers.emplace_back(true, is_const);

				type_id = this->src_manager.getOrCreateTypeID(rhs_type_copy).id;



				if(value_kind != ExprValueKind::None){
					// set that parameters may have been edited
					if(rhs_info.value().expr->kind == PIR::Expr::Kind::Param){
						PIR::Param& param = this->source.getParam(rhs_info.value().expr->param);
						param.mayHaveBeenEdited = true;

					}else if(rhs_info.value().expr->kind == PIR::Expr::Kind::Accessor){
						PIR::Accessor& accessor = this->source.getAccessor(rhs_info.value().expr->accessor);
						PIR::Expr* lhs = &accessor.lhs;

						while(lhs->kind == PIR::Expr::Kind::Accessor){
							PIR::Accessor& lhs_accessor = this->source.getAccessor(lhs->accessor);
							lhs = &lhs_accessor.lhs;
						};

						if(lhs->kind == PIR::Expr::Kind::Param){
							PIR::Param& param = this->source.getParam(lhs->param);
							param.mayHaveBeenEdited = true;
						}
					}

					const PIR::Prefix::ID prefix_id = this->source.createPrefix(prefix.op, *rhs_info.value().expr);
					expr = PIR::Expr(prefix_id);
				}
			} break;

			case Token::get("-"): {
				if(rhs_info.value().type_id.has_value() == false){
					this->source.error("Cannot negate ([-]) the expression [uninit]", prefix.rhs);
					return evo::resultError;
				}

				const PIR::Type& rhs_type = this->src_manager.getType(*rhs_info.value().type_id);
				const PIR::BaseType& rhs_base_type = this->src_manager.getBaseType(rhs_type.baseType);

				if(rhs_base_type.ops.negate.empty()){
					this->source.error(
						"This type does not have a negate (-a) operator", prefix.rhs,
						std::vector<Message::Info>{
							Message::Info(std::format("Type of right-hand-side: {}", this->src_manager.printType(*rhs_info.value().type_id))),
						}
					);
					return evo::resultError;
				}

				const PIR::Intrinsic::ID intrinsic_id = rhs_base_type.ops.negate[0].intrinsic;
				const PIR::BaseType::ID intrinsic_base_type_id = this->src_manager.getIntrinsic(intrinsic_id).baseType;
				type_id = this->src_manager.getBaseType(intrinsic_base_type_id).callOperator->returnType.typeID();

				if(value_kind == ExprValueKind::Runtime){
					const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(intrinsic_id, std::vector<PIR::Expr>{*rhs_info.value().expr});
					expr = PIR::Expr(func_call_id);

				}else if(value_kind == ExprValueKind::ConstEval){
					this->source.error("At this time, constant-evaluated expressions cannot be negation ([-])", node);
					return evo::resultError;
				}

			} break;

			case Token::get("!"): {
				if(rhs_info.value().type_id.has_value() == false){
					this->source.error("Cannot logical not ([!]) the expression [uninit]", prefix.rhs);
					return evo::resultError;
				}

				const PIR::Type& rhs_type = this->src_manager.getType(*rhs_info.value().type_id);
				const PIR::BaseType& rhs_base_type = this->src_manager.getBaseType(rhs_type.baseType);

				if(rhs_base_type.ops.logicalNot.empty()){
					this->source.error(
						"This type does not have a logical not (!a) operator", prefix.rhs,
						std::vector<Message::Info>{
							Message::Info(std::format("Type of right-hand-side: {}", this->src_manager.printType(*rhs_info.value().type_id))),
						}
					);
					return evo::resultError;
				}

				const PIR::Intrinsic::ID intrinsic_id = rhs_base_type.ops.logicalNot[0].intrinsic;
				const PIR::BaseType::ID intrinsic_base_type_id = this->src_manager.getIntrinsic(intrinsic_id).baseType;
				type_id = this->src_manager.getBaseType(intrinsic_base_type_id).callOperator->returnType.typeID();

				if(value_kind == ExprValueKind::Runtime){
					const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(intrinsic_id, std::vector<PIR::Expr>{*rhs_info.value().expr});
					expr = PIR::Expr(func_call_id);
				}else if(value_kind == ExprValueKind::ConstEval){
					this->source.error("At this time, constant-evaluated expressions cannot be logical not ([!])", node);
					return evo::resultError;
				}
			} break;
		};


		return ExprInfo{
			.value_type = ExprInfo::ValueType::Ephemeral,
			.type_id    = type_id,
			.expr       = expr,
		};
	};




	auto SemanticAnalyzer::analyze_infix_expr(
		AST::Node::ID node_id, ScopeManager& scope_manager, ExprValueKind value_kind, const AST::FuncCall* lookup_func_call
	) noexcept -> evo::Result<ExprInfo> {
		auto output = ExprInfo{};

		const AST::Node& node = this->source.getNode(node_id);
		const AST::Infix& infix = this->source.getInfix(node);
		const Token::Kind infix_op_kind = this->source.getToken(infix.op).kind;


		switch(infix_op_kind){
			case Token::get("."): {
				const evo::Result<ExprInfo> lhs_info = this->analyze_expr(infix.lhs, scope_manager);
				if(lhs_info.isError()){ return evo::resultError; }

				if(lhs_info.value().type_id.has_value() == false){
					this->source.error("The expression [uninit] has no members", infix.lhs);
					return evo::resultError;
				}

				const Token& rhs_ident = this->source.getIdent(infix.rhs);

				if(*lhs_info.value().type_id == SourceManager::getTypeImport()){
					evo::debugAssert(lhs_info.value().expr->kind == PIR::Expr::Kind::Import, "incorrect expr kind gotten");

					const Source& import_source = this->src_manager.getSource(lhs_info.value().expr->import);

					if(import_source.pir.pub_funcs.contains(rhs_ident.value.string)){
						output.value_type = ExprInfo::ValueType::ConcreteConst;

						const std::vector<PIR::Func::ID>& imported_func_list = import_source.pir.pub_funcs.at(rhs_ident.value.string);

						if(imported_func_list.size() == 1){
							const PIR::Func& imported_func = Source::getFunc(imported_func_list[0]);

							output.type_id = this->src_manager.getOrCreateTypeID(PIR::Type(imported_func.baseType)).id;

						}else if(lookup_func_call != nullptr){
							const evo::Result<PIR::Func::ID> imported_func_id = this->lookup_func_in_import(
								rhs_ident.value.string, import_source, *lookup_func_call, scope_manager
							);
							if(imported_func_id.isError()){ return evo::resultError; }

							const PIR::Func& imported_func = Source::getFunc(imported_func_id.value());

							output.type_id = this->src_manager.getOrCreateTypeID(PIR::Type(imported_func.baseType)).id;

						}else{
							this->source.error("Cannot get overloaded function", infix.rhs);
							return evo::resultError;
						}

						if(value_kind == ExprValueKind::Runtime){
							this->source.error("Functions as values is not supported yet", infix.lhs);
							return evo::resultError;
						}else if(value_kind == ExprValueKind::ConstEval){
							this->source.error("At this time, constant-evaluated expressions cannot be accessor ([.])", node);
							return evo::resultError;
						}

					}else if(import_source.pir.pub_vars.contains(rhs_ident.value.string)){
						const PIR::Var::ID imported_var_id = import_source.pir.pub_vars.at(rhs_ident.value.string);
						const PIR::Var& imported_var = Source::getVar(imported_var_id);

						output.value_type = imported_var.isDef ? ExprInfo::ValueType::ConcreteConst : ExprInfo::ValueType::ConcreteMutable;
						output.type_id = imported_var.type;

						if(value_kind == ExprValueKind::Runtime){
							output.expr = PIR::Expr(imported_var_id);
						}else if(value_kind == ExprValueKind::ConstEval){
							this->source.error("At this time, constant-evaluated expressions cannot be accessor ([.])", node);
							return evo::resultError;
						}

					}else if(import_source.pir.pub_imports.contains(rhs_ident.value.string)){
						output.value_type = ExprInfo::ValueType::Import;

						output.type_id = this->src_manager.getTypeImport();

						if(value_kind == ExprValueKind::Runtime){
							const Source::ID imported_source_id = import_source.pir.pub_imports.at(rhs_ident.value.string);
							output.expr = PIR::Expr(imported_source_id);
						}else if(value_kind == ExprValueKind::ConstEval){
							this->source.error("At this time, constant-evaluated expressions cannot be accessor ([.])", node);
							return evo::resultError;
						}

					}else{
						this->source.error(std::format("import does not have public member \"{}\"", rhs_ident.value.string), infix.rhs);
						return evo::resultError;
					}

				}else{
					output.value_type = lhs_info.value().value_type;

					const PIR::Type& lhs_type = this->src_manager.getType(*lhs_info.value().type_id);
					if(lhs_type.qualifiers.empty() == false){
						// TODO: better messaging
						this->source.error("Type does not have a valid accessor operator", infix.lhs);
						return evo::resultError;
					}

					const PIR::BaseType& lhs_base_type = this->src_manager.getBaseType(lhs_type.baseType);
					const PIR::BaseType::StructData& struct_data = std::get<PIR::BaseType::StructData>(lhs_base_type.data);
					for(const PIR::BaseType::StructData::MemberVar& member : struct_data.memberVars){
						if(member.name == rhs_ident.value.string){
							output.type_id = member.type;

							if(value_kind == ExprValueKind::Runtime){
								const PIR::Accessor::ID accessor_id = this->source.createAccessor(
									*lhs_info.value().expr, *lhs_info.value().type_id, rhs_ident.value.string
								);
								output.expr = PIR::Expr(accessor_id);
							}else if(value_kind == ExprValueKind::ConstEval){
								this->source.error("At this time, constant-evaluated expressions cannot be accessor ([.])", node);
								return evo::resultError;
							}

							break;
						}
					}

					if(output.type_id.has_value() == false){
						// TODO: better messaging
						this->source.error(std::format("Type does not have member variable \"{}\"", rhs_ident.value.string), infix.rhs);
						return evo::resultError;
					}
				}
			} break;

			case Token::get("+"): case Token::get("+@"):
			case Token::get("-"): case Token::get("-@"):
			case Token::get("*"): case Token::get("*@"):
			case Token::get("/"):
			case Token::get("=="): case Token::get("!="):
			case Token::get("<"):  case Token::get("<="):
			case Token::get(">"):  case Token::get(">="): 
			case Token::KeywordAnd: case Token::KeywordOr: {
				output.value_type = ExprInfo::ValueType::Ephemeral;

				///////////////////////////////////
				// lhs

				const evo::Result<ExprInfo> lhs_info = this->analyze_expr(infix.lhs, scope_manager, value_kind);
				if(lhs_info.isError()){ return evo::resultError; }

				if(lhs_info.value().type_id.has_value() == false){
					this->source.error(
						std::format("The [{}] operator does not support [uninit]", Token::printKind(infix_op_kind)),
						infix.lhs
					);
					return evo::resultError;
				}

				const PIR::Type& lhs_type = this->src_manager.getType(*lhs_info.value().type_id);

				if(lhs_type.qualifiers.empty() == false){
					this->source.error(
						std::format("Types with qualifiers do not support the [{}] operator", Token::printKind(infix_op_kind)),
						infix.lhs
					);
					return evo::resultError;
				}

				const PIR::BaseType& lhs_base_type = this->src_manager.getBaseType(lhs_type.baseType);


				const bool has_operator = [&]() noexcept {
					switch(infix_op_kind){
						case Token::get("+"):   return lhs_base_type.ops.add.empty() == false;
						case Token::get("+@"):  return lhs_base_type.ops.addWrap.empty() == false;
						case Token::get("-"):   return lhs_base_type.ops.sub.empty() == false;
						case Token::get("-@"):  return lhs_base_type.ops.subWrap.empty() == false;
						case Token::get("*"):   return lhs_base_type.ops.mul.empty() == false;
						case Token::get("*@"):  return lhs_base_type.ops.mulWrap.empty() == false;
						case Token::get("/"):   return lhs_base_type.ops.div.empty() == false;

						case Token::get("=="):  return lhs_base_type.ops.logicalEqual.empty() == false;
						case Token::get("!="):  return lhs_base_type.ops.notEqual.empty() == false;
						case Token::get("<"):   return lhs_base_type.ops.lessThan.empty() == false;
						case Token::get("<="):  return lhs_base_type.ops.lessThanEqual.empty() == false;
						case Token::get(">"):   return lhs_base_type.ops.greaterThan.empty() == false;
						case Token::get(">="):  return lhs_base_type.ops.greaterThanEqual.empty() == false;
						case Token::KeywordAnd: return lhs_base_type.ops.logicalAnd.empty() == false;
						case Token::KeywordOr:  return lhs_base_type.ops.logicalOr.empty() == false;
					};

					evo::debugFatalBreak("Unknown intrinsic kind");
				}();

				if(has_operator == false){
					this->source.error(
						std::format("This type does not have a [{}] operator", Token::printKind(infix_op_kind)), infix.lhs,
						std::vector<Message::Info>{ 
							Message::Info(std::format("Type of left-hand-side: {}", this->src_manager.printType(*lhs_info.value().type_id))),
						}
					);
					return evo::resultError;
				}


				///////////////////////////////////
				// rhs

				const evo::Result<ExprInfo> rhs_info = this->analyze_expr(infix.rhs, scope_manager, value_kind);
				if(rhs_info.isError()){ return evo::resultError; }

				if(rhs_info.value().type_id.has_value() == false){
					this->source.error(
						std::format("The [{}] operator does not support [uninit]", Token::printKind(infix_op_kind)),
						infix.rhs
					);
					return evo::resultError;
				}

				const PIR::Type& rhs_type = this->src_manager.getType(*rhs_info.value().type_id);

				if(rhs_type.qualifiers.empty() == false){
					this->source.error(
						std::format("Types with qualifiers do not support the [{}] operator", Token::printKind(infix_op_kind)), infix.rhs
					);
					return evo::resultError;
				}

				const PIR::BaseType& rhs_base_type = this->src_manager.getBaseType(rhs_type.baseType);


				///////////////////////////////////
				// op checking

				evo::debugAssert(lhs_base_type.kind != PIR::BaseType::Kind::Import, "should have been caught already");
				evo::debugAssert(lhs_base_type.kind != PIR::BaseType::Kind::Function, "should have been caught already");

				const PIR::BaseType* base_type_to_use = &lhs_base_type;

				if(*lhs_info.value().type_id != *rhs_info.value().type_id){
					if(this->is_implicitly_convertable_to(lhs_type, rhs_type, this->source.getNode(infix.lhs))){
						base_type_to_use = &rhs_base_type;
						// do conversion (when needed/implemented)

					}else if(this->is_implicitly_convertable_to(rhs_type, lhs_type, this->source.getNode(infix.rhs))){
						// do conversion (when needed/implemented)
						
					}else{
						// TODO: better messaging
						this->source.error(
							std::format("No matching [{}] operator found", Token::printKind(infix_op_kind)), infix.lhs
						);
						return evo::resultError;
					}
				}


				if(lhs_base_type.kind == PIR::BaseType::Kind::Builtin){
					const PIR::Intrinsic::ID intrinsic_id = [&]() noexcept {
						switch(infix_op_kind){
							case Token::get("+"):  return base_type_to_use->ops.add[0].intrinsic;
							case Token::get("+@"): return base_type_to_use->ops.addWrap[0].intrinsic;
							case Token::get("-"):  return base_type_to_use->ops.sub[0].intrinsic;
							case Token::get("-@"): return base_type_to_use->ops.subWrap[0].intrinsic;
							case Token::get("*"):  return base_type_to_use->ops.mul[0].intrinsic;
							case Token::get("*@"): return base_type_to_use->ops.mulWrap[0].intrinsic;
							case Token::get("/"):  return base_type_to_use->ops.div[0].intrinsic;

							case Token::get("=="): return base_type_to_use->ops.logicalEqual[0].intrinsic;
							case Token::get("!="): return base_type_to_use->ops.notEqual[0].intrinsic;
							case Token::get("<"):  return base_type_to_use->ops.lessThan[0].intrinsic;
							case Token::get("<="): return base_type_to_use->ops.lessThanEqual[0].intrinsic;
							case Token::get(">"):  return base_type_to_use->ops.greaterThan[0].intrinsic;
							case Token::get(">="): return base_type_to_use->ops.greaterThanEqual[0].intrinsic;
							case Token::KeywordAnd: return base_type_to_use->ops.logicalAnd[0].intrinsic;
							case Token::KeywordOr: return base_type_to_use->ops.logicalOr[0].intrinsic;
						};

						evo::debugFatalBreak("Unknown intrinsic kind");
					}();

					const PIR::BaseType::ID intrinsic_base_type_id = this->src_manager.getIntrinsic(intrinsic_id).baseType;
					output.type_id = this->src_manager.getBaseType(intrinsic_base_type_id).callOperator->returnType.typeID();

					if(value_kind == ExprValueKind::Runtime){
						const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(
							intrinsic_id, std::vector<PIR::Expr>{*lhs_info.value().expr, *rhs_info.value().expr}
						);
						output.expr = PIR::Expr(func_call_id);
					}else if(value_kind == ExprValueKind::ConstEval){
						this->source.error(std::format("At this time, constant-evaluated expressions cannot be [{}]", Token::printKind(infix_op_kind)), node);
						return evo::resultError;
					}

				}else if(lhs_base_type.kind == PIR::BaseType::Kind::Struct){
					evo::fatalBreak("Struct operators are not supported yet");

				}else{
					evo::debugFatalBreak("Unknown or unsupported base type kind");
				}

			} break;


			case Token::KeywordAs: case Token::KeywordCast: {
				output.value_type = ExprInfo::ValueType::Ephemeral;

				///////////////////////////////////
				// lhs

				const evo::Result<ExprInfo> lhs_info = this->analyze_expr(infix.lhs, scope_manager, value_kind);
				if(lhs_info.isError()){ return evo::resultError; }

				if(lhs_info.value().type_id.has_value() == false){
					this->source.error("the expression [uninit] cannot be converted to another type", infix.lhs);
					return evo::resultError;
				}

				const PIR::Type& lhs_type = this->src_manager.getType(*lhs_info.value().type_id);

				if(lhs_type.qualifiers.empty() == false){
					this->source.error(
						std::format("Types with qualifiers do not support the [{}] operator", Token::printKind(infix_op_kind)), infix.lhs
					);
					return evo::resultError;
				}

				const PIR::BaseType& lhs_base_type = this->src_manager.getBaseType(lhs_type.baseType);


				///////////////////////////////////
				// rhs

				const evo::Result<PIR::Type::VoidableID> rhs_type_id = this->get_type_id(infix.rhs, scope_manager);
				if(rhs_type_id.isError()){ return evo::resultError; }


				///////////////////////////////////
				// op checking

				evo::debugAssert(lhs_base_type.kind != PIR::BaseType::Kind::Import, "should have been caught already");
				evo::debugAssert(lhs_base_type.kind != PIR::BaseType::Kind::Function, "should have been caught already");


				const std::vector<PIR::BaseType::OverloadedOperator>& op_list = [&]() noexcept {
					switch(infix_op_kind){
						case Token::KeywordAs:   return lhs_base_type.ops.as;
						case Token::KeywordCast: return lhs_base_type.ops.cast;
					};

					evo::debugFatalBreak("invalid op for this infix case");
				}();

				for(const PIR::BaseType::OverloadedOperator& op : op_list){
					if(lhs_base_type.kind == PIR::BaseType::Kind::Builtin){
						const PIR::Intrinsic& intrinsic = this->src_manager.getIntrinsic(op.intrinsic);
						const PIR::BaseType& intrinsic_base_type = this->src_manager.getBaseType(intrinsic.baseType);

						if(intrinsic_base_type.callOperator->returnType == rhs_type_id.value()){
							output.type_id = rhs_type_id.value().typeID();

							if(value_kind == ExprValueKind::Runtime){
								const PIR::FuncCall::ID func_call_id = 
									this->source.createFuncCall(op.intrinsic, std::vector<PIR::Expr>{*lhs_info.value().expr});
								output.expr = PIR::Expr(func_call_id);
							}

							break;
						}

					}else{
						evo::fatalBreak("Overloaded operators on user-defined types are not supported");
					}
				}


				if(output.type_id.has_value() == false){
					// TODO: better messaging
					this->source.error(
						std::format("This type does not have a valid [{}] operator to that type", Token::printKind(infix_op_kind)), infix.rhs,
						std::vector<Message::Info>{
							Message::Info(std::format("From: {}", this->src_manager.printType(*lhs_info.value().type_id))),
							Message::Info(std::format("To:   {}", this->src_manager.printType(rhs_type_id.value().typeID())))
						}
					);
					return evo::resultError;
				}
			} break;

			default: evo::debugFatalBreak("Unknown infix kind");
		};
		


		return output;
	};




	auto SemanticAnalyzer::analyze_postfix_expr(AST::Node::ID node_id, ScopeManager& scope_manager, ExprValueKind value_kind) noexcept -> evo::Result<ExprInfo> {
		auto output = ExprInfo{};

		const AST::Node& node = this->source.getNode(node_id);
		const AST::Postfix& postfix = this->source.getPostfix(node);


		switch(this->source.getToken(postfix.op).kind){
			case Token::get(".&"): {

				// check that it's not taking the address of a dereference
				if(this->source.getConfig().badPracticeDerefOfAddr && this->source.getNode(postfix.lhs).kind == AST::Kind::Prefix){
					const AST::Prefix& rhs_prefix = this->source.getPrefix(postfix.lhs);
					if(this->source.getToken(rhs_prefix.op).kind == Token::KeywordAddr){
						this->source.error(
							"[Bad Practice] Should not dereference ([.&]) an address of ([addr]) expression", postfix.lhs,
							{ 
								Message::Info("Remove both the [addr] and the [.&] as it will give you the same value"),
								Message::Info("Note: (addr x).& == x"),
								Message::Info("You can change this with the config option \"badPracticeDerefOfAddr\""),
							}
						);
						return evo::resultError;
					}
				}


				// get type of lhs
				const evo::Result<ExprInfo> lhs_info = this->analyze_expr(postfix.lhs, scope_manager, value_kind);
				if(lhs_info.isError()){ return evo::resultError; }

				if(lhs_info.value().type_id.has_value() == false){
					this->source.error("Cannot dereference ([.&]) the expression [uninit]", postfix.lhs);
					return evo::resultError;
				}

				// check that type of lhs is pointer
				const PIR::Type& lhs_type = this->src_manager.getType(*lhs_info.value().type_id);
				if(lhs_type.qualifiers.empty() || lhs_type.qualifiers.back().isPtr == false){
					this->source.error(
						"left-hand-side of dereference expression must be of a pointer type", postfix.op,
						std::vector<Message::Info>{
							{std::string("expression is of type: ") + this->src_manager.printType(*lhs_info.value().type_id)},
						}
					);
					return evo::resultError;
				}

				// get expr value type
				if(lhs_type.qualifiers.back().isConst){
					output.value_type = ExprInfo::ValueType::ConcreteConst;
				}else{
					output.value_type = ExprInfo::ValueType::ConcreteMutable;
				}


				// get dereferenced type
				PIR::Type lhs_type_copy = lhs_type;
				lhs_type_copy.qualifiers.pop_back();

				output.type_id = this->src_manager.getOrCreateTypeID(lhs_type_copy).id;

				if(value_kind == ExprValueKind::Runtime){
					const PIR::Type::ID deref_type_id = this->src_manager.getOrCreateTypeID(lhs_type_copy).id;

					const PIR::Deref::ID deref_id = this->source.createDeref(*lhs_info.value().expr, deref_type_id);
					output.expr = PIR::Expr(deref_id);
				}
			} break;

			default: evo::debugFatalBreak("Unknown postfix operator");
		};


		return output;
	};






	auto SemanticAnalyzer::analyze_func_call_expr(AST::Node::ID node_id, ScopeManager& scope_manager, ExprValueKind value_kind) noexcept -> evo::Result<ExprInfo> {
		const AST::Node& node = this->source.getNode(node_id);
		const AST::FuncCall& func_call = this->source.getFuncCall(node);

		// get target type
		const evo::Result<ExprInfo> target_info = this->analyze_expr(func_call.target, scope_manager, ExprValueKind::None, &func_call);
		if(target_info.isError()){ return evo::resultError; }

		// check function call / arguments
		if(this->check_func_call(func_call, *target_info.value().type_id, scope_manager) == false){ return evo::resultError; }

		// check it returns a value
		const PIR::Type& type = this->src_manager.getType(*target_info.value().type_id);
		const PIR::BaseType& base_type = this->src_manager.getBaseType(type.baseType);
		const PIR::Type::VoidableID return_type = base_type.callOperator->returnType;
		if(return_type.isVoid()){
			// TODO: better messaging
			this->source.error("Function does not return a value", func_call.target);
			return evo::resultError;
		}


		auto expr = std::optional<PIR::Expr>();
		const AST::Node& target_node = this->source.getNode(func_call.target);


		if(target_node.kind == AST::Kind::Ident){
			if(value_kind == ExprValueKind::Runtime){
				const evo::Result<std::vector<PIR::Expr>> args = this->get_func_call_args(func_call, scope_manager);
				if(args.isError()){ return evo::resultError; }

				const std::string_view ident = this->source.getIdent(func_call.target).value.string;

				// get func
				const evo::Result<PIR::Func::ID> func_id = this->lookup_func_in_scope(ident, func_call, scope_manager);
				if(func_id.isError()){ return evo::resultError; }


				const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(func_id.value(), std::move(args.value()));
				expr = PIR::Expr(func_call_id);
			}

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

				evo::debugFatalBreak("Unknown intrinsic");
			}();


			// imports
			if(intrinsic_id == SourceManager::getIntrinsicID(PIR::Intrinsic::Kind::import)){
				const evo::Result<std::vector<PIR::Expr>> args = this->get_func_call_args(func_call, scope_manager);
				if(args.isError()){ return evo::resultError; }

				const evo::Result<Source::ID> import_source_id = this->get_import_source_id(args.value()[0], func_call.target);
				if(import_source_id.isError()){ return evo::resultError; }

				expr = PIR::Expr(import_source_id.value());

			}else{
				if(value_kind == ExprValueKind::Runtime){
					const evo::Result<std::vector<PIR::Expr>> args = this->get_func_call_args(func_call, scope_manager);
					if(args.isError()){ return evo::resultError; }

					// function calls normally
					const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(intrinsic_id, std::move(args.value()));	
					expr = PIR::Expr(func_call_id);
				}
			}


		}else if(target_node.kind == AST::Kind::Infix){
			if(value_kind == ExprValueKind::Runtime){
				const evo::Result<std::vector<PIR::Expr>> args = this->get_func_call_args(func_call, scope_manager);
				if(args.isError()){ return evo::resultError; }

				const AST::Infix& infix = this->source.getInfix(func_call.target);

				switch(this->source.getToken(infix.op).kind){
					case Token::get("."): {
						const evo::Result<ExprInfo> lhs_info = this->analyze_expr(infix.lhs, scope_manager);
						if(lhs_info.isError()){ return evo::resultError; }
						evo::debugAssert(lhs_info.value().expr->kind == PIR::Expr::Kind::Import, "incorrect expr kind gotten");

						const Source& import_source = this->src_manager.getSource(lhs_info.value().expr->import);
						const Token& rhs_ident = this->source.getIdent(infix.rhs);

						// get func
						const evo::Result<PIR::Func::ID> imported_func_id = this->lookup_func_in_import(
							rhs_ident.value.string, import_source, func_call, scope_manager
						);
						if(imported_func_id.isError()){ return evo::resultError; }

						
						// create object
						const PIR::FuncCall::ID func_call_id = this->source.createFuncCall(imported_func_id.value(), std::move(args.value()));
						expr = PIR::Expr(func_call_id);
					} break;

				};

				evo::debugFatalBreak("Unknown or unsupported infix type");
			}

		}else{
			evo::debugFatalBreak("Unknown func target kind");
		}


		return ExprInfo{
			.value_type = ExprInfo::ValueType::Ephemeral,
			.type_id    = return_type.typeID(),
			.expr       = expr,
		};
	};





	auto SemanticAnalyzer::analyze_initializer_expr(AST::Node::ID node_id, ScopeManager& scope_manager, ExprValueKind value_kind) noexcept -> evo::Result<ExprInfo> {
		const AST::Node& node = this->source.getNode(node_id);
		const AST::Initializer& initializer = this->source.getInitializer(node);

		const evo::Result<PIR::Type::VoidableID> initializer_type_id = this->get_type_id(initializer.type, scope_manager);
		if(initializer_type_id.isError()){ return evo::resultError; }

		if(initializer_type_id.value().isVoid()){
			this->source.error("Struct initializer cannot be type Void", initializer.type);
			return evo::resultError;
		}

		const PIR::Type& initializer_type = this->src_manager.getType(initializer_type_id.value().typeID());
		evo::debugAssert(initializer_type.qualifiers.empty(), "Struct initializer should not have qualifiers");

		const PIR::BaseType& initializer_base_type = this->src_manager.getBaseType(initializer_type.baseType);
		evo::debugAssert(initializer_base_type.kind == PIR::BaseType::Kind::Struct, "Expected struct type in initializer");

		const PIR::BaseType::StructData& struct_data = std::get<PIR::BaseType::StructData>(initializer_base_type.data);

		// check which members have values and type-check the members
		auto members_with_value = std::vector<bool>(struct_data.memberVars.size(), false);
		for(size_t i = 0; i < struct_data.memberVars.size(); i+=1){
			if(struct_data.memberVars[i].defaultValue.kind != PIR::Expr::Kind::None){
				members_with_value[i] = true;
			}
		}

		for(const AST::Initializer::Member& member_val : initializer.members){
			const std::string_view member_ident = this->source.getIdent(member_val.ident).value.string;

			bool found_member = false;
			for(size_t i = 0; i < struct_data.memberVars.size(); i+=1){

				if(struct_data.memberVars[i].name == member_ident){
					members_with_value[i] = true;
					found_member = true;

					const AST::Node& member_val_node = this->source.getNode(member_val.value);
					if(member_val_node.kind == AST::Kind::Uninit){ break; }

					const evo::Result<ExprInfo> member_val_info = this->analyze_expr(member_val.value, scope_manager, ExprValueKind::None);
					if(member_val_info.isError()){ return evo::resultError; }


					if(this->is_implicitly_convertable_to(
						this->src_manager.getType(*member_val_info.value().type_id), 
						this->src_manager.getType(struct_data.memberVars[i].type),
						member_val_node
					) == false){
						this->source.error(
							"Member cannot be assigned a value of a different type, and the value expression cannot be implicitly converted", 
							member_val_node,
							std::vector<Message::Info>{
								{std::string("Member is of type:     ") + this->src_manager.printType(struct_data.memberVars[i].type)},
								{std::string("Expression is of type: ") + this->src_manager.printType(*member_val_info.value().type_id)}
							}
						);
						return evo::resultError;	
					}

					break;
				}
			}

			if(found_member == false){
				// TODO: better messaging
				this->source.error(std::format("Member \"{}\" does not exist", member_ident), member_val.ident);
				return evo::resultError;
			}
		}

		// check that all members have values
		for(size_t i = 0; i < members_with_value.size(); i+=1){
			if(members_with_value[i] == false){
				const std::string_view member_ident = struct_data.memberVars[i].name;
				this->source.error(std::format("In struct initializer, member \"{}\" was not given a value", member_ident), node);
				return evo::resultError;
			}
		}

		const PIR::Type::ID type_id = initializer_type_id.value().typeID();


		auto expr = std::optional<PIR::Expr>();
		if(value_kind == ExprValueKind::Runtime){
			// get member values
			auto member_values = std::vector<PIR::Expr>(struct_data.memberVars.size(), PIR::Expr());
			auto members_set = std::vector<bool>(struct_data.memberVars.size(), false);
			for(const AST::Initializer::Member& member_val : initializer.members){
				const std::string_view member_ident = this->source.getIdent(member_val.ident).value.string;

				for(size_t i = 0; i < struct_data.memberVars.size(); i+=1){
					if(struct_data.memberVars[i].name == member_ident){
						members_set[i] = true;

						if(this->source.getNode(member_val.value).kind == AST::Kind::Uninit){ break; }

						const evo::Result<ExprInfo> member_val_expr_info = this->analyze_expr(member_val.value, scope_manager);
						if(member_val_expr_info.isError()){ return evo::resultError; }

						member_values[i] = *member_val_expr_info.value().expr;

						break;
					}
				}
			}


			// get defaults
			for(size_t i = 0; i < member_values.size(); i+=1){
				if(members_set[i]){ continue; }

				const PIR::Expr& default_value = struct_data.memberVars[i].defaultValue;

				if(default_value.kind != PIR::Expr::Kind::ASTNode){
					member_values[i] = default_value;
					continue;
				}

				const AST::Node& default_value_node = this->source.getNode(default_value.astNode);
				if(default_value_node.kind != AST::Kind::Uninit){
					member_values[i] = default_value;
				}
			}

			expr = PIR::Expr(this->source.createInitializer(initializer_type_id.value().typeID(), std::move(member_values)));
		}


		return ExprInfo{
			.value_type = ExprInfo::ValueType::Ephemeral,
			.type_id    = type_id,
			.expr       = expr,
		};
	};





	auto SemanticAnalyzer::analyze_ident_expr(
		AST::Node::ID node_id, ScopeManager& scope_manager, ExprValueKind value_kind, const AST::FuncCall* lookup_func_call
	) noexcept -> evo::Result<ExprInfo> {
		auto output = ExprInfo{};


		const AST::Node& node = this->source.getNode(node_id);
		const Token& ident = this->source.getIdent(node);
		std::string_view ident_str = ident.value.string;

		for(size_t scope_index : scope_manager.get_scopes()){
			const ScopeManager::Scope& scope = this->scope_alloc[scope_index];

			if(scope.vars.contains(ident_str)){
				const PIR::Var::ID var_id = scope.vars.at(ident_str);
				const PIR::Var& var = this->source.getVar(var_id);

				// get value type
				if(var.isDef){
					output.value_type = ExprInfo::ValueType::ConcreteConst;
				}else{
					output.value_type = ExprInfo::ValueType::ConcreteMutable;
				}

				// get type
				output.type_id = var.type;

				// get expr
				if(value_kind == ExprValueKind::Runtime){
					output.expr = PIR::Expr(var_id);

				}else if(value_kind == ExprValueKind::ConstEval){
					if(scope_manager.is_global_scope() == false){
						this->source.error("Constant-evaluated expressions cannot be the value of a local variable", node);
						return evo::resultError;						
					}

					this->source.error("At this time, constant-evaluated expressions cannot be the value of a variable", node);
					return evo::resultError;
				}

				break;

			}else if(scope.funcs.contains(ident_str)){
				if(value_kind != ExprValueKind::None || lookup_func_call == nullptr){
					this->source.error("Functions as values are not supported yet", node);
					return evo::resultError;
				}

				const evo::Result<PIR::Func::ID> lookup_func_id = this->lookup_func_in_scope(ident_str, *lookup_func_call, scope_manager);
				if(lookup_func_id.isError()){ return evo::resultError; }

				// get value type
				output.value_type = ExprInfo::ValueType::ConcreteConst;

				// get type
				const PIR::Func& lookup_func = this->source.getFunc(lookup_func_id.value());
				output.type_id = this->src_manager.getOrCreateTypeID(PIR::Type(lookup_func.baseType)).id;

				break;

			}else if(scope.structs.contains(ident_str)){
				this->source.error("Types cannot be used as expressions", node);
				return evo::resultError;

			}else if(scope.params.contains(ident_str)){
				const PIR::Param::ID param_id = scope.params.at(ident_str);
				const PIR::Param& param = this->source.getParam(param_id);

				// get expr value type
				using ParamKind = AST::FuncParams::Param::Kind;
				switch(param.kind){
					break; case ParamKind::Read:  output.value_type = ExprInfo::ValueType::ConcreteConst;
					break; case ParamKind::Write: output.value_type = ExprInfo::ValueType::ConcreteMutable;
					break; case ParamKind::In:    output.value_type = ExprInfo::ValueType::ConcreteMutable;
				};

				// get type
				output.type_id = param.type;

				// get expr
				if(value_kind == ExprValueKind::Runtime){
					output.expr = PIR::Expr(param_id);

				}else if(value_kind == ExprValueKind::ConstEval){
					this->source.error("Constant-evaluated expressions cannot be the value of a parameter", node);
					return evo::resultError;
				}

				break;

			}else if(scope.imports.contains(ident_str)){
				// get value type
				output.value_type = ExprInfo::ValueType::Import;

				// get type
				output.type_id = this->src_manager.getTypeImport();

				// get expr
				const Source::ID import_source_id = scope.imports.at(ident_str).source_id;
				output.expr = PIR::Expr(import_source_id);

				break;

			}else if(scope.aliases.contains(ident_str)){
				this->source.error("Type aliases cannot be used as expressions", node);
				return evo::resultError;

			}else if(scope.template_args.contains(ident_str)){
				const PIR::TemplateArg& template_arg = scope.template_args.at(ident_str);

				if(template_arg.isType){
					this->source.error("Type template args cannot be used as expressions", node);
					return evo::resultError;
				}

				output.type_id = template_arg.typeID.typeID();
				output.expr = *template_arg.expr;
				break;
			}
		}

		if(output.type_id.has_value() == false){
			this->source.error(std::format("Identifier \"{}\" does not exist", ident_str), ident);
			return evo::resultError;
		}

		return output;
	};





	auto SemanticAnalyzer::analyze_literal_expr(AST::Node::ID node_id, ExprValueKind value_kind) const noexcept -> evo::Result<ExprInfo> {
		const Token& literal_value = this->source.getLiteral(node_id);

		if(literal_value.kind == Token::LiteralFloat){
			this->source.error("Literal floats are not supported yet", node_id);
			return evo::resultError;
		}
		if(literal_value.kind == Token::LiteralChar){
			this->source.error("Literal chars are not supported yet", node_id);
			return evo::resultError;
		}

		const Token::Kind base_type = [&]() noexcept {
			switch(literal_value.kind){
				break; case Token::LiteralInt: return Token::TypeInt;
				break; case Token::LiteralBool: return Token::TypeBool;
				break; case Token::LiteralString: return Token::TypeString;
			};

			evo::debugFatalBreak("Unkonwn literal type");
		}();

		const PIR::Type::ID type_id = this->src_manager.getOrCreateTypeID(
			PIR::Type( this->src_manager.getBaseTypeID(base_type) )
		).id;


		auto expr = std::optional<PIR::Expr>();
		if(value_kind != ExprValueKind::None){
			expr = PIR::Expr(node_id);
		}

		return ExprInfo{
			.value_type = ExprInfo::ValueType::Ephemeral,
			.type_id    = type_id,
			.expr       = expr,
		};
	};





	auto SemanticAnalyzer::analyze_intrinsic_expr(AST::Node::ID node_id, ExprValueKind value_kind) const noexcept -> evo::Result<ExprInfo> {
		evo::debugAssert(value_kind == ExprValueKind::None, "getting value of intrinsic is unsupported at the moment");

		auto type_id = std::optional<PIR::Type::ID>();

		const AST::Node& node = this->source.getNode(node_id);
		const Token& intrinsic_tok = this->source.getIntrinsic(node);

		for(const PIR::Intrinsic& intrinsic : this->src_manager.getIntrinsics()){
			if(intrinsic.ident == intrinsic_tok.value.string){
				return ExprInfo{
					.value_type = ExprInfo::ValueType::ConcreteConst,
					.type_id    = this->src_manager.getOrCreateTypeID(PIR::Type(intrinsic.baseType)).id,
					.expr       = std::nullopt,
				};
			}
		}

		this->source.error(std::format("Intrinsic \"@{}\" does not exist", intrinsic_tok.value.string), intrinsic_tok);
		return evo::resultError;
	};





	auto SemanticAnalyzer::analyze_uninit_expr(AST::Node::ID node_id, ExprValueKind value_kind) const noexcept -> evo::Result<ExprInfo> {
		auto expr = std::optional<PIR::Expr>();
		if(value_kind == ExprValueKind::Runtime){
			expr = PIR::Expr(node_id);
		}else if(value_kind == ExprValueKind::ConstEval){
			this->source.error("Constant-evaluated expressions cannot be [uninit]", node_id);
			return evo::resultError;
		}

		return ExprInfo{
			.value_type = ExprInfo::ValueType::Ephemeral,
			.type_id    = std::nullopt,
			.expr       = expr,
		};
	};













	auto SemanticAnalyzer::get_type_id(AST::Node::ID node_id, ScopeManager& scope_manager) noexcept -> evo::Result<PIR::Type::VoidableID> {
		const AST::Node& node = this->source.getNode(node_id);

		const AST::Type type = [&]() noexcept {
			if(node.kind == AST::Kind::Type){
				return this->source.getType(node);

			}else if(node.kind == AST::Kind::Ident || node.kind == AST::Kind::Infix || node.kind == AST::Kind::TemplatedExpr){
				return AST::Type(false, AST::Type::Base{ .node = node_id }, {});

			}else{
				evo::debugFatalBreak("Unknown node kind");
			}
		}();

		auto base_type_id = std::optional<PIR::BaseType::ID>();
		auto type_qualifiers = std::vector<AST::Type::Qualifier>();

		if(type.isBuiltin){
			const Token& type_token = this->source.getToken(type.base.token);


			if(type_token.kind == Token::TypeVoid){
				if(type.qualifiers.empty() == false){
					this->source.error("Void type cannot have qualifiers", node);
					return evo::resultError;
				}

				return PIR::Type::VoidableID::Void();
			}

			if(type_token.kind == Token::TypeString){
				this->source.error("String type is not supported yet", node);
				return evo::resultError;
			}



			base_type_id = this->src_manager.getBaseTypeID(type_token.kind);
			type_qualifiers = type.qualifiers;

		}else{
			// not builtin-type

			const AST::Node& base_type_node = this->source.getNode(type.base.node);

			switch(base_type_node.kind){
				case AST::Kind::Ident: {
					const Token& ident_tok = this->source.getIdent(base_type_node);
					const std::string_view ident = ident_tok.value.string;

					for(size_t scope_index : scope_manager.get_scopes()){
						const ScopeManager::Scope& scope = this->scope_alloc[scope_index];

						if(scope.aliases.contains(ident)){
							const ScopeManager::Alias& alias = scope.aliases.at(ident);

							if(type.qualifiers.empty() == false && alias.type_id.isVoid()){
								this->source.error("Void type cannot have qualifiers", node); 
								return evo::resultError;
							}

							const PIR::Type& alias_type = this->src_manager.getType(alias.type_id.typeID());

							base_type_id = alias_type.baseType;
							type_qualifiers = alias_type.qualifiers;

							for(const AST::Type::Qualifier& qualifier : type.qualifiers){
								type_qualifiers.push_back(qualifier);
							}

						}else if(scope.structs.contains(ident)){
							const ScopeManager::Scope::StructData& struct_data = scope.structs.at(ident);

							if(struct_data.is_template){
								// TODO: better messaging
								this->source.error("No template pack given for a templated type", node);
								return evo::resultError;
							}

							const PIR::Struct& pir_struct = this->source.getStruct(struct_data.struct_id);

							base_type_id = pir_struct.baseType;
							type_qualifiers = type.qualifiers;

						}else if(scope.template_args.contains(ident)){
							const PIR::TemplateArg& template_arg = scope.template_args.at(ident);

							if(template_arg.isType == false){
								// TODO: better messaging
								this->source.error("Template arg used is not a type", node);
								return evo::resultError;
							}

							if(type.qualifiers.empty() == false && template_arg.typeID.isVoid()){
								this->source.error("Void type cannot have qualifiers", node); 
								return evo::resultError;
							}

							const PIR::Type& template_arg_type = this->src_manager.getType(template_arg.typeID.typeID());

							base_type_id = template_arg_type.baseType;
							type_qualifiers = template_arg_type.qualifiers;

							for(const AST::Type::Qualifier& qualifier : type.qualifiers){
								type_qualifiers.push_back(qualifier);
							}
						}
					}
				} break;


				case AST::Kind::Infix: {
					const AST::Infix& infix = this->source.getInfix(base_type_node);

					if(this->source.getToken(infix.op).kind != Token::get(".")){
						// TODO: better messaging
						this->source.error("Invalid base type", base_type_node);
						return evo::resultError;
					}

					const ExprValueKind value_kind = scope_manager.is_global_scope() ? ExprValueKind::ConstEval : ExprValueKind::Runtime;
					const evo::Result<ExprInfo> lhs_info = this->analyze_expr(infix.lhs, scope_manager, value_kind); 
					if(lhs_info.isError()){ return evo::resultError; }
					evo::debugAssert(lhs_info.value().expr->kind == PIR::Expr::Kind::Import, "incorrect expr kind gotten");

					const Source& import_source = this->src_manager.getSource(lhs_info.value().expr->import);
					const Token& rhs_ident = this->source.getIdent(infix.rhs);

					if(import_source.pir.pub_aliases.contains(rhs_ident.value.string)){
						const PIR::Type::VoidableID type_id =  import_source.pir.pub_aliases.at(rhs_ident.value.string);

						if(type.qualifiers.empty() == false && type_id.isVoid()){
							this->source.error("Void type cannot have qualifiers", node_id); 
							return evo::resultError;
						}

						const PIR::Type& alias_type = this->src_manager.getType(type_id.typeID());

						base_type_id = alias_type.baseType;
						type_qualifiers = alias_type.qualifiers;

						for(const AST::Type::Qualifier& qualifier : type.qualifiers){
							type_qualifiers.push_back(qualifier);
						}

					}else if(import_source.pir.pub_structs.contains(rhs_ident.value.string)){
						const PIR::Struct::ID struct_id = import_source.pir.pub_structs.at(rhs_ident.value.string);
						const PIR::Struct& struct_info = struct_id.source.getStruct(struct_id);

						base_type_id = struct_info.baseType;
						type_qualifiers = type.qualifiers;

					}

				} break;


				case AST::Kind::TemplatedExpr: {
					const AST::TemplatedExpr& templated_expr = this->source.getTemplatedExpr(base_type_node);
					const AST::Node& template_base_node = this->source.getNode(templated_expr.expr);

					if(template_base_node.kind == AST::Kind::Infix){
						this->source.error("Imported template types are not supported yet", template_base_node);
						return evo::resultError;
					}

					const Token::ID ident_tok_id = template_base_node.token;
					const Token& ident_tok = this->source.getToken(ident_tok_id);
					const std::string_view ident = ident_tok.value.string;


					type_qualifiers = type.qualifiers;

					for(size_t scope_index : scope_manager.get_scopes()){
						ScopeManager::Scope& scope = this->scope_alloc[scope_index];

						if(scope.aliases.contains(ident)){
							this->source.error("Templated aliases are not supported yet", template_base_node);
							return evo::resultError;

						}else if(scope.structs.contains(ident)){
							ScopeManager::Scope::StructData& scope_struct_data = scope.structs.at(ident);


							if(scope_struct_data.is_template == false){
								this->source.error("Template pack given for a non-templated type", template_base_node);
								return evo::resultError;
							}


							const AST::Struct& ast_struct = *scope_struct_data.template_info.ast_struct;
							const AST::TemplatePack& template_pack = this->source.getTemplatePack(*ast_struct.templatePack);

							if(templated_expr.templateArgs.size() != template_pack.templates.size()){
								// TODO: better messaging
								this->source.error("Incorrect number of struct template arguments recieved", template_base_node);
								return evo::resultError;
							}


							// get template args
							auto template_args = std::vector<PIR::TemplateArg>();
							for(AST::Node::ID template_arg_node_id : templated_expr.templateArgs){
								const AST::Node& template_arg_node = this->source.getNode(template_arg_node_id);

								if(template_arg_node.kind == AST::Kind::Type){
									const evo::Result<PIR::Type::VoidableID> template_arg_type_id = this->get_type_id(template_arg_node_id, scope_manager);
									if(template_arg_type_id.isError()){ return evo::resultError; }

									template_args.emplace_back(template_arg_type_id.value());

								}else{
									const evo::Result<ExprInfo> template_arg_expr_info = this->analyze_expr(template_arg_node_id, scope_manager, ExprValueKind::ConstEval);
									if(template_arg_expr_info.isError()){ return evo::resultError; }

									template_args.emplace_back(*template_arg_expr_info.value().type_id, *template_arg_expr_info.value().expr);
								}
							}


							const SourceManager::GottenBaseTypeID gotten_base_type_id = this->src_manager.getOrCreateBaseType(
								PIR::BaseType(PIR::BaseType::Kind::Struct, ident, &this->source, template_args)
							);

							if(gotten_base_type_id.created == false){
								base_type_id = gotten_base_type_id.id;

							}else{
								const PIR::Struct::ID struct_id = this->source.createStruct(ident_tok_id, scope_struct_data.template_info.num_created, gotten_base_type_id.id);
								scope_struct_data.template_info.num_created += 1;

								ScopeManager& template_scope_manager = *scope_struct_data.template_info.scope_manager;

								template_scope_manager.enter_scope(nullptr);


									for(size_t i = 0; i < template_args.size(); i+=1){
										const AST::TemplatePack::Template template_param = template_pack.templates[i];
										const PIR::TemplateArg& template_arg = template_args[i];



										// check in for ident redefinition
										const Token& template_param_ident_tok = this->source.getIdent(template_param.ident);
										const std::string_view template_param_ident = template_param_ident_tok.value.string;
										if(template_scope_manager.has_in_scope(template_param_ident)){
											this->already_defined(template_param_ident_tok, template_scope_manager);
											return evo::resultError;
										}

										// add to scope
										if(template_arg.isType){
											template_scope_manager.add_template_arg_to_scope(template_param_ident, PIR::TemplateArg(template_arg.typeID));

										}else{
											// check that template args match template params
											const evo::Result<PIR::Type::VoidableID> template_param_type = this->get_type_id(template_param.typeNode, template_scope_manager);
											if(template_param_type.isError()){ return evo::resultError; }
											evo::debugAssert(template_param_type.value().isVoid() == false, "template arg expr is of type Void");

											if(template_param_type.value().typeID() != template_arg.typeID.typeID()){
												const PIR::Type& template_param_pir_type = this->src_manager.getType(template_param_type.value().typeID());
												const PIR::Type& template_arg_pir_type = this->src_manager.getType(template_arg.typeID.typeID());

												if(this->is_implicitly_convertable_to(
													template_arg_pir_type, template_param_pir_type, this->source.getNode(templated_expr.templateArgs[i])
												) == false){
													this->source.error("Template parameter expected different type", template_base_node,
														{
															Message::Info(
																std::format("In template argument: {}", i)
															),
															Message::Info(
																std::format("Template parameter type: {}", this->src_manager.printType(template_param_type.value().typeID()))
															),
															Message::Info(
																std::format("Template argument type:  {}", this->src_manager.printType(template_arg.typeID.typeID()))
															),
														}
													);
													return evo::resultError;
												}
											}

											template_scope_manager.add_template_arg_to_scope(template_param_ident, PIR::TemplateArg(template_arg.typeID, *template_arg.expr));
										}
									}

									const bool analyze_struct_block_result = this->analyze_struct_block(
										this->source.getStruct(struct_id), ast_struct, template_scope_manager
									);
									
									if(analyze_struct_block_result == false){ return evo::resultError; }

								template_scope_manager.leave_scope();

								base_type_id = gotten_base_type_id.id;
							}
						}
					}



				} break;


				default: {
					// TODO: better messaging
					this->source.error("Invalid base type", base_type_node);
					return evo::resultError;
				} break;
			};
		}


		if(base_type_id.has_value() == false){
			this->source.error("Type does not exist", node_id);
			return evo::resultError;
		}


		// checking const-ness of type levels
		bool type_qualifiers_has_a_const = false;
		bool has_warned = false;
		for(auto i = type_qualifiers.rbegin(); i != type_qualifiers.rend(); ++i){
			if(type_qualifiers_has_a_const){
				if(i->isConst == false && has_warned == false){
					has_warned = true;
					this->source.warning("If one type qualifier level is const, all previous levels will automatically be made const as well", node_id);
				}

				i->isConst = true;

			}else if(i->isConst){
				type_qualifiers_has_a_const = true;
			}
		}


		return PIR::Type::VoidableID(
			this->src_manager.getOrCreateTypeID(
				PIR::Type(*base_type_id, type_qualifiers)
			).id
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
			from.baseType == this->src_manager.getBaseTypeID(Token::TypeUInt) ||
			from.baseType == this->src_manager.getBaseTypeID(Token::TypeISize) ||
			from.baseType == this->src_manager.getBaseTypeID(Token::TypeUSize);

		const bool to_is_integral = 
			to.baseType == this->src_manager.getBaseTypeID(Token::TypeInt) || 
			to.baseType == this->src_manager.getBaseTypeID(Token::TypeUInt) ||
			to.baseType == this->src_manager.getBaseTypeID(Token::TypeISize) ||
			to.baseType == this->src_manager.getBaseTypeID(Token::TypeUSize);

		if(from_is_integral && to_is_integral){ return true; }

		return false;
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

			evo::debugFatalBreak("Unkonwn or unsupported error code");
		}

		return imported_source_id_result.value();
	};


	// TODO: check for exported functions in all files (through saving in SourceManager)
	auto SemanticAnalyzer::is_valid_export_name(std::string_view name) const noexcept -> bool {
		if(name == "main"){ return false; }
		if(name == "puts"){ return false; }
		if(name == "printf"){ return false; }

		return true;
	};


	auto SemanticAnalyzer::already_defined(const Token& ident, ScopeManager& scope_manager) const noexcept -> void {
		const std::string_view ident_str = ident.value.string;

		for(size_t scope_index : scope_manager.get_scopes()){
			const ScopeManager::Scope& scope = this->scope_alloc[scope_index];

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

			if(scope.structs.contains(ident_str)){
				const ScopeManager::Scope::StructData& struct_data = scope.structs.at(ident_str);
				const Location location = [&]() noexcept {
					if(struct_data.is_template){
						return this->source.getIdent(struct_data.template_info.ast_struct->ident).location;
					}else{
						const PIR::Struct& struct_decl = this->source.getStruct(struct_data.struct_id);
						return this->source.getToken(struct_decl.ident).location;
					}
				}();

				this->source.error(
					std::format("Identifier \"{}\" already defined", ident.value.string), ident,
					std::vector<Message::Info>{ Message::Info("First defined here:", location) }
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
				const ScopeManager::Import& import = scope.imports.at(ident_str);
				const Location location = this->source.getIdent(import.ident).location;

				this->source.error(
					std::format("Identifier \"{}\" already defined", ident.value.string), ident,
					std::vector<Message::Info>{ Message::Info("First defined here:", location) }
				);
				return;
			}

			if(scope.aliases.contains(ident_str)){
				const ScopeManager::Alias& alias = scope.aliases.at(ident_str);
				const Location location = this->source.getIdent(alias.ident).location;

				this->source.error(
					std::format("Identifier \"{}\" already defined", ident.value.string), ident,
					std::vector<Message::Info>{ Message::Info("First defined here:", location) }
				);
				return;
			}

			if(scope.template_args.contains(ident_str)){
				// const PIR::TemplateArg& template_arg = scope.template_args.at(ident_str);
				// const Location location = this->source.getIdent(alias.ident).location;

				this->source.error(
					std::format("Identifier \"{}\" already defined", ident.value.string), ident,
					{ Message::Info("(Location info not supported yet)") }
				);
				return;
			}
		}

		evo::debugFatalBreak("Didn't find ident");
	};



	//////////////////////////////////////////////////////////////////////
	// scope

	auto SemanticAnalyzer::ScopeManager::enter_scope(PIR::StmtBlock* stmts_entry) noexcept -> void {
		this->scope_alloc.emplace_back(stmts_entry);
		this->scopes.emplace_back(this->scope_alloc.size() - 1);
	};

	auto SemanticAnalyzer::ScopeManager::leave_scope() noexcept -> void {
		this->scopes.pop_back();
	};


	auto SemanticAnalyzer::ScopeManager::add_var_to_scope(std::string_view str, PIR::Var::ID id) noexcept -> void {
		this->scope_alloc[this->scopes.back()].vars.emplace(str, id);
	};

	auto SemanticAnalyzer::ScopeManager::add_func_to_scope(std::string_view str, PIR::Func::ID id) noexcept -> void {
		ScopeManager::Scope& current_scope = this->scope_alloc[this->scopes.back()];

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

	auto SemanticAnalyzer::ScopeManager::add_struct_to_scope(std::string_view str, Scope::StructData struct_data) noexcept -> void {
		this->scope_alloc[this->scopes.back()].structs.emplace(str, struct_data);
	};

	auto SemanticAnalyzer::ScopeManager::add_param_to_scope(std::string_view str, PIR::Param::ID id) noexcept -> void {
		this->scope_alloc[this->scopes.back()].params.emplace(str, id);
	};

	auto SemanticAnalyzer::ScopeManager::add_import_to_scope(std::string_view str, ScopeManager::Import import) noexcept -> void {
		this->scope_alloc[this->scopes.back()].imports.emplace(str, import);
	};

	auto SemanticAnalyzer::ScopeManager::add_alias_to_scope(std::string_view str, ScopeManager::Alias alias) noexcept -> void {
		this->scope_alloc[this->scopes.back()].aliases.emplace(str, alias);
	};

	auto SemanticAnalyzer::ScopeManager::add_template_arg_to_scope(std::string_view str, PIR::TemplateArg template_arg) noexcept -> void {
		this->scope_alloc[this->scopes.back()].template_args.emplace(str, template_arg);
	};


	auto SemanticAnalyzer::ScopeManager::set_scope_terminated() noexcept -> void {
		this->scope_alloc[this->scopes.back()].is_terminated = true;
	};

	auto SemanticAnalyzer::ScopeManager::scope_is_terminated() const noexcept -> bool {
		return this->scope_alloc[this->scopes.back()].is_terminated;
	};


	auto SemanticAnalyzer::ScopeManager::get_stmts_entry() noexcept -> PIR::StmtBlock& {
		ScopeManager::Scope& current_scope = this->scope_alloc[this->scopes.back()];

		evo::debugAssert(current_scope.stmts_entry != nullptr, "Cannot get stmts entry as it doesn't exist for this scope");

		return *current_scope.stmts_entry;
	};

	auto SemanticAnalyzer::ScopeManager::has_in_scope(std::string_view ident) const noexcept -> bool {
		for(size_t scope_index : this->scopes){
			const ScopeManager::Scope& scope = this->scope_alloc[scope_index];

			if(scope.vars.contains(ident)){ return true; }
			if(scope.funcs.contains(ident)){ return true; }
			if(scope.structs.contains(ident)){ return true; }
			if(scope.params.contains(ident)){ return true; }
			if(scope.imports.contains(ident)){ return true; }
			if(scope.aliases.contains(ident)){ return true; }
			if(scope.template_args.contains(ident)){ return true; }
		}

		return false;
	};


	auto SemanticAnalyzer::ScopeManager::is_in_func_base_scope() const noexcept -> bool {
		return this->scope_alloc[this->scopes.back()].stmts_entry == &this->get_current_func().stmts;
	};


	auto SemanticAnalyzer::lookup_func_in_scope(std::string_view ident, const AST::FuncCall& func_call, ScopeManager& scope_manager) noexcept
	-> evo::Result<PIR::Func::ID> {
		auto func_list = std::vector<PIR::Func::ID>();
		
		for(size_t scope_index : scope_manager.get_scopes()){
			const ScopeManager::Scope& scope = this->scope_alloc[scope_index];

			using ConstFuncScopeListIter = std::unordered_map<std::string_view, std::vector<PIR::Func::ID>>::const_iterator;
			ConstFuncScopeListIter func_scope_list_iter = scope.funcs.find(ident);
			if(func_scope_list_iter == scope.funcs.end()){ continue; }

			for(PIR::Func::ID func : func_scope_list_iter->second){
				func_list.emplace_back(func);
			}
		}

		return this->match_function_to_overloads(ident, func_call, func_list, scope_manager);
	};


	auto SemanticAnalyzer::lookup_func_in_import(std::string_view ident, const Source& import, const AST::FuncCall& func_call, ScopeManager& scope_manager) noexcept
	-> evo::Result<PIR::Func::ID> {
		using ConstPubFuncListIter = std::unordered_map<std::string_view, std::vector<PIR::Func::ID>>::const_iterator;
		ConstPubFuncListIter pub_func_list_iter = import.pir.pub_funcs.find(ident);

		if(pub_func_list_iter != import.pir.pub_funcs.end()){
			return this->match_function_to_overloads(ident, func_call, pub_func_list_iter->second, scope_manager);
		}else{
			return evo::resultError;
		}
	};




	auto SemanticAnalyzer::match_function_to_overloads(
		std::string_view ident, const AST::FuncCall& func_call, evo::ArrayProxy<PIR::Func::ID> overload_list, ScopeManager& scope_manager
	) noexcept -> evo::Result<PIR::Func::ID> {
		if(overload_list.empty()){
			this->source.error(std::format("function \"{}\" is undefined", ident), func_call.target);
			return evo::resultError;
		}

		// if only has one function, use SemanticAnalyzer::check_func_call() which is able to give better error messages
		if(overload_list.size() == 1){
			if(this->check_func_call(func_call, this->src_manager.getOrCreateTypeID(PIR::Type(Source::getFunc(overload_list[0]).baseType)).id, scope_manager) == false){
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
				const evo::Result<ExprInfo> arg_info = this->analyze_expr(arg_id, scope_manager, ExprValueKind::None);
				if(arg_info.isError()){ return evo::resultError; }


				{
					const PIR::Type& param_type = this->src_manager.getType(param.type);
					const PIR::Type& arg_type = this->src_manager.getType(*arg_info.value().type_id);

					if(this->is_implicitly_convertable_to(arg_type, param_type, arg_node) == false){
						func_is_candidate = false;
						break;
					}
				}


			
				// check param kind accepts arg value type
				using ParamKind = AST::FuncParams::Param::Kind;
				switch(param.kind){
					case ParamKind::Read: {
						// accepts any value type
					} break;

					case ParamKind::Write: {
						if(arg_info.value().value_type != ExprInfo::ValueType::ConcreteMutable){
							func_is_candidate = false;
							break;
						}
					} break;

					case ParamKind::In: {
						if(arg_info.value().value_type != ExprInfo::ValueType::Ephemeral){
							func_is_candidate = false;
							break;
						}
					} break;
				};

				// if param is write and arg is a param, mark it as edited
				// if(arg_node.kind == AST::Kind::Ident && param.kind == ParamKind::Write){
				// 	std::string_view param_ident_str = this->source.getIdent(arg_node).value.string;

				// 	for(size_t scope_index : scope_manager.get_scopes()){
				// 		const ScopeManager::Scope& scope = this->scope_alloc[scope_index];

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
	// type scope

	auto SemanticAnalyzer::ScopeManager::enter_type_scope(TypeScope::Kind kind, PIR::Func& func) noexcept -> void {
		evo::debugAssert(kind == TypeScope::Kind::Func, "incorrect kind for pir type");
		this->type_scope_alloc.emplace_back(kind, &func);
		this->type_scopes.emplace_back(this->type_scope_alloc.size() - 1);
	};

	auto SemanticAnalyzer::ScopeManager::enter_type_scope(TypeScope::Kind kind, PIR::Struct& struct_decl) noexcept -> void {
		evo::debugAssert(kind == TypeScope::Kind::Struct, "incorrect kind for pir type");
		this->type_scope_alloc.emplace_back(kind, &struct_decl);
		this->type_scopes.emplace_back(this->type_scope_alloc.size() - 1);
	};

	auto SemanticAnalyzer::ScopeManager::leave_type_scope() noexcept -> void {
		evo::debugAssert(this->in_type_scope(), "Not in a type scope");
		this->type_scopes.pop_back();
	};


	auto SemanticAnalyzer::ScopeManager::in_type_scope() const noexcept -> bool {
		return this->type_scopes.empty() == false;
	};



	auto SemanticAnalyzer::ScopeManager::in_func_scope() const noexcept -> bool {
		return this->in_type_scope() && this->type_scope_alloc[this->type_scopes.back()].kind == TypeScope::Kind::Func;
	};

	auto SemanticAnalyzer::ScopeManager::get_current_func() noexcept -> PIR::Func& {
		evo::debugAssert(this->in_func_scope(), "Not in a func scope");
		return *(this->type_scope_alloc[this->type_scopes.back()].func);
	};

	auto SemanticAnalyzer::ScopeManager::get_current_func() const noexcept -> const PIR::Func& {
		evo::debugAssert(this->in_func_scope(), "Not in a func scope");
		return *(this->type_scope_alloc[this->type_scopes.back()].func);
	};



	auto SemanticAnalyzer::ScopeManager::in_struct_scope() const noexcept -> bool {
		return this->in_type_scope() && this->type_scope_alloc[this->type_scopes.back()].kind == TypeScope::Kind::Struct;
	};

	auto SemanticAnalyzer::ScopeManager::get_current_struct() noexcept -> PIR::Struct& {
		evo::debugAssert(this->in_struct_scope(), "Not in a struct scope");
		return *(this->type_scope_alloc[this->type_scopes.back()].struct_decl);
	};

	auto SemanticAnalyzer::ScopeManager::get_current_struct() const noexcept -> const PIR::Struct& {
		evo::debugAssert(this->in_struct_scope(), "Not in a struct scope");
		return *(this->type_scope_alloc[this->type_scopes.back()].struct_decl);
	};



	//////////////////////////////////////////////////////////////////////
	// scope level

	auto SemanticAnalyzer::ScopeManager::enter_scope_level() noexcept -> void {
		this->scope_levels_alloc.emplace_back();
		this->scope_levels.emplace_back(this->scope_levels_alloc.size() - 1);
	};

	auto SemanticAnalyzer::ScopeManager::leave_scope_level() noexcept -> void {
		const bool scope_level_terminated = [&]() noexcept {
			const ScopeLevel& scope_level = this->scope_levels_alloc[this->scope_levels.back()];
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

	auto SemanticAnalyzer::ScopeManager::add_scope_level_scope() noexcept -> void {
		this->scope_levels_alloc[this->scope_levels.back()].num_scopes += 1;
	};

	auto SemanticAnalyzer::ScopeManager::add_scope_level_terminated() noexcept -> void {
		this->scope_levels_alloc[this->scope_levels.back()].num_terminated += 1;
	};

	
};
