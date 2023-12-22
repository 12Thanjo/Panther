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



			EVO_NODISCARD auto parse_var_decl() noexcept -> Result;

			EVO_NODISCARD auto parse_ident() noexcept -> Result;
			EVO_NODISCARD auto parse_type() noexcept -> Result;


			EVO_NODISCARD auto parse_expr() noexcept -> Result;
			EVO_NODISCARD auto parse_term() noexcept -> Result;
			EVO_NODISCARD auto parse_literal() noexcept -> Result;



			// TODO: maybe make these private
			auto error(const std::string& message, TokenID token) const noexcept -> void;
			auto error_info(const std::string& message) const noexcept -> void;
			auto error_info(const std::string& message, TokenID token) const noexcept -> void;
			auto fatal(const std::string& message, TokenID token) const noexcept -> void;

		private:
			TokenizerReader& reader;


			std::vector<AST::NodeID> top_level_statements{};

			std::vector<AST::Node> nodes{};
			std::vector<AST::VarDecl> var_decls{};
			std::vector<AST::Ident> idents{};
			std::vector<AST::Literal> literals{};
			std::vector<AST::Type> types{};
			std::vector<AST::Term> terms{};

			friend class ParserReader;
	};




};
