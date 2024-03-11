#include "SemanticAnalyzer.h"

#include "SourceManager.h"

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


		// check typing
		const AST::Type& type = this->source.getType(var_decl.type);
		const Token& type_token = this->source.getToken(type.token);

		const Token& literal_value = this->source.getLiteral(var_decl.expr);


		if(type_token.kind == Token::TypeVoid){
			this->source.error("Variable cannot be of type Void", type.token);
			return false;

		}else if(type_token.kind == Token::TypeInt){
			if(literal_value.kind != Token::LiteralInt){
				this->source.error(
					std::format("Variable of type Int cannot be set to value of type [{}]", Token::printKind(literal_value.kind)),
					literal_value
				);

				return false;
			}


		}else if(type_token.kind == Token::TypeBool){
			if(literal_value.kind != Token::LiteralBool){
				this->source.error(
					std::format("Variable of type Bool cannot be set to value of type [{}]", Token::printKind(literal_value.kind)),
					literal_value
				);

				return false;
			}

		}


		// create object
		
		SourceManager& src_manager = this->source.getSourceManager();

		const object::Type::ID var_type = src_manager.getType(
			object::Type{
				.base_type = src_manager.getBaseTypeID(type_token.kind),
			}
		);

		const object::Var::ID var_id = this->source.createVar(ident_tok_id, var_type);

		this->add_var_to_scope(ident.value.string, var_id);


		return true;
	};





	auto SemanticAnalyzer::analyze_func(const AST::Func& func) noexcept -> bool {
		// check ident is unused
		const Token::ID ident_tok_id = this->source.getNode(func.ident).token;
		const Token& ident = this->source.getToken(ident_tok_id);

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


		// analyze block
		if(this->analyze_block(this->source.getBlock(func.block)) == false){
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
