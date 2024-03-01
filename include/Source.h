#pragma once


#include <Evo.h>

#include "Token.h"


namespace panther{


	class Source{
		public:
			struct ID{ uint32_t id; };

		public:
			// TODO: other permutations of refs
			Source(const std::string& src_location, std::string&& src_data, class SourceManager& src_manager)
				: location(src_location), data(std::move(src_data)), source_manager(src_manager) {};


			EVO_NODISCARD inline auto getLocation() const noexcept -> const std::string& { return this->location; };
			EVO_NODISCARD inline auto getData() const noexcept -> const std::string& { return this->data; };
			EVO_NODISCARD inline auto getSourceManager() const noexcept -> const SourceManager& { return this->source_manager; };


			// returns if successful (no errors)
			EVO_NODISCARD auto tokenize() noexcept -> bool;




			///////////////////////////////////
			// messaging / errors

			auto fatal(const std::string& msg, uint32_t line, uint32_t collumn) noexcept -> void;
			auto fatal(const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void;


			auto error(const std::string& msg, uint32_t line, uint32_t collumn) noexcept -> void;
			auto error(const std::string& msg, uint32_t line, uint32_t collumn, std::vector<std::string>&& infos) noexcept -> void;
			auto error(const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void;
			auto error(
				const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end, std::vector<std::string>&& infos
			) noexcept -> void;




			EVO_NODISCARD inline auto hasErrored() const noexcept -> bool { return this->has_errored; };

		public:
			std::vector<Token> tokens{};


		private:
			std::string location; 
			std::string data;
			class SourceManager& source_manager;

			bool has_errored = false;
	};


};