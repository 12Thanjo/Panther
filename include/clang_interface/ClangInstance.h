#pragma once


#include <Evo.h>

namespace panther{


	class ClangInstance{
		public:
			ClangInstance() = default;
			~ClangInstance() = default;

			auto init() noexcept -> void;

			auto run() noexcept -> bool;
	
		private:
			
	};


};