#include "SemanticAnalyzer.h"

#include "SourceManager.h"

namespace panther{


	auto SemanticAnalyzer::semantic_analysis() noexcept -> bool {
		for(AST::Node::ID global_stmt : this->source.global_stmts){
			const AST::Node& node = this->source.getNode(global_stmt);


			switch(node.kind){
				break; case AST::Kind::VarDecl: {
					if(this->analyize_global_var(this->source.getVarDecl(node)) == false){
						return false;
					}
				} break;

				default: {
					EVO_FATAL_BREAK("This AST Kind is not handled");
				} break;
			};
		}

		return true;
	};





	auto SemanticAnalyzer::analyize_global_var(const AST::VarDecl& var_decl) noexcept -> bool {
		const AST::Type& type = this->source.getType(var_decl.type);
		const Token& type_token = this->source.getToken(type.token);

		const Token& literal_value = this->source.getLiteral(var_decl.expr);

		const Token::ID ident_tok_id = this->source.getNode(var_decl.ident).token;
		const Token& ident = this->source.getToken(ident_tok_id);


		if(this->objects.contains(ident.value.string)){
			this->source.error(
				std::format("Identifier \"{}\" already defined", ident.value.string),
				ident
			);
			return false;
		}




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

		

		SourceManager& src_manager = this->source.getSourceManager();

		const object::Type::ID var_type = src_manager.getType(
			object::Type{
				.base_type = src_manager.getBaseTypeID(type_token.kind),
			}
		);

		src_manager.createGlobalVar(this->source.getID(), ident_tok_id, var_type);

		this->objects.insert(ident.value.string);


		return true;
	};

	
};
