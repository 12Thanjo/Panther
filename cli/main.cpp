#include <evo.h>
#include <iostream>

#include "frontend/Printer.h"
#include "frontend/SourceManager.h"
#include "frontend/CharStream.h"
#include "frontend/Tokenizer.h"
#include "frontend/ParserReader.h"
#include "frontend/Parser.h"






auto main([[maybe_unused]] int argc, [[maybe_unused]] const char* args[]) noexcept -> int {
	// evo::log("Panther:\n");

	const auto printer = panther::Printer{true};


	auto end_early = [&](bool cond) noexcept -> void {
		if(cond == false){
			printer.error("Failed to compile\n");
			printer.trace("Press Enter to close...\n");

			std::cin.get();

			std::exit(1);
		}
	};




	std::string const test_file_path = "./testing/test.pthr";

	auto test_file = evo::fs::File{};
	test_file.open(test_file_path, evo::fs::FileMode::Read);

	std::string test_file_data = test_file.read().value();

	test_file.close();


	auto source_manager = panther::SourceManager{printer};

	auto src_id = source_manager.addSourceFile(test_file_path, std::move(test_file_data));


	auto char_stream = panther::CharStream{source_manager, src_id};

	auto tokenizer = panther::Tokenizer{char_stream};
	const bool tokenize_successful = tokenizer.tokenize();

	end_early(tokenize_successful);


	auto tokenizer_reader = panther::TokenizerReader{tokenizer, printer};
	tokenizer_reader.print_to_console();


	printer.trace("--------------------------\n");

	auto parser = panther::Parser{tokenizer_reader};
	const bool parser_successful = parser.parse();

	end_early(parser_successful);

	auto parser_reader = panther::ParserReader{parser, printer};
	parser_reader.print_to_console();



	printer.trace("Press Enter to close...\n");
	std::cin.get();

	return 0;
}