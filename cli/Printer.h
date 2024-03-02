#pragma once


#include <Evo.h>


#include "Source.h"

namespace panther{
	struct Message;


	namespace cli{


		class Printer{
			public:
				Printer(bool should_use_colors) : use_colors(should_use_colors) {};
				~Printer() = default;

				auto fatal(evo::CStrProxy msg) const noexcept -> void;
				auto error(evo::CStrProxy msg) const noexcept -> void;
				auto warning(evo::CStrProxy msg) const noexcept -> void;
				auto success(evo::CStrProxy msg) const noexcept -> void;
				auto info(evo::CStrProxy msg) const noexcept -> void;
				auto debug(evo::CStrProxy msg) const noexcept -> void;
				auto trace(evo::CStrProxy msg) const noexcept -> void;

				auto print(evo::CStrProxy msg) const noexcept -> void;


				EVO_NODISCARD inline auto usesColors() const noexcept -> bool { return this->use_colors; };


				auto print_message(const panther::Message& msg) const noexcept -> void;
				
				auto print_tokens(const Source& source) const noexcept -> void;

			private:
				bool use_colors;
		};
		

	};
};