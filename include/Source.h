#pragma once


#include <Evo.h>

#include "./Token.h"
#include "./AST.h"


namespace panther{


	class Source{
		public:
			struct ID{ uint32_t id; };

		public:
			// TODO: other permutations of refs
			Source(const std::string& src_location, std::string&& src_data, class SourceManager& src_manager)
				: location(src_location), data(std::move(src_data)), source_manager(src_manager) {};


			EVO_NODISCARD inline auto getLocation() const noexcept -> const std::string& { return this->location; };
			EVO_NODISCARD inline auto getData() const noexcept -> const std::string& { return this->data; };
			EVO_NODISCARD inline auto getSourceManager() const noexcept -> const SourceManager& { return this->source_manager; };


			// returns if successful (no errors)
			EVO_NODISCARD auto tokenize() noexcept -> bool;

			// returns if successful (no errors)
			EVO_NODISCARD auto parse() noexcept -> bool;


			///////////////////////////////////
			// gettting

			EVO_NODISCARD auto getNode(AST::Node::ID node_id) const noexcept -> const AST::Node&;
			EVO_NODISCARD auto getToken(Token::ID token_id) const noexcept -> const Token&;

			EVO_NODISCARD auto getVarDecl(const AST::Node& node) const noexcept -> const AST::VarDecl&;
			EVO_NODISCARD auto getType(const AST::Node& node) const noexcept -> const AST::Type&;


			///////////////////////////////////
			// messaging / errors

			auto fatal(const std::string& msg, uint32_t line, uint32_t collumn) noexcept -> void;
			auto fatal(const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void;


			auto error(const std::string& msg, uint32_t line, uint32_t collumn) noexcept -> void;
			auto error(const std::string& msg, Token::ID token_id) noexcept -> void;
			auto error(const std::string& msg, uint32_t line, uint32_t collumn, std::vector<std::string>&& infos) noexcept -> void;
			auto error(const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void;
			auto error(
				const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end, std::vector<std::string>&& infos
			) noexcept -> void;




			EVO_NODISCARD inline auto hasErrored() const noexcept -> bool { return this->has_errored; };

		public:
			std::vector<Token> tokens{};

			std::vector<AST::Node> nodes{};
			std::vector<AST::Node::ID> global_stmts{};
			std::vector<AST::VarDecl> var_decls{};
			std::vector<AST::Type> types{};


		private:
			std::string location; 
			std::string data;
			class SourceManager& source_manager;

			bool has_errored = false;
	};


};