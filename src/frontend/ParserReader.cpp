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



	auto ParserReader::getPrefix(AST::NodeID id) noexcept -> AST::Prefix& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Prefix, "Not a prefix");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.prefixes[index];
	};

	auto ParserReader::getPrefix(AST::NodeID id) const noexcept -> const AST::Prefix& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Prefix, "Not a prefix");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.prefixes[index];
	};


	auto ParserReader::getInfix(AST::NodeID id) noexcept -> AST::Infix& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Infix, "Not an infix");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.infixes[index];
	};

	auto ParserReader::getInfix(AST::NodeID id) const noexcept -> const AST::Infix& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Infix, "Not an infix");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.infixes[index];
	};


	auto ParserReader::getPostfix(AST::NodeID id) noexcept -> AST::Postfix& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Postfix, "Not a postfix");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.postfixes[index];
	};

	auto ParserReader::getPostfix(AST::NodeID id) const noexcept -> const AST::Postfix& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Postfix, "Not a postfix");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.postfixes[index];
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
		auto indenter = Indenter{this->printer};

		for(const AST::NodeID& stmt : this->parser.top_level_statements){
			switch(this->getNodeKind(stmt)){
				break; case AST::Kind::VarDecl: this->print_var_decl(stmt, indenter);
			};
		}
	};



	auto ParserReader::print_var_decl(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::VarDecl& var_decl = this->getVarDecl(id);

		indenter.print();
		
		this->printer.info("Variable Declaration:\n");
		
		indenter.push();
		indenter.print();
		this->printer.info("Ident: ");
		this->printer.debug(this->getIdentName(var_decl.ident) + '\n');

		indenter.set_arrow();
		indenter.print();
		if(var_decl.type.has_value()){
			this->printer.info("Type: \n");
			indenter.push();
			indenter.set_end();
			this->print_type(*var_decl.type, indenter);
			indenter.pop();
		}else{
			this->printer.info("Type: ");
			this->printer.debug("[INFER]\n");
		}


		indenter.set_arrow();
		indenter.print();
		this->printer.info("Decl Type: ");
		if(var_decl.decl_type == AST::VarDecl::DeclType::Var){
			this->printer.debug("var\n");
		}else{
			this->printer.debug("def\n");
		}

		indenter.set_arrow();
		indenter.print();
		this->printer.info("Static: ");
		if(var_decl.is_static){
			this->printer.debug("true\n");
		}else{
			this->printer.debug("false\n");
		}

		indenter.set_arrow();
		indenter.print();
		this->printer.info("Static: ");
		if(var_decl.is_public){
			this->printer.debug("true\n");
		}else{
			this->printer.debug("false\n");
		}

		indenter.set_end();
		indenter.print();
		this->printer.info("Value: \n");
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
				this->printer.debug(this->parser.reader.getStringValue(type.value.builtin.token) + '\n');

			} break;

			case AST::Type::Kind::Builtin: {
				indenter.print();
				const Token::Kind kind = this->parser.reader.getKind(type.value.builtin.token);
				this->printer.debug(std::format("{} [BUILTIN]\n", Token::print_kind(kind)));
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
				this->printer.debug("uninit\n");
			} break;

			case AST::Kind::Literal: {
				this->print_literal(id, indenter);
			} break;

			case AST::Kind::Ident: {
				indenter.print();
				this->printer.debug(this->getIdentName(id) + '\n');
			} break;

			case AST::Kind::Null: {
				indenter.print();
				this->printer.debug("null\n");
			} break;

			case AST::Kind::This: {
				indenter.print();
				this->printer.debug("this\n");
			} break;


			case AST::Kind::Type: {
				this->print_type(id, indenter);
			} break;


			case AST::Kind::Prefix: {
				const AST::Prefix& infix = this->getPrefix(id);

				indenter.print();
				this->printer.info("Prefix Operator:\n");

				indenter.push();

					indenter.set_arrow();
					indenter.print();
					this->printer.info("Op: ");
					this->printer.debug(std::format("{}\n", Token::print_kind(this->parser.reader.getKind(infix.op))));

					indenter.set_end();
					indenter.print();
					this->printer.info("Right-Hand Side: \n");
					indenter.push();
						indenter.set_end();
						this->print_expr(infix.rhs, indenter);
					indenter.pop();


				indenter.pop();
			} break;

			case AST::Kind::Infix: {
				const AST::Infix& infix = this->getInfix(id);

				indenter.print();
				this->printer.info("Infix Operator:\n");

				indenter.push();

					indenter.set_arrow();
					indenter.print();
					this->printer.info("Op: ");
					this->printer.debug(std::format("{}\n", Token::print_kind(this->parser.reader.getKind(infix.op))));

					indenter.set_arrow();
					indenter.print();
					this->printer.info("Left-Hand Side: \n");
					indenter.push();
						indenter.set_end();
						this->print_expr(infix.lhs, indenter);
					indenter.pop();

					indenter.set_end();
					indenter.print();
					this->printer.info("Right-Hand Side: \n");
					indenter.push();
						indenter.set_end();
						this->print_expr(infix.rhs, indenter);
					indenter.pop();


				indenter.pop();
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
				this->printer.debug(std::format("{} [LiteralBool]\n", evo::boolStr(this->parser.reader.getBoolValue(literal.token))));
			} break;

			case Token::LiteralString: {
				indenter.print();
				this->printer.debug(std::format("{} [LiteralString]\n", this->parser.reader.getStringValue(literal.token)));
			} break;

			case Token::LiteralChar: {
				indenter.print();
				this->printer.debug(std::format("{} [LiteralChar]\n", this->parser.reader.getStringValue(literal.token)));
			} break;


			case Token::LiteralInt: {
				indenter.print();
				this->printer.debug(std::format("{} [LiteralInt]\n", this->parser.reader.getIntegerValue(literal.token)));
			} break;

			case Token::LiteralFloat: {
				indenter.print();
				this->printer.debug(std::format("{} [LiteralFloat]\n", this->parser.reader.getFloatingPointValue(literal.token)));
			} break;
		};


	};



};
