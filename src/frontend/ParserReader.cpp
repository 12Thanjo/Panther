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

	auto ParserReader::getIntrinsicName(AST::NodeID id) const noexcept -> const std::string& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Intrinsic, "Not an intrinsic");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		const AST::Intrinsic& intrinsic = this->parser.intrinsics[index];
		return this->parser.reader.getStringValue(intrinsic.token);
	};


	auto ParserReader::getAttributeName(TokenID id) const noexcept -> const std::string& {
		evo::debugAssert(this->parser.reader.getKind(id) == Token::Attribute, "Not an attribute");

		return this->parser.reader.getStringValue(id);
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


	auto ParserReader::getMultipleAssignment(AST::NodeID id) noexcept -> AST::MultipleAssignment& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::MultipleAssignment, "Not a multiple assignment");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.multiple_assignments[index];
	};

	auto ParserReader::getMultipleAssignment(AST::NodeID id) const noexcept -> const AST::MultipleAssignment& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::MultipleAssignment, "Not a multiple assignment");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.multiple_assignments[index];
	};





	auto ParserReader::getFuncDef(AST::NodeID id) noexcept -> AST::FuncDef& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::FuncDef, "Not a func def");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.func_defs[index];
	};

	auto ParserReader::getFuncDef(AST::NodeID id) const noexcept -> const AST::FuncDef& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::FuncDef, "Not a func def");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.func_defs[index];
	};


	auto ParserReader::getFuncParams(AST::NodeID id) noexcept -> AST::FuncParams& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::FuncParams, "Not a func params");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.func_params[index];
	};

	auto ParserReader::getFuncParams(AST::NodeID id) const noexcept -> const AST::FuncParams& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::FuncParams, "Not a func params");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.func_params[index];
	};


	auto ParserReader::getFuncOutputs(AST::NodeID id) noexcept -> AST::FuncOutputs& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::FuncOutputs, "Not a func outputs");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.func_outputs[index];
	};

	auto ParserReader::getFuncOutputs(AST::NodeID id) const noexcept -> const AST::FuncOutputs& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::FuncOutputs, "Not a func outputs");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.func_outputs[index];
	};


	auto ParserReader::getBlock(AST::NodeID id) noexcept -> AST::Block& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Block, "Not a block");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.blocks[index];
	};

	auto ParserReader::getBlock(AST::NodeID id) const noexcept -> const AST::Block& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Block, "Not a block");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.blocks[index];
	};



	auto ParserReader::getConditional(AST::NodeID id) noexcept -> AST::Conditional& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Conditional, "Not a conditional");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.conditionals[index];
	};

	auto ParserReader::getConditional(AST::NodeID id) const noexcept -> const AST::Conditional& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Conditional, "Not a conditional");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.conditionals[index];
	};


	auto ParserReader::getWhileLoop(AST::NodeID id) noexcept -> AST::WhileLoop& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::WhileLoop, "Not a while loop");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.while_loops[index];
	};

	auto ParserReader::getWhileLoop(AST::NodeID id) const noexcept -> const AST::WhileLoop& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::WhileLoop, "Not a while loop");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.while_loops[index];
	};


	auto ParserReader::getReturn(AST::NodeID id) noexcept -> AST::Return& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Return, "Not a return");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.returns[index];
	};

	auto ParserReader::getReturn(AST::NodeID id) const noexcept -> const AST::Return& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Return, "Not a return");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.returns[index];
	};





	auto ParserReader::getAttributes(AST::NodeID id) noexcept -> AST::Attributes& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Attributes, "Not attributes");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.attributes[index];
	};

	auto ParserReader::getAttributes(AST::NodeID id) const noexcept -> const AST::Attributes& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::Attributes, "Not attributes");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.attributes[index];
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


	auto ParserReader::getIndexOp(AST::NodeID id) noexcept -> AST::IndexOp& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::IndexOp, "Not an index op");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.index_ops[index];
	};

	auto ParserReader::getIndexOp(AST::NodeID id) const noexcept -> const AST::IndexOp& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::IndexOp, "Not an index op");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.index_ops[index];
	};


	auto ParserReader::getFuncCall(AST::NodeID id) noexcept -> AST::FuncCall& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::FuncCall, "Not an func call");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.func_calls[index];
	};

	auto ParserReader::getFuncCall(AST::NodeID id) const noexcept -> const AST::FuncCall& {
		evo::debugAssert(this->getNodeKind(id) == AST::Kind::FuncCall, "Not an func call");

		const uint32_t index = this->parser.nodes[id.id].value_index;
		return this->parser.func_calls[index];
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
			this->print_stmt(stmt, indenter);
		}
	};


	auto ParserReader::print_stmt(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		switch(this->getNodeKind(id)){
			break; case AST::Kind::VarDecl: this->print_var_decl(id, indenter);
			break; case AST::Kind::MultipleAssignment: this->print_multiple_assignment(id, indenter);
			break; case AST::Kind::FuncDef: this->print_func_def(id, indenter);
			break; case AST::Kind::Conditional: this->print_conditional(id, indenter);
			break; case AST::Kind::WhileLoop: this->print_while_loop(id, indenter);
			break; case AST::Kind::Return: this->print_return(id, indenter);

			break; case AST::Kind::Block: {
				indenter.print();
				this->printer.info("Block:\n");

				indenter.push();
				this->print_block(id, indenter);
				indenter.pop();
			} break;

			break; default:
				this->print_expr(id, indenter);
		};
	};




	auto ParserReader::print_var_decl(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::VarDecl& var_decl = this->getVarDecl(id);

		indenter.print();
		
		this->printer.info("Variable Declaration:\n");
		
		indenter.push();
		indenter.print();
		this->printer.info("Identifier: ");
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
			this->printer.debug("yes\n");
		}else{
			this->printer.debug("no\n");
		}

		indenter.set_arrow();
		indenter.print();
		this->printer.info("Public: ");
		if(var_decl.is_public){
			this->printer.debug("yes\n");
		}else{
			this->printer.debug("no\n");
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



	auto ParserReader::print_multiple_assignment(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::MultipleAssignment& multi_assign = this->getMultipleAssignment(id);

		indenter.print();
		
		this->printer.info("MultipleAssignment:\n");

		indenter.push();

		// static
		indenter.set_arrow();
		indenter.print();
		this->printer.info("Static: ");
		if(multi_assign.is_static){
			this->printer.debug("yes\n");
		}else{
			this->printer.debug("no\n");
		}

		// public
		indenter.set_arrow();
		indenter.print();
		this->printer.info("Public: ");
		if(multi_assign.is_public){
			this->printer.debug("yes\n");
		}else{
			this->printer.debug("no\n");
		}



		indenter.set_arrow();
		indenter.print();
		this->printer.info("Decl Type: ");
		switch(multi_assign.decl_type){
			break; case AST::MultipleAssignment::DeclType::Var: this->printer.debug("var\n");
			break; case AST::MultipleAssignment::DeclType::Def: this->printer.debug("var\n");
			break; case AST::MultipleAssignment::DeclType::None: this->printer.debug("[NONE]\n");
		};


		indenter.set_arrow();
		indenter.print();
		this->printer.info("Targets:\n");
		indenter.push();
		for(int i = 0; i < multi_assign.targets.size(); i+=1){
			if(i < multi_assign.targets.size() - 1){
				indenter.set_arrow();
			}else{
				indenter.set_end();
			}

			indenter.print();

			this->printer.info(std::to_string(i) + ":\n");

			indenter.push();
			indenter.set_end();
			this->print_expr(multi_assign.targets[i], indenter);
			indenter.pop();
		}
		indenter.pop();



		indenter.set_end();
		indenter.print();
		this->printer.info("Value:\n");
		indenter.push();
		indenter.set_end();
		this->print_expr(multi_assign.value, indenter);
		indenter.pop();



		indenter.pop();
	};


	auto ParserReader::print_func_def(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::FuncDef& func_def = this->getFuncDef(id);

		indenter.print();
		
		this->printer.info("Function Definition:\n");

		indenter.push();

		// ident
		indenter.print();
		this->printer.info("Identifier: ");
		this->printer.debug(this->getIdentName(func_def.ident) + '\n');

		// static
		indenter.set_arrow();
		indenter.print();
		this->printer.info("Static: ");
		if(func_def.is_static){
			this->printer.debug("yes\n");
		}else{
			this->printer.debug("no\n");
		}

		// public
		indenter.set_arrow();
		indenter.print();
		this->printer.info("Public: ");
		if(func_def.is_public){
			this->printer.debug("yes\n");
		}else{
			this->printer.debug("no\n");
		}

		// parmas
		indenter.set_arrow();
		this->print_func_params(func_def.func_params, indenter);

		

		// capture
		// indenter.set_arrow();
		// indenter.print();
		// if(func_def.captures.has_value()){
		// 	const std::vector<AST::FuncDef::Capture>& captures = *func_def.captures;

		// 	if(captures.empty()){
		// 		this->printer.info("Captures: ");
		// 		this->printer.debug("[NONE]\n");

		// 	}else{
		// 		this->printer.info("Captures: \n");
		// 		indenter.push();
		// 		for(int i = 0; i < captures.size(); i+=1){
		// 			const AST::FuncDef::Capture& capture = captures[i];

		// 			if(i < captures.size() - 1){
		// 				indenter.set_arrow();
		// 			}else{
		// 				indenter.set_end();
		// 			}
		// 			indenter.print();

		// 			this->printer.info(std::to_string(i) + ":\n");
		// 			indenter.push();

		// 			indenter.print();
		// 			this->printer.info("Ident: ");
		// 			this->printer.debug(this->getIdentName(capture.ident) + '\n');

		// 			indenter.set_end();
		// 			indenter.print();
		// 			this->printer.info("Kind: ");
		// 			switch(capture.kind){
		// 				break; case AST::FuncDef::Capture::Kind::Read: this->printer.debug("Read\n");
		// 				break; case AST::FuncDef::Capture::Kind::Write: this->printer.debug("Write\n");
		// 				break; case AST::FuncDef::Capture::Kind::In: this->printer.debug("In\n");
		// 			};
					

		// 			indenter.pop();
		// 		}
		// 		indenter.pop();
		// 	}

		// }else{
		// 	this->printer.info("Captures: ");
		// 	this->printer.debug("[OPEN]\n");
		// }



		// attributes
		indenter.set_arrow();
		indenter.print();
		const AST::Attributes& attributes = this->getAttributes(func_def.attributes);
		if(attributes.tokens.empty()){
			this->printer.info("Attributes: ");
			this->printer.debug("[NONE]\n");

		}else{
			this->printer.info("Attributes: \n");
			indenter.push();
			for(int i = 0; i < attributes.tokens.size(); i+=1){
				if(i < attributes.tokens.size() - 1){
					indenter.set_arrow();
				}else{
					indenter.set_end();
				}
				indenter.print();

				this->printer.debug(this->getAttributeName(attributes.tokens[i]) + '\n');
			}
			indenter.pop();
		}


		// return type
		indenter.set_arrow();
		indenter.print();
		this->printer.info("Return Types:\n");
		indenter.push();
		const AST::FuncOutputs& returns = this->getFuncOutputs(func_def.returns);
		if(returns.values.size() == 1 && returns.values[0].name.has_value() == false){
			indenter.set_end();
			// indenter.print();
			this->print_type(returns.values[0].type, indenter);
		}else{
			for(int i = 0; i < returns.values.size(); i+=1){
				if(i < returns.values.size() - 1){
					indenter.set_arrow();
				}else{
					indenter.set_end();
				}

				indenter.print();

				this->printer.info(std::to_string(i) + ": \n");

				indenter.push();

				indenter.print();
				this->printer.info("Ident: ");
				this->printer.debug(this->getIdentName(*returns.values[i].name) + '\n');

				indenter.set_end();
				indenter.print();
				this->printer.info("Type: \n");

				indenter.push();
				indenter.set_end();
				this->print_type(returns.values[i].type, indenter);
				indenter.pop();

				indenter.pop();
			}
		}
		indenter.pop();

		// error type
		indenter.set_arrow();
		indenter.print();
		this->printer.info("Error Types:\n");
		indenter.push();
		const AST::FuncOutputs& errors = this->getFuncOutputs(func_def.errors);
		if(errors.values.size() == 1 && errors.values[0].name.has_value() == false){
			indenter.set_end();
			// indenter.print();
			this->print_type(errors.values[0].type, indenter);
		}else{
			for(int i = 0; i < errors.values.size(); i+=1){
				if(i < errors.values.size() - 1){
					indenter.set_arrow();
				}else{
					indenter.set_end();
				}

				indenter.print();

				this->printer.info(std::to_string(i) + ": \n");

				indenter.push();

				indenter.print();
				this->printer.info("Ident: ");
				this->printer.debug(this->getIdentName(*errors.values[i].name) + '\n');

				indenter.set_end();
				indenter.print();
				this->printer.info("Type: \n");
				
				indenter.push();
				indenter.set_end();
				this->print_type(errors.values[i].type, indenter);
				indenter.pop();

				indenter.pop();
			}
		}
		indenter.pop();

		// block
		indenter.set_end();
		indenter.print();
		this->printer.info("Block:\n");
		indenter.push();
			this->print_block(func_def.block, indenter);
		indenter.pop();


		indenter.pop();
	};




	auto ParserReader::print_func_params(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::FuncParams& params = this->getFuncParams(id);

		indenter.print();

		if(params.params.empty() == false){
			this->printer.info("Parameters:\n");
			indenter.push();
				for(int i = 0; i < params.params.size(); i+=1){
					const AST::FuncParams::Param& param = params.params[i];

					if(i < params.params.size() - 1){
						indenter.set_arrow();
					}else{
						indenter.set_end();
					}

					indenter.print();
					this->printer.info(std::format("{}:\n", i));
					indenter.push();

						// ident
						indenter.print();
						this->printer.info("Identifier: ");
						this->printer.debug(this->getIdentName(param.ident) + '\n');

						// type
						indenter.set_arrow();
						indenter.print();
						this->printer.info("Type: \n");
						indenter.push();
							indenter.set_end();
							this->print_type(param.type, indenter);
						indenter.pop();

						// kind
						indenter.set_arrow();
						indenter.print();
						this->printer.info("Kind: ");
						switch(param.kind){
							break; case AST::FuncParams::Param::Kind::Read: this->printer.debug("Read\n");
							break; case AST::FuncParams::Param::Kind::Write: this->printer.debug("Write\n");
							break; case AST::FuncParams::Param::Kind::In: this->printer.debug("In\n");
						};


						// default value
						indenter.set_end();
						indenter.print();
						if(param.default_value.has_value()){
							this->printer.info("Default Value: \n");
							indenter.push();
								indenter.set_end();
								this->print_expr(*param.default_value, indenter);
							indenter.pop();
							
						}else{
							this->printer.info("Default Value: ");
							this->printer.debug("[NONE]\n");
						}


					indenter.pop();
				}
			indenter.pop();

		}else{
			this->printer.info("Parameters: ");
			this->printer.debug("[NONE]\n");
		}
	};



	auto ParserReader::print_block(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::Block& block = this->getBlock(id);

		if(block.stmts.empty()){
			indenter.set_end();
			indenter.print();
			this->printer.debug("[EMPTY]\n");
			return;
		}

		for(int i = 0; i < block.stmts.size(); i+=1){
			const AST::NodeID& stmt = block.stmts[i];

			if(i < block.stmts.size() - 1){
				indenter.set_arrow();
			}else{
				indenter.set_end();
			}

			this->print_stmt(stmt, indenter);
		}
	};




	auto ParserReader::print_conditional(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::Conditional& conditional = this->getConditional(id);

		indenter.print();

		this->printer.info("Conditional:\n");

		indenter.push();

		indenter.print();
		this->printer.info("If:\n");
		indenter.push();
		indenter.set_end();
		this->print_expr(conditional.if_stmt, indenter);
		indenter.pop();

		indenter.set_arrow();
		indenter.print();
		this->printer.info("Then:\n");
		indenter.push();
		indenter.set_end();
		this->print_block(conditional.then_stmt, indenter);
		indenter.pop();

		indenter.set_end();
		indenter.print();
		if(conditional.else_stmt.has_value()){
			this->printer.info("Else:\n");
			indenter.push();
			indenter.set_end();
			this->print_stmt(*conditional.else_stmt, indenter);
			indenter.pop();
		}else{
			this->printer.info("Else: ");
			this->printer.debug("[NONE]\n");
		}

		indenter.pop();
	};



	auto ParserReader::print_while_loop(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::WhileLoop& while_loop = this->getWhileLoop(id);

		indenter.print();

		if(while_loop.is_do_while){
			this->printer.info("Do-While Loop:\n");
		}else{
			this->printer.info("While Loop:\n");
		}

		indenter.push();

			indenter.print();
			this->printer.info("Condition:\n");
			indenter.push();
				indenter.set_end();
				this->print_expr(while_loop.condition, indenter);
			indenter.pop();

			indenter.set_end();
			indenter.print();
			this->printer.info("Then:\n");
			indenter.push();
				indenter.set_end();
				this->print_block(while_loop.block, indenter);
			indenter.pop();

		indenter.pop();
	};


	auto ParserReader::print_return(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::Return& return_stmt = this->getReturn(id);

		indenter.print();

		if(return_stmt.is_throw){
			this->printer.info("Throw:");
		}else{
			this->printer.info("Return:");
		}


		if(return_stmt.kind == AST::Return::Kind::Nothing){
			this->printer.debug(" [NONE]\n");

		}else if(return_stmt.kind == AST::Return::Kind::Ellipsis){
			this->printer.debug(" ...\n");

		}else if(return_stmt.kind == AST::Return::Kind::Expr){
			this->printer.info("\n");
			indenter.push();
				indenter.set_end();
				this->print_expr(*return_stmt.expr, indenter);
			indenter.pop();
		}

	};






	auto ParserReader::print_type(AST::NodeID id, Indenter& indenter) const noexcept -> void {
		const AST::Type& type = this->getType(id);

		switch(type.kind){
			case AST::Type::Kind::Basic: {
				indenter.print();
				const Token::Kind kind = this->parser.reader.getKind(type.value.basic.token);
				auto type_str = std::string{};

				if(kind == Token::Ident){
					type_str = this->parser.reader.getStringValue(type.value.basic.token);

				}else{
					type_str =  Token::print_kind(kind);
				}


				for(const AST::Type::Qualifier& qualifier : type.qualifiers){
					type_str += ' ';
					if(qualifier.is_pointer){ type_str += '^'; }
					if(qualifier.is_const){ type_str += '|'; }
					if(qualifier.is_optional){ type_str += '?'; }
				}


				this->printer.debug(type_str + '\n');

			} break;


			case AST::Type::Kind::Array: {
				indenter.print();
				this->printer.info("Array Type:\n");

				indenter.push();

					indenter.print();
					this->printer.info("Element Type: \n");
					indenter.push();
						indenter.set_end();
						this->print_type(type.value.array.type, indenter);
					indenter.pop();

					indenter.set_arrow();
					indenter.print();
					if(this->getNodeKind(type.value.array.length) == AST::Kind::Underscore){
						this->printer.info("Length: ");
						this->printer.debug("[INFER]\n");
						

					}else{
						this->printer.info("Length: \n");
						indenter.push();
							indenter.set_end();
							this->print_expr(type.value.array.length, indenter);
						indenter.pop();
					}


					indenter.set_end();
					indenter.print();
					this->printer.info("Qualifiers:");

					if(type.qualifiers.empty()){
						this->printer.debug(" [NONE]\n");

					}else{
						for(const AST::Type::Qualifier& qualifier : type.qualifiers){
							this->printer.debug(' ');
							if(qualifier.is_pointer){ this->printer.debug('*'); }
							if(qualifier.is_const){ this->printer.debug('|'); }
							if(qualifier.is_optional){ this->printer.debug('?'); }
						}
						this->printer.debug('\n');
					}


				indenter.pop();

			} break;


			case AST::Type::Kind::Func: {
				indenter.print();

				this->printer.info("Function Type:\n");

				indenter.push();

					this->print_func_params(type.value.func.func_params, indenter);

					// attributes
					indenter.set_arrow();
					indenter.print();
					const AST::Attributes& attributes = this->getAttributes(type.value.func.attributes);
					if(attributes.tokens.empty()){
						this->printer.info("Attributes: ");
						this->printer.debug("[NONE]\n");

					}else{
						this->printer.info("Attributes: \n");
						indenter.push();
						for(int i = 0; i < attributes.tokens.size(); i+=1){
							if(i < attributes.tokens.size() - 1){
								indenter.set_arrow();
							}else{
								indenter.set_end();
							}
							indenter.print();

							this->printer.debug(this->getAttributeName(attributes.tokens[i]) + '\n');
						}
						indenter.pop();
					}


					// return type
					indenter.set_arrow();
					indenter.print();
					this->printer.info("Return Types:\n");
					indenter.push();
					const AST::FuncOutputs& returns = this->getFuncOutputs(type.value.func.returns);
					if(returns.values.size() == 1 && returns.values[0].name.has_value() == false){
						indenter.set_end();
						// indenter.print();
						this->print_type(returns.values[0].type, indenter);
					}else{
						for(int i = 0; i < returns.values.size(); i+=1){
							if(i < returns.values.size() - 1){
								indenter.set_arrow();
							}else{
								indenter.set_end();
							}

							indenter.print();

							this->printer.info(std::to_string(i) + ": \n");

							indenter.push();

							indenter.print();
							this->printer.info("Ident: ");
							this->printer.debug(this->getIdentName(*returns.values[i].name) + '\n');

							indenter.set_end();
							indenter.print();
							this->printer.info("Type: \n");

							indenter.push();
							indenter.set_end();
							this->print_type(returns.values[i].type, indenter);
							indenter.pop();

							indenter.pop();
						}
					}
					indenter.pop();

					// error type
					indenter.set_arrow();
					indenter.print();
					this->printer.info("Error Types:\n");
					indenter.push();
					const AST::FuncOutputs& errors = this->getFuncOutputs(type.value.func.errors);
					if(errors.values.size() == 1 && errors.values[0].name.has_value() == false){
						indenter.set_end();
						// indenter.print();
						this->print_type(errors.values[0].type, indenter);
					}else{
						for(int i = 0; i < errors.values.size(); i+=1){
							if(i < errors.values.size() - 1){
								indenter.set_arrow();
							}else{
								indenter.set_end();
							}

							indenter.print();

							this->printer.info(std::to_string(i) + ": \n");

							indenter.push();

							indenter.print();
							this->printer.info("Ident: ");
							this->printer.debug(this->getIdentName(*errors.values[i].name) + '\n');

							indenter.set_end();
							indenter.print();
							this->printer.info("Type: \n");
							
							indenter.push();
							indenter.set_end();
							this->print_type(errors.values[i].type, indenter);
							indenter.pop();

							indenter.pop();
						}
					}
					indenter.pop();


					indenter.set_end();
					indenter.print();
					this->printer.info("Qualifiers:");

					if(type.qualifiers.empty()){
						this->printer.debug(" [NONE]\n");

					}else{
						for(const AST::Type::Qualifier& qualifier : type.qualifiers){
							this->printer.debug(' ');
							if(qualifier.is_pointer){ this->printer.debug('*'); }
							if(qualifier.is_const){ this->printer.debug('|'); }
							if(qualifier.is_optional){ this->printer.debug('?'); }
						}
						this->printer.debug('\n');
					}


				indenter.pop();
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

			case AST::Kind::Intrinsic: {
				indenter.print();
				this->printer.debug("@" + this->getIntrinsicName(id) + '\n');
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


			case AST::Kind::Postfix: {
				const AST::Postfix& infix = this->getPostfix(id);

				indenter.print();
				this->printer.info("Postfix Operator:\n");

				indenter.push();

					indenter.print();
					this->printer.info("Left-Hand Side: \n");
					indenter.push();
						indenter.set_end();
						this->print_expr(infix.lhs, indenter);
					indenter.pop();

					indenter.set_end();
					indenter.print();
					this->printer.info("Op: ");
					this->printer.debug(std::format("{}\n", Token::print_kind(this->parser.reader.getKind(infix.op))));

				indenter.pop();
			} break;

			case AST::Kind::IndexOp: {
				const AST::IndexOp& infix = this->getIndexOp(id);

				indenter.print();
				this->printer.info("Index Operator:\n");

				indenter.push();

					indenter.print();
					this->printer.info("Target: \n");
					indenter.push();
						indenter.set_end();
						this->print_expr(infix.target, indenter);
					indenter.pop();

					indenter.set_end();
					indenter.print();
					this->printer.info("Index: \n");
					indenter.push();
						indenter.set_end();
						this->print_expr(infix.index, indenter);
					indenter.pop();

				indenter.pop();
			} break;


			case AST::Kind::FuncCall: {
				const AST::FuncCall& func_call = this->getFuncCall(id);

				indenter.print();
				this->printer.info("Function Call:\n");

				indenter.push();

					indenter.print();
					this->printer.info("Target: \n");
					indenter.push();
						indenter.set_end();
						this->print_expr(func_call.target, indenter);
					indenter.pop();

					indenter.set_end();
					indenter.print();

					if(func_call.arguments.empty()){
						this->printer.info("Arguments: ");
						this->printer.debug("[NONE]\n");

					}else{
						this->printer.info("Arguments: \n");
						indenter.push();
							for(int i = 0; i < func_call.arguments.size(); i+=1){
								if(i < func_call.arguments.size() - 1){
									indenter.set_arrow();
								}else{
									indenter.set_end();
								}

								indenter.print();
								this->printer.info(std::to_string(i) + ":\n");

								indenter.push();
									indenter.set_end();
									this->print_expr(func_call.arguments[i], indenter);
								indenter.pop();
							}

						indenter.pop();
					}

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
