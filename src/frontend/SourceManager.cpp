#include "frontend/SourceManager.h"



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


	auto SourceManager::getSources() noexcept -> std::vector<Source>& {
		evo::debugAssert(this->isLocked(), "Can only get sources when locked");

		return this->sources;
	};

	auto SourceManager::getSources() const noexcept -> const std::vector<Source>& {
		evo::debugAssert(this->isLocked(), "Can only get sources when locked");

		return this->sources;
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


		/*PIR::BaseType& Int = */this->base_types.emplace_back(PIR::BaseType::Kind::Builtin, Token::TypeInt);


		/*PIR::BaseType& Bool = */this->base_types.emplace_back(PIR::BaseType::Kind::Builtin, Token::TypeBool);

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

	auto SourceManager::createBaseType(PIR::BaseType&& base_type) noexcept -> PIR::BaseType::ID {
		for(size_t i = 0; i < this->base_types.size(); i+=1){
			if(base_type == this->base_types[i]){
				return PIR::BaseType::ID( uint32_t(i) );
			}
		}


		this->base_types.emplace_back(std::move(base_type));
		return PIR::BaseType::ID( uint32_t(this->base_types.size() - 1) );
	};


	auto SourceManager::getBaseType(Token::Kind tok_kind) noexcept -> PIR::BaseType& {
		for(PIR::BaseType& base_type : this->base_types){
			if(base_type == tok_kind){
				return base_type;
			}
		}

		EVO_FATAL_BREAK("Cannot get unknown builtin base type");
	};


	auto SourceManager::getBaseTypeID(Token::Kind tok_kind) const noexcept -> PIR::BaseType::ID {
		for(size_t i = 0; i < this->base_types.size(); i+=1){
			const PIR::BaseType& base_type = this->base_types[i];

			if(base_type == tok_kind){
				return PIR::BaseType::ID( uint32_t(i) );
			}
		}

		EVO_FATAL_BREAK("Cannot get unknown builtin base type");
	};



	auto SourceManager::getTypeID(const PIR::Type& type) noexcept -> PIR::Type::ID {
		// find existing type
		for(size_t i = 0; i < this->types.size(); i+=1){
			const PIR::Type& type_ref = this->types[i];

			if(type_ref == type){
				return PIR::Type::ID( uint32_t(i) );
			}
		}


		// create new type
		this->types.emplace_back(type);
		return PIR::Type::ID( uint32_t(this->types.size() - 1) );
	};



	auto SourceManager::printType(PIR::Type::ID id) const noexcept -> std::string {
		const PIR::Type& type = this->getType(id);
		const PIR::BaseType& base_type = this->base_types[type.base_type.id];


		std::string base_type_str = [&]() noexcept {
			if(base_type.kind == PIR::BaseType::Kind::Builtin){
				return Token::printKind(base_type.builtin.kind);

			}else{
				// TODO:
				EVO_FATAL_BREAK("Printing of non-builtin types are not supported yet");
			}
		}();
		

		return base_type_str;
	};




	auto SourceManager::addEntry(Source::ID src_id, PIR::Func::ID func_id) noexcept -> void {
		evo::debugAssert(this->isLocked(), "Can only do add entry when locked");
		evo::debugAssert(this->hasEntry() == false, "Already has an entry function");

		this->entry = Entry(src_id, func_id);
	};


	auto SourceManager::getEntry() const noexcept -> Entry {
		evo::debugAssert(this->isLocked(), "Can only do add entry when locked");
		evo::debugAssert(this->hasEntry(), "SourceManager does not have an entry");

		return *this->entry;
	};


	
};
