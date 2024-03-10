#pragma once


#include <Evo.h>


#include "Source.h"
#include "AST.h"

#include <unordered_set>

namespace panther{


	class SemanticAnalyzer{
		public:
			SemanticAnalyzer(Source& src) : source(src) {};
			~SemanticAnalyzer() = default;


			EVO_NODISCARD auto semantic_analysis() noexcept -> bool;


		private:
			EVO_NODISCARD auto analyize_global_var(const AST::VarDecl& var_decl) noexcept -> bool;

			std::unordered_set<std::string_view> objects{};

	
		private:
			Source& source;
	};


};