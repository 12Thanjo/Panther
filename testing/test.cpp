#include <iostream>
#include <cstdint>

extern "C" {
	auto P.0.set_global_to_14() -> void;
	auto get_global() -> uint64_t;
}


auto main() -> int {

	std::cout << "value of global: " << get_global() << std::endl;

	set_global_to_14();

	std::cout << "value of global: " << get_global() << std::endl;	


	return 0;
}



