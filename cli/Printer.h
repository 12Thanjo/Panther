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

				auto fatal(evo::CStrProxy msg) const noexcept -> void;
				auto error(evo::CStrProxy msg) const noexcept -> void;
				auto warning(evo::CStrProxy msg) const noexcept -> void;
				auto success(evo::CStrProxy msg) const noexcept -> void;
				auto info(evo::CStrProxy msg) const noexcept -> void;
				auto debug(evo::CStrProxy msg) const noexcept -> void;
				auto trace(evo::CStrProxy msg) const noexcept -> void;

				auto print(evo::CStrProxy msg) const noexcept -> void;


				EVO_NODISCARD inline auto usesColors() const noexcept -> bool { return this->use_colors; };


				auto print_message(const panther::Message& msg) const noexcept -> void;
				
				auto print_tokens(const Source& source) const noexcept -> void;

				auto print_ast(const Source& source) noexcept -> void;


			private:
				auto print_location(const Source& source, Location location, Message::Type type) const noexcept -> void;


				///////////////////////////////////
				// ast

				auto print_stmt(const Source& source, const AST::Node& node_id) noexcept -> void;
				auto print_var_decl(const Source& source, const AST::Node& node_id) noexcept -> void;
				auto print_func(const Source& source, const AST::Node& node_id) noexcept -> void;
				auto print_conditional(const Source& source, const AST::Node& node_id) noexcept -> void;
				auto print_return(const Source& source, const AST::Node& node_id) noexcept -> void;

				auto print_infix(const Source& source, const AST::Node& node_id) noexcept -> void;
				
				auto print_type(const Source& source, const AST::Node& node_id) noexcept -> void;
				auto print_block(const Source& source, const AST::Node& node_id) noexcept -> void;

				auto print_expr(const Source& source, const AST::Node& node_id) noexcept -> void;

				auto print_literal(const Source& source, const AST::Node& node_id) noexcept -> void;


				///////////////////////////////////
				// indenter
				auto indenter_push() noexcept -> void;
				auto indenter_pop() noexcept -> void;

				auto indenter_set_arrow() noexcept -> void;
				auto indenter_set_end() noexcept -> void;	

				auto indenter_print() noexcept -> void;

				enum class IndenterType{
					Line,
					Arrow,
					EndArrow,
					None,
				};

				std::vector<IndenterType> indents{};

			private:
				bool use_colors;
		};
		

	};
};