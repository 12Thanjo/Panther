#pragma once


#include <Evo.h>


#include "frontend/Source.h"
#include "frontend/AST.h"

namespace panther{


	class Parser{
		public:
			Parser(Source& src) : source(src) {};
			~Parser() = default;

			EVO_NODISCARD auto parse() noexcept -> bool;




		private:
			class Result{
				public:
					enum class Code{
						None, // for default construction

						Success,
						WrongType,
						Error,
					};

					using enum class Code;

				public:
					Result() : result_code(Code::None) {};
					Result(Code res_code) : result_code(res_code) {};
					Result(AST::Node::ID val) : result_code(Code::Success), data(val) {};
					~Result() = default;

					EVO_NODISCARD inline auto code() const noexcept -> Code { return this->result_code; };
					EVO_NODISCARD inline auto value() const noexcept -> const AST::Node::ID& {
						evo::debugAssert(this->result_code == Code::Success, "Attempted to get value from result that has no value");
						return this->data;
					};
			
				private:
					Code result_code;

					union {
						evo::byte dummy_data[1];
						AST::Node::ID data;
					};
			};




			EVO_NODISCARD auto parse_stmt() noexcept -> Result;
			EVO_NODISCARD auto parse_var_decl() noexcept -> Result;
			EVO_NODISCARD auto parse_func() noexcept -> Result;
			EVO_NODISCARD auto parse_conditional() noexcept -> Result;
			EVO_NODISCARD auto parse_return() noexcept -> Result;
			EVO_NODISCARD auto parse_assignment() noexcept -> Result;

			EVO_NODISCARD auto parse_type() noexcept -> Result;
			EVO_NODISCARD auto parse_block() noexcept -> Result;


			///////////////////////////////////
			// expr

			EVO_NODISCARD auto parse_expr() noexcept -> Result;
			EVO_NODISCARD auto parse_uninit() noexcept -> Result;

			EVO_NODISCARD auto parse_infix_expr() noexcept -> Result;
			EVO_NODISCARD auto parse_infix_expr_impl(AST::Node::ID lhs, int prec_level) noexcept -> Result;
			EVO_NODISCARD auto parse_prefix_expr() noexcept -> Result;
			EVO_NODISCARD auto parse_postfix_expr() noexcept -> Result;

			EVO_NODISCARD auto parse_accessor_expr() noexcept -> Result;

			EVO_NODISCARD auto parse_paren_expr() noexcept -> Result;


			EVO_NODISCARD auto parse_term() noexcept -> Result;

			EVO_NODISCARD auto parse_ident() noexcept -> Result;
			EVO_NODISCARD auto parse_literal() noexcept -> Result;
			EVO_NODISCARD auto parse_intrinsic() noexcept -> Result;



		private:

			EVO_NODISCARD auto inline create_token_node(AST::Kind kind, Token::ID id) noexcept -> AST::Node::ID {
				const uint32_t node_index = uint32_t(this->source.nodes.size());
				this->source.nodes.emplace_back(kind, id);
				return AST::Node::ID(node_index);
			};


			template<typename T, typename... Args>
			EVO_NODISCARD auto inline create_node(std::vector<T>& instances, AST::Kind kind, Args&&... constructor_args) noexcept -> AST::Node::ID {
				const uint32_t node_index = uint32_t(this->source.nodes.size());
				const uint32_t instance_index = uint32_t(instances.size());

				this->source.nodes.emplace_back(kind, instance_index);
				instances.emplace_back(std::forward<decltype(constructor_args)>(constructor_args)...);

				return AST::Node::ID(node_index);
			};



			///////////////////////////////////
			// messaging

			auto expect_token(Token::Kind kind, evo::CStrProxy location) noexcept -> bool; // returns if got punctuation

			auto expected_but_got(evo::CStrProxy msg, Token::ID token_id, std::vector<Message::Info>&& infos = {}) const noexcept -> void;
			auto expected_but_got(evo::CStrProxy msg, std::vector<Message::Info>&& infos = {}) const noexcept -> void {
				this->expected_but_got(msg, this->peek(), std::move(infos));
			};


			EVO_NODISCARD auto check_result_fail(const Result& result, evo::CStrProxy msg) const noexcept -> bool;
			



			///////////////////////////////////
			// reading

			EVO_NODISCARD inline auto eof() const noexcept -> bool { return size_t(this->cursor) >= this->source.tokens.size(); };

			EVO_NODISCARD inline auto peek(ptrdiff_t peek_ammount = 0) const noexcept -> Token::ID {
				evo::debugAssert(this->cursor + peek_ammount >= 0, "cannot peek before the start of tokens");
				evo::debugAssert(size_t(this->cursor + peek_ammount) < this->source.tokens.size(), "cannot peek past end of tokens");

				return Token::ID(uint32_t(this->cursor + peek_ammount));
			};

			EVO_NODISCARD inline auto next() noexcept -> Token::ID {
				evo::debugAssert(this->eof() == false, "Parser cannot go to next token (already eof)");

				auto output = Token::ID(uint32_t(this->cursor));
				this->cursor += 1;
				return output;
			};

			EVO_NODISCARD inline auto get(Token::ID id) const noexcept -> const Token& {
				return this->source.tokens[id.id];
			};

			EVO_NODISCARD inline auto skip(uint32_t skip_ammount) noexcept -> void {
				evo::debugAssert(skip_ammount > 0, "skip ammount should be more than 0");
				evo::debugAssert(size_t(this->cursor + skip_ammount) <= this->source.tokens.size(), "cannot skip past end of tokens");

				this->cursor += ptrdiff_t(skip_ammount);
			};


			inline auto go_back(Token::ID id) noexcept -> void {
				this->cursor = ptrdiff_t(id.id);
			};

	
		private:
			Source& source;

			ptrdiff_t cursor = 0;
	};


};