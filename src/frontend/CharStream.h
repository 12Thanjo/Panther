#pragma once


#include <Evo.h>

namespace panther{


	class CharStream{
		public:
			CharStream(std::string_view source_data) : data(source_data), data_size(source_data.size()) {};
			~CharStream() = default;


			EVO_NODISCARD auto peek(ptrdiff_t offset = 0) const noexcept -> char;
			EVO_NODISCARD auto next() noexcept -> char;
			auto skip(size_t ammount) noexcept -> void;

			
			EVO_NODISCARD inline auto eof() const noexcept -> bool { return this->cursor >= ptrdiff_t(this->size()); }; 

			EVO_NODISCARD inline auto size() const noexcept -> size_t { return this->data_size; };
			EVO_NODISCARD inline auto ammount_left() const noexcept -> size_t { return this->size() - size_t(this->cursor); };

			EVO_NODISCARD inline auto get_raw_ptr() const noexcept -> const char* { return &this->data[this->cursor]; };
			EVO_NODISCARD inline auto get_line() const noexcept -> uint32_t { return this->line; };
			EVO_NODISCARD inline auto get_collumn() const noexcept -> uint32_t { return this->collumn; };

	
		private:
			std::string_view data;
			size_t data_size;

			ptrdiff_t cursor = 0;
			uint32_t line = 1;
			uint32_t collumn = 1;
	};


};