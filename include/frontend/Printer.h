#pragma once


#include <Evo.h>

namespace panther{
	

	class Printer{
		public:
			Printer(bool using_colors) : use_colors(using_colors) {};
			~Printer() = default;

			auto fatal(evo::CStrProxy message) const noexcept -> void;
			auto error(evo::CStrProxy message) const noexcept -> void;
			auto warning(evo::CStrProxy message) const noexcept -> void;
			auto info(evo::CStrProxy message) const noexcept -> void;
			auto debug(evo::CStrProxy message) const noexcept -> void;
			auto trace(evo::CStrProxy message) const noexcept -> void;

	
		private:
			bool use_colors;
	};


};