#pragma once


#include <Evo.h>

#include "./Token.h"
#include "./AST.h"
#include "Message.h"
#include "PIR.h"


namespace panther{


	class Source{
		public:
			struct ID{ // typesafe identifier
				uint32_t id;
				explicit ID(uint32_t _id) noexcept : id(_id) {};
			};

		public:
			// TODO: other permutations of refs
			Source(const std::string& src_location, std::string&& src_data, class SourceManager& src_manager, ID id)
				: src_location(src_location), data(std::move(src_data)), source_manager(src_manager), src_id(id) {};


			EVO_NODISCARD inline auto getLocation() const noexcept -> const std::string& { return this->src_location; };
			EVO_NODISCARD inline auto getData() const noexcept -> const std::string& { return this->data; };
			EVO_NODISCARD inline auto getSourceManager() noexcept -> SourceManager& { return this->source_manager; };
			EVO_NODISCARD inline auto getSourceManager() const noexcept -> const SourceManager& { return this->source_manager; };
			EVO_NODISCARD inline auto getID() const noexcept -> ID { return this->src_id; };

			// returns true if successful (no errors)
			EVO_NODISCARD auto tokenize() noexcept -> bool;

			// returns true if successful (no errors)
			EVO_NODISCARD auto parse() noexcept -> bool;

			// returns true if successful (no errors)
			EVO_NODISCARD auto semantic_analysis() noexcept -> bool;


			//////////////////////////////////////////////////////////////////////
			// getting

			///////////////////////////////////
			// AST

			EVO_NODISCARD auto getNode(AST::Node::ID node_id) const noexcept -> const AST::Node&;
			EVO_NODISCARD auto getToken(Token::ID token_id) const noexcept -> const Token&;


			EVO_NODISCARD auto getVarDecl(AST::Node::ID node_id) const noexcept -> const AST::VarDecl&;
			EVO_NODISCARD auto getVarDecl(const AST::Node& node) const noexcept -> const AST::VarDecl&;

			EVO_NODISCARD auto getFunc(AST::Node::ID node_id) const noexcept -> const AST::Func&;
			EVO_NODISCARD auto getFunc(const AST::Node& node) const noexcept -> const AST::Func&;

			EVO_NODISCARD auto getConditional(AST::Node::ID node_id) const noexcept -> const AST::Conditional&;
			EVO_NODISCARD auto getConditional(const AST::Node& node) const noexcept -> const AST::Conditional&;

			EVO_NODISCARD auto getReturn(AST::Node::ID node_id) const noexcept -> const AST::Return&;
			EVO_NODISCARD auto getReturn(const AST::Node& node) const noexcept -> const AST::Return&;


			EVO_NODISCARD auto getType(AST::Node::ID node_id) const noexcept -> const AST::Type&;
			EVO_NODISCARD auto getType(const AST::Node& node) const noexcept -> const AST::Type&;

			EVO_NODISCARD auto getBlock(AST::Node::ID node_id) const noexcept -> const AST::Block&;
			EVO_NODISCARD auto getBlock(const AST::Node& node) const noexcept -> const AST::Block&;

			EVO_NODISCARD auto getPrefix(AST::Node::ID node_id) const noexcept -> const AST::Prefix&;
			EVO_NODISCARD auto getPrefix(const AST::Node& node) const noexcept -> const AST::Prefix&;

			EVO_NODISCARD auto getInfix(AST::Node::ID node_id) const noexcept -> const AST::Infix&;
			EVO_NODISCARD auto getInfix(const AST::Node& node) const noexcept -> const AST::Infix&;

			EVO_NODISCARD auto getPostfix(AST::Node::ID node_id) const noexcept -> const AST::Postfix&;
			EVO_NODISCARD auto getPostfix(const AST::Node& node) const noexcept -> const AST::Postfix&;

			EVO_NODISCARD auto getFuncCall(AST::Node::ID node_id) const noexcept -> const AST::FuncCall&;
			EVO_NODISCARD auto getFuncCall(const AST::Node& node) const noexcept -> const AST::FuncCall&;


