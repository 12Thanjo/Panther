#include "frontend/ParserReader.h"

#include "./Indenter.h"



namespace panther{

	auto ParserReader::getNodeKind(AST::NodeID id) const noexcept -> AST::Kind {
		return this->parser.nodes[id.id].kind;
	};


	auto ParserReader::getIdentName(AST::NodeID id) const noexcept -> const std::string& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Ident, "Not an ident");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		const AST::Ident& ident = this->parser.idents[index];
		return this->parser.reader.getStringValue(ident.token);
	};





	auto ParserReader::getVarDecl(AST::NodeID id) noexcept -> AST::VarDecl& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::VarDecl, "Not a var decl");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.var_decls[index];
	};

	auto ParserReader::getVarDecl(AST::NodeID id) const noexcept -> const AST::VarDecl& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::VarDecl, "Not a var decl");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.var_decls[index];
	};



	auto ParserReader::getType(AST::NodeID id) noexcept -> AST::Type& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Type, "Not a type");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.types[index];
	};

	auto ParserReader::getType(AST::NodeID id) const noexcept -> const AST::Type& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Type, "Not a type");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.types[index];
	};



	auto ParserReader::getLiteral(AST::NodeID id) noexcept -> AST::Literal& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Literal, "Not a literal");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.literals[index];
	};

	auto ParserReader::getLiteral(AST::NodeID id) const noexcept -> const AST::Literal& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Literal, "Not a literal");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.literals[index];
	};



	//////////////////////////////////////////////////////////////////////
	// printing


	auto ParserReader::print_to_console() const noexcept -> void {
		auto indenter = Indenter{};

		for(const AST::NodeID& stmt : this->parser.top_level_statements){
			switch(this->getNodeKind(stmt)){
				break; case AST::Kind::VarDecl: this->print_var_decl(stmt, indenter);
			};
		}
	};



	auto ParserReader::print_var_decl(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::VarDecl& var_decl = this->getVarDecl(id);

		indenter.print();
		
		evo::logInfo("Variable Declaration:");
		
		indenter.push();
		indenter.print();
		evo::logInfo(std::format("Ident: {}", this->getIdentName(var_decl.ident)));

		indenter.set_arrow();
		indenter.print();
		if(var_decl.type.has_value()){
			evo::logInfo("Type: ");
			indenter.push();
			indenter.set_end();
			this->print_type(*var_decl.type, indenter);
			indenter.pop();
		}else{
			evo::logInfo("Type: [INFER]");
		}


		indenter.set_arrow();
		indenter.print();
		if(var_decl.decl_type == AST::VarDecl::DeclType::Var){
			evo::logInfo("Decl Type: var");
		}else{
			evo::logInfo("Decl Type: def");
		}

		indenter.set_arrow();
		indenter.print();
		if(var_decl.is_static){
			evo::logInfo("Static: true");
		}else{
			evo::logInfo("Static: false");
		}

		indenter.set_arrow();
		indenter.print();
		if(var_decl.is_public){
			evo::logInfo("Public: true");
		}else{
			evo::logInfo("Public: false");
		}

		indenter.set_end();
		indenter.print();
		evo::logInfo("Value: ");
		indenter.push();
		indenter.set_end();
		this->print_expr(var_decl.expr, indenter);
		indenter.pop();


		indenter.pop();
	};



	auto ParserReader::print_type(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Type, "Node is not type");

		const AST::Type& type = this->getType(id);


		switch(type.kind){
			case AST::Type::Kind::Ident: {
				indenter.print();
				evo::logInfo(this->parser.reader.getStringValue(type.value.builtin.token));

			} break;

			case AST::Type::Kind::Builtin: {
				indenter.print();
				const Token::Kind kind = this->parser.reader.getKind(type.value.builtin.token);
				evo::logInfo(std::format("{} [BUILTIN]", Token::print_kind(kind)));
			} break;


			default: {
				EVO_FATAL_BREAK("Unknown Type kind");				
			} break;
		};
	};


	auto ParserReader::print_expr(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		switch(this->getNodeKind(id)){
			case AST::Kind::Uninit: {
				indenter.print();
				evo::logInfo("uninit");
			} break;

			case AST::Kind::Literal: {
				this->print_literal(id, indenter);
			} break;

			case AST::Kind::Ident: {
				indenter.print();
				evo::logInfo(this->getIdentName(id));
			} break;

			case AST::Kind::Null: {
				indenter.print();
				evo::logInfo("null");
			} break;

			case AST::Kind::This: {
				indenter.print();
				evo::logInfo("this");
			} break;


			default: {
				EVO_FATAL_BREAK("Unknown Expr type");
			} break;
		};
	};


	auto ParserReader::print_literal(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::Literal& literal = this->getLiteral(id);

		switch(this->parser.reader.getKind(literal.token)){
			case Token::LiteralBool: {
				indenter.print();
				evo::logInfo(std::format("{} [LiteralBool]", evo::boolStr(this->parser.reader.getBoolValue(literal.token))));
			} break;

			case Token::LiteralString: {
				indenter.print();
				evo::logInfo(std::format("{} [LiteralString]", this->parser.reader.getStringValue(literal.token)));
			} break;

			case Token::LiteralChar: {
				indenter.print();
				evo::logInfo(std::format("{} [LiteralChar]", this->parser.reader.getStringValue(literal.token)));
			} break;


			case Token::LiteralInt: {
				indenter.print();
				evo::logInfo(std::format("{} [LiteralInt]", this->parser.reader.getIntegerValue(literal.token)));
			} break;

			case Token::LiteralFloat: {
				indenter.print();
				evo::logInfo(std::format("{} [LiteralFloat]", this->parser.reader.getFloatingPointValue(literal.token)));
			} break;
		};


	};



};
