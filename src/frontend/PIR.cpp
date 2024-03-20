#include "frontend/PIR.h"



namespace panther{
	namespace PIR{

		auto BaseType::Operator::operator==(const Operator& rhs) const noexcept -> bool {
			return this->params == rhs.params && this->return_type == rhs.return_type;
		};

		

		auto BaseType::operator==(Token::Kind tok_kind) const noexcept -> bool {
			if(this->kind != Kind::Builtin){ return false; }

			return tok_kind == this->builtin.kind;
		};



		auto BaseType::operator==(const BaseType& rhs) const noexcept -> bool {
			if(this->kind != rhs.kind){ return false; }

			if(this->kind == BaseType::Kind::Builtin){
				if(this->builtin.kind != rhs.builtin.kind){ return false; }

			}else if(this->kind == BaseType::Kind::Function){
				if(this->call_operators != rhs.call_operators){ return false; }
				
			}




			return true;
		};




		auto Type::operator==(const Type& rhs) const noexcept -> bool {
			return this->base_type.id == rhs.base_type.id;
		};



	};
};
