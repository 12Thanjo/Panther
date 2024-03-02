#pragma once


#include <Evo.h>


#include "Source.h"

#include <functional>

namespace panther{

	struct Message{
		enum class Type{
			Fatal,
			Error,
			Warning,
		};


		Type type;
		const Source& source;
		const std::string message;

		uint32_t line;
		uint32_t collumn_start;
		uint32_t collumn_end;

		std::vector<std::string> infos{};
	};


	class SourceManager{
		public:
			using MessageCallback = std::function<void(const Message&)>;

			SourceManager(MessageCallback msg_callback) : message_callback(msg_callback) {};
			~SourceManager() = default;


			EVO_NODISCARD inline auto numSources() const noexcept -> size_t { return this->sources.size(); };


			// The purpose of locking is to make sure no sources are added after doing things like tokenizing
			// 	this is to ensure that pointers/references still point to the right place

			EVO_NODISCARD inline auto lock() noexcept -> void {
				evo::debugAssert(this->is_locked == false, "SourceManager is already locked");

				this->is_locked = true;
			};

			EVO_NODISCARD inline auto isLocked() const noexcept -> bool { return this->is_locked; };


			//////////////////////////////////////////////////////////////////////
			// funcs that require it is unlocked

			// TODO: other permutations of refs
			EVO_NODISCARD auto addSource(const std::string& location, std::string&& data) noexcept -> Source::ID;



			//////////////////////////////////////////////////////////////////////
			// funcs that require it to be locked


			EVO_NODISCARD auto emitMessage(const Message& msg) const noexcept -> void;


			// EVO_NODISCARD auto getSource(Source::ID id)       noexcept ->       Source&;
			EVO_NODISCARD auto getSource(Source::ID id) const noexcept -> const Source&;



			// returns if tokenizing all sources was successful
			EVO_NODISCARD auto tokenize() noexcept -> evo::uint;


			// returns if parsing all sources was successful
			EVO_NODISCARD auto parse() noexcept -> evo::uint;


		private:
			std::vector<Source> sources{};
			bool is_locked = false;

			MessageCallback message_callback;

	};


};