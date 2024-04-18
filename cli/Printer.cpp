#include "Printer.h"



#include "frontend/SourceManager.h"

#include <algorithm>

namespace panther{
	namespace cli{
		

		auto Printer::fatal(evo::CStrProxy msg) const noexcept -> void {
			if(this->use_colors){ evo::styleConsoleFatal(); };
			evo::log(msg);
			if(this->use_colors){ evo::styleConsoleReset(); };
		};


		auto Printer::error(evo::CStrProxy msg) const noexcept -> void {
			if(this->use_colors){ evo::styleConsoleError(); };
			evo::log(msg);
			if(this->use_colors){ evo::styleConsoleReset(); };
		};


		auto Printer::warning(evo::CStrProxy msg) const noexcept -> void {
			if(this->use_colors){ evo::styleConsoleWarning(); };
			evo::log(msg);
			if(this->use_colors){ evo::styleConsoleReset(); };
		};


		auto Printer::success(evo::CStrProxy msg) const noexcept -> void {
			if(this->use_colors){ evo::styleConsoleSuccess(); };
			evo::log(msg);
			if(this->use_colors){ evo::styleConsoleReset(); };
		};


		auto Printer::info(evo::CStrProxy msg) const noexcept -> void {
			if(this->use_colors){ evo::styleConsoleInfo(); };
			evo::log(msg);
			if(this->use_colors){ evo::styleConsoleReset(); };
		};


		auto Printer::debug(evo::CStrProxy msg) const noexcept -> void {
			if(this->use_colors){ evo::styleConsoleDebug(); };
			evo::log(msg);
			if(this->use_colors){ evo::styleConsoleReset(); };
		};


		auto Printer::trace(evo::CStrProxy msg) const noexcept -> void {
			if(this->use_colors){ evo::styleConsoleTrace(); };
			evo::log(msg);
			if(this->use_colors){ evo::styleConsoleReset(); };
		};


		auto Printer::print(evo::CStrProxy msg) const noexcept -> void {
			evo::log(msg);
		};






		auto Printer::print_message(const panther::Message& msg) const noexcept -> void {
			switch(msg.type){
				break; case panther::Message::Type::Fatal:
					this->fatal( std::format("Fatal: {}\n", msg.message) );
					this->fatal( std::format("\tThis is most likely a bug in the compiler\n", msg.message) );

				break; case panther::Message::Type::Error:   this->error( std::format("Error: {}\n", msg.message) );
				break; case panther::Message::Type::Warning: this->warning( std::format("Warning: {}\n", msg.message) );
			};

			if(msg.source != nullptr){
				this->trace( std::format("\t{}:{}:{}\n", msg.source->getLocation(), msg.location.line_start, msg.location.collumn_start) );
				this->print_location(*msg.source, msg.location, msg.type);
			}else{
				this->trace("\t[BUILTIN]\n");
			}




			///////////////////////////////////
			// print infos

			for(const Message::Info& info : msg.infos){
				this->info( std::format("\tNote: {}\n", info.string) );

				if(msg.source != nullptr && info.location.has_value()){
					this->print_location(*msg.source, *info.location, Message::Type::Info);
				}
			}
		};




		auto Printer::print_tokens(const Source& source) const noexcept -> void {
			this->info( std::format("Tokens: {}\n", source.getLocation()) );
			if(source.tokens.size() == 0){
				this->trace("(NONE)\n");
			}else if(source.tokens.size() == 1){
				this->trace("(1 token)\n");
			}else{
				this->trace( std::format("({} tokens)\n", source.tokens.size()) );
			}

			auto location_strings = std::vector<std::string>();

			for(const Token& token : source.tokens){
				location_strings.push_back( std::format("<{}:{}>", token.location.line_start, token.location.collumn_start) );
			};

			const size_t longest_location_string_length = std::ranges::max_element(
				location_strings,
				[](const std::string& lhs, const std::string& rhs) noexcept -> bool {
					return lhs.size() < rhs.size();
				}
			)->size();

			for(std::string& str : location_strings){
				while(str.size() < longest_location_string_length){
					str += ' ';
				};

				str += ' ';
			}


			for(size_t i = 0; i < source.tokens.size(); i+=1){
				this->trace(location_strings[i]);

				const Token& token = source.tokens[i];
				this->info( std::format("[{}]", Token::printKind(token.kind)) );


				switch(token.kind){
					break; case Token::Ident: this->debug( std::format(" \"{}\"", token.value.string) );
					break; case Token::Intrinsic: this->debug( std::format(" \"@{}\"", token.value.string) );
					break; case Token::Attribute: this->debug( std::format(" \"#{}\"", token.value.string) );

					break; case Token::LiteralBool: this->debug( std::format(" \"{}\"", token.value.boolean) );
					break; case Token::LiteralInt: this->debug( std::format(" \"{}\"", token.value.integer) );
					break; case Token::LiteralFloat: this->debug( std::format(" \"{}\"", token.value.floating_point) );
					break; case Token::LiteralChar: this->debug( std::format(" \'{}\'", token.value.string) );
					break; case Token::LiteralString: this->debug( std::format(" \"{}\"", token.value.string) );
				};


				this->print("\n");
			}

		};






