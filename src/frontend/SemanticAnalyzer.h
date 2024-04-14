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
			EVO_NODISCARD auto analyze_func_block(PIR::Func& pir_func, const AST::Func& ast_func) noexcept -> bool;
			EVO_NODISCARD auto analyze_conditional(const AST::Conditional& cond) noexcept -> bool;
			EVO_NODISCARD auto analyze_conditional_recursive(const AST::Conditional& cond) noexcept -> bool;
			EVO_NODISCARD auto analyze_return(const AST::Return& return_stmt) noexcept -> bool;
			EVO_NODISCARD auto analyze_infix(const AST::Infix& infix) noexcept -> bool;
			EVO_NODISCARD auto analyze_func_call(const AST::FuncCall& func_call) noexcept -> bool;
			EVO_NODISCARD auto analyze_unreachable(const Token& unreachable) noexcept -> bool;
			EVO_NODISCARD auto analyze_assignment(const AST::Infix& infix) noexcept -> bool;			
			EVO_NODISCARD auto analyze_block(const AST::Block& block, PIR::StmtBlock& stmts_entry) noexcept -> bool;

			// returns nullopt if error occurs
			EVO_NODISCARD auto analyze_and_get_type_of_expr(const AST::Node& node) const noexcept -> std::optional<PIR::Type::ID>;

			// Not for use in global variables
			EVO_NODISCARD auto get_expr_value(AST::Node::ID node_id) const noexcept -> PIR::Expr;

			EVO_NODISCARD auto get_const_expr_value(AST::Node::ID node_id) const noexcept -> std::optional<PIR::Expr>;
			EVO_NODISCARD auto get_const_expr_value_recursive(AST::Node::ID node_id) const noexcept -> std::optional<PIR::Expr>;


			enum class ExprValueType{
				Concrete,
				Ephemeral,
			};
			EVO_NODISCARD auto get_expr_value_type(AST::Node::ID node_id) const noexcept -> ExprValueType;

			// must be given a concrete value
			EVO_NODISCARD auto is_expr_mutable(AST::Node::ID node_id) const noexcept -> bool;


			EVO_NODISCARD auto is_valid_export_name(std::string_view name) const noexcept -> bool;


			auto already_defined(const Token& ident) const noexcept -> void;

			///////////////////////////////////
			// scope

			auto enter_scope(PIR::StmtBlock* stmts_entry) noexcept -> void;
			auto leave_scope() noexcept -> void;

			auto add_var_to_scope(std::string_view str, PIR::Var::ID id) noexcept -> void;
			auto add_func_to_scope(std::string_view str, PIR::Func::ID id) noexcept -> void;

			auto set_scope_terminated() noexcept -> void;
			EVO_NODISCARD auto scope_is_terminated() const noexcept -> bool;

			EVO_NODISCARD auto get_stmts_entry() noexcept -> PIR::StmtBlock&;
			EVO_NODISCARD auto has_in_scope(std::string_view ident) const noexcept -> bool;
			EVO_NODISCARD inline auto is_global_scope() const noexcept -> bool { return this->scopes.size() == 1; };

			EVO_NODISCARD auto is_in_func_base_scope() const noexcept -> bool;


			///////////////////////////////////
			// func scope

			struct FuncScope{
				PIR::Func& func;
			};

			EVO_NODISCARD auto in_func_scope() const noexcept -> bool;
			auto enter_func_scope(PIR::Func& func) noexcept -> void;
			auto leave_func_scope() noexcept -> void;
			EVO_NODISCARD auto get_current_func() noexcept -> FuncScope&;
			EVO_NODISCARD auto get_current_func() const noexcept -> const FuncScope&;

			// auto set_current_func_terminated() noexcept -> void;
			// EVO_NODISCARD auto current_func_is_terminated() const noexcept -> bool;


			///////////////////////////////////
			// scope level

			auto enter_scope_level() noexcept -> void;
			auto leave_scope_level() noexcept -> void;

			auto add_scope_level_scope() noexcept -> void;
			auto add_scope_level_terminated() noexcept -> void;


		private:
			Source& source;


			struct Scope{
				PIR::StmtBlock* stmts_entry;

				std::unordered_map<std::string_view, PIR::Var::ID> vars{};
				std::unordered_map<std::string_view, PIR::Func::ID> funcs{};

				// all control paths return or are unreachable
				bool is_terminated = false;
			};
			std::vector<Scope> scopes{};

			
			std::vector<FuncScope> func_scopes{};


			struct ScopeLevel{
				evo::uint num_scopes = 0;
				evo::uint num_terminated = 0;
			};
			std::vector<ScopeLevel> scope_levels{};
	};


};