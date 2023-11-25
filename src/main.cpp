
#include <evo.h>
#include <iostream>


auto main([[maybe_unused]] int argc, [[maybe_unused]] const char* args[]) noexcept -> int {
	evo::log("Panther :\n");



	evo::logTrace("Press Enter to close...");
	std::cin.get();

	return 0;
}