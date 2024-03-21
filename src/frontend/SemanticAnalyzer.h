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
			EVO_NODISCARD auto analyze_return(const AST::Return& return_stmt) noexcept -> bool;
			EVO_NODISCARD auto analyze_infix(const AST::Infix& infix) noexcept -> bool;
			EVO_NODISCARD auto analyze_func_call(const AST::FuncCall& func_call) noexcept -> bool;
			EVO_NODISCARD auto analyze_assignment(const AST::Infix& infix) noexcept -> bool;
			EVO_NODISCARD auto analyze_block(const AST::Block& block) noexcept -> bool;

			// returns nullopt if error occurs
			EVO_NODISCARD auto analyze_and_get_type_of_expr(const AST::Node& node) const noexcept -> std::optional<PIR::Type::ID>;

			// Not for use in global variables
			EVO_NODISCARD auto get_expr_value(AST::Node::ID node_id) const noexcept -> PIR::Expr;

			enum class ExprValueType{
				Concrete,
				Ephemeral,
			};
			EVO_NODISCARD auto get_expr_value_type(AST::Node::ID node_id) const noexcept -> ExprValueType;


			EVO_NODISCARD auto is_valid_export_name(std::string_view name) const noexcept -> bool;


			struct Scope{
				std::unordered_map<std::string_view, PIR::Var::ID> vars{};
				std::unordered_map<std::string_view, PIR::Func::ID> funcs{};
			};

			std::vector<Scope> scopes{};
			PIR::Func* current_func = nullptr;


			auto enter_scope() noexcept -> void;
			auto leave_scope() noexcept -> void;

			auto add_var_to_scope(std::string_view str, PIR::Var::ID id) noexcept -> void;
			auto add_func_to_scope(std::string_view str, PIR::Func::ID id) noexcept -> void;

			EVO_NODISCARD inline auto is_global_scope() const noexcept -> bool { return this->scopes.size() == 1; };



			auto has_in_scope(std::string_view ident) const noexcept -> bool;

			auto already_defined(const Token& ident) const noexcept -> void;




	
		private:
			Source& source;
	};


};