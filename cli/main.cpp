
#include "./Printer.h"
#include "frontend/SourceManager.h"
#include "LLVM_interface/Context.h"
#include "LLD_interface/LLDInterface.h"
#include "PIRToLLVMIR.h"


#include <Evo.h>
#include <iostream>
#include <filesystem>



#if !defined(WIN32_LEAN_AND_MEAN)
	#define WIN32_LEAN_AND_MEAN
#endif

#if !defined(NOCOMM)
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
		Executable,
		Run,
	} target;
	std::filesystem::path output_path{};

	std::filesystem::path relative_directory{};
	bool relative_directory_set = false;
};


auto main([[maybe_unused]] int argc, [[maybe_unused]] const char* args[]) noexcept -> int {
	auto config = Config{
		.name		  = "testing",
		.print_colors = true,
		.verbose      = true,
		.target       = Config::Target::PrintLLVMIR,
	};


	// print UTF-8 characters on windows
	#if defined(EVO_PLATFORM_WINDOWS)
		::SetConsoleOutputCP(CP_UTF8);
	#endif
	

	auto printer = panther::cli::Printer(config.print_colors);

	auto llvm_context = panther::llvmint::Context();
	auto pir_to_llvmir = panther::PIRToLLVMIR();


	auto exit = [&](){
		if(pir_to_llvmir.isInitialized()){
			pir_to_llvmir.shutdown();
		}

		if(llvm_context.isInitialized()){
			llvm_context.shutdown();
		}

		#if !defined(PANTHER_BUILD_DIST) && defined(EVO_COMPILER_MSVC)
			printer.trace("Press Enter to close...\n");
			std::cin.get();
		#endif
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



	if(config.output_path.empty()){
		const char* file_ext = [&]() noexcept {
			switch(config.target){
				break; case Config::Target::LLVMIR: return "ll";
				break; case Config::Target::Object: return "o";

				#if defined(EVO_PLATFORM_WINDOWS)
					break; case Config::Target::Executable: return "exe";
				#elif defined(EVO_PLATFORM_LINUX)
					break; case Config::Target::Executable: return "out";
				#endif

				// I'm not entirely sure why I need this static cast here, but MSVC complains
				//  (I haven't bothered checking other compilers)
				break; default: return static_cast<const char*>(nullptr);
			};
		}();

		if(file_ext != nullptr){
			config.output_path = config.relative_directory / std::format("{}.{}", config.name, file_ext);
		}
	}






	if(config.verbose){
		printer.info("Panther Compiler\n");
		printer.trace("----------------\n");

		printer.debug( std::format("Relative Directory: {}\n", config.relative_directory.string()) );

		switch(config.target){
			break; case Config::Target::PrintTokens:      printer.debug("Target: PrintTokens\n");
			break; case Config::Target::PrintAST:         printer.debug("Target: PrintAST\n");
			break; case Config::Target::SemanticAnalysis: printer.debug("Target: SemanticAnalysis\n");
			break; case Config::Target::PrintLLVMIR:      printer.debug("Target: PrintLLVMIR\n");
			break; case Config::Target::LLVMIR:           printer.debug("Target: LLVMIR\n");
			break; case Config::Target::Object:           printer.debug("Target: Object\n");
			break; case Config::Target::Executable:       printer.debug("Target: Executable\n");
			break; case Config::Target::Run:              printer.debug("Target: Run\n");
			break; default: EVO_FATAL_BREAK("Unknown target");
		};
	}


	auto source_manager = panther::SourceManager([&](const panther::Message& message){
		printer.print_message(message);
	});




	//////////////////////////////////////////////////////////////////////
	// get code


	auto file_paths = std::vector<std::string>{
		(config.relative_directory / "test.pthr").make_preferred().string(),
		// (config.relative_directory / "test2.pthr").make_preferred().string(),
	};

	auto source_ids = std::vector<panther::Source::ID>();



	auto file = evo::fs::File{};

	for(const std::string& file_path : file_paths){
		const bool opened_successfully = file.open(file_path, evo::fs::FileMode::Read);
		if(opened_successfully == false){
			printer.error(std::format("Failed to open file: {}\n", file_path));
			exit();
			return 1;
		}

		std::string file_data = file.read().value();

		file.close();

		source_ids.emplace_back(source_manager.addSource(file_path, std::move(file_data)));
	}



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
		for(panther::Source::ID source_id : source_ids){
			printer.trace("------------------------------\n");
			printer.print_tokens(source_manager.getSource(source_id));
		}

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
		for(panther::Source::ID source_id : source_ids){
			if(config.verbose){ printer.trace("------------------------------\n"); }
			printer.print_ast(source_manager.getSource(source_id));
		}

		exit();
		return 0;
	}


	///////////////////////////////////
	// semantic analysis

	source_manager.initBuiltinTypes();
	source_manager.initIntrinsics();


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


	pir_to_llvmir.init(config.name, llvm_context);
	pir_to_llvmir.initLibC();

	pir_to_llvmir.lower(source_manager);


	if(config.verbose){ printer.success("Lowered to LLVM IR\n"); }


	if(source_manager.hasEntry()){
		pir_to_llvmir.addRuntime(source_manager, source_manager.getEntry());

		if(config.verbose){ printer.success("Added Panther runtime to LLVM IR\n"); }
	}




	if(config.target == Config::Target::PrintLLVMIR){
		if(config.verbose){ printer.trace("------------------------------\n"); }
		printer.info(pir_to_llvmir.printLLVMIR());

		pir_to_llvmir.shutdown();

		exit();
		return 0;

	}else if(config.target == Config::Target::LLVMIR){
		const std::string output = pir_to_llvmir.printLLVMIR();
		pir_to_llvmir.shutdown();


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
		const std::optional< std::vector<evo::byte> > output = pir_to_llvmir.compileToObjectFile();
		if(output.has_value() == false){
			printer.fatal("Target machine cannot output object file");
			exit();
			return 1;
		}

		pir_to_llvmir.shutdown();

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

	}


	if(source_manager.hasEntry() == false){
		printer.error("Error: Cannot run because no entry point was defined\n");
		printer.info("\tNote: an entry point is defined by giving a function the attribute \"#entry\"\n");

		exit();
		return 1;
	}


	if(config.target == Config::Target::Run){
		if(config.verbose){ printer.trace("------------------------------\nRunning:\n"); }

		const uint64_t return_code = pir_to_llvmir.run<uint64_t>("main");

		// if(config.verbose){ printer.trace("------------------------------\n"); }

		printer.info(std::format("Return Code: {}\n", return_code));

		exit();
		return 0;

	}else if(config.target == Config::Target::Executable){

		///////////////////////////////////
		// create object file

		const std::optional< std::vector<evo::byte> > output = pir_to_llvmir.compileToObjectFile();
		if(output.has_value() == false){
			printer.fatal("Target machine cannot output object file");
			exit();
			return 1;
		}

		pir_to_llvmir.shutdown();


		///////////////////////////////////
		// write object file

		// TODO: better output path for obj_path_str
		const std::string obj_path_str = (config.relative_directory / (config.name + ".o")).string();
		const std::string path_str = config.output_path.string();


		auto output_file = evo::fs::BinaryFile();
		if(output_file.open(obj_path_str, evo::fs::FileMode::Write) == false){
			printer.error( std::format("Failed to open file: \"{}\"\n", obj_path_str) );
			exit();
			return 1;
		}

		if(output_file.write(*output) == false){
			printer.error( std::format("Failed to write to file: \"{}\"\n", obj_path_str) );
			exit();
			return 1;
		}

		output_file.close();

		if(config.verbose){
			printer.success( std::format("Successfully wrote object file to: \"{}\"\n", obj_path_str) );
		}


		///////////////////////////////////
		// link / create executable

		auto lld_interface = panther::LLDInterface();

		const panther::LLDInterface::LinkerOutput linking_result = lld_interface.link(obj_path_str, path_str, panther::LLDInterface::Linker::WinLink);

		if(linking_result.succeeded() == false){
			for(const std::string& link_err_msg : linking_result.err_messages){
				// TODO: nicer printint (note giving a path_str that has a non-existant path will give an error)
				printer.error(std::format("Error: Linker: {}", link_err_msg));
			}

			printer.error("Error: Failed to link executable\n");

			exit();
			return 1;	
		}

		if(config.verbose){
			printer.success( std::format("Successfully wrote output to: \"{}\"\n", path_str) );
		}
		
		exit();
		return 0;
	}



	//////////////////////////////////////////////////////////////////////
	// done


	exit();
	return 0;
}