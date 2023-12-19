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




	auto SourceManager::error(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void {
		this->has_errored = true;

		evo::logError("Error: " + message);

		this->print_location(MessageType::Error, id, line, collumn);
	};

	auto SourceManager::error_info(const std::string& message) noexcept -> void {
		evo::logInfo('\t' + message);
	};
	
	auto SourceManager::error_info(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void {
		evo::logInfo('\t' + message);
		this->print_location(MessageType::ErrorInfo, id, line, collumn);
	};





	auto SourceManager::fatal(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void {
		this->has_errored = true;

		evo::logFatal(message);
		evo::logFatal("This is an error in the compiler");

		this->print_location(MessageType::Error, id, line, collumn);
	};





	auto SourceManager::print_location(MessageType type, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void {
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
		size_t point_collumn = collumn;
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
			evo::logTrace("\t\t" + source.location);
		}else{
			evo::logTrace('\t' + source.location);
		}


		auto line_str_to_print = std::string{'\t'};
		if(type == MessageType::ErrorInfo){
			line_str_to_print += '\t';
		}
		line_str_to_print += std::format("{}| {}", line, line_str);;
		evo::logTrace(line_str_to_print);

		auto point_str = std::string{'\t'};
		size_t pointer_indentation = line_str_to_print.size() - line_str.size() - 2 + point_collumn;
		if(type == MessageType::ErrorInfo){
			pointer_indentation -= 1;
			point_str += '\t';
		}
		for(size_t i = 0; i < pointer_indentation; i+=1){
			point_str += ' ';
		}
		point_str += '^';


		switch(type){
			break; case MessageType::Error:     evo::logError(point_str);
			break; case MessageType::ErrorInfo: evo::logInfo(point_str);
		};

	};

	
};