		auto Printer::print_location(const Source& source, Location location, Message::Type type) const noexcept -> void {

			// find line in the source code
			size_t cursor = 0;
			size_t current_line = 1;
			while(current_line < location.line_start){
				evo::debugAssert(cursor < source.getData().size(), "out of bounds looking for line in source code for error");

				if(source.getData()[cursor] == '\n'){
					current_line += 1;

				}else if(source.getData()[cursor] == '\r'){
					current_line += 1;

					if(source.getData()[cursor + 1] == '\n'){
						cursor += 1;
					}
				}

				cursor += 1;
			};

			// get actual line and remove leading whitespace

			auto line_str = std::string{};
			size_t point_collumn = location.collumn_start;
			bool remove_whitespace = true;

			while(source.getData()[cursor] != '\n' && source.getData()[cursor] != '\r' && cursor < source.getData().size()){
				if(remove_whitespace && (source.getData()[cursor] == '\t' || source.getData()[cursor] == ' ')){
					// remove leading whitespace
					point_collumn -= 1;

				}else{
					line_str += source.getData()[cursor];
					remove_whitespace = false;
				}

				cursor += 1;
			};


			// print line
			const std::string line_number_str = std::to_string(location.line_start);

			this->trace( std::format("\t{} | {}\n", line_number_str, line_str) );


			// print out pointer
			auto pointer_str = std::string("\t");
			for(size_t i = 0; i < line_number_str.size() + 1; i+=1){
				pointer_str.push_back(' ');
			}

			pointer_str += '|';

			for(size_t i = 0; i < point_collumn; i+=1){
				pointer_str += ' ';
			}

			this->trace(pointer_str);

			pointer_str.clear();

			if(location.line_start == location.line_end){
				for(uint32_t i = location.collumn_start; i < location.collumn_end + 1; i+=1){
					pointer_str += '^';
				}
			}else{
				for(size_t i = point_collumn; i < line_str.size() + 1; i+=1){
					if(i == point_collumn){
						pointer_str += '^';
					}else{
						pointer_str += '~';
					}
				}
			}

			pointer_str += '\n';

			switch(type){
				break; case panther::Message::Type::Fatal:   this->fatal(pointer_str);
				break; case panther::Message::Type::Error:   this->error(pointer_str);
				break; case panther::Message::Type::Warning: this->warning(pointer_str);
				break; case panther::Message::Type::Info:    this->info(pointer_str);
			};
		};



		//////////////////////////////////////////////////////////////////////
		// print AST


		auto Printer::print_ast(const Source& source) noexcept -> void {
			this->info( std::format("AST: {}\n", source.getLocation()) );
			if(source.global_stmts.size() == 0){
				this->trace("(NONE)\n");
			}else if(source.global_stmts.size() == 1){
				this->trace("(1 global statement)\n");
			}else{
				this->trace( std::format("({} global statements)\n", source.global_stmts.size()) );
			}


			for(AST::Node::ID node_id : source.global_stmts){
				this->print_stmt(source, source.nodes[node_id.id]);
			}


			this->indents.clear();
		};




		auto Printer::print_stmt(const Source& source, const AST::Node& node) noexcept -> void {
			switch(node.kind){
				break; case AST::Kind::VarDecl: this->print_var_decl(source, node);
				break; case AST::Kind::Func: this->print_func(source, node);
				break; case AST::Kind::Conditional: this->print_conditional(source, node);
				break; case AST::Kind::Return: this->print_return(source, node);
				break; case AST::Kind::Infix: this->print_infix(source, node);

				break; case AST::Kind::FuncCall: this->print_expr(source, node);

				case AST::Kind::Unreachable: {
					this->indenter_print();
					this->info("[UNREACHABLE]\n");
				} break;

				break; default: EVO_FATAL_BREAK("Unknown stmt type");
			};
		};


