#include "frontend/Printer.h"



namespace panther{
	
	auto Printer::fatal(evo::CStrProxy message) const noexcept -> void {
		if(this->use_colors){ evo::styleConsoleFatal(); }
		evo::log(message);
		if(this->use_colors){ evo::styleConsoleReset(); }
	};

	auto Printer::error(evo::CStrProxy message) const noexcept -> void {
		if(this->use_colors){ evo::styleConsoleError(); }
		evo::log(message);
		if(this->use_colors){ evo::styleConsoleReset(); }
	};

	auto Printer::warning(evo::CStrProxy message) const noexcept -> void {
		if(this->use_colors){ evo::styleConsoleWarning(); }
		evo::log(message);
		if(this->use_colors){ evo::styleConsoleReset(); }
	};

	auto Printer::info(evo::CStrProxy message) const noexcept -> void {
		if(this->use_colors){ evo::styleConsoleInfo(); }
		evo::log(message);
		if(this->use_colors){ evo::styleConsoleReset(); }
	};

	auto Printer::debug(evo::CStrProxy message) const noexcept -> void {
		if(this->use_colors){ evo::styleConsoleDebug(); }
		evo::log(message);
		if(this->use_colors){ evo::styleConsoleReset(); }
	};

	auto Printer::trace(evo::CStrProxy message) const noexcept -> void {
		if(this->use_colors){ evo::styleConsoleTrace(); }
		evo::log(message);
		if(this->use_colors){ evo::styleConsoleReset(); }
	};


};