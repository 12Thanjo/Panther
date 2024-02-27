#include <Evo.h>
#include <iostream>








auto main([[maybe_unused]] int argc, [[maybe_unused]] const char* args[]) noexcept -> int {
	evo::logInfo("Panther:");

	

	evo::logTrace("Press Enter to close...\n");
	std::cin.get();

	return 0;
}