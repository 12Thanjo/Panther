#include "objects.h"



namespace panther{
	namespace object{
		

		auto BaseType::operator==(Token::Kind kind) const noexcept -> bool {
			if(this->is_builtin == false){ return false; }

			return kind == this->builtin;
		};




		auto Type::operator==(const Type& rhs) const noexcept -> bool {
			return this->base_type.id == rhs.base_type.id;
		};



	};
};
