#include "Source.h"


#include "./frontend/Tokenizer.h"
#include "./frontend/Parser.h"
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
		this->has_errored = true;

		auto message = Message{
			.type          = Message::Type::Error,
			.source        = *this,
			.message       = msg,
			.line          = line,
			.collumn_start = collumn,
			.collumn_end   = collumn,
		};

		this->source_manager.emitMessage(message);
	};

	auto Source::error(const std::string& msg, uint32_t line, uint32_t collumn, std::vector<std::string>&& infos) noexcept -> void {
		this->has_errored = true;

		auto message = Message{
			.type          = Message::Type::Error,
			.source        = *this,
			.message       = msg,
			.line          = line,
			.collumn_start = collumn,
			.collumn_end   = collumn,
			.infos		   = std::move(infos),
		};

		this->source_manager.emitMessage(message);
	};


	auto Source::error(const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void {
		this->has_errored = true;

		auto message = Message{
			.type          = Message::Type::Error,
			.source        = *this,
			.message       = msg,
			.line          = line,
			.collumn_start = collumn_start,
			.collumn_end   = collumn_end,
		};

		this->source_manager.emitMessage(message);
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
