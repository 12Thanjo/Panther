#pragma once


#include <Evo.h>


#include "frontend/Source.h"
#include "frontend/AST.h"
#include "frontend/SourceManager.h"

#include <unordered_map>

namespace panther{


	class SemanticAnalyzer{
		public:
			SemanticAnalyzer(Source& src) : source(src), src_manager(src.getSourceManager()) {};
			~SemanticAnalyzer() = default;


			EVO_NODISCARD auto semantic_analysis_declarations() noexcept -> bool;
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
			
			EVO_NODISCARD auto analyze_block(const AST::Block& block, PIR::StmtBlock& stmts_entry) noexcept -> bool; // enters a new scope
			EVO_NODISCARD auto analyze_block(const AST::Block& block) noexcept -> bool;

			EVO_NODISCARD auto check_func_call(const AST::FuncCall& func_call, PIR::Type::ID type_id) const noexcept -> bool;
			EVO_NODISCARD auto get_func_call_args(const AST::FuncCall& func_call) const noexcept -> std::vector<PIR::Expr>;

			// note: func_call is only used when looking up a function overload
			EVO_NODISCARD auto analyze_and_get_type_of_expr(const AST::Node& node, const AST::FuncCall* lookup_func_call = nullptr) const noexcept
				-> evo::Result<PIR::Type::ID>;

			EVO_NODISCARD auto get_type_id(AST::Node::ID node_id) const noexcept -> evo::Result<PIR::Type::VoidableID>;

			EVO_NODISCARD auto is_implicitly_convertable_to(const PIR::Type& from, const PIR::Type& to, const AST::Node& from_expr) const noexcept -> bool;


			// Not for use in global variables
			EVO_NODISCARD auto get_expr_value(AST::Node::ID node_id) const noexcept -> evo::Result<PIR::Expr>;

			EVO_NODISCARD auto get_const_expr_value(AST::Node::ID node_id) const noexcept -> evo::Result<PIR::Expr>;
			EVO_NODISCARD auto get_const_expr_value_recursive(AST::Node::ID node_id) const noexcept -> evo::Result<PIR::Expr>;

			EVO_NODISCARD auto get_import_source_id(const PIR::Expr& import_path, AST::Node::ID expr_node) const noexcept -> evo::Result<Source::ID>;


			enum class ExprValueType{
				Concrete,
				Ephemeral,
				Import,
			};
			EVO_NODISCARD auto get_expr_value_type(AST::Node::ID node_id) const noexcept -> evo::Result<ExprValueType>;

			// must be given a concrete value
			EVO_NODISCARD auto is_expr_mutable(AST::Node::ID node_id) const noexcept -> bool;


			EVO_NODISCARD auto is_valid_export_name(std::string_view name) const noexcept -> bool;


			auto already_defined(const Token& ident) const noexcept -> void;



			///////////////////////////////////
			// scope

			struct Import{
				Source::ID source_id;
				AST::Node::ID ident;
			};

			auto enter_scope(PIR::StmtBlock* stmts_entry) noexcept -> void;
			auto leave_scope() noexcept -> void;

			auto add_var_to_scope(std::string_view str, PIR::Var::ID id) noexcept -> void;
			auto add_func_to_scope(std::string_view str, PIR::Func::ID id) noexcept -> void;
			auto add_param_to_scope(std::string_view str, PIR::Param::ID id) noexcept -> void;
			auto add_import_to_scope(std::string_view str, Import import) noexcept -> void;

			auto set_scope_terminated() noexcept -> void;
			EVO_NODISCARD auto scope_is_terminated() const noexcept -> bool;

			EVO_NODISCARD auto get_stmts_entry() noexcept -> PIR::StmtBlock&;
			EVO_NODISCARD auto has_in_scope(std::string_view ident) const noexcept -> bool;
			EVO_NODISCARD inline auto is_global_scope() const noexcept -> bool { return this->scopes.size() == 1; };

			EVO_NODISCARD auto is_in_func_base_scope() const noexcept -> bool;


			EVO_NODISCARD auto lookup_func_in_scope(std::string_view ident, const AST::FuncCall& func_call) const noexcept -> evo::Result<PIR::Func::ID>;
			EVO_NODISCARD auto lookup_func_in_import(std::string_view ident, const Source& import, const AST::FuncCall& func_call) const noexcept
			-> evo::Result<PIR::Func::ID>;
			EVO_NODISCARD auto match_function_to_overloads(
				std::string_view ident, const AST::FuncCall& func_call, evo::ArrayProxy<PIR::Func::ID> overload_list
			) const noexcept -> evo::Result<PIR::Func::ID>;


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
			SourceManager& src_manager;

			struct Scope{
				PIR::StmtBlock* stmts_entry;

				std::unordered_map<std::string_view, PIR::Var::ID> vars{};
				std::unordered_map<std::string_view, std::vector<PIR::Func::ID>> funcs{};
				std::unordered_map<std::string_view, PIR::Param::ID> params{};
				std::unordered_map<std::string_view, Import> imports{};


				// all control paths return or are unreachable
				bool is_terminated = false;
			};
			std::vector<Scope> scopes{};

			struct GlobalFunc{
				PIR::Func::ID pir_id;
				const AST::Func& ast;
			};
			std::vector<GlobalFunc> global_funcs{};

			
			std::vector<FuncScope> func_scopes{};


			struct ScopeLevel{
				evo::uint num_scopes = 0;
				evo::uint num_terminated = 0;
			};
			std::vector<ScopeLevel> scope_levels{};
	};


};