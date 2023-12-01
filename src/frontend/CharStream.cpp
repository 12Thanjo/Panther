#include "frontend/CharStream.h"


namespace panther{


	auto CharStream::peek(int64_t offset) noexcept -> char {
		evo::debugAssert(offset > 0 || (offset * -1) <= this->cursor, "CharStream peek+offset is less than 0");

		const size_t target_location = size_t(this->cursor + int64_t(offset));

		evo::debugAssert(target_location < this->src.data.size(), "CharStream peek+offset is larger than src data");


		return this->src.data[target_location];
	};



	auto CharStream::next() noexcept -> char {
		evo::debugAssert(this->cursor < int64_t(this->src.data.size()), "CharStream cannot get next char - already at EOF");

		const char current_char = this->peek();


		if(current_char == '\n'){
			this->current_line += 1;
			this->current_collumn = 1;

		}else if(current_char == '\r'){
			this->current_line += 1;
			this->current_collumn = 1;

			const bool is_at_end = this->cursor + 1 >= int64_t(this->src.data.size());
			if(is_at_end == false && this->peek(1) == '\n'){
				this->cursor += 1;	
			}

		}else{
			this->current_collumn += 1;
		}


		this->cursor += 1;

		return current_char;
	};


	auto CharStream::skip(unsigned ammount) noexcept -> void {
		evo::debugAssert(this->ammount_left() >= ammount, std::format("CharStream cannot skip {} chars - goes past EOF", ammount));

		char discard;
		while(ammount > 0){
			discard = this->next();
			ammount -= 1;
		};
	}





	auto CharStream::error(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void {
		this->src_manager.error(message, this->src_id, line, collumn);
	};

	auto CharStream::error_info(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void {
		this->src_manager.error_info(message, this->src_id, line, collumn);
	};

	auto CharStream::fatal(const std::string& message, uint32_t line, uint32_t collumn) const noexcept -> void {
		this->src_manager.fatal(message, this->src_id, line, collumn);
	};


	
};
