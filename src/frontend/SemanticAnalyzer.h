#pragma once


#include <Evo.h>


#include "Source.h"
#include "AST.h"
#include "SourceManager.h"

#include <unordered_map>

namespace panther{


	class SemanticAnalyzer{
		public:
			SemanticAnalyzer(Source& src) : source(src) {};
			~SemanticAnalyzer() = default;


			EVO_NODISCARD auto semantic_analysis() noexcept -> bool;


		private:
			EVO_NODISCARD auto analyize_global_var(const AST::VarDecl& var_decl) noexcept -> bool;

			std::unordered_map<std::string_view, SourceManager::GlobalVarID> global_vars{};

	
		private:
			Source& source;
	};


};