#pragma once


#include <Evo.h>


#include "frontend/Printer.h"

namespace panther{

	
	class Indenter{
		public:
			Indenter(const Printer& _printer) : printer(_printer) {};
			~Indenter() = default;


			auto push() noexcept -> void;
			auto pop() noexcept -> void;
			auto set_arrow() noexcept -> void;
			auto set_end() noexcept -> void;


			auto print() noexcept -> void;

	
		private:
			const Printer& printer;

			enum class Type{
				Line,
				Arrow,
				EndArrow,
				None,
			};

			std::vector<Type> indents{};
	};


};