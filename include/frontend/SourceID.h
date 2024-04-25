#pragma once


//////////////////////////////////////////////////////////////////////
// 																	//
// 	This file just exists so Source::ID could be used in PIR.h      //
// 		User does not have to inlcude this							//
// 																	//
//////////////////////////////////////////////////////////////////////



namespace panther{


	struct SourceID{ // typesafe identifier
		uint32_t id;
		explicit SourceID(uint32_t _id) noexcept : id(_id) {};
	};


};