		auto Printer::print_var_decl(const Source& source, const AST::Node& node) noexcept -> void {
			const AST::VarDecl& var_decl = source.getVarDecl(node);

			this->indenter_print();
			this->info("VarDecl:\n");
			this->indenter_push();

			this->indenter_print();
			this->info("Ident: ");
			this->debug( std::format("{}\n", source.getToken(source.getNode(var_decl.ident).token).value.string) );

			this->indenter_set_arrow();
			this->indenter_print();
			this->info("Decl Type: ");
			if(var_decl.is_def){
				this->debug("def\n");
			}else{
				this->debug("var\n");
			}

			this->indenter_set_arrow();
			this->indenter_print();
			this->info("Type:\n");
			this->indenter_push();
				this->indenter_set_end();
				this->print_type(source, source.getNode(var_decl.type));
			this->indenter_pop();

			this->indenter_set_end();
			this->indenter_print();
			this->info("Expr:\n");
			this->indenter_push();
				this->indenter_set_end();
				this->print_expr(source, source.getNode(var_decl.expr));
			this->indenter_pop();



			this->indenter_pop();
		};


		auto Printer::print_func(const Source& source, const AST::Node& node) noexcept -> void {
			const AST::Func& func = source.getFunc(node);

			this->indenter_print();
			this->info("Func:\n");
			this->indenter_push();

				this->indenter_print();
				this->info("Ident: ");
				this->debug( std::format("{}\n", source.getToken(source.getNode(func.ident).token).value.string) );

				this->indenter_set_arrow();
				this->print_func_params(source, source.getNode(func.params));

				this->indenter_set_arrow();
				this->indenter_print();
				if(func.attributes.empty()){
					this->info("Attributes: ");
					this->debug("[None]\n");
				}else{
					this->info("Attributes:\n");

					this->indenter_push();
					for(size_t i = 0; i < func.attributes.size(); i+=1){
						if(i < func.attributes.size() - 1){
							this->indenter_set_arrow();
						}else{
							this->indenter_set_end();
						}

						this->indenter_print();

						this->debug( std::format("#{}\n", source.getToken(func.attributes[i]).value.string) );
					}
					this->indenter_pop();
				}



				this->indenter_set_arrow();
				this->indenter_print();
				this->info("Return Type:\n");
				this->indenter_push();
					this->indenter_set_end();
					this->print_type(source, source.getNode(func.return_type));
				this->indenter_pop();


				this->indenter_set_end();
				this->indenter_print();
				this->info("Block:\n");
				this->indenter_push();
					this->indenter_set_end();
					this->print_block(source, source.getNode(func.block));
				this->indenter_pop();

			this->indenter_pop();
		};



		auto Printer::print_func_params(const Source& source, const AST::Node& node) noexcept -> void {
			const AST::FuncParams& func_params = source.getFuncParams(node);
			
			this->indenter_print();
			if(func_params.params.empty()){
				this->info("Params: ");
				this->debug("[None]\n");
			}else{
				this->info("Params:\n");
				this->indenter_push();

				for(size_t i = 0; i < func_params.params.size(); i+=1){
					const AST::FuncParams::Param& param = func_params.params[i];

					if(i < func_params.params.size() - 1){
						this->indenter_set_arrow();
					}else{
						this->indenter_set_end();
					}

					this->indenter_print();
					this->info( std::format("param {}:\n", i) );
					this->indenter_push();

						this->indenter_set_arrow();
						this->indenter_print();
						this->info("Ident: ");
						this->debug(std::format("{}\n", source.getToken(source.getNode(param.ident).token).value.string));

						this->indenter_set_arrow();
						this->indenter_print();
						this->info("Type:\n");
						this->indenter_push();
							this->indenter_set_end();
							this->print_type(source, source.getNode(param.type));
						this->indenter_pop();


						this->indenter_set_end();
						this->indenter_print();
						this->info("Kind: ");
						switch(param.kind){
							break; case AST::FuncParams::Param::Kind::Read:  this->debug("read\n");
							break; case AST::FuncParams::Param::Kind::Write: this->debug("write\n");
							break; case AST::FuncParams::Param::Kind::In:    this->debug("in\n");
						};

					this->indenter_pop();
				}

				this->indenter_pop();
			}
		};


