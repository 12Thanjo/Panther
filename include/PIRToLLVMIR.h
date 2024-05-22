#pragma once


//////////////////////////////////////////////////////////////////////
// 																	//
//   NOTE: This file is to only be included in executables			//
//		This is due to linking issues with LLVM and Clang			//
// 																	//
//////////////////////////////////////////////////////////////////////



#include <Evo.h>

#include "./LLVM_interface/Context.h"
#include "./LLVM_interface/IRBuilder.h"
#include "./LLVM_interface/Module.h"
#include "./LLVM_interface/ExecutionEngine.h"
#include "./LLVM_interface/misc.h"


namespace panther{


	class PIRToLLVMIR{
		public:
			PIRToLLVMIR() = default;
			~PIRToLLVMIR() noexcept {
				evo::debugAssert(this->isInitialized() == false, "Destructor of PIRToLLVMIR called without being shutdown");
			};


			auto init(std::string_view name, llvmint::Context& context) noexcept -> void {
				evo::debugAssert(this->isInitialized() == false, "PIRToLLVMIR already initialized");

				this->builder = new llvmint::IRBuilder(context.getContext());
				this->module = new llvmint::Module(name, context.getContext());


				const std::string target_triple = llvmint::Module::getDefaultTargetTriple();
				


				const std::string data_layout_err = this->module->setDataLayout(target_triple, "generic", "");

				if(data_layout_err.empty() == false){
					// TODO: maybe handle this more?
					evo::debugFatalBreak(data_layout_err);
				}


				this->module->setTargetTriple(target_triple);
			};


			auto shutdown() noexcept -> void {
				evo::debugAssert(this->isInitialized(), "PIRToLLVMIR is not initialized");
				
				delete this->builder;
				this->builder = nullptr;

				delete this->module;
				this->module = nullptr;
			};



			auto lower(SourceManager& source_manager) noexcept -> void {
				this->src_manager = &source_manager;

				std::vector<Source>& sources = source_manager.getSources();


				for(Source& source_ref : sources){
					this->source = &source_ref;

					for(PIR::Struct& struct_decl : source_ref.pir.structs){
						this->lower_struct_declaration(struct_decl);
					}
				}

				for(Source& source_ref : sources){
					this->source = &source_ref;

					for(PIR::Struct& struct_decl : source_ref.pir.structs){
						this->lower_struct_body(struct_decl);
					}

					for(PIR::Var::ID global_var_id : source_ref.pir.global_vars){
						PIR::Var& var = Source::getVar(global_var_id);

						this->lower_global_var(var);
					}
				}

				for(Source& source_ref : sources){
					this->source = &source_ref;

					for(PIR::Func& func : source_ref.pir.funcs){
						this->lower_func_declaration(func);
					}
				}

				for(Source& source_ref : sources){
					this->source = &source_ref;

					for(PIR::Func& func : source_ref.pir.funcs){
						this->lower_func(func);
					}
				}

				this->source = nullptr;
				this->src_manager = nullptr;
			};



			EVO_NODISCARD inline auto initLibC() noexcept -> void {
				llvm::FunctionType* puts_proto = this->builder->getFuncProto(
					this->builder->getTypeVoid(), { llvmint::ptrcast<llvm::Type>(this->builder->getTypePtr()) }, false
				);
				this->libc.puts = this->module->createFunction("puts", puts_proto, llvmint::LinkageTypes::ExternalLinkage, true, false);
				llvmint::setupFuncParams(this->libc.puts, { llvmint::ParamInfo("str", false, true, true) });


				llvm::FunctionType* printf_proto = this->builder->getFuncProto(
					this->builder->getTypeVoid(), { llvmint::ptrcast<llvm::Type>(this->builder->getTypePtr()) }, true
				);
				this->libc.printf = this->module->createFunction("printf", printf_proto, llvmint::LinkageTypes::ExternalLinkage, true, false);
				llvmint::setupFuncParams(this->libc.printf, { llvmint::ParamInfo("str", false, true, true) });
			};




			EVO_NODISCARD inline auto addRuntime(const SourceManager::Entry& entry_func) noexcept -> void {
				const PIR::Func& func = Source::getFunc(entry_func.func_id);

				llvm::FunctionType* prototype = this->builder->getFuncProto(llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64()), {}, false);
				llvm::Function* main_func = this->module->createFunction("main", prototype, llvmint::LinkageTypes::ExternalLinkage, true, false);

				llvm::BasicBlock* begin_block = this->builder->createBasicBlock(main_func, "begin");
				this->builder->setInsertionPoint(begin_block);

				llvm::Value* begin_ret = llvmint::ptrcast<llvm::Value>(this->builder->createCall(func.llvmFunc, {}, '\0'));
				this->builder->createRet(begin_ret);
			};




			EVO_NODISCARD auto printLLVMIR() const noexcept -> std::string {
				return this->module->print();
			};


			// return nullopt means target machine cannot output object file
			EVO_NODISCARD auto compileToObjectFile() noexcept -> evo::Result<std::vector<evo::byte>> {
				return this->module->compileToObjectFile();
			};



			EVO_NODISCARD inline auto isInitialized() const noexcept -> bool { return this->builder != nullptr; };


			template<typename ReturnType>
			EVO_NODISCARD inline auto run(std::string_view func_name) noexcept -> ReturnType {
				auto execution_engine = llvmint::ExecutionEngine();
				execution_engine.createEngine(*this->module);

				const ReturnType output = execution_engine.runFunction<ReturnType>(func_name);

				execution_engine.shutdownEngine();

				return output;
			};

			template<>
			inline auto run<void>(std::string_view func_name) noexcept -> void {
				auto execution_engine = llvmint::ExecutionEngine();
				execution_engine.createEngine(*this->module);

				execution_engine.runFunction<void>(func_name);

				execution_engine.shutdownEngine();
			};



		private:
			inline auto lower_global_var(PIR::Var& var) noexcept -> void {
				const std::string mangled_name = PIRToLLVMIR::mangle_name(*this->source, var);

				const PIR::Type& type = this->src_manager->getType(var.type);

				llvm::Type* llvm_type = this->get_type(type);
				llvm::Constant* value = this->get_const_value(var.value);
				const llvmint::LinkageTypes linkage = var.isExport ? llvmint::LinkageTypes::ExternalLinkage : llvmint::LinkageTypes::PrivateLinkage;
				evo::debugAssert(value != nullptr, "invalid const value");


				llvm::GlobalVariable* global_val = this->builder->valueGlobal(*this->module, value, llvm_type, linkage, var.isDef, mangled_name.c_str());

				var.llvm.global = global_val;
			};



