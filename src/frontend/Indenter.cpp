#include "./Indenter.h"


namespace panther{


	auto Indenter::push() noexcept -> void {
		this->indents.push_back(Type::Arrow);
	};


	auto Indenter::pop() noexcept -> void {
		this->indents.pop_back();
	};


	auto Indenter::set_arrow() noexcept -> void {
		this->indents.back() = Type::Arrow;
	};


	auto Indenter::set_end() noexcept -> void {
		this->indents.back() = Type::EndArrow;	
	};



	auto Indenter::print() noexcept -> void {
		auto print_string = std::string{};

		for(const Type& indent : this->indents){
			switch(indent){
				break; case Type::Line:     print_string += "|   ";
				break; case Type::Arrow:    print_string += "|-> ";
				break; case Type::EndArrow: print_string += "\\-> ";
				break; case Type::None:     print_string += "    ";
			};
		}


		this->printer.trace(print_string);


		// change types
		for(Type& indent : this->indents){
			switch(indent){
				break; case Type::Line:     break;
				break; case Type::Arrow:    indent = Type::Line;
				break; case Type::EndArrow: indent = Type::None;
				break; case Type::None:     break;
			};
		}
	};


};