		auto Printer::print_conditional(const Source& source, const AST::Node& node) noexcept -> void {
			const AST::Conditional& conditional = source.getConditional(node);

			this->indenter_print();
			this->info("Conditional:\n");
			this->indenter_push();

				this->indenter_print();
				this->info("If:\n");
				this->indenter_push();
					this->indenter_set_end();
					this->print_expr(source, source.getNode(conditional.if_expr));
				this->indenter_pop();


				this->indenter_set_arrow();
				this->indenter_print();
				this->info("Then:\n");
				this->indenter_push();
					this->indenter_set_end();
					this->print_block(source, source.getNode(conditional.then_block));
				this->indenter_pop();


				this->indenter_set_end();
				this->indenter_print();
				if(conditional.else_block.has_value()){
					this->info("Else:\n");
					this->indenter_push();
						this->indenter_set_end();

						const AST::Node& else_block = source.getNode(*conditional.else_block);
						if(else_block.kind == AST::Kind::Block){
							this->print_block(source, else_block);

						}else if(else_block.kind == AST::Kind::Conditional){
							this->print_conditional(source, else_block);

						}else{
							EVO_FATAL_BREAK("Unsupported else-block kind");
						}

					this->indenter_pop();

				}else{
					this->info("Else: ");
					this->debug("[NONE]\n");
				}

			this->indenter_pop();
		};


		auto Printer::print_return(const Source& source, const AST::Node& node) noexcept -> void {
			const AST::Return& return_stmt = source.getReturn(node);

			this->indenter_print();

			if(return_stmt.value.has_value()){
				this->info("Return:\n");
				this->indenter_push();
					this->indenter_set_end();
					this->print_expr(source, source.getNode(*return_stmt.value));
				this->indenter_pop();

			}else{
				this->info("Return: ");
				this->debug("[NONE]\n");
			}

		};


		auto Printer::print_infix(const Source& source, const AST::Node& node) noexcept -> void {
			const AST::Infix& infix = source.getInfix(node);

			this->indenter_print();
			this->info("Infix Op:\n");

			this->indenter_push();
				this->indenter_print();
				this->info("op: ");
				this->debug( std::format("{}\n", Token::printKind(source.getToken(infix.op).kind)) );

				this->indenter_set_arrow();
				this->indenter_print();
				this->info("lhs:\n");
				this->indenter_push();
					this->indenter_set_end();
					this->print_expr(source, source.getNode(infix.lhs));
				this->indenter_pop();

				this->indenter_set_end();
				this->indenter_print();
				this->info("rhs:\n");
				this->indenter_push();
					this->indenter_set_end();
					this->print_expr(source, source.getNode(infix.rhs));
				this->indenter_pop();

			this->indenter_pop();
		};



		auto Printer::print_type(const Source& source, const AST::Node& node) noexcept -> void {
			const AST::Type& type = source.getType(node);

			this->indenter_print();

			auto print_str = std::string( Token::printKind(source.getToken(type.token).kind) );
			bool is_first_qualifer = true;
			for(const AST::Type::Qualifier& qualifier : type.qualifiers){
				if(type.qualifiers.size() > 1){
					if(is_first_qualifer){
						is_first_qualifer = false;
					}else{
						print_str += ' ';
					}
				}
				if(qualifier.is_ptr){ print_str += '^'; }
				if(qualifier.is_const){ print_str += '|'; }
			}

			this->debug(print_str);


			// TODO: check if builtin
			this->trace(" [BUILTIN]\n");
		};

		auto Printer::print_block(const Source& source, const AST::Node& node) noexcept -> void {
			const AST::Block& block = source.getBlock(node);

			if(block.nodes.empty()){
				this->indenter_set_end();
				this->indenter_print();
				this->debug("[EMPTY]\n");
				return;
			}

			for(int i = 0; i < block.nodes.size(); i+=1){
				AST::Node::ID stmt = block.nodes[i];

				if(i < block.nodes.size() - 1){
					this->indenter_set_arrow();
				}else{
					this->indenter_set_end();
				}

				this->print_stmt(source, source.getNode(stmt));
			}
		};



