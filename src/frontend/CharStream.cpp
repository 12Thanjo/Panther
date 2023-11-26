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
			this->line += 1;
			this->collumn = 1;

		}else if(current_char == '\r'){
			this->line += 1;
			this->collumn = 1;

			const bool is_at_end = this->cursor + 1 >= int64_t(this->src.data.size());
			if(is_at_end == false && this->peek(1) == '\n'){
				this->cursor += 1;	
			}

		}else{
			this->collumn += 1;
		}


		this->cursor += 1;

		return current_char;
	};


	
};
