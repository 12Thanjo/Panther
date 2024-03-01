#include "./CharStream.h"


namespace panther{

	auto CharStream::peek(ptrdiff_t offset) const noexcept -> char {
		evo::debugAssert(this->cursor + offset < ptrdiff_t(this->size()), "Peek offset is out of bounds of the data string");

		return this->data[this->cursor + offset];
	};


	auto CharStream::next() noexcept -> char {
		evo::debugAssert(this->eof() == false, "CharStream already eof");

		const char current_char = this->peek();


		if(current_char == '\n'){
			this->line += 1;
			this->collumn = 1;

		}else if(current_char == '\r'){
			this->line += 1;
			this->collumn = 1;

			const bool is_at_end = this->cursor + 1 >= ptrdiff_t(this->size());
			if(is_at_end == false && this->peek(1) == '\n'){
				this->cursor += 1;	
			}

		}else{
			this->collumn += 1;
		}


		this->cursor += 1;

		return current_char;
	};




	auto CharStream::skip(size_t ammount) noexcept -> void {
		evo::debugAssert(this->cursor + ptrdiff_t(ammount) - 1 < ptrdiff_t(this->size()), "Skip is out of bounds of the data string");
		evo::debugAssert(ammount > 0, "Skip should be greater than 0");

		char discarded_return;
		while(ammount > 0){
			discarded_return = this->next();
			ammount -= 1;
		};
	};


	
};
