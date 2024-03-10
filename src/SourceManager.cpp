#include "SourceManager.h"


namespace panther{


	auto SourceManager::addSource(const std::string& location, std::string&& data) noexcept -> Source::ID {
		evo::debugAssert(this->isLocked() == false, "Can only add sources to SourceManager when it is locked");

		const auto src_id = Source::ID( uint32_t(this->sources.size()) );

		this->sources.emplace_back(location, std::move(data), *this, src_id);

		return src_id;
	};



	//////////////////////////////////////////////////////////////////////
	// locked


	auto SourceManager::emitMessage(const Message& msg) const noexcept -> void {
		evo::debugAssert(this->isLocked(), "Can only emit messages when locked");


		this->message_callback(msg);
	};




	// auto SourceManager::getSource(Source::ID id) noexcept -> Source& {
	// 	evo::debugAssert(id.id < this->sources.size(), "Attempted to get invalid source file id");
	// 	evo::debugAssert(this->isLocked(), "Can only get sources when locked");

	// 	return this->sources[id.id];
	// };

	auto SourceManager::getSource(Source::ID id) const noexcept -> const Source& {
		evo::debugAssert(id.id < this->sources.size(), "Attempted to get invalid source file id");
		evo::debugAssert(this->isLocked(), "Can only get sources when locked");

		return this->sources[id.id];
	};




	// TODO: multithreading
	auto SourceManager::tokenize() noexcept -> evo::uint {
		evo::debugAssert(this->isLocked(), "Can only tokenize when locked");

		evo::uint total_fails = 0;

		for(Source& source : this->sources){
			if(source.tokenize() == false){
				total_fails += 1;
			}
		}


		return total_fails;
	};


	// TODO: multithreading
	auto SourceManager::parse() noexcept -> evo::uint {
		evo::debugAssert(this->isLocked(), "Can only parse when locked");

		evo::uint total_fails = 0;

		for(Source& source : this->sources){
			if(source.parse() == false){
				total_fails += 1;
			}
		}


		return total_fails;
	};


	auto SourceManager::initBuiltinTypes() noexcept -> void {
		evo::debugAssert(this->isLocked(), "Can only initialize builtin types when locked");


		/*object::BaseType& Int = */this->base_types.emplace_back(
			object::BaseType{
				.is_builtin = true,
				.builtin = Token::TypeInt,
			}
		);


		/*object::BaseType& Bool = */this->base_types.emplace_back(
			object::BaseType{
				.is_builtin = true,
				.builtin = Token::TypeBool,
			}
		);

	};



	// TODO: multithreading
	auto SourceManager::semanticAnalysis() noexcept -> evo::uint {
		evo::debugAssert(this->isLocked(), "Can only do semantic analysis when locked");


		evo::uint total_fails = 0;

		for(Source& source : this->sources){
			if(source.semantic_analysis() == false){
				total_fails += 1;
			}
		}


		return total_fails;
	};




	//////////////////////////////////////////////////////////////////////
	// objects

	auto SourceManager::getBaseType(Token::Kind tok_kind) noexcept -> object::BaseType& {
		for(object::BaseType& base_type : this->base_types){
			if(base_type == tok_kind){
				return base_type;
			}
		}

		EVO_FATAL_BREAK("Cannot get unknown builtin base type");
	};


	auto SourceManager::getBaseTypeID(Token::Kind tok_kind) const noexcept -> object::BaseType::ID {
		for(size_t i = 0; i < this->base_types.size(); i+=1){
			const object::BaseType& base_type = this->base_types[i];

			if(base_type == tok_kind){
				return object::BaseType::ID( uint32_t(i) );
			}
		}

		EVO_FATAL_BREAK("Cannot get unknown builtin base type");
	};



	auto SourceManager::getType(const object::Type& type) noexcept -> object::Type::ID {
		// find existing type
		for(size_t i = 0; i < this->types.size(); i+=1){
			const object::Type& type_ref = this->types[i];

			if(type_ref == type){
				return object::Type::ID( uint32_t(i) );
			}
		}


		// create new type
		this->types.emplace_back(type);
		return object::Type::ID( uint32_t(this->types.size() - 1) );
	};


	
};
