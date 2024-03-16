
#include "./Printer.h"
#include "frontend/SourceManager.h"
#include "middleend/Context.h"
#include "ObjectsToIR.h"


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
	std::string name;
	bool print_colors;
	bool verbose;

	enum class Target{
		PrintTokens,
		PrintAST,
		SemanticAnalysis,
		PrintLLVMIR,
		LLVMIR,
		Object,
		Run,
	} target;
	std::filesystem::path output_path{};

	std::filesystem::path relative_directory{};
	bool relative_directory_set = false;
};


auto main([[maybe_unused]] int argc, [[maybe_unused]] const char* args[]) noexcept -> int {


	// print UTF-8 characters on windows
	#if defined(EVO_PLATFORM_WINDOWS)
		::SetConsoleOutputCP(CP_UTF8);
	#endif



	auto config = Config{
		.name		  = "testing",
		.print_colors = true,
		.verbose      = true,
		.target       = Config::Target::PrintLLVMIR,
	};



	auto printer = panther::cli::Printer(config.print_colors);

	auto llvm_context = panther::llvmint::Context();


	auto exit = [&](){
		if(llvm_context.isInitialized()){
			llvm_context.shutdown();
		}

		printer.trace("Press Enter to close...\n");
		std::cin.get();
	};


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

	config.output_path = config.relative_directory / "testing/output.o";






	if(config.verbose){
		printer.info("Panther Compiler\n");
		printer.trace("----------------\n");
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
		printer.error( std::format("Tokenizing failed for {} / {} files\n", tokenizing_successful, source_manager.numSources()) );
		exit();
		return 1;
	}


	if(config.verbose){
		printer.success("Successfully Tokenized all files\n");
	}


	if(config.target == Config::Target::PrintTokens){
		printer.trace("------------------------------\n");
		printer.print_tokens(source_manager.getSource(test_file_id));

		exit();
		return 0;
	}


	
	///////////////////////////////////
	// parse

	const evo::uint parsing_successful = source_manager.parse();

	if(parsing_successful > 0){
		printer.error( std::format("Parsing failed for {} / {} files\n", parsing_successful, source_manager.numSources()) );

		exit();
		return 1;
	}


	if(config.verbose){
		printer.success("Successfully Parsed all files\n");
	}


	if(config.target == Config::Target::PrintAST){
		if(config.verbose){ printer.trace("------------------------------\n"); }
		printer.print_ast(source_manager.getSource(test_file_id));

		exit();
		return 0;
	}


	///////////////////////////////////
	// semantic analysis

	source_manager.initBuiltinTypes();


	const evo::uint semantic_analysis_successful = source_manager.semanticAnalysis();

	if(semantic_analysis_successful > 0){
		printer.error( std::format("Semantic Analysis failed for {} / {} files\n", semantic_analysis_successful, source_manager.numSources()) );

		exit();
		return 1;
	}


	if(config.verbose){
		printer.success("Semantic Analysis succeeded for all files\n");
	}


	if(config.target == Config::Target::SemanticAnalysis){
		// Do nothing...

		exit();
		return 0;
	}



	//////////////////////////////////////////////////////////////////////
	// lowering to IR

	llvm_context.init();


	auto objects_to_ir = panther::ObjectsToIR();
	objects_to_ir.init(config.name, llvm_context);

	objects_to_ir.lower(source_manager);


	if(config.verbose){
		printer.success("Lowered to LLVM IR\n");
	}



	if(config.target == Config::Target::PrintLLVMIR){
		if(config.verbose){ printer.trace("------------------------------\n"); }
		printer.info(objects_to_ir.printLLVMIR());

		objects_to_ir.shutdown();

		exit();
		return 0;

	}else if(config.target == Config::Target::LLVMIR){
		const std::string output = objects_to_ir.printLLVMIR();
		objects_to_ir.shutdown();


		const std::string path_str = config.output_path.string();

		auto output_file = evo::fs::File();
		if(output_file.open(path_str, evo::fs::FileMode::Write) == false){
			printer.error( std::format("Failed to open file: \"{}\"\n", path_str) );
			exit();
			return 1;
		}

		if(output_file.write(output) == false){
			printer.error( std::format("Failed to write to file: \"{}\"\n", path_str) );
			exit();
			return 1;
		}

		output_file.close();




		if(config.verbose){
			printer.success( std::format("Successfully wrote output to: \"{}\"\n", path_str) );
		}

		exit();
		return 0;

	}else if(config.target == Config::Target::Object){
		const std::optional< std::vector<evo::byte> > output = objects_to_ir.compileToObjectFile();
		if(output.has_value() == false){
			printer.fatal("Target machine cannot output object file");
			exit();
			return 1;
		}

		objects_to_ir.shutdown();

		const std::string path_str = config.output_path.string();

		auto output_file = evo::fs::BinaryFile();
		if(output_file.open(path_str, evo::fs::FileMode::Write) == false){
			printer.error( std::format("Failed to open file: \"{}\"\n", path_str) );
			exit();
			return 1;
		}

		if(output_file.write(*output) == false){
			printer.error( std::format("Failed to write to file: \"{}\"\n", path_str) );
			exit();
			return 1;
		}

		output_file.close();



		if(config.verbose){
			printer.success( std::format("Successfully wrote output to: \"{}\"\n", path_str) );
		}

		exit();
		return 0;

	}else if(config.target == Config::Target::Run){
		if(config.verbose){ printer.trace("------------------------------\nRunning:\n"); }

		uint64_t return_code = objects_to_ir.run<uint64_t>("main");

		// if(config.verbose){ printer.trace("------------------------------\n"); }

		printer.info(std::format("Return Code: {}\n", return_code));

		objects_to_ir.shutdown();


		exit();
		return 0;		
	}



	//////////////////////////////////////////////////////////////////////
	// done

	objects_to_ir.shutdown();
	

	exit();

	return 0;
}