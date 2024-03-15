#pragma once


#include <Evo.h>


#include "frontend/Source.h"
#include "frontend/AST.h"
#include "frontend/SourceManager.h"

#include <unordered_map>

namespace panther{


	class SemanticAnalyzer{
		public:
			SemanticAnalyzer(Source& src) : source(src) {};
			~SemanticAnalyzer() = default;


			EVO_NODISCARD auto semantic_analysis() noexcept -> bool;


		private:
			EVO_NODISCARD auto analyze_stmt(const AST::Node& node) noexcept -> bool;
			EVO_NODISCARD auto analyze_var(const AST::VarDecl& var_decl) noexcept -> bool;
			EVO_NODISCARD auto analyze_func(const AST::Func& func) noexcept -> bool;
			EVO_NODISCARD auto analyze_block(const AST::Block& block) noexcept -> bool;

			// also analyzes the expr
			// returns nullopt if error occurs
			EVO_NODISCARD auto get_type_of_expr(const AST::Node& node) const noexcept -> std::optional<object::Type::ID>;


			struct Scope{
				std::unordered_map<std::string_view, object::Var::ID> vars{};
				std::unordered_map<std::string_view, object::Func::ID> funcs{};

			};

			std::vector<Scope> scopes{};
			object::Func* current_func = nullptr;


			auto enter_scope() noexcept -> void;
			auto leave_scope() noexcept -> void;

			auto add_var_to_scope(std::string_view str, object::Var::ID id) noexcept -> void;
			auto add_func_to_scope(std::string_view str, object::Func::ID id) noexcept -> void;

			EVO_NODISCARD inline auto is_global_scope() const noexcept -> bool { return this->scopes.size() == 1; };



			auto has_in_scope(std::string_view ident) const noexcept -> bool;

			auto already_defined(const Token& ident) const noexcept -> void;




	
		private:
			Source& source;
	};


};