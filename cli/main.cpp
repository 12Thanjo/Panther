#include <evo.h>
#include <iostream>


#include "frontend/SourceManager.h"
#include "frontend/CharStream.h"




auto main([[maybe_unused]] int argc, [[maybe_unused]] const char* args[]) noexcept -> int {
	evo::log("Panther:\n");


	std::string const test_file_path = "./testing/test.pthr";

	auto test_file = evo::fs::File{};
	test_file.open(test_file_path, evo::fs::FileMode::Read);

	std::string test_file_data = test_file.read().value();

	test_file.close();


	auto source_manager = panther::SourceManager{};

	auto src_id = source_manager.addSourceFile(test_file_path, std::move(test_file_data));


	auto char_stream = panther::CharStream{source_manager, src_id};


	while(char_stream.is_eof() == false){
		evo::logInfo(evo::printCharName(char_stream.next()));
	};



	evo::logTrace("Press Enter to close...");
	std::cin.get();

	return 0;
}