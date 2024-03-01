
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

	std::filesystem::path relative_directory{};
	bool relative_directory_set = false;
};


auto main([[maybe_unused]] int argc, [[maybe_unused]] const char* args[]) noexcept -> int {
	auto config = Config{
		.print_colors = true,
		.verbose = false,
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

	auto fail_exit = [&](){
		exit();
		std::exit(1);
	};




	if(config.verbose){
		printer.info("Panther Compiler\n");
	}



	if(config.relative_directory_set == false){
		std::error_code ec;
		config.relative_directory = std::filesystem::current_path(ec);
		if(ec){
			printer.error("Failed to get relative directory\n");
			printer.error(std::format("\tcode: \"{}\"\n", ec.value()));
			printer.error(std::format("\tmessage: \"{}\"\n", ec.message()));

			fail_exit();
		}
	}


	if(config.verbose){
		printer.debug( std::format("Relative Directory: {}\n", config.relative_directory.string()) );
	}


	auto source_manager = panther::SourceManager([&](const panther::Message& message){
		printer.print_message(message);
	});


	///////////////////////////////////
	// get code

	const std::string file_path = (config.relative_directory / "testing/test.pthr").make_preferred().string();

	auto file = evo::fs::File{};
	{
		const bool opened_successfully = file.open(file_path, evo::fs::FileMode::Read);
		if(opened_successfully == false){
			printer.error(std::format("Failed to open file: {}\n", file_path));
			fail_exit();
		}
	}

	std::string file_data = file.read().value();

	file.close();

	panther::Source::ID test_file = source_manager.addSource(file_path, std::move(file_data));


	source_manager.lock();



	///////////////////////////////////
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
	

	const evo::uint tokenizing_successful = source_manager.tokenize();

	if(tokenizing_successful > 0){
		// panther::cli::print_messages(source_manager, printer);

		printer.error( std::format("Failed to tokenize {} / {} files\n", tokenizing_successful, source_manager.numSources()) );
		fail_exit();
	}


	printer.success("Successfully Tokenized all files\n");

	



	///////////////////////////////////
	// done

	exit();

	return 0;
}