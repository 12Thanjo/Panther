#pragma once


#include <Evo.h>

#include "./Token.h"
#include "./AST.h"
#include "Message.h"
#include "objects.h"


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


			///////////////////////////////////
			// gettting

			EVO_NODISCARD auto getNode(AST::Node::ID node_id) const noexcept -> const AST::Node&;
			EVO_NODISCARD auto getToken(Token::ID token_id) const noexcept -> const Token&;


			EVO_NODISCARD auto getVarDecl(AST::Node::ID node_id) const noexcept -> const AST::VarDecl&;
			EVO_NODISCARD auto getVarDecl(const AST::Node& node) const noexcept -> const AST::VarDecl&;

			EVO_NODISCARD auto getFunc(AST::Node::ID node_id) const noexcept -> const AST::Func&;
			EVO_NODISCARD auto getFunc(const AST::Node& node) const noexcept -> const AST::Func&;


			EVO_NODISCARD auto getType(AST::Node::ID node_id) const noexcept -> const AST::Type&;
			EVO_NODISCARD auto getType(const AST::Node& node) const noexcept -> const AST::Type&;

			EVO_NODISCARD auto getBlock(AST::Node::ID node_id) const noexcept -> const AST::Block&;
			EVO_NODISCARD auto getBlock(const AST::Node& node) const noexcept -> const AST::Block&;


			EVO_NODISCARD auto getLiteral(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getLiteral(const AST::Node& node) const noexcept -> const Token&;

			EVO_NODISCARD auto getIdent(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getIdent(const AST::Node& node) const noexcept -> const Token&;

			EVO_NODISCARD auto getUninit(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getUninit(const AST::Node& node) const noexcept -> const Token&;





			template<typename... Args>
			EVO_NODISCARD inline auto createVar(Args... args) noexcept -> object::Var::ID {
				this->objects.vars.emplace_back(args...);

				return object::Var::ID( uint32_t(this->objects.vars.size() - 1) );
			};


			EVO_NODISCARD inline auto getVar(object::Var::ID id) const noexcept -> const object::Var& {
				return this->objects.vars[size_t(id.id)];
			};

			EVO_NODISCARD inline auto getVar(object::Var::ID id) noexcept -> object::Var& {
				return this->objects.vars[size_t(id.id)];
			};



			template<typename... Args>
			EVO_NODISCARD inline auto createFunc(Args... args) noexcept -> object::Func::ID {
				this->objects.funcs.emplace_back(args...);

				return object::Func::ID( uint32_t(this->objects.funcs.size() - 1) );
			};


			EVO_NODISCARD inline auto getFunc(object::Func::ID id) const noexcept -> const object::Func& {
				return this->objects.funcs[size_t(id.id)];
			};



			///////////////////////////////////
			// messaging / errors

			auto fatal(const std::string& msg, uint32_t line, uint32_t collumn) noexcept -> void;
			auto fatal(const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void;


			auto error(const std::string& msg, uint32_t line, uint32_t collumn, std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, Token::ID token_id,              std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, const Token& token,              std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, AST::Node::ID node_id,           std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, const AST::Node& node,           std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, Location location,               std::vector<Message::Info>&& infos = {}) noexcept -> void;





			EVO_NODISCARD inline auto hasErrored() const noexcept -> bool { return this->has_errored; };

		public:
			std::vector<Token> tokens{};

			std::vector<AST::Node::ID> global_stmts{};
			std::vector<AST::Node> nodes{};
			std::vector<AST::VarDecl> var_decls{};
			std::vector<AST::Func> funcs{};
			std::vector<AST::Type> types{};
			std::vector<AST::Block> blocks{};



			struct /* objects */ {
				std::vector<object::Var> vars{};
				std::vector<object::Func> funcs{};

				std::vector<object::Var::ID> global_vars{};
			} objects;


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