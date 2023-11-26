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
			CharStream(const SourceManager& source_manager_ref, SourceManager::SourceFileID source_file_id)
				: src_manager(source_manager_ref), src_id(source_file_id), src(this->src_manager.getSourceFile(this->src_id)) {};
			~CharStream() = default;


			EVO_NODISCARD auto peek(int64_t offset) noexcept -> char;

			EVO_NODISCARD auto next() noexcept -> char;



			EVO_NODISCARD inline auto is_eof() const noexcept -> bool { return this->cursor >= int64_t(this->src.data.size()); };


			EVO_NODISCARD inline auto get_line() const noexcept -> uint32_t { return this->line; };
			EVO_NODISCARD inline auto get_collumn() const noexcept -> uint32_t { return this->collumn; };

	
		private:
			// TODO: maybe remove these
			const SourceManager& src_manager;
			SourceManager::SourceFileID src_id;



			const SourceManager::SourceFile& src;

			int64_t cursor = 0;
			uint32_t line = 1;
			uint32_t collumn = 1;
	};


};
