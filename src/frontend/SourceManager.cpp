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

		// ISize (5)
		this->base_types.emplace_back(PIR::BaseType::Kind::Builtin, Token::TypeISize);
		this->types.emplace_back( PIR::Type(PIR::BaseType::ID(5)) );

		// USize (6)
		this->base_types.emplace_back(PIR::BaseType::Kind::Builtin, Token::TypeUSize);
		this->types.emplace_back( PIR::Type(PIR::BaseType::ID(6)) );
	};


	// TODO: optimize base types
	auto SourceManager::initIntrinsics() noexcept -> void {
		evo::debugAssert(this->isLocked(), "Can only initialize intrinsics when locked");
		using ParamKind = AST::FuncParams::Param::Kind;


		auto create_func_base_type = [&](evo::ArrayProxy<PIR::BaseType::Operator::Param> params, PIR::Type::ID return_type) noexcept -> PIR::BaseType::ID {
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>(params.begin(), params.end()),
				return_type
			);

			return this->createBaseType(std::move(base_type));
		};


		///////////////////////////////////
		// setup protos

		const PIR::BaseType::ID math_ops_Int_type = create_func_base_type({{this->getTypeInt(), ParamKind::Read}, {this->getTypeInt(), ParamKind::Read}}, this->getTypeInt());
		const PIR::BaseType::ID math_ops_UInt_type = create_func_base_type({{this->getTypeUInt(), ParamKind::Read}, {this->getTypeUInt(), ParamKind::Read}}, this->getTypeUInt());
		const PIR::BaseType::ID math_ops_ISize_type = create_func_base_type({{this->getTypeISize(), ParamKind::Read}, {this->getTypeISize(), ParamKind::Read}}, this->getTypeISize());
		const PIR::BaseType::ID math_ops_USize_type = create_func_base_type({{this->getTypeUSize(), ParamKind::Read}, {this->getTypeUSize(), ParamKind::Read}}, this->getTypeUSize());


		const PIR::BaseType::ID logical_Int_type = create_func_base_type({{this->getTypeInt(), ParamKind::Read}, {this->getTypeInt(), ParamKind::Read}}, this->getTypeBool());
		const PIR::BaseType::ID logical_UInt_type = create_func_base_type({{this->getTypeUInt(), ParamKind::Read}, {this->getTypeUInt(), ParamKind::Read}}, this->getTypeBool());
		const PIR::BaseType::ID logical_Bool_type = create_func_base_type({{this->getTypeBool(), ParamKind::Read}, {this->getTypeBool(), ParamKind::Read}}, this->getTypeBool());
		const PIR::BaseType::ID logical_ISize_type = create_func_base_type({{this->getTypeISize(), ParamKind::Read}, {this->getTypeISize(), ParamKind::Read}}, this->getTypeBool());
		const PIR::BaseType::ID logical_USize_type = create_func_base_type({{this->getTypeUSize(), ParamKind::Read}, {this->getTypeUSize(), ParamKind::Read}}, this->getTypeBool());


		const PIR::BaseType::ID Int_in_Int_out_type = create_func_base_type({ {this->getTypeInt(), ParamKind::Read} }, this->getTypeInt());
		const PIR::BaseType::ID ISize_in_ISize_out_type = create_func_base_type({ {this->getTypeISize(), ParamKind::Read} }, this->getTypeISize());
		const PIR::BaseType::ID Bool_in_Bool_out_type = create_func_base_type({ {this->getTypeBool(), ParamKind::Read} }, this->getTypeBool());
		

		// Int
		const PIR::BaseType::ID Int_in_UInt_out_type = create_func_base_type({ {this->getTypeInt(), ParamKind::Read} }, this->getTypeUInt());
		const PIR::BaseType::ID Int_in_Bool_out_type = create_func_base_type({ {this->getTypeInt(), ParamKind::Read} }, this->getTypeBool());
		const PIR::BaseType::ID Int_in_ISize_out_type = create_func_base_type({ {this->getTypeInt(), ParamKind::Read} }, this->getTypeISize());
		const PIR::BaseType::ID Int_in_USize_out_type = create_func_base_type({ {this->getTypeInt(), ParamKind::Read} }, this->getTypeUSize());

		// UInt
		const PIR::BaseType::ID UInt_in_Int_out_type = create_func_base_type({ {this->getTypeUInt(), ParamKind::Read} }, this->getTypeInt());
		const PIR::BaseType::ID UInt_in_Bool_out_type = create_func_base_type({ {this->getTypeUInt(), ParamKind::Read} }, this->getTypeBool());
		const PIR::BaseType::ID UInt_in_ISize_out_type = create_func_base_type({ {this->getTypeUInt(), ParamKind::Read} }, this->getTypeISize());
		const PIR::BaseType::ID UInt_in_USize_out_type = create_func_base_type({ {this->getTypeUInt(), ParamKind::Read} }, this->getTypeUSize());

		// Bool
		const PIR::BaseType::ID Bool_in_Int_out_type = create_func_base_type({ {this->getTypeBool(), ParamKind::Read} }, this->getTypeInt());
		const PIR::BaseType::ID Bool_in_UInt_out_type = create_func_base_type({ {this->getTypeBool(), ParamKind::Read} }, this->getTypeUInt());
		const PIR::BaseType::ID Bool_in_ISize_out_type = create_func_base_type({ {this->getTypeBool(), ParamKind::Read} }, this->getTypeISize());
		const PIR::BaseType::ID Bool_in_USize_out_type = create_func_base_type({ {this->getTypeBool(), ParamKind::Read} }, this->getTypeUSize());

		// ISize
		const PIR::BaseType::ID ISize_in_Int_out_type = create_func_base_type({ {this->getTypeISize(), ParamKind::Read} }, this->getTypeInt());
		const PIR::BaseType::ID ISize_in_UInt_out_type = create_func_base_type({ {this->getTypeISize(), ParamKind::Read} }, this->getTypeUInt());
		const PIR::BaseType::ID ISize_in_Bool_out_type = create_func_base_type({ {this->getTypeISize(), ParamKind::Read} }, this->getTypeBool());
		const PIR::BaseType::ID ISize_in_USize_out_type = create_func_base_type({ {this->getTypeISize(), ParamKind::Read} }, this->getTypeUSize());

		// USize
		const PIR::BaseType::ID USize_in_Int_out_type = create_func_base_type({ {this->getTypeUSize(), ParamKind::Read} }, this->getTypeInt());
		const PIR::BaseType::ID USize_in_UInt_out_type = create_func_base_type({ {this->getTypeUSize(), ParamKind::Read} }, this->getTypeUInt());
		const PIR::BaseType::ID USize_in_Bool_out_type = create_func_base_type({ {this->getTypeUSize(), ParamKind::Read} }, this->getTypeBool());
		const PIR::BaseType::ID USize_in_ISize_out_type = create_func_base_type({ {this->getTypeUSize(), ParamKind::Read} }, this->getTypeISize());



		





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
		// arithmetic

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addInt, "addInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addUInt, "addUInt", math_ops_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addISize, "addISize", math_ops_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addUSize, "addUSize", math_ops_USize_type);

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addWrapInt, "addWrapInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addWrapUInt, "addWrapUInt", math_ops_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addWrapISize, "addWrapISize", math_ops_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::addWrapUSize, "addWrapUSize", math_ops_USize_type);


		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subInt, "subInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subUInt, "subUInt", math_ops_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subISize, "subISize", math_ops_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subUSize, "subUSize", math_ops_USize_type);

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subWrapInt, "subWrapInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subWrapUInt, "subWrapUInt", math_ops_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subWrapISize, "subWrapISize", math_ops_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::subWrapUSize, "subWrapUSize", math_ops_USize_type);


		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulInt, "mulInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulUInt, "mulUInt", math_ops_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulISize, "mulISize", math_ops_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulUSize, "mulUSize", math_ops_USize_type);
		
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulWrapInt, "mulWrapInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulWrapUInt, "mulWrapUInt", math_ops_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulWrapISize, "mulWrapISize", math_ops_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::mulWrapUSize, "mulWrapUSize", math_ops_USize_type);


		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::divInt, "divInt", math_ops_Int_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::divUInt, "divUInt", math_ops_UInt_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::divISize, "divISize", math_ops_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::divUSize, "divUSize", math_ops_USize_type);



		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::negateInt, "negateInt", Int_in_Int_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::negateISize, "negateISize", ISize_in_ISize_out_type);


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

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::equalISize, "equalISize", logical_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::notEqualISize, "notEqualISize", logical_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::lessThanISize, "lessThanISize", logical_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::lessThanEqualISize, "lessThanEqualISize", logical_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::greaterThanISize, "greaterThanISize", logical_ISize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::greaterThanEqualISize, "greaterThanEqualISize", logical_ISize_type);

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::equalUSize, "equalUSize", logical_USize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::notEqualUSize, "notEqualUSize", logical_USize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::lessThanUSize, "lessThanUSize", logical_USize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::lessThanEqualUSize, "lessThanEqualUSize", logical_USize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::greaterThanUSize, "greaterThanUSize", logical_USize_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::greaterThanEqualUSize, "greaterThanEqualUSize", logical_USize_type);


		///////////////////////////////////
		// type conversion

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convIntToUInt, "convIntToUInt", Int_in_UInt_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convIntToBool, "convIntToBool", Int_in_Bool_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convIntToISize, "convIntToISize", Int_in_ISize_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convIntToUSize, "convIntToUSize", Int_in_USize_out_type);

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convUIntToInt, "convUIntToInt", UInt_in_Int_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convUIntToBool, "convUIntToBool", UInt_in_Bool_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convUIntToISize, "convUIntToISize", UInt_in_ISize_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convUIntToUSize, "convUIntToUSize", UInt_in_USize_out_type);

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convBoolToInt, "convBoolToInt", Bool_in_Int_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convBoolToUInt, "convBoolToUInt", Bool_in_UInt_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convBoolToISize, "convBoolToISize", Bool_in_ISize_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convBoolToUSize, "convBoolToUSize", Bool_in_USize_out_type);

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convISizeToInt, "convISizeToInt", ISize_in_Int_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convISizeToUInt, "convISizeToUInt", ISize_in_UInt_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convISizeToBool, "convISizeToBool", ISize_in_Bool_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convISizeToUSize, "convISizeToUSize", ISize_in_USize_out_type);

		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convUSizeToInt, "convUSizeToInt", USize_in_Int_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convUSizeToUInt, "convUSizeToUInt", USize_in_UInt_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convUSizeToBool, "convUSizeToBool", USize_in_Bool_out_type);
		this->intrinsics.emplace_back(PIR::Intrinsic::Kind::convUSizeToISize, "convUSizeToISize", USize_in_ISize_out_type);



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


		{ // __printBool()
			auto base_type = PIR::BaseType(PIR::BaseType::Kind::Function);
			base_type.callOperator = PIR::BaseType::Operator(
				std::vector<PIR::BaseType::Operator::Param>{ {this->getTypeBool(), ParamKind::Read} },
				PIR::Type::VoidableID::Void()
			);

			const PIR::BaseType::ID base_type_id = this->createBaseType(std::move(base_type));
			this->intrinsics.emplace_back(PIR::Intrinsic::Kind::__printBool, "__printBool", base_type_id);
		}



		///////////////////////////////////
		// debug checking of ordering

		#if defined(PANTHER_CONFIG_DEBUG)
			for(uint32_t i = 0; i < uint32_t(PIR::Intrinsic::Kind::_MAX_); i+=1){
				const PIR::Intrinsic::Kind i_kind = static_cast<PIR::Intrinsic::Kind>(i);
				const PIR::Intrinsic& intrinsic = this->getIntrinsic(i_kind);

				evo::Assert(
					i_kind == intrinsic.kind,
					std::format("found an intrinsic with incorrect cooresponding kind (kind: {} != intrinsic: {})", int(i_kind), int(intrinsic.kind))
				);
			}
		#endif


		//////////////////////////////////////////////////////////////////////
		// set operators

		PIR::BaseType& type_Int = this->getBaseType(Token::TypeInt);
		PIR::BaseType& type_UInt = this->getBaseType(Token::TypeUInt);
		PIR::BaseType& type_Bool = this->getBaseType(Token::TypeBool);
		PIR::BaseType& type_ISize = this->getBaseType(Token::TypeISize);
		PIR::BaseType& type_USize = this->getBaseType(Token::TypeUSize);


		///////////////////////////////////
		// math

		type_Int.ops.add.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addInt));
		type_UInt.ops.add.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addUInt));
		type_ISize.ops.add.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addISize));
		type_USize.ops.add.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addUSize));

		type_Int.ops.addWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addWrapInt));
		type_UInt.ops.addWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addWrapUInt));
		type_ISize.ops.addWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addWrapISize));
		type_USize.ops.addWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::addWrapUSize));


		type_Int.ops.sub.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subInt));
		type_UInt.ops.sub.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subUInt));
		type_ISize.ops.sub.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subISize));
		type_USize.ops.sub.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subUSize));

		type_Int.ops.subWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subWrapInt));
		type_UInt.ops.subWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subWrapUInt));
		type_ISize.ops.subWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subWrapISize));
		type_USize.ops.subWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::subWrapUSize));


		type_Int.ops.mul.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulInt));
		type_UInt.ops.mul.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulUInt));
		type_ISize.ops.mul.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulISize));
		type_USize.ops.mul.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulUSize));

		type_Int.ops.mulWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulWrapInt));
		type_UInt.ops.mulWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulWrapUInt));
		type_ISize.ops.mulWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulWrapISize));
		type_USize.ops.mulWrap.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::mulWrapUSize));


		type_Int.ops.div.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::divInt));
		type_UInt.ops.div.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::divUInt));
		type_ISize.ops.div.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::divISize));
		type_USize.ops.div.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::divUSize));


		type_Int.ops.negate.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::negateInt));
		type_ISize.ops.negate.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::negateISize));


		///////////////////////////////////
		// logical

		type_Int.ops.logicalEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::equalInt));
		type_UInt.ops.logicalEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::equalUInt));
		type_Bool.ops.logicalEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::equalBool));
		type_ISize.ops.logicalEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::equalISize));
		type_USize.ops.logicalEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::equalUSize));

		type_Int.ops.notEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::notEqualInt));
		type_UInt.ops.notEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::notEqualUInt));
		type_Bool.ops.notEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::notEqualBool));
		type_ISize.ops.notEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::notEqualISize));
		type_USize.ops.notEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::notEqualUSize));

		type_Int.ops.lessThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanInt));
		type_UInt.ops.lessThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanUInt));
		type_ISize.ops.lessThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanISize));
		type_USize.ops.lessThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanUSize));

		type_Int.ops.lessThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanEqualInt));
		type_UInt.ops.lessThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanEqualUInt));
		type_ISize.ops.lessThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanEqualISize));
		type_USize.ops.lessThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::lessThanEqualUSize));



		type_Int.ops.greaterThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanInt));
		type_UInt.ops.greaterThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanUInt));
		type_ISize.ops.greaterThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanISize));
		type_USize.ops.greaterThan.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanUSize));

		type_Int.ops.greaterThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanEqualInt));
		type_UInt.ops.greaterThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanEqualUInt));
		type_ISize.ops.greaterThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanEqualISize));
		type_USize.ops.greaterThanEqual.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::greaterThanEqualUSize));


		type_Bool.ops.logicalNot.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::logicalNot));
		type_Bool.ops.logicalAnd.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::logicalAnd));
		type_Bool.ops.logicalOr.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::logicalOr));


		///////////////////////////////////
		// casting

		type_Int.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convIntToUInt));
		type_Int.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convIntToBool));
		type_Int.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convIntToISize));
		type_Int.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convIntToUSize));

		type_UInt.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convUIntToInt));
		type_UInt.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convUIntToBool));
		type_UInt.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convUIntToISize));
		type_UInt.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convUIntToUSize));

		type_Bool.ops.as.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convBoolToInt));
		type_Bool.ops.as.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convBoolToUInt));
		type_Bool.ops.as.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convBoolToISize));
		type_Bool.ops.as.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convBoolToUSize));

		type_ISize.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convISizeToInt));
		type_ISize.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convISizeToUInt));
		type_ISize.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convISizeToBool));
		type_ISize.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convISizeToUSize));

		type_USize.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convUSizeToInt));
		type_USize.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convUSizeToUInt));
		type_USize.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convUSizeToBool));
		type_USize.ops.cast.emplace_back(this->getIntrinsicID(PIR::Intrinsic::Kind::convUSizeToISize));
	};



	// TODO: multithreading
	auto SourceManager::semanticAnalysis() noexcept -> evo::uint {
		evo::debugAssert(this->isLocked(), "Can only do semantic analysis when locked");


		evo::uint total_fails = 0;


		for(Source& source : this->sources){
			if(source.semantic_analysis_global_idents_and_imports() == false){
				total_fails += 1;
			}
		}
		if(total_fails != 0){ return total_fails; }


		for(Source& source : this->sources){
			if(source.semantic_analysis_global_aliases() == false){
				total_fails += 1;
			}
		}
		if(total_fails != 0){ return total_fails; }


		for(Source& source : this->sources){
			if(source.semantic_analysis_global_types() == false){
				total_fails += 1;
			}
		}
		if(total_fails != 0){ return total_fails; }


		for(Source& source : this->sources){
			if(source.semantic_analysis_global_values() == false){
				total_fails += 1;
			}
		}
		if(total_fails != 0){ return total_fails; }


		for(Source& source : this->sources){
			if(source.semantic_analysis_runtime() == false){
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
					return std::string( Token::printKind(std::get<PIR::BaseType::BuiltinData>(base_type.data).kind) );
				} break;

				case PIR::BaseType::Kind::Function: {
					// TODO:
					return std::string("[[FUNCTION]]");
				} break;

				case PIR::BaseType::Kind::Struct: {
					// TODO:
					return std::string(std::get<PIR::BaseType::StructData>(base_type.data).name);
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

			if(qualifier.isPtr){ base_type_str += '&'; }
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
