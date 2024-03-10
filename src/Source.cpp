#include "Source.h"


#include "./frontend/Tokenizer.h"
#include "./frontend/Parser.h"
#include "./frontend/SemanticAnalyzer.h"
#include "SourceManager.h"

namespace panther{


	auto Source::tokenize() noexcept -> bool {
		auto tokenizer = Tokenizer(*this);
		return tokenizer.tokenize();
	};


	auto Source::parse() noexcept -> bool {
		auto parser = Parser(*this);
		return parser.parse();
	};


	auto Source::semantic_analysis() noexcept -> bool {
		auto semantic_analizer = SemanticAnalyzer(*this);
		return semantic_analizer.semantic_analysis();
	};



	//////////////////////////////////////////////////////////////////////
	// getting

	auto Source::getNode(AST::Node::ID node_id) const noexcept -> const AST::Node& {
		return this->nodes[node_id.id];
	};

	auto Source::getToken(Token::ID token_id) const noexcept -> const Token& {
		return this->tokens[token_id.id];
	};



	auto Source::getVarDecl(AST::Node::ID node_id) const noexcept -> const AST::VarDecl& {
		return this->getVarDecl(this->getNode(node_id));
	};

	auto Source::getVarDecl(const AST::Node& node) const noexcept -> const AST::VarDecl& {
		evo::debugAssert(node.kind == AST::Kind::VarDecl, "Node is not a VarDecl");
		return this->var_decls[node.index];
	};



	auto Source::getType(AST::Node::ID node_id) const noexcept -> const AST::Type& {
		return this->getType(this->getNode(node_id));
	};

	auto Source::getType(const AST::Node& node) const noexcept -> const AST::Type& {
		evo::debugAssert(node.kind == AST::Kind::Type, "Node is not a Type");
		return this->types[node.index];
	};



	auto Source::getLiteral(AST::Node::ID node_id) const noexcept -> const Token& {
		return this->getLiteral(this->getNode(node_id));
	};

	auto Source::getLiteral(const AST::Node& node) const noexcept -> const Token& {
		evo::debugAssert(node.kind == AST::Kind::Literal, "Node is not a Literal");
		return this->getToken(node.token);
	};



	//////////////////////////////////////////////////////////////////////
	// messaging

	auto Source::fatal(const std::string& msg, uint32_t line, uint32_t collumn) noexcept -> void {
		this->has_errored = true;

		auto message = Message{
			.type          = Message::Type::Fatal,
			.source        = *this,
			.message       = msg,
			.line          = line,
			.collumn_start = collumn,
			.collumn_end   = collumn,
		};

		this->source_manager.emitMessage(message);
	};


	auto Source::fatal(const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void {
		this->has_errored = true;

		auto message = Message{
			.type          = Message::Type::Fatal,
			.source        = *this,
			.message       = msg,
			.line          = line,
			.collumn_start = collumn_start,
			.collumn_end   = collumn_end,
		};

		this->source_manager.emitMessage(message);
	};





	auto Source::error(const std::string& msg, uint32_t line, uint32_t collumn) noexcept -> void {
		this->error(msg, line, collumn, collumn, std::vector<std::string>{});
	};

	auto Source::error(const std::string& msg, Token::ID token_id) noexcept -> void {
		const Token& token = this->getToken(token_id);
		this->error(msg, token.line_start, token.collumn_start, token.collumn_end, std::vector<std::string>{});
	};

	auto Source::error(const std::string& msg, const Token& token) noexcept -> void {
		this->error(msg, token.line_start, token.collumn_start, token.collumn_end, std::vector<std::string>{});
	};

	auto Source::error(const std::string& msg, uint32_t line, uint32_t collumn, std::vector<std::string>&& infos) noexcept -> void {
		this->error(msg, line, collumn, collumn, std::move(infos));
	};


	auto Source::error(const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void {
		this->error(msg, line, collumn_start, collumn_end, std::vector<std::string>{});
	};


	auto Source::error(
		const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end, std::vector<std::string>&& infos
	) noexcept -> void {
		this->has_errored = true;

		auto message = Message{
			.type          = Message::Type::Error,
			.source        = *this,
			.message       = msg,
			.line          = line,
			.collumn_start = collumn_start,
			.collumn_end   = collumn_end,
			.infos		   = std::move(infos),
		};

		this->source_manager.emitMessage(message);
	};

	
};
