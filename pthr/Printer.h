#pragma once


#include <Evo.h>


#include "frontend/Source.h"

namespace panther{
	struct Message;


	namespace cli{


		class Printer{
			public:
				Printer(bool should_use_colors) : use_colors(should_use_colors) {};
				~Printer() = default;

				auto fatal(std::string_view msg) const noexcept -> void;
				auto error(std::string_view msg) const noexcept -> void;
				auto warning(std::string_view msg) const noexcept -> void;
				auto success(std::string_view msg) const noexcept -> void;
				auto info(std::string_view msg) const noexcept -> void;
				auto debug(std::string_view msg) const noexcept -> void;
				auto trace(std::string_view msg) const noexcept -> void;

				auto print(std::string_view msg) const noexcept -> void;


				EVO_NODISCARD inline auto usesColors() const noexcept -> bool { return this->use_colors; };


				auto print_message(const panther::Message& msg) const noexcept -> void;
				
				auto print_tokens(const Source& source) const noexcept -> void;

				auto print_ast(const Source& source) noexcept -> void;


			private:
				auto print_location(const Source& source, Location location, Message::Type type) const noexcept -> void;


				///////////////////////////////////
				// ast

				auto print_stmt(const AST::Node& node) noexcept -> void;
				auto print_var_decl(const AST::Node& node) noexcept -> void;
				auto print_func(const AST::Node& node) noexcept -> void;
				auto print_struct(const AST::Node& node) noexcept -> void;
				auto print_template_pack(const std::optional<AST::Node::ID>& opt_node_id) noexcept -> void;
				auto print_template_args(const std::vector<AST::Node::ID>& template_args) noexcept -> void;
				auto print_func_params(const AST::Node& node) noexcept -> void;
				auto print_conditional(const AST::Node& node) noexcept -> void;
				auto print_return(const AST::Node& node) noexcept -> void;
				auto print_alias(const AST::Node& node) noexcept -> void;

				auto print_infix(const AST::Node& node) noexcept -> void;
				
				auto print_type(const AST::Node& node) noexcept -> void;
				auto print_block(const AST::Node& node) noexcept -> void;

				auto print_expr(const AST::Node& node) noexcept -> void;

				auto print_ident(const AST::Node& node) noexcept -> void;
				auto print_literal(const AST::Node& node) noexcept -> void;


				///////////////////////////////////
				// indenter
				auto indenter_push() noexcept -> void;
				auto indenter_pop() noexcept -> void;

				auto indenter_set_arrow() noexcept -> void;
				auto indenter_set_end() noexcept -> void;	

				auto indenter_print() noexcept -> void;
				auto indenter_print_arrow() noexcept -> void;
				auto indenter_print_end() noexcept -> void;

				enum class IndenterType{
					Line,
					Arrow,
					EndArrow,
					None,
				};

				std::vector<IndenterType> indents{};

			private:
				bool use_colors;
				const Source* ast_source = nullptr;
		};
		

	};
};