			EVO_NODISCARD auto getLiteral(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getLiteral(const AST::Node& node) const noexcept -> const Token&;

			EVO_NODISCARD auto getIdent(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getIdent(const AST::Node& node) const noexcept -> const Token&;

			EVO_NODISCARD auto getIntrinsic(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getIntrinsic(const AST::Node& node) const noexcept -> const Token&;

			EVO_NODISCARD auto getUninit(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getUninit(const AST::Node& node) const noexcept -> const Token&;

			EVO_NODISCARD auto getUnreachable(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getUnreachable(const AST::Node& node) const noexcept -> const Token&;




			///////////////////////////////////
			// PIR

			EVO_NODISCARD inline auto createVar(auto&&... args) noexcept -> PIR::Var::ID {
				this->pir.vars.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Var::ID( uint32_t(this->pir.vars.size() - 1) );
			};

			EVO_NODISCARD inline auto getVar(PIR::Var::ID id) const noexcept -> const PIR::Var& {
				return this->pir.vars[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getVar(PIR::Var::ID id) noexcept -> PIR::Var& {
				return this->pir.vars[size_t(id.id)];
			};



			EVO_NODISCARD inline auto createFunc(auto&&... args) noexcept -> PIR::Func::ID {
				this->pir.funcs.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Func::ID( uint32_t(this->pir.funcs.size() - 1) );
			};

			EVO_NODISCARD inline auto getFunc(PIR::Func::ID id) const noexcept -> const PIR::Func& {
				return this->pir.funcs[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getFunc(PIR::Func::ID id) noexcept -> PIR::Func& {
				return this->pir.funcs[size_t(id.id)];
			};

			EVO_NODISCARD inline auto createConditional(auto&&... args) noexcept -> PIR::Conditional::ID {
				this->pir.conditionals.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Conditional::ID( uint32_t(this->pir.conditionals.size() - 1) );
			};

			EVO_NODISCARD inline auto getConditional(PIR::Conditional::ID id) const noexcept -> const PIR::Conditional& {
				return this->pir.conditionals[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getConditional(PIR::Conditional::ID id) noexcept -> PIR::Conditional& {
				return this->pir.conditionals[size_t(id.id)];
			};



			EVO_NODISCARD inline auto createReturn(auto&&... args) noexcept -> PIR::Return::ID {
				this->pir.returns.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Return::ID( uint32_t(this->pir.returns.size() - 1) );
			};

			EVO_NODISCARD inline auto getReturn(PIR::Return::ID id) const noexcept -> const PIR::Return& {
				return this->pir.returns[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getReturn(PIR::Return::ID id) noexcept -> PIR::Return& {
				return this->pir.returns[size_t(id.id)];
			};



			EVO_NODISCARD inline auto createAssignment(auto&&... args) noexcept -> PIR::Assignment::ID {
				this->pir.assignments.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Assignment::ID( uint32_t(this->pir.assignments.size() - 1) );
			};

			EVO_NODISCARD inline auto getAssignment(PIR::Assignment::ID id) const noexcept -> const PIR::Assignment& {
				return this->pir.assignments[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getAssignment(PIR::Assignment::ID id) noexcept -> PIR::Assignment& {
				return this->pir.assignments[size_t(id.id)];
			};



			EVO_NODISCARD inline auto createFuncCall(auto&&... args) noexcept -> PIR::FuncCall::ID {
				this->pir.func_calls.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::FuncCall::ID( uint32_t(this->pir.func_calls.size() - 1) );
			};

			EVO_NODISCARD inline auto getFuncCall(PIR::FuncCall::ID id) const noexcept -> const PIR::FuncCall& {
				return this->pir.func_calls[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getFuncCall(PIR::FuncCall::ID id) noexcept -> PIR::FuncCall& {
				return this->pir.func_calls[size_t(id.id)];
			};



			EVO_NODISCARD inline auto createPrefix(auto&&... args) noexcept -> PIR::Prefix::ID {
				this->pir.prefixes.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Prefix::ID( uint32_t(this->pir.prefixes.size() - 1) );
			};

			EVO_NODISCARD inline auto getPrefix(PIR::Prefix::ID id) const noexcept -> const PIR::Prefix& {
				return this->pir.prefixes[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getPrefix(PIR::Prefix::ID id) noexcept -> PIR::Prefix& {
				return this->pir.prefixes[size_t(id.id)];
			};


			EVO_NODISCARD inline auto createDeref(auto&&... args) noexcept -> PIR::Deref::ID {
				this->pir.derefs.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Deref::ID( uint32_t(this->pir.derefs.size() - 1) );
			};

			EVO_NODISCARD inline auto getDeref(PIR::Deref::ID id) const noexcept -> const PIR::Deref& {
				return this->pir.derefs[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getDeref(PIR::Deref::ID id) noexcept -> PIR::Deref& {
				return this->pir.derefs[size_t(id.id)];
			};



			//////////////////////////////////////////////////////////////////////
			// messaging / errors

			auto fatal(const std::string& msg, uint32_t line, uint32_t collumn) noexcept -> void;
			auto fatal(const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void;


			auto error(const std::string& msg, uint32_t line, uint32_t collumn, std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, Token::ID token_id,              std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, const Token& token,              std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, AST::Node::ID node_id,           std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, const AST::Node& node,           std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, Location location,               std::vector<Message::Info>&& infos = {}) noexcept -> void;


			auto warning(const std::string& msg, uint32_t line, uint32_t collumn, std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto warning(const std::string& msg, Token::ID token_id,              std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto warning(const std::string& msg, const Token& token,              std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto warning(const std::string& msg, AST::Node::ID node_id,           std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto warning(const std::string& msg, const AST::Node& node,           std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto warning(const std::string& msg, Location location,               std::vector<Message::Info>&& infos = {}) noexcept -> void;



			EVO_NODISCARD inline auto hasErrored() const noexcept -> bool { return this->has_errored; };

		public:
			std::vector<Token> tokens{};
			std::vector<std::unique_ptr<std::string>> string_literal_values{};

			std::vector<AST::Node::ID> global_stmts{};
			std::vector<AST::Node> nodes{};
			std::vector<AST::VarDecl> var_decls{};
			std::vector<AST::Func> funcs{};
			std::vector<AST::Conditional> conditionals{};
			std::vector<AST::Return> returns{};
			std::vector<AST::Prefix> prefixes{};
			std::vector<AST::Infix> infixes{};
			std::vector<AST::Postfix> postfixes{};
			std::vector<AST::FuncCall> func_calls{};
			std::vector<AST::Type> types{};
			std::vector<AST::Block> blocks{};



			struct /* pir */ {
				std::vector<PIR::Var> vars{};
				std::vector<PIR::Func> funcs{};
				std::vector<PIR::Conditional> conditionals{};
				std::vector<PIR::Return> returns{};
				std::vector<PIR::Assignment> assignments{};
				std::vector<PIR::FuncCall> func_calls{};
				std::vector<PIR::Prefix> prefixes{};
				std::vector<PIR::Deref> derefs{};

				std::vector<PIR::Var::ID> global_vars{};
			} pir;


		private:
			EVO_NODISCARD auto get_node_location(AST::Node::ID node_id) const noexcept -> Location;
			EVO_NODISCARD auto get_node_location(const AST::Node& node) const noexcept -> Location;


		private:
			std::string src_location;
			std::string data;
			ID src_id;
			class SourceManager& source_manager;

			bool has_errored = false;
	};


};