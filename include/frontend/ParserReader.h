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
			EVO_NODISCARD auto getIntrinsicName(AST::NodeID id) const noexcept -> const std::string&;
			EVO_NODISCARD auto getAttributeName(TokenID id) const noexcept -> const std::string&;

			EVO_NODISCARD auto getVarDecl(AST::NodeID id) noexcept -> AST::VarDecl&;
			EVO_NODISCARD auto getVarDecl(AST::NodeID id) const noexcept -> const AST::VarDecl&;

			EVO_NODISCARD auto getMultipleAssignment(AST::NodeID id) noexcept -> AST::MultipleAssignment&;
			EVO_NODISCARD auto getMultipleAssignment(AST::NodeID id) const noexcept -> const AST::MultipleAssignment&;


			EVO_NODISCARD auto getFuncDef(AST::NodeID id) noexcept -> AST::FuncDef&;
			EVO_NODISCARD auto getFuncDef(AST::NodeID id) const noexcept -> const AST::FuncDef&;

			EVO_NODISCARD auto getFuncParams(AST::NodeID id) noexcept -> AST::FuncParams&;
			EVO_NODISCARD auto getFuncParams(AST::NodeID id) const noexcept -> const AST::FuncParams&;

			EVO_NODISCARD auto getFuncOutputs(AST::NodeID id) noexcept -> AST::FuncOutputs&;
			EVO_NODISCARD auto getFuncOutputs(AST::NodeID id) const noexcept -> const AST::FuncOutputs&;

			EVO_NODISCARD auto getBlock(AST::NodeID id) noexcept -> AST::Block&;
			EVO_NODISCARD auto getBlock(AST::NodeID id) const noexcept -> const AST::Block&;

			EVO_NODISCARD auto getAttributes(AST::NodeID id) noexcept -> AST::Attributes&;
			EVO_NODISCARD auto getAttributes(AST::NodeID id) const noexcept -> const AST::Attributes&;


			EVO_NODISCARD auto getConditional(AST::NodeID id) noexcept -> AST::Conditional&;
			EVO_NODISCARD auto getConditional(AST::NodeID id) const noexcept -> const AST::Conditional&;

			EVO_NODISCARD auto getWhileLoop(AST::NodeID id) noexcept -> AST::WhileLoop&;
			EVO_NODISCARD auto getWhileLoop(AST::NodeID id) const noexcept -> const AST::WhileLoop&;

			EVO_NODISCARD auto getReturn(AST::NodeID id) noexcept -> AST::Return&;
			EVO_NODISCARD auto getReturn(AST::NodeID id) const noexcept -> const AST::Return&;

			EVO_NODISCARD auto getStruct(AST::NodeID id) noexcept -> AST::Struct&;
			EVO_NODISCARD auto getStruct(AST::NodeID id) const noexcept -> const AST::Struct&;

			EVO_NODISCARD auto getTryCatch(AST::NodeID id) noexcept -> AST::TryCatch&;
			EVO_NODISCARD auto getTryCatch(AST::NodeID id) const noexcept -> const AST::TryCatch&;



			EVO_NODISCARD auto getPrefix(AST::NodeID id) noexcept -> AST::Prefix&;
			EVO_NODISCARD auto getPrefix(AST::NodeID id) const noexcept -> const AST::Prefix&;

			EVO_NODISCARD auto getInfix(AST::NodeID id) noexcept -> AST::Infix&;
			EVO_NODISCARD auto getInfix(AST::NodeID id) const noexcept -> const AST::Infix&;

			EVO_NODISCARD auto getPostfix(AST::NodeID id) noexcept -> AST::Postfix&;
			EVO_NODISCARD auto getPostfix(AST::NodeID id) const noexcept -> const AST::Postfix&;

			EVO_NODISCARD auto getIndexOp(AST::NodeID id) noexcept -> AST::IndexOp&;
			EVO_NODISCARD auto getIndexOp(AST::NodeID id) const noexcept -> const AST::IndexOp&;

			EVO_NODISCARD auto getFuncCall(AST::NodeID id) noexcept -> AST::FuncCall&;
			EVO_NODISCARD auto getFuncCall(AST::NodeID id) const noexcept -> const AST::FuncCall&;



			EVO_NODISCARD auto getType(AST::NodeID id) noexcept -> AST::Type&;
			EVO_NODISCARD auto getType(AST::NodeID id) const noexcept -> const AST::Type&;

			EVO_NODISCARD auto getLiteral(AST::NodeID id) noexcept -> AST::Literal&;
			EVO_NODISCARD auto getLiteral(AST::NodeID id) const noexcept -> const AST::Literal&;

			EVO_NODISCARD auto getOperator(AST::NodeID id) noexcept -> AST::Operator&;
			EVO_NODISCARD auto getOperator(AST::NodeID id) const noexcept -> const AST::Operator&;



			auto print_to_console() const noexcept -> void;


		private:
			auto print_stmt(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

			auto print_var_decl(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_multiple_assignment(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

			auto print_func_def(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_func_params(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

			auto print_block(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

			auto print_conditional(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_while_loop(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_return(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_struct(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_try_catch(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

			auto print_type(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_expr(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_literal(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

	
		private:
			Parser& parser;
			const Printer& printer;

			uint32_t cursor;
	};


};
