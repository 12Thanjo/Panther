#include "./Parser.h"


namespace panther{


	auto Parser::parse() noexcept -> bool {
		return true;
	};



	auto Parser::parse_stmt() noexcept -> Result<AST::NodeID> {
		return ResultCode::WrongType;
	};


	
};
