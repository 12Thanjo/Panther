#pragma once


#include <Evo.h>

namespace panther{

	
	class Indenter{
		public:
			Indenter() = default;
			~Indenter() = default;


			auto push() noexcept -> void;
			auto pop() noexcept -> void;
			auto set_arrow() noexcept -> void;
			auto set_end() noexcept -> void;


			auto print() noexcept -> void;

	
		private:
			enum class Type{
				Line,
				Arrow,
				EndArrow,
				None,
			};

			std::vector<Type> indents{};
	};


};