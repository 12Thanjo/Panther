#pragma once


#include <Evo.h>

namespace panther{

	struct Location{
		uint32_t line;
		uint32_t collumn_start;
		uint32_t collumn_end;
	};


	struct Message{
		enum class Type{
			Fatal,
			Error,
			Warning,
			Info,
		};


		Type type;
		const class Source* source;
		const std::string message;

		Location location;


		struct Info{
			std::string string;
			std::optional<Location> location;

			Info(const std::string& str) : string(str), location(std::nullopt) {};
			Info(std::string&& str) : string(std::move(str)), location(std::nullopt) {};

			Info(const std::string& str, Location loc) : string(str), location(loc) {};
			Info(std::string&& str, Location loc) : string(std::move(str)), location(loc) {};
		};

		std::vector<Info> infos{};
	};


};