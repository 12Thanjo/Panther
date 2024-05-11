#include "frontend/SourceManager.h"



namespace panther{


	auto SourceManager::addSource(std::filesystem::path&& location, std::string&& data) noexcept -> Source::ID {
		evo::debugAssert(this->isLocked() == false, "Can only add sources to SourceManager when it is locked");

		const auto src_id = Source::ID( uint32_t(this->sources.size()) );

		this->sources.emplace_back(std::move(location), std::move(data), *this, src_id);

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


	auto SourceManager::getSourceID(const std::filesystem::path& file_location, std::string_view src_path) const noexcept
		-> evo::Expected<Source::ID, GetSourceIDError>
	{
		// check is valid path
		if(src_path.empty()){ return evo::Unexpected(GetSourceIDError::EmptyPath); }

		std::filesystem::path relative_dir = file_location;
		relative_dir.remove_filename();

		// generate path
		const std::filesystem::path lookup_path = [&]() noexcept {
			if(src_path.starts_with("./")){
				return relative_dir / std::filesystem::path(src_path.substr(2));

			}else if(src_path.starts_with(".\\")){
				return relative_dir / std::filesystem::path(src_path.substr(3));

			// }else if(src_path.starts_with("/") || src_path.starts_with("\\")){
			// 	return relative_dir / std::filesystem::path(src_path.substr(1));

			}else if(src_path.starts_with("../") || src_path.starts_with("..\\")){
				return relative_dir / std::filesystem::path(src_path);

			}else{
				return this->config.basePath / std::filesystem::path(src_path);
			}
		}().lexically_normal();

		if(file_location == lookup_path){
			return evo::Unexpected(GetSourceIDError::SameAsCaller);
		}

		// look for path
		for(size_t i = 0; i < this->sources.size(); i+=1){
			const Source& source = this->sources[i];

			if(source.getLocation() == lookup_path){
				return Source::ID(uint32_t(i));
			}
		}


		if(evo::fs::exists(lookup_path.string())){
			return evo::Unexpected(GetSourceIDError::NotOneOfSources);			
		}

		return evo::Unexpected(GetSourceIDError::DoesntExist);
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

		// Import (0)
		this->base_types.emplace_back(PIR::BaseType::Kind::Import);
		this->types.emplace_back( PIR::Type(PIR::BaseType::ID(0)) );

		// Int (1)
		this->base_types.emplace_back(PIR::BaseType::Kind::Builtin, Token::TypeInt);
		this->types.emplace_back( PIR::Type(PIR::BaseType::ID(1)) );

		// UInt (2)
		this->base_types.emplace_back(PIR::BaseType::Kind::Builtin, Token::TypeUInt);
		this->types.emplace_back( PIR::Type(PIR::BaseType::ID(2)) );

		// Bool (3)
		this->base_types.emplace_back(PIR::BaseType::Kind::Builtin, Token::TypeBool);
		this->types.emplace_back( PIR::Type(PIR::BaseType::ID(3)) );

		// String (4)
		this->base_types.emplace_back(PIR::BaseType::Kind::Builtin, Token::TypeString);
		this->types.emplace_back( PIR::Type(PIR::BaseType::ID(4)) );
	};


	// TODO: optimize base types
	auto SourceManager::initIntrinsics() noexcept -> void {
		evo::debugAssert(this->isLocked(), "Can only initialize intrinsics when locked");
		using ParamKind = AST::FuncParams::Param::Kind;


		///////////////////////////////////
		// setup protos

		const PIR::BaseType::ID math_ops_Int_type = [&]() noexcept {
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeInt(), ParamKind::Read}, {this->getTypeInt(), ParamKind::Read}, },
				this->getTypeInt()
			);

