#include "Printer.h"



#include "SourceManager.h"

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


	
	};
};
