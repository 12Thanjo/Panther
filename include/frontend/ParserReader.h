#pragma once


#include <Evo.h>


#include "./Parser.h"
#include "./Printer.h"


namespace panther{



	class ParserReader{
		public:
			ParserReader(Parser& _parser, const Printer& _printer) : parser(_parser), printer(_printer) {};
			~ParserReader() = default;


			EVO_NODISCARD auto getNodeKind(AST::NodeID id) const noexcept -> AST::Kind;
			EVO_NODISCARD auto getIdentName(AST::NodeID id) const noexcept -> const std::string&;
			EVO_NODISCARD auto getAttributeName(TokenID id) const noexcept -> const std::string&;

			EVO_NODISCARD auto getVarDecl(AST::NodeID id) noexcept -> AST::VarDecl&;
			EVO_NODISCARD auto getVarDecl(AST::NodeID id) const noexcept -> const AST::VarDecl&;

			EVO_NODISCARD auto getFuncDef(AST::NodeID id) noexcept -> AST::FuncDef&;
			EVO_NODISCARD auto getFuncDef(AST::NodeID id) const noexcept -> const AST::FuncDef&;

			EVO_NODISCARD auto getFuncParams(AST::NodeID id) noexcept -> AST::FuncParams&;
			EVO_NODISCARD auto getFuncParams(AST::NodeID id) const noexcept -> const AST::FuncParams&;

			EVO_NODISCARD auto getBlock(AST::NodeID id) noexcept -> AST::Block&;
			EVO_NODISCARD auto getBlock(AST::NodeID id) const noexcept -> const AST::Block&;

			EVO_NODISCARD auto getAttributes(AST::NodeID id) noexcept -> AST::Attributes&;
			EVO_NODISCARD auto getAttributes(AST::NodeID id) const noexcept -> const AST::Attributes&;



			EVO_NODISCARD auto getPrefix(AST::NodeID id) noexcept -> AST::Prefix&;
			EVO_NODISCARD auto getPrefix(AST::NodeID id) const noexcept -> const AST::Prefix&;

			EVO_NODISCARD auto getInfix(AST::NodeID id) noexcept -> AST::Infix&;
			EVO_NODISCARD auto getInfix(AST::NodeID id) const noexcept -> const AST::Infix&;

			EVO_NODISCARD auto getPostfix(AST::NodeID id) noexcept -> AST::Postfix&;
			EVO_NODISCARD auto getPostfix(AST::NodeID id) const noexcept -> const AST::Postfix&;


			EVO_NODISCARD auto getType(AST::NodeID id) noexcept -> AST::Type&;
			EVO_NODISCARD auto getType(AST::NodeID id) const noexcept -> const AST::Type&;

			EVO_NODISCARD auto getLiteral(AST::NodeID id) noexcept -> AST::Literal&;
			EVO_NODISCARD auto getLiteral(AST::NodeID id) const noexcept -> const AST::Literal&;



			auto print_to_console() const noexcept -> void;


		private:
			auto print_stmt(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

			auto print_var_decl(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

			auto print_func_def(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_func_params(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

			auto print_block(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

			auto print_type(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_expr(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_literal(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

	
		private:
			Parser& parser;
			const Printer& printer;

			uint32_t cursor;
	};


};
