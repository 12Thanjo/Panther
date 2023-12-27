#pragma once


#include <Evo.h>


#include "./Tokenizer.h"
#include "./AST.h"


namespace panther{

	

	class Parser{
		public:
			Parser(TokenizerReader& tokenizer_reader) : reader(tokenizer_reader) {};
			~Parser() = default;


			// returns false if error
			EVO_NODISCARD auto parse() noexcept -> bool;

	
		private:
			class Result{
				public:
					enum class Code{
						Success,
						WrongType,
						Error,
						UnreportedError,
					};

					using enum class Code;



					Result(Code res_code) : result_code(res_code), data(std::nullopt) {};
					Result(AST::NodeID&& value) : result_code(Code::Success), data(std::move(value)) {};
					~Result() = default;

					EVO_NODISCARD inline auto code() const noexcept -> Code { return this->result_code; };
					EVO_NODISCARD inline auto value() const noexcept -> const AST::NodeID& {
						evo::debugAssert(this->data.has_value(), "Attempted to get value from result thas has no value");
						return *(this->data); 
					};

			
				private:
					Code result_code;
					std::optional<AST::NodeID> data;
			};

			EVO_NODISCARD auto parse_stmt() noexcept -> Result;


			EVO_NODISCARD auto parse_var_decl() noexcept -> Result;

			EVO_NODISCARD auto parse_func_def() noexcept -> Result;
			EVO_NODISCARD auto parse_func_params() noexcept -> Result;

			EVO_NODISCARD auto parse_block() noexcept -> Result;

			EVO_NODISCARD auto parse_ident() noexcept -> Result;
			EVO_NODISCARD auto parse_attributes() noexcept -> Result;


			EVO_NODISCARD auto parse_type() noexcept -> Result;
			EVO_NODISCARD auto parse_arr_type() noexcept -> Result;
			EVO_NODISCARD auto parse_func_type() noexcept -> Result;
			EVO_NODISCARD auto parse_type_qualifiers() noexcept -> std::vector<AST::Type::Qualifier>;



			EVO_NODISCARD auto parse_expr() noexcept -> Result;
			EVO_NODISCARD auto parse_infix_expr(int prec_level = 1) noexcept -> Result;

			EVO_NODISCARD auto parse_prefix_expr() noexcept -> Result;
			EVO_NODISCARD auto parse_postfix_expr() noexcept -> Result;
			EVO_NODISCARD auto parse_paren_expr() noexcept -> Result;

			EVO_NODISCARD auto parse_term() noexcept -> Result;
			EVO_NODISCARD auto parse_literal() noexcept -> Result;





			// TODO: maybe make these private
			auto error(const std::string& message, TokenID token) const noexcept -> void;
			auto error_info(const std::string& message) const noexcept -> void;
			auto error_info(const std::string& message, TokenID token) const noexcept -> void;
			auto fatal(const std::string& message, TokenID token) const noexcept -> void;

			auto unexpected_eof(const std::string& location) noexcept -> void;
			auto expected_but_got(const std::string& expected, TokenID token) noexcept -> void;
			inline auto expected_but_got(const std::string& expected) noexcept -> void {
				this->expected_but_got(expected, this->reader.peek());
			};


		private:
			TokenizerReader& reader;


			std::vector<AST::NodeID> top_level_statements{};

			std::vector<AST::Node> nodes{};
			std::vector<AST::VarDecl> var_decls{};

			std::vector<AST::FuncDef> func_defs{};
			std::vector<AST::FuncParams> func_params{};

			std::vector<AST::Block> blocks{};

			std::vector<AST::Prefix> prefixes{};
			std::vector<AST::Infix> infixes{};
			std::vector<AST::Postfix> postfixes{};

			std::vector<AST::Ident> idents{};
			std::vector<AST::Attributes> attributes{};
			std::vector<AST::Literal> literals{};
			std::vector<AST::Type> types{};
			std::vector<AST::Term> terms{};

			friend class ParserReader;
	};




};
