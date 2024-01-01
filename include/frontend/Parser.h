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
			EVO_NODISCARD auto parse_assignment() noexcept -> Result;
			EVO_NODISCARD auto parse_multiple_assignment() noexcept -> Result;

			EVO_NODISCARD auto parse_func_def() noexcept -> Result;
			EVO_NODISCARD auto parse_func_params() noexcept -> Result;
			EVO_NODISCARD auto parse_func_returns() noexcept -> Result;
			EVO_NODISCARD auto parse_func_errors() noexcept -> Result;

			EVO_NODISCARD auto parse_block() noexcept -> Result;

			EVO_NODISCARD auto parse_conditional() noexcept -> Result;
			EVO_NODISCARD auto parse_while() noexcept -> Result;
			EVO_NODISCARD auto parse_return() noexcept -> Result;

			EVO_NODISCARD auto parse_ident() noexcept -> Result;
			EVO_NODISCARD auto parse_intrinsic() noexcept -> Result;
			EVO_NODISCARD auto parse_attributes() noexcept -> Result;


			EVO_NODISCARD auto parse_type() noexcept -> Result;
			EVO_NODISCARD auto parse_arr_type() noexcept -> Result;
			EVO_NODISCARD auto parse_func_type() noexcept -> Result;
			EVO_NODISCARD auto parse_type_qualifiers() noexcept -> std::vector<AST::Type::Qualifier>;



			EVO_NODISCARD auto parse_expr() noexcept -> Result;

			EVO_NODISCARD auto parse_infix_expr() noexcept -> Result;
			EVO_NODISCARD auto parse_infix_expr_impl(AST::NodeID lhs, int prec_level) noexcept -> Result;
			EVO_NODISCARD auto parse_prefix_expr() noexcept -> Result;
			EVO_NODISCARD auto parse_postfix_expr() noexcept -> Result;

			EVO_NODISCARD auto parse_accessor_expr() noexcept -> Result;

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




			template<typename T, typename... Args>
			EVO_NODISCARD inline auto create_node(std::vector<T>& instances, AST::Kind kind, Args&&... constructor_args) noexcept -> AST::NodeID {
				const uint32_t node_index = uint32_t(this->nodes.size());
				const uint32_t instance_index = uint32_t(instances.size());

				this->nodes.emplace_back(kind, instance_index);
				instances.emplace_back(std::forward<decltype(constructor_args)>(constructor_args)...);

				return AST::NodeID{node_index};
			};


			EVO_NODISCARD inline auto create_node(AST::Kind kind) noexcept -> AST::NodeID {
				const uint32_t node_index = uint32_t(this->nodes.size());
				this->nodes.emplace_back(kind);

				return AST::NodeID{node_index};
			};


		private:
			TokenizerReader& reader;


			std::vector<AST::NodeID> top_level_statements{};

			std::vector<AST::Node> nodes{};

			std::vector<AST::VarDecl> var_decls{};
			std::vector<AST::MultipleAssignment> multiple_assignments{};

			std::vector<AST::FuncDef> func_defs{};
			std::vector<AST::FuncParams> func_params{};
			std::vector<AST::FuncOutputs> func_outputs{};

			std::vector<AST::Block> blocks{};

			std::vector<AST::Conditional> conditionals{};
			std::vector<AST::WhileLoop> while_loops{};
			std::vector<AST::Return> returns{};

			std::vector<AST::Prefix> prefixes{};
			std::vector<AST::Infix> infixes{};
			std::vector<AST::Postfix> postfixes{};
			std::vector<AST::IndexOp> index_ops{};
			std::vector<AST::FuncCall> func_calls{};

			std::vector<AST::Ident> idents{};
			std::vector<AST::Intrinsic> intrinsics{};
			std::vector<AST::Attributes> attributes{};
			std::vector<AST::Literal> literals{};
			std::vector<AST::Type> types{};

			friend class ParserReader;
	};




};
