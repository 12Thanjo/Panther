#pragma once

///////////////////////////////////
// 								 //
// meant for internal use		 //
//								 //
///////////////////////////////////

#include <Evo.h>


#include "./SourceManager.h"


namespace panther{


	class CharStream{
		public:
			CharStream(SourceManager& source_manager_ref, SourceManager::SourceFileID source_file_id)
				: src_manager(source_manager_ref), src_id(source_file_id), src(this->src_manager.getSourceFile(this->src_id)) {};
			~CharStream() = default;


			EVO_NODISCARD auto peek(int64_t offset = 0) noexcept -> char;

			EVO_NODISCARD auto next() noexcept -> char;
			auto skip(unsigned ammount) noexcept -> void;



			EVO_NODISCARD inline auto is_eof() const noexcept -> bool { return this->cursor >= int64_t(this->src.data.size()); };
			EVO_NODISCARD inline auto ammount_left() const noexcept -> size_t { return this->src.data.size() - size_t(this->cursor); };


			EVO_NODISCARD inline auto get_line() const noexcept -> uint32_t { return this->current_line; };
			EVO_NODISCARD inline auto get_collumn() const noexcept -> uint32_t { return this->current_collumn; };



			auto error(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void;
			auto error_info(const std::string& message) const noexcept -> void;
			auto error_info(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void;
			auto fatal(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void;


			inline auto get_source_manager() const noexcept -> SourceManager& { return this->src_manager; };

	
		private:
			SourceManager& src_manager;
			SourceManager::SourceFileID src_id;


			const SourceManager::SourceFile& src;

			int64_t cursor = 0;
			uint32_t current_line = 1;
			uint32_t current_collumn = 1;
	};


};
