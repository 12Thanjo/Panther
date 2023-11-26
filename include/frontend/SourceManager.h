#pragma once


#include <Evo.h>


namespace panther{


	class SourceManager{
		public:
			SourceManager() = default;
			~SourceManager() = default;


			struct SourceFileID{ uint32_t id; };


			EVO_NODISCARD auto addSourceFile(const std::string& location, std::string&& data) noexcept -> SourceFileID;


			struct SourceFile{
				std::string location;
				std::string data;
			};
			EVO_NODISCARD auto getSourceFile(SourceFileID id) const noexcept -> const SourceFile&;



			auto error(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void;
			auto fatal(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void;

	
		private:
			std::vector<SourceFile> sources{};

			bool has_errored = false;

	};

	
};