			return this->createBaseType(std::move(base_type));
		}();

		const PIR::BaseType::ID math_ops_UInt_type = [&]() noexcept {
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeUInt(), ParamKind::Read}, {this->getTypeUInt(), ParamKind::Read}, },
				this->getTypeUInt()
			);

			return this->createBaseType(std::move(base_type));
		}();


		const PIR::BaseType::ID logical_Int_type = [&]() noexcept {
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeInt(), ParamKind::Read}, {this->getTypeInt(), ParamKind::Read}, },
				this->getTypeBool()
			);

			return this->createBaseType(std::move(base_type));
		}();
		const PIR::BaseType::ID logical_UInt_type = [&]() noexcept {
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeUInt(), ParamKind::Read}, {this->getTypeUInt(), ParamKind::Read}, },
				this->getTypeBool()
			);

			return this->createBaseType(std::move(base_type));
		}();
		const PIR::BaseType::ID logical_Bool_type = [&]() noexcept {
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeBool(), ParamKind::Read}, {this->getTypeBool(), ParamKind::Read}, },
				this->getTypeBool()
			);

			return this->createBaseType(std::move(base_type));
		}();


		const PIR::BaseType::ID Int_in_Int_out_type = [&]() noexcept {
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeInt(), ParamKind::Read} },
				this->getTypeInt()
			);

			return this->createBaseType(std::move(base_type));
		}();
		const PIR::BaseType::ID Bool_in_Bool_out_type = [&]() noexcept {
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeBool(), ParamKind::Read} },
				this->getTypeBool()
			);

			return this->createBaseType(std::move(base_type));
		}();


		const PIR::BaseType::ID no_in_no_out_type = [&]() noexcept {
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(std::vector<PIR::BaseType::Operator::Param>{}, PIR::Type::VoidableID::Void());

			return this->createBaseType(std::move(base_type));
		}();



		const PIR::BaseType::ID import_type = [&]() noexcept {
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeString(), AST::FuncParams::Param::Kind::Read} },
				this->getTypeImport()
			);

			return this->createBaseType(std::move(base_type));
		}();



		//////////////////////////////////////////////////////////////////////
		// 																	//
		// IMPORTANT: these must be in the same order as 					//
		// 		defined in PIR::Intrinsic::Kind								//
		// 																	//
		//////////////////////////////////////////////////////////////////////


		///////////////////////////////////
		// misc

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::import, "import", import_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::breakpoint, "breakpoint", no_in_no_out_type);


		///////////////////////////////////
		// math

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addInt, "addInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addUInt, "addUInt", math_ops_UInt_type);

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addWrapInt, "addWrapInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addWrapUInt, "addWrapUInt", math_ops_UInt_type);


		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subInt, "subInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subUInt, "subUInt", math_ops_UInt_type);

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subWrapInt, "subWrapInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subWrapUInt, "subWrapUInt", math_ops_UInt_type);


		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulInt, "mulInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulUInt, "mulUInt", math_ops_UInt_type);
		
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulWrapInt, "mulWrapInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulWrapUInt, "mulWrapUInt", math_ops_UInt_type);


		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::divInt, "divInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::divUInt, "divUInt", math_ops_UInt_type);



		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::negateInt, "negateInt", Int_in_Int_out_type);


		///////////////////////////////////
		// logical

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::equalInt, "equalInt", logical_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::notEqualInt, "notEqualInt", logical_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::lessThanInt, "lessThanInt", logical_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::lessThanEqualInt, "lessThanEqualInt", logical_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::greaterThanInt, "greaterThanInt", logical_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::greaterThanEqualInt, "greaterThanEqualInt", logical_Int_type);


		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::equalUInt, "equalUInt", logical_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::notEqualUInt, "notEqualUInt", logical_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::lessThanUInt, "lessThanUInt", logical_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::lessThanEqualUInt, "lessThanEqualUInt", logical_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::greaterThanUInt, "greaterThanUInt", logical_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::greaterThanEqualUInt, "greaterThanEqualUInt", logical_UInt_type);

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::equalBool, "equalBool", logical_Bool_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::notEqualBool, "notEqualBool", logical_Bool_type);
		
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::logicalAnd, "logicalAnd", logical_Bool_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::logicalOr, "logicalOr", logical_Bool_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::logicalNot, "logicalNot", Bool_in_Bool_out_type);


		///////////////////////////////////
		// temporary

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::__printHelloWorld, "__printHelloWorld", no_in_no_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::__printSeparator, "__printSeparator", no_in_no_out_type);



		{ // __printInt()
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeInt(), ParamKind::Read} },
				PIR::Type::VoidableID::Void()
			);

			const PIR::BaseType::ID base_type_id = this->createBaseType(std::move(base_type));
			this->intrinsics.emplace_back(PIR::Intrinsic::Kind::__printInt, "__printInt", base_type_id);
		}

		{ // __printUInt()
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeUInt(), ParamKind::Read} },
				PIR::Type::VoidableID::Void()
			);

			const PIR::BaseType::ID base_type_id = this->createBaseType(std::move(base_type));
			this->intrinsics.emplace_back(PIR::Intrinsic::Kind::__printUInt, "__printUInt", base_type_id);
		}



		///////////////////////////////////
		// debug checking of ordering

		#if defined(PANTHER_CONFIG_DEBUG)
			for(uint32_t i = 0; i < uint32_t(PIR::Intrinsic::Kind::_MAX_); i+=1){
				const PIR::Intrinsic::Kind i_kind = static_cast<PIR::Intrinsic::Kind>(i);
				const PIR::Intrinsic& intrinsic = this->getIntrinsic(i_kind);
				evo::Assert(intrinsic.kind == i_kind, "found an intrinsic with incorrect cooresponding kind");
			}
		#endif


		//////////////////////////////////////////////////////////////////////
		// set operators

		PIR::BaseType& type_Int = this->getBaseType(Token::TypeInt);
		PIR::BaseType& type_UInt = this->getBaseType(Token::TypeUInt);
		PIR::BaseType& type_Bool = this->getBaseType(Token::TypeBool);


		///////////////////////////////////
		// math

		type_Int.ops.add.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addInt));
		type_UInt.ops.add.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addUInt));

		type_Int.ops.addWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addWrapInt));
		type_UInt.ops.addWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addWrapUInt));


		type_Int.ops.sub.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subInt));
		type_UInt.ops.sub.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subUInt));

		type_Int.ops.subWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subWrapInt));
		type_UInt.ops.subWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subWrapUInt));


		type_Int.ops.mul.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulInt));
		type_UInt.ops.mul.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulUInt));

		type_Int.ops.mulWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulWrapInt));
		type_UInt.ops.mulWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulWrapUInt));


		type_Int.ops.div.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::divInt));
		type_UInt.ops.div.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::divUInt));


		type_Int.ops.negate.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::negateInt));


		///////////////////////////////////
		// logical

		type_Int.ops.logicalEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::equalInt));
		type_UInt.ops.logicalEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::equalUInt));
		type_Bool.ops.logicalEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::equalBool));

		type_Int.ops.notEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::notEqualInt));
		type_UInt.ops.notEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::notEqualUInt));
		type_Bool.ops.notEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::notEqualBool));

		type_Int.ops.lessThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanInt));
		type_UInt.ops.lessThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanUInt));

		type_Int.ops.lessThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanEqualInt));
		type_UInt.ops.lessThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanEqualUInt));

		type_Int.ops.greaterThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanInt));
		type_UInt.ops.greaterThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanUInt));

		type_Int.ops.greaterThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanEqualInt));
		type_UInt.ops.greaterThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanEqualUInt));


		type_Bool.ops.logicalNot.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::logicalNot));
		type_Bool.ops.logicalAnd.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::logicalAnd));
		type_Bool.ops.logicalOr.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::logicalOr));



	};



	// TODO: multithreading
	auto SourceManager::semanticAnalysis() noexcept -> evo::uint {
		evo::debugAssert(this->isLocked(), "Can only do semantic analysis when locked");


		evo::uint total_fails = 0;

		for(Source& source : this->sources){
			if(source.semantic_analysis_declarations() == false){
				total_fails += 1;
			}
		}

		if(total_fails != 0){ return total_fails; }

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



	auto SourceManager::getOrCreateTypeID(const PIR::Type& type) noexcept -> PIR::Type::ID {
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

	auto SourceManager::getTypeID(const PIR::Type& type) const noexcept -> PIR::Type::ID {
		// find existing type
		for(size_t i = 0; i < this->types.size(); i+=1){
			const PIR::Type& type_ref = this->types[i];

			if(type_ref == type){
				return PIR::Type::ID( uint32_t(i) );
			}
		}

		EVO_FATAL_BREAK("Unknown type");
	};



	auto SourceManager::printType(PIR::Type::ID id) const noexcept -> std::string {
		const PIR::Type& type = this->getType(id);
		const PIR::BaseType& base_type = this->base_types[type.baseType.id];


		std::string base_type_str = [&]() noexcept {
			switch(base_type.kind){
				case PIR::BaseType::Kind::Import: {
					return std::string("[[IMPORT]]");
				} break;

				case PIR::BaseType::Kind::Builtin: {
					return std::string( Token::printKind(base_type.builtin.kind) );
				} break;

				case PIR::BaseType::Kind::Function: {
					// TODO:
					return std::string("[[FUNCTION]]");
				} break;

				default: EVO_FATAL_BREAK("Unknown base-type kind");
			};
		}();
		


		bool is_first_qualifer = true;
		for(const AST::Type::Qualifier& qualifier : type.qualifiers){
			if(type.qualifiers.size() > 1){
				if(is_first_qualifer){
					is_first_qualifer = false;
				}else{
					base_type_str += ' ';
				}
			}

			if(qualifier.isPtr){ base_type_str += '^'; }
			if(qualifier.isConst){ base_type_str += '|'; }
		}


		return base_type_str;
	};




	auto SourceManager::addEntry(Source::ID src_id, PIR::Func::ID func_id) noexcept -> void {
		evo::debugAssert(this->isLocked(), "Can only add entry when locked");
		evo::debugAssert(this->hasEntry() == false, "Already has an entry function");

		this->entry.emplace(src_id, func_id);
	};


	auto SourceManager::getEntry() const noexcept -> Entry {
		evo::debugAssert(this->isLocked(), "Can only get entry when locked");
		evo::debugAssert(this->hasEntry(), "SourceManager does not have an entry");

		return *this->entry;
	};


	auto SourceManager::hasExport(std::string_view ident) const noexcept -> bool {
		evo::debugAssert(this->isLocked(), "Can only check export when locked");

		return this->exported_funcs.contains(ident);
	};

	auto SourceManager::addExport(std::string_view ident) noexcept -> void {
		evo::debugAssert(this->isLocked(), "Can only add export when locked");
		evo::debugAssert(this->hasExport(ident) == false, "can't add export that's already in list");

		this->exported_funcs.emplace(ident);
	};





	auto SourceManager::getIntrinsics() const noexcept -> evo::ArrayProxy<PIR::Intrinsic> {
		evo::debugAssert(this->isLocked(), "Can only get intrinsics when locked");

		return this->intrinsics;
	};


	auto SourceManager::getIntrinsic(PIR::Intrinsic::ID id) const noexcept -> const PIR::Intrinsic& {
		evo::debugAssert(this->isLocked(), "Can only get intrinsics when locked");

		return this->intrinsics[id.id];
	};

	auto SourceManager::getIntrinsic(PIR::Intrinsic::Kind kind) const noexcept -> const PIR::Intrinsic& {
		evo::debugAssert(this->isLocked(), "Can only get intrinsics when locked");

		return this->intrinsics[static_cast<size_t>(kind)];
	};


	
};