			inline auto lower_stmt(const PIR::Stmt& stmt) noexcept -> void {
				switch(stmt.kind){
					break; case PIR::Stmt::Kind::Var: this->lower_var(Source::getVar(stmt.var));
					break; case PIR::Stmt::Kind::Conditional: this->lower_conditional(this->source->getConditional(stmt.conditional));
					break; case PIR::Stmt::Kind::Return: this->lower_return(this->source->getReturn(stmt.ret));
					break; case PIR::Stmt::Kind::Assignment: this->lower_assignment(this->source->getAssignment(stmt.assignment));
					break; case PIR::Stmt::Kind::FuncCall: this->lower_func_call(this->source->getFuncCall(stmt.funcCall));
					break; case PIR::Stmt::Kind::Unreachable: this->lower_unreachable();
					break; default: evo::debugFatalBreak("Unknown stmt kind");
				};
			};



			inline auto lower_func_declaration(PIR::Func& func) noexcept -> void {
				this->current_func = &func;
				const std::string mangled_name = PIRToLLVMIR::mangle_name(*this->source, func);

				llvm::Type* return_type = [&]() noexcept {
					if(func.returnType.isVoid()){
						return this->builder->getTypeVoid();
					}else{
						const PIR::Type& return_type_obj = this->src_manager->getType(func.returnType.typeID());
						return this->get_type(return_type_obj);
					}
				}();


				auto param_types = std::vector<llvm::Type*>();
				auto param_infos = std::vector<llvmint::ParamInfo>();

				for(size_t i = 0; i < func.params.size(); i+=1){
					const PIR::Param& param = this->source->getParam(func.params[i]);

					llvm::Type* param_type = this->get_type(this->src_manager->getType(param.type));
					// param_types.emplace_back(param_type);
					param_types.emplace_back(llvmint::ptrcast<llvm::Type>(this->builder->getTypePtr()));

					using ParamKind = AST::FuncParams::Param::Kind;
					const bool readonly = false;
					const bool nonnull = true;
					const bool noalias = param.kind == ParamKind::Write;
					const auto deref = llvmint::ParamInfo::Dereferenceable(this->module->getTypeSize(param_type), false);
					param_infos.emplace_back(this->source->getToken(param.ident).value.string, readonly, nonnull, noalias, deref);
				}


				llvm::FunctionType* prototype = this->builder->getFuncProto(return_type, param_types, false);
				const auto linkage = func.isExport ? llvmint::LinkageTypes::ExternalLinkage : llvmint::LinkageTypes::InternalLinkage;
				const bool fast_call_conv = !func.isExport;
				llvm::Function* llvm_func = this->module->createFunction(mangled_name, prototype, linkage, true, fast_call_conv);
				func.llvmFunc = llvm_func;



				if(func.params.empty()){
					this->builder->createBasicBlock(llvm_func, "begin");
					
				}else{
					llvmint::setupFuncParams(llvm_func, param_infos);

					llvm::BasicBlock* setup = this->builder->createBasicBlock(llvm_func, "setup");
					llvm::BasicBlock* begin = this->builder->createBasicBlock(llvm_func, "begin");

					this->builder->setInsertionPoint(setup);

					const std::vector<llvm::Argument*> arguments = llvmint::getFuncArguments(llvm_func);
					for(size_t i = 0; i < arguments.size(); i+=1){
						llvm::AllocaInst* arg_alloca = this->builder->createAlloca(param_types[i], std::format("{}.addr", param_infos[i].name));
						this->source->getParam(func.params[i]).alloca = arg_alloca;

						this->builder->createStore(arg_alloca, llvmint::ptrcast<llvm::Value>(arguments[i]), false);
					}

					this->builder->createBranch(begin);

				}
			};


			inline auto lower_struct_declaration(const PIR::Struct& struct_decl) noexcept -> void {
				const std::string mangled_name = PIRToLLVMIR::mangle_name(*this->source, struct_decl);

				PIR::BaseType& base_type = this->src_manager->getBaseType(struct_decl.baseType);
				PIR::BaseType::StructData& struct_data = std::get<PIR::BaseType::StructData>(base_type.data);

				llvm::StructType* struct_type = this->module->createStructType(mangled_name);
				struct_data.llvm_type = struct_type;
			};


			inline auto lower_struct_body(const PIR::Struct& struct_decl) noexcept -> void {
				PIR::BaseType& base_type = this->src_manager->getBaseType(struct_decl.baseType);
				PIR::BaseType::StructData& struct_data = std::get<PIR::BaseType::StructData>(base_type.data);

				auto member_types = std::vector<llvm::Type*>();
				for(auto& member : struct_data.memberVars){
					member_types.emplace_back(this->get_type(this->src_manager->getType(member.type)));
				}

				this->module->setStructBody(struct_data.llvm_type, member_types, struct_decl.isPacked);
			};





			inline auto lower_func(PIR::Func& func) noexcept -> void {
				this->current_func = &func;

				this->builder->setInsertionPointAtBack(func.llvmFunc);

				for(const PIR::Stmt& stmt : func.stmts){
					this->lower_stmt(stmt);
				}


				if(func.returnType.isVoid()){
					if(func.stmts.isTerminated()){
						if(func.terminatesInBaseScope == false){
							this->builder->createUnreachable();
						}
					}else{						
						this->builder->createRet();
					}
				}else if(func.terminatesInBaseScope == false){
					this->builder->createUnreachable();
				}




				this->current_func = nullptr;
			};




			inline auto lower_var(PIR::Var& var) noexcept -> void {
				const std::string ident = std::string(this->source->getToken(var.ident).value.string);

				const PIR::Type& type = this->src_manager->getType(var.type);

				llvm::Type* llvm_type = this->get_type(type);

				llvm::AllocaInst* alloca_val = this->builder->createAlloca(llvm_type, ident);

				var.llvm.alloca = alloca_val;
				var.is_alloca = true;


				if(var.value.kind == PIR::Expr::Kind::ASTNode){
					const AST::Node& var_value_node = this->source->getNode(var.value.astNode);
					if(var_value_node.kind != AST::Kind::Uninit){
						this->builder->createStore(alloca_val, this->get_value(var.value), false);
					}
					
				}else{
					this->builder->createStore(alloca_val, this->get_value(var.value), false);
				}

			};


