#include "frontend/SourceManager.h"


namespace panther{


	auto SourceManager::addSourceFile(const std::string& location, std::string&& data) noexcept -> SourceFileID {
		this->sources.emplace_back(location, std::move(data));

		return SourceFileID{uint32_t(this->sources.size()) - 1};
	};



	auto SourceManager::getSourceFile(SourceFileID id) const noexcept -> const SourceFile& {
		evo::debugAssert(id.id < this->sources.size(), "Attempted to get invalid source file id");

		return this->sources.front();
	};




	auto SourceManager::error(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void {
		this->has_errored = true;

		evo::logError(message);
	};


	auto SourceManager::fatal(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void {
		this->has_errored = true;

		evo::logFatal(message);
		evo::logFatal("This is an error in the compiler");
	};

	
};
