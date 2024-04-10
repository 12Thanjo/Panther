#pragma once


#include <Evo.h>

namespace panther{


	class LLDInterface{
		public:
			enum class Linker{
				WinLink,
				Gnu,
				MinGW,
				Darwin,
				Wasm,
			};

			struct LinkerOutput{
				int returnCode;
				bool canRunAgain;
				std::vector<std::string> err_messages;

				EVO_NODISCARD inline auto succeeded() const noexcept -> bool {
					return this->returnCode == 0 && this->canRunAgain && this->err_messages.empty();
				};
			};

		public:
			LLDInterface() = default;
			~LLDInterface() = default;

			// auto init() noexcept -> void;

			EVO_NODISCARD auto link(const std::string& input_file_path, const std::string& target_output, Linker linker ) noexcept -> LinkerOutput;
	
		private:
			
	};


};