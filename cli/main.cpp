
#include "./Printer.h"
#include "SourceManager.h"


#include <Evo.h>
#include <iostream>
#include <filesystem>



#if !defined(NOMINMAX)
	#define WIN32_LEAN_AND_MEAN
#endif

#if !defined(NOMINMAX)
	#define NOCOMM
#endif

#if !defined(NOMINMAX)
	#define NOMINMAX
#endif

#include <Windows.h>





struct Config{
	bool print_colors;
	bool verbose;

	enum class Output{
		PrintTokens,
		PrintAST,
	} output;

	std::filesystem::path relative_directory{};
	bool relative_directory_set = false;
};


auto main([[maybe_unused]] int argc, [[maybe_unused]] const char* args[]) noexcept -> int {

	auto config = Config{
		.print_colors = true,
		.verbose      = true,
		.output       = Config::Output::PrintAST,
	};



	// print UTF-8 characters on windows
	#if defined(EVO_PLATFORM_WINDOWS)
		SetConsoleOutputCP(CP_UTF8);
	#endif


	auto printer = panther::cli::Printer(config.print_colors);



	auto exit = [&](){
		printer.trace("Press Enter to close...\n");
		std::cin.get();
	};






	if(config.verbose){
		printer.info("Panther Compiler\n");
		printer.trace("----------------\n");
	}



	if(config.relative_directory_set == false){
		std::error_code ec;
		config.relative_directory = std::filesystem::current_path(ec);
		if(ec){
			printer.error("Failed to get relative directory\n");
			printer.error(std::format("\tcode: \"{}\"\n", ec.value()));
			printer.error(std::format("\tmessage: \"{}\"\n", ec.message()));

			exit();
			return 1;
		}
	}


	if(config.verbose){
		printer.debug( std::format("Relative Directory: {}\n", config.relative_directory.string()) );
	}


	auto source_manager = panther::SourceManager([&](const panther::Message& message){
		printer.print_message(message);
	});


	//////////////////////////////////////////////////////////////////////
	// get code

	const std::string file_path = (config.relative_directory / "testing/test.pthr").make_preferred().string();

	auto file = evo::fs::File{};
	{
		const bool opened_successfully = file.open(file_path, evo::fs::FileMode::Read);
		if(opened_successfully == false){
			printer.error(std::format("Failed to open file: {}\n", file_path));
			exit();
			return 1;
		}
	}

	std::string file_data = file.read().value();

	file.close();

	panther::Source::ID test_file_id = source_manager.addSource(file_path, std::move(file_data));


	source_manager.lock();



	//////////////////////////////////////////////////////////////////////
	// frontend

	#if defined(PANTHER_BUILD_DEBUG)
		printer.trace("source manager locked\n");
	#endif

	if(config.verbose){
		if(source_manager.numSources() > 1){
			printer.trace( std::format("Compiling {} files\n", source_manager.numSources()) );
		}else{
			printer.trace( std::format("Compiling 1 file\n") );
		}
	}
	

	///////////////////////////////////
	// tokenize

	const evo::uint tokenizing_successful = source_manager.tokenize();

	if(tokenizing_successful > 0){
		printer.error( std::format("Failed to tokenize {} / {} files\n", tokenizing_successful, source_manager.numSources()) );
		exit();
		return 1;
	}


	if(config.verbose){
		printer.success("Successfully Tokenized all files\n");
	}


	if(config.output == Config::Output::PrintTokens){
		printer.trace("------------------------------\n");
		printer.print_tokens(source_manager.getSource(test_file_id));

		exit();
		return 0;
	}


	
	///////////////////////////////////
	// parse

	const evo::uint parsing_successful = source_manager.parse();

	if(parsing_successful > 0){
		printer.error( std::format("Failed to parse {} / {} files\n", parsing_successful, source_manager.numSources()) );

		exit();
		return 1;
	}


	if(config.verbose){
		printer.success("Successfully Parsed all files\n");
	}


	if(config.output == Config::Output::PrintAST){
		if(config.verbose){ printer.trace("------------------------------\n"); }
		printer.print_ast(source_manager.getSource(test_file_id));

		exit();
		return 0;
	}



	//////////////////////////////////////////////////////////////////////
	// done

	exit();

	return 0;
}