		auto Printer::print_expr(const Source& source, const AST::Node& node) noexcept -> void {
			switch(node.kind){
				case AST::Kind::Literal: {
					this->print_literal(source, node);
				} break;

				case AST::Kind::Ident: {
					this->indenter_print();
					this->debug( std::format("{}\n", source.getToken(node.token).value.string) );
				} break;

				case AST::Kind::Intrinsic: {
					this->indenter_print();
					this->debug( std::format("@{}\n", source.getToken(node.token).value.string) );
				} break;

				case AST::Kind::Uninit: {
					this->indenter_print();
					this->debug("[uninit]\n");
				} break;


				case AST::Kind::FuncCall: {
					const AST::FuncCall& func_call = source.getFuncCall(node);

					this->indenter_print();
					this->info("Function Call:\n");

					this->indenter_push();
						this->indenter_print();
						this->info("Target:\n");
						this->indenter_push();
							this->indenter_set_end();
							this->print_expr(source, source.getNode(func_call.target));
						this->indenter_pop();

						this->indenter_set_end();
						this->indenter_print();
						if(func_call.args.empty()){
							this->info("Arguments: ");
							this->debug("[None]\n");

						}else{
							this->info("Arguments:\n");
							this->indenter_push();
								for(size_t i = 0; i < func_call.args.size(); i+=1){
									if(i - 1 < func_call.args.size()){
										this->indenter_set_arrow();
									}else{
										this->indenter_set_end();
									}
									this->indenter_print();

									this->info(std::format("arg {}:\n", i));
									this->indenter_push();
										this->indenter_set_end();
										this->print_expr(source, source.getNode(func_call.args[i]));
									this->indenter_pop();
								}
							this->indenter_pop();
						}

					this->indenter_pop();
				} break;


				case AST::Kind::Prefix: {
					const AST::Prefix& prefix = source.getPrefix(node);

					this->indenter_print();
					this->info("Prefix Op:\n");

					this->indenter_push();

						this->indenter_set_end();
						this->indenter_print();
						this->info("Op: ");
						this->debug( std::format("{}\n", Token::printKind(source.getToken(prefix.op).kind)) );

						this->indenter_set_end();
						this->indenter_print();
						this->info("RHS:\n");
						this->indenter_push();
							this->indenter_set_end();
							this->print_expr(source, source.getNode(prefix.rhs));
						this->indenter_pop();

					this->indenter_pop();
				} break;


				case AST::Kind::Postfix: {
					const AST::Postfix& postfix = source.getPostfix(node);

					this->indenter_print();
					this->info("Postfix Op:\n");

					this->indenter_push();

						this->indenter_print();
						this->info("Op: ");
						this->debug( std::format("{}\n", Token::printKind(source.getToken(postfix.op).kind)) );

						this->indenter_set_end();
						this->indenter_print();
						this->info("LHS:\n");
						this->indenter_push();
							this->indenter_set_end();
							this->print_expr(source, source.getNode(postfix.lhs));
						this->indenter_pop();

					this->indenter_pop();
				} break;

				break; default: EVO_FATAL_BREAK("Node is not an expr");
			};
		};



		auto Printer::print_literal(const Source& source, const AST::Node& node) noexcept -> void {
			evo::debugAssert(node.kind == AST::Kind::Literal, "Node is not a literal");

			const Token& token = source.getToken(node.token);

			this->indenter_print();

			switch(token.kind){
				break; case Token::LiteralInt: this->debug(std::to_string(token.value.integer)); this->trace(" [LiteralInt]");
				break; case Token::LiteralFloat: this->debug(std::to_string(token.value.floating_point)); this->trace(" [LiteralFloat]");
				break; case Token::LiteralBool: this->debug(evo::boolStr(token.value.boolean)); this->trace(" [LiteralBool]");
			};

			this->print('\n');
		};


		//////////////////////////////////////////////////////////////////////
		// indenter

		auto Printer::indenter_push() noexcept -> void {
			this->indents.emplace_back(IndenterType::Arrow);
		};

		auto Printer::indenter_pop() noexcept -> void {
			this->indents.pop_back();
		};

		auto Printer::indenter_set_arrow() noexcept -> void {
			this->indents.back() = IndenterType::Arrow;
		};

		auto Printer::indenter_set_end() noexcept -> void {
			this->indents.back() = IndenterType::EndArrow;		
		};


		auto Printer::indenter_print() noexcept -> void {
			auto print_string = std::string{};

			for(const IndenterType& indent : this->indents){
				switch(indent){
					break; case IndenterType::Line:     print_string += "|   ";
					break; case IndenterType::Arrow:    print_string += "|-> ";
					break; case IndenterType::EndArrow: print_string += "\\-> ";
					break; case IndenterType::None:     print_string += "    ";
				};
			}


			this->trace(print_string);


			if(this->indents.empty() == false){
				     if(this->indents.back() == IndenterType::Arrow){    this->indents.back() = IndenterType::Line; }
				else if(this->indents.back() == IndenterType::EndArrow){ this->indents.back() = IndenterType::None; }
			}
		};


	
	};
};
