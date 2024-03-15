#pragma once


#include <Evo.h>


#include "Source.h"
#include "objects.h"
#include "Message.h"

#include <functional>

namespace panther{


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

			EVO_NODISCARD auto getSources() noexcept -> std::vector<Source>&;
			EVO_NODISCARD auto getSources() const noexcept -> const std::vector<Source>&;



			// returns number of sources taht failed tokenizing
			EVO_NODISCARD auto tokenize() noexcept -> evo::uint;

			// returns number of sources taht failed parsing
			EVO_NODISCARD auto parse() noexcept -> evo::uint;

			auto initBuiltinTypes() noexcept -> void;

			// returns number of sources taht failed parsing
			EVO_NODISCARD auto semanticAnalysis() noexcept -> evo::uint;





			///////////////////////////////////
			// objects

			EVO_NODISCARD inline auto getBaseType(object::BaseType::ID id) const noexcept -> const object::BaseType& {
				return this->base_types[id.id];
			};

			EVO_NODISCARD auto getBaseType(Token::Kind tok_kind) noexcept -> object::BaseType&;
			EVO_NODISCARD auto getBaseTypeID(Token::Kind tok_kind) const noexcept -> object::BaseType::ID;


			// gets type with matching ID
			// if type doesn't already exist, create a new one
			// TODO: &&
			EVO_NODISCARD auto getTypeID(const object::Type& type) noexcept -> object::Type::ID;

			EVO_NODISCARD inline auto getType(object::Type::ID id) const noexcept -> const object::Type& {
				return this->types[id.id];
			};


			EVO_NODISCARD auto printType(object::Type::ID id) const noexcept -> std::string;



		private:
			std::vector<Source> sources{};
			bool is_locked = false;

			std::vector<object::BaseType> base_types{};
			std::vector<object::Type> types{};


			MessageCallback message_callback;

	};


};