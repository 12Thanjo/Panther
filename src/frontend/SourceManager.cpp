#include "frontend/SourceManager.h"


namespace panther{


	auto SourceManager::addSourceFile(const std::string& location, std::string&& data) noexcept -> SourceFileID {
		this->sources.emplace_back(location, std::move(data));

		return SourceFileID{uint32_t(this->sources.size()) - 1};
	};



	auto SourceManager::getSourceFile(SourceFileID id) const noexcept -> const SourceFile& {
		evo::debugAssert(id.id < this->sources.size(), "Attempted to get invalid source file id");

		return this->sources.front();
	};



	auto SourceManager::error(
		const std::string& message, SourceFileID id, uint32_t line_start, uint32_t collumn_start, uint32_t line, uint32_t collumn
	) noexcept -> void {
		this->has_errored = true;

		this->printer.error("Error: " + message + '\n');

		if(line_start != line){
			this->print_location(MessageType::Error, id, line, 1, collumn);
			this->error_info("begins here", id, line_start, collumn_start);

		}else{
			this->print_location(MessageType::Error, id, line, collumn_start, collumn);
		}
	};

	auto SourceManager::error(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void {
		this->has_errored = true;

		this->printer.error("Error: " + message + '\n');

		this->print_location(MessageType::Error, id, line, collumn);
	};

	auto SourceManager::error_info(const std::string& message) noexcept -> void {
		this->printer.info("\tNote: " + message + '\n');
	};
	
	auto SourceManager::error_info(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void {
		this->printer.info("\tNote: " + message + '\n');
		this->print_location(MessageType::ErrorInfo, id, line, collumn);
	};





	auto SourceManager::fatal(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void {
		this->has_errored = true;

		this->printer.fatal("Fatal Error: " + message + '\n');
		this->printer.fatal("This is an error in the compiler" + '\n');

		this->print_location(MessageType::Error, id, line, collumn);
	};


	auto SourceManager::fatal(
		const std::string& message, SourceFileID id, uint32_t line_start, uint32_t collumn_start, uint32_t line, uint32_t collumn
	) noexcept -> void {
		this->has_errored = true;

		this->printer.fatal("Fatal Error: " + message + '\n');
		this->printer.fatal("This is an error in the compiler" + '\n');

		if(line_start != line){
			this->print_location(MessageType::Error, id, line, 1, collumn);
			this->error_info("begins here", id, line_start, collumn_start);

		}else{
			this->print_location(MessageType::Error, id, line, collumn_start, collumn);
		}
	};





	auto SourceManager::print_location(MessageType type, SourceFileID id, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void {
		size_t cursor = 0;
		size_t current_line = 1;
		const SourceFile& source = this->sources[id.id];

		while(current_line < line){
			evo::debugAssert(cursor < source.data.size(), "out of bounds looking for line in source.data file for error");

			if(source.data[cursor] == '\n'){
				current_line += 1;

			}else if(source.data[cursor] == '\r'){
				current_line += 1;

				if(source.data[cursor + 1] == '\n'){
					cursor += 1;
				}
			}

			cursor += 1;
		};

		auto line_str = std::string{};
		size_t point_collumn = collumn_start;
		bool remove_whitespace = true;

		while(source.data[cursor] != '\n' && source.data[cursor] != '\r' && cursor < source.data.size()){
			if(remove_whitespace && (source.data[cursor] == '\t' || source.data[cursor] == ' ')){
				// remove leading whitespace
				point_collumn -= 1;

			}else{
				line_str += source.data[cursor];
				remove_whitespace = false;
			}

			cursor += 1;
		};

		if(type == MessageType::ErrorInfo){
			this->printer.trace("\t\t" + source.location + '\n');
		}else{
			this->printer.trace('\t' + source.location + '\n');
		}


		auto line_str_to_print = std::string{'\t'};
		if(type == MessageType::ErrorInfo){
			line_str_to_print += '\t';
		}
		line_str_to_print += std::format("{}| {}", line, line_str);
		this->printer.trace(line_str_to_print + '\n');

		auto point_str = std::string{'\t'};
		size_t pointer_indentation = line_str_to_print.size() - line_str.size() - 2 + point_collumn;
		if(type == MessageType::ErrorInfo){
			pointer_indentation -= 1;
			point_str += '\t';
		}
		for(size_t i = 0; i < pointer_indentation; i+=1){
			point_str += ' ';
		}

		for(uint32_t i = collumn_start; i < collumn_end + 1; i+=1){
			point_str += '^';
		}


		switch(type){
			break; case MessageType::Error:     this->printer.error(point_str + '\n');
			break; case MessageType::ErrorInfo: this->printer.info(point_str + '\n');
		};
	};

	
};
