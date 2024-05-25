#include "frontend/PIR.h"

#include "frontend/SourceManager.h"


namespace panther{
	namespace PIR{

		auto Expr::equals(const Expr& rhs, const SourceManager& src_manager) const noexcept -> bool {
			if(this->kind != rhs.kind){ return false; }

			switch(this->kind){
				case Kind::None:        return true;
				case Kind::Var:         evo::debugFatalBreak("Kind::Var in Expr::Equals() is not supported yet");
				case Kind::Param:       evo::debugFatalBreak("Kind::Param in Expr::Equals() is not supported yet");
				case Kind::Literal: {
					const Source& this_src = src_manager.getSource(this->src_id);
					const Source& rhs_src = src_manager.getSource(rhs.src_id);

					const Token& this_token = this_src.getLiteral(this->literal);
					const Token& rhs_token = rhs_src.getLiteral(rhs.literal);

					return this_token == rhs_token;
				} break;
				case Kind::Uninit:      return true;
				case Kind::FuncCall:    evo::debugFatalBreak("Kind::FuncCall in Expr::Equals() is not supported yet");
				case Kind::Initializer: evo::debugFatalBreak("Kind::Initializer in Expr::Equals() is not supported yet");
				case Kind::Prefix:      evo::debugFatalBreak("Kind::Prefix in Expr::Equals() is not supported yet");
				case Kind::Deref:       evo::debugFatalBreak("Kind::Deref in Expr::Equals() is not supported yet");
				case Kind::Accessor:    evo::debugFatalBreak("Kind::Accessor in Expr::Equals() is not supported yet");
				case Kind::Import:      return this->import == rhs.import;
				default: evo::debugFatalBreak("Unknown expr kind in " __FUNCTION__);
			};
		};


		auto TemplateArg::equals(const TemplateArg& rhs, const SourceManager& src_manager) const noexcept -> bool {
			if(this->isType != rhs.isType){ return false; }
			if(this->typeID != rhs.typeID){ return false; }

			if(this->expr.has_value()){
				if(rhs.expr.has_value() == false){ return false; }
				return this->expr->equals(*rhs.expr, src_manager);
			}else{
				return !rhs.expr.has_value();
			}
		};


		auto BaseType::Operator::Param::operator==(const Operator::Param& rhs) const noexcept -> bool {
			return this->type == rhs.type && this->kind == rhs.kind;
		};

		auto BaseType::Operator::operator==(const Operator& rhs) const noexcept -> bool {
			return this->params == rhs.params && this->returnType == rhs.returnType;
		};

		

		auto BaseType::operator==(Token::Kind tok_kind) const noexcept -> bool {
			if(this->kind != Kind::Builtin){ return false; }

			return tok_kind == std::get<BuiltinData>(this->data).kind;
		};



		auto BaseType::equals(const BaseType& rhs, const SourceManager& src_manager) const noexcept -> bool {
			if(this->kind != rhs.kind){ return false; }


			switch(this->kind){
				case BaseType::Kind::Builtin: {
					if(std::get<BuiltinData>(this->data).kind != std::get<BuiltinData>(rhs.data).kind){ return false; }
				} break;

				case BaseType::Kind::Function: {
					if(this->callOperator != rhs.callOperator){ return false; }
				} break;

				case BaseType::Kind::Struct: {
					const StructData& this_data = std::get<StructData>(this->data);
					const StructData& rhs_data = std::get<StructData>(rhs.data);

					if(this_data.name != rhs_data.name){ return false; }
					if(this_data.source != rhs_data.source){ return false; }


					if(this_data.templateArgs.size() != rhs_data.templateArgs.size()){ return false; }
					for(size_t i = 0; i < this_data.templateArgs.size(); i+=1){
						if(this_data.templateArgs[i].equals(rhs_data.templateArgs[i], src_manager) == false){
							return false;		
						}
					}

					return true;
				} break;

				default: {
					evo::debugFatalBreak("unknown BaseType::Kind");
				} break;
			};

			return true;
		};




		auto Type::operator==(const Type& rhs) const noexcept -> bool {
			if(this->baseType.id != rhs.baseType.id){ return false; }

			if(this->qualifiers.size() != rhs.qualifiers.size()){ return false; }

			for(size_t i = 0; i < this->qualifiers.size(); i+=1){
				const AST::Type::Qualifier& this_qualifiers = this->qualifiers[i];
				const AST::Type::Qualifier& rhs_qualifiers = rhs.qualifiers[i];

				if(this_qualifiers.isPtr != rhs_qualifiers.isPtr){ return false; }
				if(this_qualifiers.isConst != rhs_qualifiers.isConst){ return false; }
			}

			return true;
		};


		auto Type::isImplicitlyConvertableTo(const Type& rhs) const noexcept -> bool {
			if(this->baseType.id != rhs.baseType.id){ return false; }

			if(this->qualifiers.size() != rhs.qualifiers.size()){ return false; }

			for(size_t i = 0; i < this->qualifiers.size(); i+=1){
				const AST::Type::Qualifier& this_qualifiers = this->qualifiers[i];
				const AST::Type::Qualifier& rhs_qualifiers = rhs.qualifiers[i];

				if(this_qualifiers.isPtr != rhs_qualifiers.isPtr){ return false; }
				if(this_qualifiers.isConst && rhs_qualifiers.isConst == false){ return false; }
			}

			return true;
		};



	};
};