			inline auto lower_conditional(PIR::Conditional& cond) noexcept -> void {
				llvm::BasicBlock* then_block = this->builder->createBasicBlock(this->current_func->llvmFunc, "if.then");
				llvm::BasicBlock* end_block = nullptr;

				llvm::Value* cond_value = this->get_value(cond.ifCond);

				if(cond.elseStmts.empty()){
					end_block = this->builder->createBasicBlock(this->current_func->llvmFunc, "if.end");

					this->builder->createCondBranch(cond_value, then_block, end_block);

					this->builder->setInsertionPoint(then_block);
					for(const PIR::Stmt& stmt : cond.thenStmts){
						this->lower_stmt(stmt);
					}

					if(cond.thenStmts.isTerminated() == false){
						this->builder->setInsertionPoint(then_block);
						this->builder->createBranch(end_block);
					}

				}else{
					llvm::BasicBlock* else_block = this->builder->createBasicBlock(this->current_func->llvmFunc, "if.else");

					this->builder->createCondBranch(cond_value, then_block, else_block);

					// then block
					this->builder->setInsertionPoint(then_block);
					for(const PIR::Stmt& stmt : cond.thenStmts){
						this->lower_stmt(stmt);
					}

					llvm::BasicBlock* then_block_end = this->builder->getInsertPoint();

					// else block
					this->builder->setInsertionPoint(else_block);
					for(const PIR::Stmt& stmt : cond.elseStmts){
						this->lower_stmt(stmt);
					}

					// end block
					end_block = this->builder->createBasicBlock(this->current_func->llvmFunc, "if.end");

					if(cond.elseStmts.isTerminated() == false){
						this->builder->createBranch(end_block);
					}

					if(cond.thenStmts.isTerminated() == false){
						this->builder->setInsertionPoint(then_block_end);
						this->builder->createBranch(end_block);
					}
				}

				this->builder->setInsertionPoint(end_block);
			};


			inline auto lower_return(const PIR::Return& ret) noexcept -> void {
				if(ret.value.has_value()){
					this->builder->createRet(this->get_value(*ret.value));

				}else{
					this->builder->createRet();
				}
			};



			inline auto lower_assignment(const PIR::Assignment& assignment) noexcept -> void {
				evo::debugAssert(
					this->source->getToken(assignment.op).kind == Token::get("="),
					"Only normal assignment (=) is supported for lowering at the moment"
				);


				llvm::Value* dst = this->get_concrete_value(assignment.dst);
				llvm::Value* value = this->get_value(assignment.value);

				this->builder->createStore(dst, value, false);
			};








			inline auto create_func_call_args(const PIR::FuncCall& func_call) noexcept -> std::vector<llvm::Value*> {
				auto args = std::vector<llvm::Value*>();

				const PIR::Func& func = Source::getFunc(func_call.func);

				for(size_t i = 0; i < func_call.args.size(); i+=1){
					const PIR::Expr& arg = func_call.args[i];

					args.emplace_back(this->get_arg_value(arg, func_call.func.source.getParam(func.params[i]).type));
				}

				return args;
			};


			inline auto lower_func_call(const PIR::FuncCall& func_call) noexcept -> void {
				switch(func_call.kind){
					case PIR::FuncCall::Kind::Func: {
						const PIR::Func& func = Source::getFunc(func_call.func);

						const std::vector<llvm::Value*> args = this->create_func_call_args(func_call);

						this->builder->createCall(func.llvmFunc, args, '\0');
					} break;


					case PIR::FuncCall::Kind::Intrinsic: {
						const PIR::Intrinsic& intrinsic = this->src_manager->getIntrinsic(func_call.intrinsic);

						switch(intrinsic.kind){
							case PIR::Intrinsic::Kind::breakpoint: {
								this->builder->createIntrinsicCall(llvmint::IRBuilder::IntrinsicID::debugtrap, {});
							} break;

							case PIR::Intrinsic::Kind::__printHelloWorld: {
								evo::debugAssert(this->libc.puts != nullptr, "libc was not initialized");

								static llvm::GlobalVariable* hello_world_str = this->builder->valueString("Hello World, I'm Panther!", "hello_world_str");
								this->builder->createCall(this->libc.puts, { llvmint::ptrcast<llvm::Value>(hello_world_str) });
							} break;

							case PIR::Intrinsic::Kind::__printInt: {
								evo::debugAssert(this->libc.printf != nullptr, "libc was not initialized");
								
								static llvm::GlobalVariable* print_int_str = this->builder->valueString("Int: %lli\n", "print_int_str");
								this->builder->createCall(
									this->libc.printf, { llvmint::ptrcast<llvm::Value>(print_int_str), this->get_value(func_call.args[0]) }
								);
							} break;

							case PIR::Intrinsic::Kind::__printUInt: {
								evo::debugAssert(this->libc.printf != nullptr, "libc was not initialized");
								
								static llvm::GlobalVariable* print_uint_str = this->builder->valueString("UInt: %llu\n", "print_uint_str");
								this->builder->createCall(
									this->libc.printf, { llvmint::ptrcast<llvm::Value>(print_uint_str), this->get_value(func_call.args[0]) }
								);
							} break;

							case PIR::Intrinsic::Kind::__printBool: {
								evo::debugAssert(this->libc.printf != nullptr, "libc was not initialized");

								llvm::Value* bool_value = this->get_value(func_call.args[0]);

								llvm::BasicBlock* true_block = this->builder->createBasicBlock(this->current_func->llvmFunc, "__printBool.true");
								llvm::BasicBlock* false_block = this->builder->createBasicBlock(this->current_func->llvmFunc, "__printBool.true");
								llvm::BasicBlock* end_block = this->builder->createBasicBlock(this->current_func->llvmFunc, "__printBool.end");

								this->builder->createCondBranch(bool_value, true_block, false_block);

								this->builder->setInsertionPoint(true_block);
								static llvm::GlobalVariable* true_str = this->builder->valueString("Bool: true");
								this->builder->createCall(this->libc.puts, { llvmint::ptrcast<llvm::Value>(true_str) });
								this->builder->createBranch(end_block);

								this->builder->setInsertionPoint(false_block);
								static llvm::GlobalVariable* false_str = this->builder->valueString("Bool: false");
								this->builder->createCall(this->libc.puts, { llvmint::ptrcast<llvm::Value>(false_str) });
								this->builder->createBranch(end_block);

								this->builder->setInsertionPoint(end_block);
							} break;

							case PIR::Intrinsic::Kind::__printSeparator: {
								evo::debugAssert(this->libc.puts != nullptr, "libc was not initialized");

								static llvm::GlobalVariable* separator_str = this->builder->valueString("------------------------------", "separator_str");
								this->builder->createCall(this->libc.puts, { llvmint::ptrcast<llvm::Value>(separator_str) });
							} break;

							default: {
								evo::debugFatalBreak("Unknown intrinsic");
							};
						};
					} break;


					default: evo::debugFatalBreak("Unknown func call kind");
				};
			};


			inline auto lower_unreachable() noexcept -> void {
				this->builder->createUnreachable();
			};





