#include <evo.h>
#include <iostream>


#include "frontend/SourceManager.h"
#include "frontend/CharStream.h"
#include "frontend/Tokenizer.h"



auto end_early(bool cond) noexcept -> void {
	if(cond == false){
		evo::logError("Failed to compile");
		evo::logTrace("Press Enter to close...");

		std::cin.get();

		std::exit(1);
	}
};


auto main([[maybe_unused]] int argc, [[maybe_unused]] const char* args[]) noexcept -> int {
	// evo::log("Panther:\n");


	std::string const test_file_path = "./testing/test.pthr";

	auto test_file = evo::fs::File{};
	test_file.open(test_file_path, evo::fs::FileMode::Read);

	std::string test_file_data = test_file.read().value();

	test_file.close();


	auto source_manager = panther::SourceManager{};

	auto src_id = source_manager.addSourceFile(test_file_path, std::move(test_file_data));


	auto char_stream = panther::CharStream{source_manager, src_id};

	auto tokenizer = panther::Tokenizer{char_stream};
	const bool tokenize_successful = tokenizer.tokenize();

	end_early(tokenize_successful);


	auto tokenizer_reader = panther::TokenizerReader{tokenizer};
	tokenizer_reader.print();



	evo::logTrace("Press Enter to close...");
	std::cin.get();

	return 0;
}