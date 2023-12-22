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


			auto error(
				const std::string& message, SourceFileID id, uint32_t line_start, uint32_t collumn_start, uint32_t line, uint32_t collumn
			) noexcept -> void;
			auto error(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void;

			auto error_info(const std::string& message) noexcept -> void;
			auto error_info(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void;

			auto fatal(
				const std::string& message, SourceFileID id, uint32_t line_start, uint32_t collumn_start, uint32_t line, uint32_t collumn
			) noexcept -> void;
			auto fatal(const std::string& message, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void;


			inline auto errored() const noexcept -> bool { return this->has_errored; };


		private:
			enum class MessageType{
				Error,
				ErrorInfo,
			};
			auto print_location(MessageType type, SourceFileID id, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void;

			auto print_location(MessageType type, SourceFileID id, uint32_t line, uint32_t collumn) noexcept -> void {
				this->print_location(type, id, line, collumn, collumn);
			};


		private:
			std::vector<SourceFile> sources{};

			bool has_errored = false;

	};

	
};