			EVO_NODISCARD inline auto get_type(const PIR::Type& type) noexcept -> llvm::Type* {
				if(type.qualifiers.empty() == false){
					if(type.qualifiers.back().isPtr){
						return llvmint::ptrcast<llvm::Type>(this->builder->getTypePtr());
					}else{
						evo::debugFatalBreak("Unsupported qualifiers");
					}
				}

				const PIR::BaseType& base_type = this->src_manager->getBaseType(type.baseType);

				switch(base_type.kind){
					case PIR::BaseType::Kind::Builtin: {
						const Token::Kind builtin_kind = std::get<PIR::BaseType::BuiltinData>(base_type.data).kind;

						if(builtin_kind == Token::TypeInt){
							// TODO: make sure is register sized
							return llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64());

						}else if(builtin_kind == Token::TypeUInt){
							return llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64());
						
						}else if(builtin_kind == Token::TypeBool){
							return llvmint::ptrcast<llvm::Type>(this->builder->getTypeBool());

						}else if(builtin_kind == Token::TypeISize){
							return llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64());
						
						}else if(builtin_kind == Token::TypeUSize){
							return llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64());
						
						}

						evo::debugFatalBreak("Unknown builtin type");
					} break;

					case PIR::BaseType::Kind::Struct: {
						const PIR::BaseType::StructData& struct_data = std::get<PIR::BaseType::StructData>(base_type.data);
						return llvmint::ptrcast<llvm::Type>(struct_data.llvm_type);
					} break;
				};

				
				evo::debugFatalBreak("Unknown type");
			};


			EVO_NODISCARD inline auto type_has_members(const PIR::Type& type) noexcept -> bool {
				if(type.qualifiers.empty() == false){
					return !type.qualifiers.back().isPtr;
				}

				const PIR::BaseType& base_type = this->src_manager->getBaseType(type.baseType);

				return base_type.kind == PIR::BaseType::Kind::Struct;
			};



			EVO_NODISCARD inline auto get_const_value(PIR::Expr value) noexcept -> llvm::Constant* {
				switch(value.kind){
					case PIR::Expr::Kind::ASTNode: {
						const Token& token = this->source->getLiteral(value.astNode);

						switch(token.kind){
							case Token::LiteralInt: {
								return llvmint::ptrcast<llvm::Constant>(this->builder->valueUI64(token.value.integer));
							} break;

							case Token::LiteralBool: {
								return llvmint::ptrcast<llvm::Constant>(this->builder->valueBool(token.value.boolean));
							} break;

							default: evo::debugFatalBreak("Invalid literal value kind");
						};
					} break;


					case PIR::Expr::Kind::Prefix: {
						const PIR::Prefix& prefix = this->source->getPrefix(value.prefix);

						switch(this->source->getToken(prefix.op).kind){
							case Token::KeywordCopy: {
								evo::debugFatalBreak("Should have been figured out in semantic analysis");
							} break;

							case Token::KeywordAddr: {
								const PIR::Var& var = this->source->getGlobalVar(prefix.rhs.var);
								evo::debugAssert(var.isGlobal(), "variable is not global");

								return llvmint::ptrcast<llvm::Constant>(var.llvm.global);
							} break;

							default: evo::debugFatalBreak("Invalid or unknown prefix operator");
						};
					} break;


					default: evo::debugFatalBreak("Invalid value kind");
				};
			};


			// should_load = false is useful when you just need the pointer to the thing(for example, GEP instructions)
			EVO_NODISCARD inline auto get_value(PIR::Expr value, bool should_load = true) noexcept -> llvm::Value* {
				switch(value.kind){
					case PIR::Expr::Kind::ASTNode: {
						const AST::Node& node = this->source->getNode(value.astNode);

						switch(node.kind){
							case AST::Kind::Literal: {
								const Token& token = this->source->getLiteral(value.astNode);

								switch(token.kind){
									case Token::LiteralInt: {
										return llvmint::ptrcast<llvm::Value>(this->builder->valueUI64(token.value.integer));
									} break;

									case Token::LiteralBool: {
										return llvmint::ptrcast<llvm::Value>(this->builder->valueBool(token.value.boolean));
									} break;
								};
							} break;
							

							default: {
								evo::debugFatalBreak("Unknown AST::Kind");
							} break;
						};
					} break;

					case PIR::Expr::Kind::Var: {
						const PIR::Var& var = Source::getVar(value.var);

						if(should_load){
							std::string load_name = std::format("{}.load", value.var.source.getToken(var.ident).value.string);
							if(var.is_alloca){
								return llvmint::ptrcast<llvm::Value>(this->builder->createLoad(var.llvm.alloca, load_name));
							}else{
								llvm::Type* var_type = this->get_type(this->src_manager->getType(var.type));
								return llvmint::ptrcast<llvm::Value>(
									this->builder->createLoad(llvmint::ptrcast<llvm::Value>(var.llvm.global), var_type, load_name)
								);
							}
						}else{
							if(var.is_alloca){
								return llvmint::ptrcast<llvm::Value>(var.llvm.alloca);
							}else{
								return llvmint::ptrcast<llvm::Value>(var.llvm.global);
							}
						}

					} break;

					case PIR::Expr::Kind::Param: {
						const PIR::Param& param = this->source->getParam(value.param);

						llvm::Type* param_type = this->get_type(this->src_manager->getType(param.type));


						std::string load_addr_name = std::format("{}.loadAddr", this->source->getToken(param.ident).value.string);
						llvm::LoadInst* load_addr = this->builder->createLoad(param.alloca, load_addr_name);

						if(should_load){
							std::string load_val_name = std::format("{}.loadVal", this->source->getToken(param.ident).value.string);
							llvm::LoadInst* load_value = this->builder->createLoad(llvmint::ptrcast<llvm::Value>(load_addr), param_type, load_val_name);

							return llvmint::ptrcast<llvm::Value>(load_value);
						}else{
							return llvmint::ptrcast<llvm::Value>(load_addr);
						}
					} break;

					case PIR::Expr::Kind::FuncCall: {
						const PIR::FuncCall& func_call = this->source->getFuncCall(value.funcCall);

						switch(func_call.kind){
							case PIR::FuncCall::Kind::Func: {
								const PIR::Func& func = Source::getFunc(func_call.func);

								const std::vector<llvm::Value*> args = this->create_func_call_args(func_call);

								llvm::Value* return_register = llvmint::ptrcast<llvm::Value>(this->builder->createCall(func.llvmFunc, args, ".call"));

								if(should_load){
									return return_register;
								}else{
									const PIR::Type& return_type = this->src_manager->getType(func.returnType.typeID());
									llvm::AllocaInst* alloca_val = this->builder->createAlloca(this->get_type(return_type), ".call.ret");
									this->builder->createStore(alloca_val, return_register);
									return llvmint::ptrcast<llvm::Value>(alloca_val);
								}
							} break;

							case PIR::FuncCall::Kind::Intrinsic: {
								const PIR::Intrinsic& intrinsic = this->src_manager->getIntrinsic(func_call.intrinsic);

								switch(intrinsic.kind){
									///////////////////////////////////
									// add 

									case PIR::Intrinsic::Kind::addInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createAdd(lhs, rhs, false, true, ".addInt");
									} break;

									case PIR::Intrinsic::Kind::addUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createAdd(lhs, rhs, true, false, ".addUInt");
									} break;

									case PIR::Intrinsic::Kind::addISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createAdd(lhs, rhs, false, true, ".addISize");
									} break;

									case PIR::Intrinsic::Kind::addUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createAdd(lhs, rhs, true, false, ".addUSize");
									} break;


									///////////////////////////////////
									// add wrap

									case PIR::Intrinsic::Kind::addWrapInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createAdd(lhs, rhs, false, false, ".addWrapInt");
									} break;

									case PIR::Intrinsic::Kind::addWrapUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createAdd(lhs, rhs, false, false, ".addWrapUInt");
									} break;

									case PIR::Intrinsic::Kind::addWrapISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createAdd(lhs, rhs, false, false, ".addWrapISize");
									} break;

									case PIR::Intrinsic::Kind::addWrapUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createAdd(lhs, rhs, false, false, ".addWrapUSize");
									} break;


									///////////////////////////////////
									// sub

									case PIR::Intrinsic::Kind::subInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, false, true, ".subInt");
									} break;

									case PIR::Intrinsic::Kind::subUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, true, false, ".subUInt");
									} break;

									case PIR::Intrinsic::Kind::subISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, false, true, ".subISize");
									} break;

									case PIR::Intrinsic::Kind::subUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, true, false, ".subUSize");
									} break;


									///////////////////////////////////
									// sub wrap

									case PIR::Intrinsic::Kind::subWrapInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, false, false, ".subWrapInt");
									} break;

									case PIR::Intrinsic::Kind::subWrapUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, false, false, ".subWrapUInt");
									} break;

									case PIR::Intrinsic::Kind::subWrapISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, false, false, ".subWrapISize");
									} break;

									case PIR::Intrinsic::Kind::subWrapUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, false, false, ".subWrapUSize");
									} break;


									///////////////////////////////////
									// mul

									case PIR::Intrinsic::Kind::mulInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createMul(lhs, rhs, false, true, ".mulInt");
									} break;

									case PIR::Intrinsic::Kind::mulUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createMul(lhs, rhs, false, false, ".mulUInt");
									} break;

									case PIR::Intrinsic::Kind::mulISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createMul(lhs, rhs, false, true, ".mulISize");
									} break;

									case PIR::Intrinsic::Kind::mulUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createMul(lhs, rhs, false, false, ".mulUSize");
									} break;


									///////////////////////////////////
									// mul wrap

									case PIR::Intrinsic::Kind::mulWrapInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, false, false, ".mulWrapInt");
									} break;

									case PIR::Intrinsic::Kind::mulWrapUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, false, false, ".mulWrapUInt");
									} break;

									case PIR::Intrinsic::Kind::mulWrapISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, false, false, ".mulWrapISize");
									} break;

									case PIR::Intrinsic::Kind::mulWrapUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSub(lhs, rhs, false, false, ".mulWrapUSize");
									} break;


									///////////////////////////////////
									// div

									case PIR::Intrinsic::Kind::divInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSDiv(lhs, rhs, ".divInt");
									} break;

									case PIR::Intrinsic::Kind::divUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createUDiv(lhs, rhs, ".divUInt");
									} break;

									case PIR::Intrinsic::Kind::divISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createSDiv(lhs, rhs, ".divISize");
									} break;

									case PIR::Intrinsic::Kind::divUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createUDiv(lhs, rhs, ".divUSize");
									} break;


									///////////////////////////////////
									// negate

									case PIR::Intrinsic::Kind::negateInt: {
										llvm::Value* zero = llvmint::ptrcast<llvm::Value>(this->builder->valueUI64(0));
										llvm::Value* rhs = this->get_value(func_call.args[0]);
										return this->builder->createSub(zero, rhs, false, true, ".negateInt");
									} break;

									case PIR::Intrinsic::Kind::negateISize: {
										llvm::Value* zero = llvmint::ptrcast<llvm::Value>(this->builder->valueUI64(0));
										llvm::Value* rhs = this->get_value(func_call.args[0]);
										return this->builder->createSub(zero, rhs, false, true, ".negateISize");
									} break;


									///////////////////////////////////
									// logical Int

									case PIR::Intrinsic::Kind::equalInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpEQ(lhs, rhs, ".equalInt");
									} break;

									case PIR::Intrinsic::Kind::notEqualInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpNE(lhs, rhs, ".notEqualInt");
									} break;

									case PIR::Intrinsic::Kind::lessThanInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpSLT(lhs, rhs, ".lessThanInt");
									} break;

									case PIR::Intrinsic::Kind::lessThanEqualInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpSLE(lhs, rhs, ".lessThanEqualInt");
									} break;

									case PIR::Intrinsic::Kind::greaterThanInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpSGT(lhs, rhs, ".greaterThanInt");
									} break;

									case PIR::Intrinsic::Kind::greaterThanEqualInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpSGE(lhs, rhs, ".greaterThanEqualInt");
									} break;


									///////////////////////////////////
									// logical UInt

									case PIR::Intrinsic::Kind::equalUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpEQ(lhs, rhs, ".equalUInt");
									} break;

									case PIR::Intrinsic::Kind::notEqualUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpNE(lhs, rhs, ".notEqualUInt");
									} break;

									case PIR::Intrinsic::Kind::lessThanUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpULT(lhs, rhs, ".lessThanUInt");
									} break;

									case PIR::Intrinsic::Kind::lessThanEqualUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpULE(lhs, rhs, ".lessThanEqualUInt");
									} break;

									case PIR::Intrinsic::Kind::greaterThanUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpUGT(lhs, rhs, ".greaterThanUInt");
									} break;

									case PIR::Intrinsic::Kind::greaterThanEqualUInt: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpUGE(lhs, rhs, ".greaterThanEqualUInt");
									} break;


									///////////////////////////////////
									// logical Bool

									case PIR::Intrinsic::Kind::equalBool: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpEQ(lhs, rhs, ".equalBool");
									} break;

									case PIR::Intrinsic::Kind::notEqualBool: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpNE(lhs, rhs, ".notEqualBool");
									} break;

									case PIR::Intrinsic::Kind::logicalAnd: {
										llvm::BasicBlock* starting_block = this->builder->getInsertPoint();
										llvm::BasicBlock* logical_and_rhs = this->builder->createBasicBlock(
											this->current_func->llvmFunc, ".logicalAnd.rhs"
										);

										llvm::Value* lhs = this->get_value(func_call.args[0]);

										this->builder->setInsertionPoint(logical_and_rhs);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										llvm::BasicBlock* block_after_rhs = this->builder->getInsertPoint();
										llvm::BasicBlock* logical_and_end = this->builder->createBasicBlock(
											this->current_func->llvmFunc, ".logicalAnd.end"
										);
										this->builder->createBranch(logical_and_end);

										this->builder->setInsertionPoint(starting_block);
										this->builder->createCondBranch(lhs, logical_and_rhs, logical_and_end);

										this->builder->setInsertionPoint(logical_and_end);

										return this->builder->createPhi(llvmint::ptrcast<llvm::Type>(this->builder->getTypeBool()), {
											llvmint::IRBuilder::PhiIncoming(llvmint::ptrcast<llvm::Value>(this->builder->valueBool(false)), starting_block),
											llvmint::IRBuilder::PhiIncoming(rhs, block_after_rhs),
										}, ".logicalAnd");
									} break;

									case PIR::Intrinsic::Kind::logicalOr: {
										llvm::BasicBlock* starting_block = this->builder->getInsertPoint();
										llvm::BasicBlock* logical_or_rhs = this->builder->createBasicBlock(
											this->current_func->llvmFunc, ".logicalOr.rhs"
										);

										llvm::Value* lhs = this->get_value(func_call.args[0]);

										this->builder->setInsertionPoint(logical_or_rhs);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										llvm::BasicBlock* block_after_rhs = this->builder->getInsertPoint();
										llvm::BasicBlock* logical_or_end = this->builder->createBasicBlock(
											this->current_func->llvmFunc, ".logicalOr.end"
										);
										this->builder->createBranch(logical_or_end);

										this->builder->setInsertionPoint(starting_block);
										this->builder->createCondBranch(lhs, logical_or_end, logical_or_rhs);

										this->builder->setInsertionPoint(logical_or_end);

										return this->builder->createPhi(llvmint::ptrcast<llvm::Type>(this->builder->getTypeBool()), {
											llvmint::IRBuilder::PhiIncoming(llvmint::ptrcast<llvm::Value>(this->builder->valueBool(true)), starting_block),
											llvmint::IRBuilder::PhiIncoming(rhs, block_after_rhs),
										}, ".logicalOr");
									} break;

									case PIR::Intrinsic::Kind::logicalNot: {
										llvm::Value* rhs = this->get_value(func_call.args[0]);
										return this->builder->createNot(rhs, ".logicalNot");
									} break;


									///////////////////////////////////
									// logical ISize

									case PIR::Intrinsic::Kind::equalISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpEQ(lhs, rhs, ".equalISize");
									} break;

									case PIR::Intrinsic::Kind::notEqualISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpNE(lhs, rhs, ".notEqualISize");
									} break;

									case PIR::Intrinsic::Kind::lessThanISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpULT(lhs, rhs, ".lessThanISize");
									} break;

									case PIR::Intrinsic::Kind::lessThanEqualISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpULE(lhs, rhs, ".lessThanEqualISize");
									} break;

									case PIR::Intrinsic::Kind::greaterThanISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpUGT(lhs, rhs, ".greaterThanISize");
									} break;

									case PIR::Intrinsic::Kind::greaterThanEqualISize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpUGE(lhs, rhs, ".greaterThanEqualISize");
									} break;


									///////////////////////////////////
									// logical USize

									case PIR::Intrinsic::Kind::equalUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpEQ(lhs, rhs, ".equalUSize");
									} break;

									case PIR::Intrinsic::Kind::notEqualUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpNE(lhs, rhs, ".notEqualUSize");
									} break;

									case PIR::Intrinsic::Kind::lessThanUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpULT(lhs, rhs, ".lessThanUSize");
									} break;

									case PIR::Intrinsic::Kind::lessThanEqualUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpULE(lhs, rhs, ".lessThanEqualUSize");
									} break;

									case PIR::Intrinsic::Kind::greaterThanUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpUGT(lhs, rhs, ".greaterThanUSize");
									} break;

									case PIR::Intrinsic::Kind::greaterThanEqualUSize: {
										llvm::Value* lhs = this->get_value(func_call.args[0]);
										llvm::Value* rhs = this->get_value(func_call.args[1]);
										return this->builder->createICmpUGE(lhs, rhs, ".greaterThanEqualUSize");
									} break;







									///////////////////////////////////
									// type conversion Int

									case PIR::Intrinsic::Kind::convIntToUInt: {
										return this->get_value(func_call.args[0]);
									} break;

									case PIR::Intrinsic::Kind::convIntToBool: {
										llvm::Value* conversion_value = this->get_value(func_call.args[0]);
										llvm::Type* type = llvmint::ptrcast<llvm::Type>(this->builder->getTypeBool());
										return this->builder->createTrunc(conversion_value, type, ".convIntToBool");
									} break;

									case PIR::Intrinsic::Kind::convIntToISize: {
										return this->get_value(func_call.args[0]);
									} break;

									case PIR::Intrinsic::Kind::convIntToUSize: {
										return this->get_value(func_call.args[0]);
									} break;


									///////////////////////////////////
									// type conversion UInt

									case PIR::Intrinsic::Kind::convUIntToInt: {
										return this->get_value(func_call.args[0]);
									} break;

									case PIR::Intrinsic::Kind::convUIntToBool: {
										llvm::Value* conversion_value = this->get_value(func_call.args[0]);
										llvm::Type* type = llvmint::ptrcast<llvm::Type>(this->builder->getTypeBool());
										return this->builder->createTrunc(conversion_value, type, ".convUIntToBool");
									} break;

									case PIR::Intrinsic::Kind::convUIntToISize: {
										return this->get_value(func_call.args[0]);
									} break;

									case PIR::Intrinsic::Kind::convUIntToUSize: {
										return this->get_value(func_call.args[0]);
									} break;


									///////////////////////////////////
									// type conversion Bool

									case PIR::Intrinsic::Kind::convBoolToInt: {
										llvm::Value* conversion_value = this->get_value(func_call.args[0]);
										llvm::Type* type = llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64());
										return this->builder->createZExt(conversion_value, type, ".convBoolToInt");
									} break;

									case PIR::Intrinsic::Kind::convBoolToUInt: {
										llvm::Value* conversion_value = this->get_value(func_call.args[0]);
										llvm::Type* type = llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64());
										return this->builder->createZExt(conversion_value, type, ".convBoolToUInt");
									} break;

									case PIR::Intrinsic::Kind::convBoolToISize: {
										llvm::Value* conversion_value = this->get_value(func_call.args[0]);
										llvm::Type* type = llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64());
										return this->builder->createZExt(conversion_value, type, ".convBoolToISize");
									} break;

									case PIR::Intrinsic::Kind::convBoolToUSize: {
										llvm::Value* conversion_value = this->get_value(func_call.args[0]);
										llvm::Type* type = llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64());
										return this->builder->createZExt(conversion_value, type, ".convBoolToUSize");
									} break;


									///////////////////////////////////
									// type conversion ISize

									case PIR::Intrinsic::Kind::convISizeToInt: {
										return this->get_value(func_call.args[0]);
									} break;

									case PIR::Intrinsic::Kind::convISizeToUInt: {
										return this->get_value(func_call.args[0]);
									} break;

									case PIR::Intrinsic::Kind::convISizeToBool: {
										llvm::Value* conversion_value = this->get_value(func_call.args[0]);
										llvm::Type* type = llvmint::ptrcast<llvm::Type>(this->builder->getTypeBool());
										return this->builder->createTrunc(conversion_value, type, ".convISizeToBool");
									} break;

									case PIR::Intrinsic::Kind::convISizeToUSize: {
										return this->get_value(func_call.args[0]);
									} break;


									///////////////////////////////////
									// type conversion USize

									case PIR::Intrinsic::Kind::convUSizeToInt: {
										return this->get_value(func_call.args[0]);
									} break;

									case PIR::Intrinsic::Kind::convUSizeToUInt: {
										return this->get_value(func_call.args[0]);
									} break;

									case PIR::Intrinsic::Kind::convUSizeToBool: {
										llvm::Value* conversion_value = this->get_value(func_call.args[0]);
										llvm::Type* type = llvmint::ptrcast<llvm::Type>(this->builder->getTypeBool());
										return this->builder->createTrunc(conversion_value, type, ".convUSizeToBool");
									} break;

									case PIR::Intrinsic::Kind::convUSizeToISize: {
										return this->get_value(func_call.args[0]);
									} break;
								};

								evo::debugFatalBreak("Unkown intrinsic");
							} break;
						};

						evo::debugFatalBreak("Unkown func call kind");

					} break;

					case PIR::Expr::Kind::Initializer: {
						const PIR::Initializer& initializer = this->source->getInitializer(value.initializer);
						const PIR::Type& initializer_type = this->src_manager->getType(initializer.type);

						llvm::Type* initializer_llvm_type = this->get_type(initializer_type);

						llvm::AllocaInst* init_alloca = this->builder->createAlloca(initializer_llvm_type, ".alloca.initializer");
						for(size_t i = 0; i < initializer.memberVals.size(); i+=1){
							if(initializer.memberVals[i].kind == PIR::Expr::Kind::None){ continue; } // skip {none} values

							const std::string gep_name = std::format(".alloca.initializer.{}.GEP", i);

							llvm::Value* gep_value = this->builder->createGEP(init_alloca, {0, int32_t(i)}, gep_name);
							llvm::Value* member_value = this->get_value(initializer.memberVals[i]);
							this->builder->createStore(gep_value, member_value);
						}

						if(should_load){
							return llvmint::ptrcast<llvm::Value>(this->builder->createLoad(init_alloca, ".alloca.initializer.load"));
						}else{
							return llvmint::ptrcast<llvm::Value>(init_alloca);
						}

					} break;

					case PIR::Expr::Kind::Prefix: {
						const PIR::Prefix& prefix = this->source->getPrefix(value.prefix);

						switch(this->source->getToken(prefix.op).kind){
							case Token::KeywordCopy: {
								return this->get_value(prefix.rhs);
							} break;

							case Token::KeywordAddr: {
								if(prefix.rhs.kind == PIR::Expr::Kind::Var){
									const PIR::Var& var = Source::getVar(prefix.rhs.var);
									if(var.is_alloca){
										return llvmint::ptrcast<llvm::Value>(var.llvm.alloca);
									}else{
										return llvmint::ptrcast<llvm::Value>(var.llvm.global);
									}
									
								}else if(prefix.rhs.kind == PIR::Expr::Kind::Accessor){
									return this->get_value(prefix.rhs, false);
									
								}else{
									evo::debugAssert(prefix.rhs.kind == PIR::Expr::Kind::Param, "unknown rhs of addr stmt");

									const PIR::Param& param = this->source->getParam(prefix.rhs.param);

									std::string load_addr_name = std::format("{}.loadAddr", this->source->getToken(param.ident).value.string);
									llvm::LoadInst* load_addr = this->builder->createLoad(param.alloca, load_addr_name);

									return llvmint::ptrcast<llvm::Value>(load_addr);
								}

							} break;

						};

						evo::debugFatalBreak("Invalid or unknown prefix operator");
					} break;


					case PIR::Expr::Kind::Deref: {
						const PIR::Deref& deref = this->source->getDeref(value.deref);

						llvm::Value* lhs_value = this->get_value(deref.ptr);

						if(should_load){
							llvm::Type* deref_type = this->get_type(this->src_manager->getType(deref.type));
							return llvmint::ptrcast<llvm::Value>(this->builder->createLoad(lhs_value, deref_type, ".deref"));
						}else{
							return lhs_value;
						}
					} break;


					case PIR::Expr::Kind::Accessor: {
						const PIR::Accessor& accessor = this->source->getAccessor(value.accessor);
						const PIR::Type& lhs_type = this->src_manager->getType(accessor.lhsType);
						const PIR::BaseType& lhs_base_type = this->src_manager->getBaseType(lhs_type.baseType);
						const PIR::BaseType::StructData& struct_data = std::get<PIR::BaseType::StructData>(lhs_base_type.data);

						int32_t member_index = std::numeric_limits<int32_t>::max(); // gave it a value to make it stop complaining
						auto member_type_id = std::optional<PIR::Type::ID>();
						for(size_t i = 0; i < struct_data.memberVars.size(); i+=1){
							const PIR::BaseType::StructData::MemberVar& member = struct_data.memberVars[i];
							if(member.name == accessor.rhs){
								member_index = int32_t(i);
								member_type_id = member.type;
								break;
							}
						}
						evo::debugAssert(member_type_id.has_value(), "uncaught unknown member");


						llvm::Value* lhs_value = this->get_value(accessor.lhs, false);
						llvm::Type* lhs_llvm_type = this->get_type(lhs_type);
						const std::string gep_name = std::format("{}.GEP", accessor.rhs);

						llvm::Value* gep_value = this->builder->createGEP(lhs_value, lhs_llvm_type, {0, member_index}, gep_name);

						if(should_load){
							const PIR::Type& member_type = this->src_manager->getType(*member_type_id);
							llvm::Type* member_llvm_type = this->get_type(member_type);
							const std::string load_name = std::format("{}.load", accessor.rhs);
							return llvmint::ptrcast<llvm::Value>(this->builder->createLoad(gep_value, member_llvm_type, load_name));
						}else{
							return gep_value;
						}
					} break;

				};


				evo::debugFatalBreak("Invalid value kind");
			};


			EVO_NODISCARD inline auto get_concrete_value(const PIR::Expr& expr) noexcept -> llvm::Value* {
				switch(expr.kind){
					case PIR::Expr::Kind::Var: {
						const PIR::Var& var = Source::getVar(expr.var);

						if(var.is_alloca){
							return llvmint::ptrcast<llvm::Value>(var.llvm.alloca);
						}else{
							return llvmint::ptrcast<llvm::Value>(var.llvm.global);
						}
					} break;

					case PIR::Expr::Kind::Param: {
						const PIR::Param& param = this->source->getParam(expr.param);
						const std::string load_name = std::format("{}.load", this->source->getToken(param.ident).value.string);
						return llvmint::ptrcast<llvm::Value>(this->builder->createLoad(param.alloca, load_name));
					} break;


					case PIR::Expr::Kind::Deref: {
						const PIR::Deref& deref = this->source->getDeref(expr.deref);
						
						return this->get_value(deref.ptr);
					} break;

					case PIR::Expr::Kind::Accessor: {
						const PIR::Accessor& accessor = this->source->getAccessor(expr.accessor);
						const PIR::Type& lhs_type = this->src_manager->getType(accessor.lhsType);
						const PIR::BaseType& lhs_base_type = this->src_manager->getBaseType(lhs_type.baseType);
						const PIR::BaseType::StructData& struct_data = std::get<PIR::BaseType::StructData>(lhs_base_type.data);

						const int32_t member_index = [&]() noexcept {
							for(size_t i = 0; i < struct_data.memberVars.size(); i+=1){
								if(struct_data.memberVars[i].name == accessor.rhs){
									return int32_t(i);
								}
							}

							evo::debugFatalBreak("uncaught unknown member");
						}();

						llvm::Value* lhs_value = this->get_value(accessor.lhs, false);
						llvm::Type* lhs_llvm_type = this->get_type(lhs_type);

						return this->builder->createGEP(lhs_value, lhs_llvm_type, {0, member_index}, std::format("{}.GEP", accessor.rhs));
					} break;

					default: evo::debugFatalBreak("Unknown or unsupported concrete expr kind");
				};
			};


			EVO_NODISCARD inline auto get_arg_value(const PIR::Expr& arg, PIR::Type::ID param_type_id) noexcept -> llvm::Value* {

				switch(arg.kind){
					case PIR::Expr::Kind::Var: {
						const PIR::Var& var = Source::getVar(arg.var);
						if(var.is_alloca){
							return llvmint::ptrcast<llvm::Value>(var.llvm.alloca);
						}else{
							return llvmint::ptrcast<llvm::Value>(var.llvm.global);
						}
					} break;

					case PIR::Expr::Kind::Param: {
						const PIR::Param& value_param = this->source->getParam(arg.param);
						const std::string load_name = std::format("{}.load", this->source->getToken(value_param.ident).value.string);
						return llvmint::ptrcast<llvm::Value>(this->builder->createLoad(value_param.alloca));
					} break;

					case PIR::Expr::Kind::ASTNode: {
						llvm::Value* temporary = this->get_value(arg);


						const PIR::Type& param_type = this->src_manager->getType(param_type_id);
						llvm::Type* arg_type = this->get_type(param_type);

						llvm::AllocaInst* temporary_storage = this->builder->createAlloca(arg_type, "temp_storage");
						this->builder->createStore(temporary_storage, temporary);

						return llvmint::ptrcast<llvm::Value>(temporary_storage);
					} break;

					case PIR::Expr::Kind::FuncCall: {
						llvm::Value* temporary = this->get_value(arg);

						const PIR::Type& param_type = this->src_manager->getType(param_type_id);
						llvm::Type* arg_type = this->get_type(param_type);

						llvm::AllocaInst* temporary_storage = this->builder->createAlloca(arg_type, "temp_storage");
						this->builder->createStore(temporary_storage, temporary);

						return llvmint::ptrcast<llvm::Value>(temporary_storage);
					} break;

					case PIR::Expr::Kind::Prefix: {
						const PIR::Prefix& prefix = this->source->getPrefix(arg.prefix);

						switch(this->source->getToken(prefix.op).kind){
							case Token::KeywordCopy: {
								return this->get_arg_value(prefix.rhs, param_type_id);
							} break;

							case Token::KeywordAddr: {
								llvm::Value* temporary = this->get_value(arg);

								llvm::AllocaInst* temporary_storage = this->builder->createAlloca(
									llvmint::ptrcast<llvm::Type>(this->builder->getTypePtr()), "temp_storage"
								);
								this->builder->createStore(temporary_storage, temporary);

								return llvmint::ptrcast<llvm::Value>(temporary_storage);
							} break;

						};

						evo::debugFatalBreak("Invalid or unknown prefix operator");
					} break;

					case PIR::Expr::Kind::Deref: {
						const PIR::Deref& deref = this->source->getDeref(arg.deref);
						
						return this->get_value(deref.ptr);
					} break;

					case PIR::Expr::Kind::Accessor: {
						return this->get_value(arg, false);
					} break;
				};

				evo::debugFatalBreak("Unknown or unsupported arg value type");
			};








			EVO_NODISCARD inline static auto mangle_name(const Source& source, const PIR::Func& func) noexcept -> std::string {
				const std::string ident = std::string(source.getToken(func.ident).value.string);

				if(func.isExport){
					return ident;
				}else{
					std::string base_name = std::format("PTHR.{}.{}", source.getID().id, ident);

					if(func.params.empty()){
						return base_name;
					}

					base_name += '-';


					for(size_t i = 0; i < func.params.size(); i+=1){
						const PIR::Param::ID param_id = func.params[i];
						const PIR::Param& param = source.getParam(param_id);

						base_name += std::to_string(param.type.id);

						switch(param.kind){
							break; case AST::FuncParams::Param::Kind::Read: base_name += 'r';
							break; case AST::FuncParams::Param::Kind::Write: base_name += 'w';
							break; default: evo::debugFatalBreak("Unknown param kind");
						};

						if(i < func.params.size() - 1){
							base_name += '-';
						}
					}

					return base_name;
				}
			};


			
			EVO_NODISCARD inline static auto mangle_name(const Source& source, const PIR::Struct& struct_decl) noexcept -> std::string {
				const std::string ident = std::string(source.getToken(struct_decl.ident).value.string);

				return std::format("PTHR.{}.{}", source.getID().id, ident);
			};


			// should only be used for globals
			EVO_NODISCARD inline static auto mangle_name(const Source& source, const PIR::Var& var) noexcept -> std::string {
				evo::debugAssert(var.isGlobal(), "Variable name mangling should only be used on globals");

				const std::string ident = std::string(source.getToken(var.ident).value.string);

				if(var.isExport){
					return ident;
				}else{
					return std::format("PTHR.{}.{}", source.getID().id, ident);
				}
			};



		private:
			llvmint::IRBuilder* builder = nullptr;
			llvmint::Module* module = nullptr;
			PIR::Func* current_func = nullptr;

			struct /* libc */ {
				llvm::Function* puts = nullptr;
				llvm::Function* printf = nullptr;
			} libc;

			Source* source = nullptr;
			SourceManager* src_manager = nullptr;
	};


};