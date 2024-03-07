#include "Printer.h"



#include "SourceManager.h"

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



		static auto print_src_location(const Printer& printer, const panther::Message& msg) noexcept -> void;



		auto Printer::print_message(const panther::Message& msg) const noexcept -> void {
			switch(msg.type){
				break; case panther::Message::Type::Fatal:
					this->fatal( std::format("Fatal: {}\n", msg.message) );
					this->fatal( std::format("\tThis is most likely a bug in the compiler\n", msg.message) );

				break; case panther::Message::Type::Error:   this->error( std::format("Error: {}\n", msg.message) );
				break; case panther::Message::Type::Warning: this->warning( std::format("Warning: {}\n", msg.message) );
			};

			this->trace( std::format("\t{}:{}:{}\n", msg.source.getLocation(), msg.line, msg.collumn_start) );



			///////////////////////////////////
			// print location

			// find line in the source code
			size_t cursor = 0;
			size_t current_line = 1;
			while(current_line < msg.line){
				evo::debugAssert(cursor < msg.source.getData().size(), "out of bounds looking for line in source code for error");

				if(msg.source.getData()[cursor] == '\n'){
					current_line += 1;

				}else if(msg.source.getData()[cursor] == '\r'){
					current_line += 1;

					if(msg.source.getData()[cursor + 1] == '\n'){
						cursor += 1;
					}
				}

				cursor += 1;
			};

			// get actual line and remove leading whitespace

			auto line_str = std::string{};
			size_t point_collumn = msg.collumn_start;
			bool remove_whitespace = true;

			while(msg.source.getData()[cursor] != '\n' && msg.source.getData()[cursor] != '\r' && cursor < msg.source.getData().size()){
				if(remove_whitespace && (msg.source.getData()[cursor] == '\t' || msg.source.getData()[cursor] == ' ')){
					// remove leading whitespace
					point_collumn -= 1;

				}else{
					line_str += msg.source.getData()[cursor];
					remove_whitespace = false;
				}

				cursor += 1;
			};


			// print line
			const std::string line_number_str = std::to_string(msg.line);

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

			for(uint32_t i = msg.collumn_start; i < msg.collumn_end + 1; i+=1){
				pointer_str += '^';
			}

			pointer_str += '\n';

			switch(msg.type){
				break; case panther::Message::Type::Fatal:   this->fatal(pointer_str);
				break; case panther::Message::Type::Error:   this->error(pointer_str);
				break; case panther::Message::Type::Warning: this->warning(pointer_str);
			};


			///////////////////////////////////
			// print infos

			for(const std::string& info : msg.infos){
				this->info( std::format("\tNote: {}\n", info) );
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
				location_strings.push_back( std::format("<{}:{}>", token.line_start, token.collumn_start) );
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

					break; case Token::LiteralBool: this->debug( std::format(" \"{}\"", token.value.boolean) );
					break; case Token::LiteralInt: this->debug( std::format(" \"{}\"", token.value.integer) );
					break; case Token::LiteralFloat: this->debug( std::format(" \"{}\"", token.value.floating_point) );
				};


				this->print("\n");
			}

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



		auto Printer::print_type(const Source& source, const AST::Node& node) noexcept -> void {
			const AST::Type& type = source.getType(node);

			this->indenter_print();
			this->debug( std::format("{}\n", Token::printKind(source.getToken(type.token).kind)) );
		};


		auto Printer::print_expr(const Source& source, const AST::Node& node) noexcept -> void {
			switch(node.kind){
				break; case AST::Kind::Literal: this->print_literal(source, node);

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
