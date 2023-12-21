#pragma once


#include <Evo.h>


#include "./Parser.h"



namespace panther{



	class ParserReader{
		public:
			ParserReader(Parser& _parser) : parser(_parser) {};
			~ParserReader() = default;


			EVO_NODISCARD auto getNodeKind(AST::NodeID id) const noexcept -> AST::Kind;
			EVO_NODISCARD auto getIdentName(AST::NodeID id) const noexcept -> const std::string&;

			EVO_NODISCARD auto getVarDecl(AST::NodeID id) noexcept -> AST::VarDecl&;
			EVO_NODISCARD auto getVarDecl(AST::NodeID id) const noexcept -> const AST::VarDecl&;

			EVO_NODISCARD auto getType(AST::NodeID id) noexcept -> AST::Type&;
			EVO_NODISCARD auto getType(AST::NodeID id) const noexcept -> const AST::Type&;



			auto print_to_console() const noexcept -> void;


		private:
			auto print_var_decl(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_type(AST::NodeID id, class Indenter& indenter) const noexcept -> void;
			auto print_expr(AST::NodeID id, class Indenter& indenter) const noexcept -> void;

	
		private:
			Parser& parser;

			uint32_t cursor;
	};


};
