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
					EVO_FATAL_BREAK(data_layout_err);
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
				std::vector<Source>& sources = source_manager.getSources();

				for(Source& source : sources){
					for(PIR::Var::ID global_var_id : source.pir.global_vars){
						PIR::Var& var = source.getVar(global_var_id);

						this->lower_global_var(source, var);
					}


					for(PIR::Func& func : source.pir.funcs){
						this->lower_func(source, func);
					}
				}
			};



			EVO_NODISCARD inline auto initLibC() noexcept -> void {
				llvm::FunctionType* printf_proto = this->builder->getFuncProto(
					this->builder->getTypeVoid(), { llvmint::ptrcast<llvm::Type>(this->builder->getTypePtr()) }, true
				);
				this->libc.printf = this->module->createFunction("printf", printf_proto, llvmint::LinkageTypes::ExternalLinkage, true, false);
			};




			EVO_NODISCARD inline auto addRuntime(SourceManager& source_manager, const SourceManager::Entry& entry_func) noexcept -> void {
				const Source& source = source_manager.getSource(entry_func.src_id);
				const PIR::Func& func = source.getFunc(entry_func.func_id);

				llvm::FunctionType* prototype = this->builder->getFuncProto(llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64()), {}, false);
				llvm::Function* main_func = this->module->createFunction("main", prototype, llvmint::LinkageTypes::ExternalLinkage, true, false);

				llvm::BasicBlock* entry_block = this->builder->createBasicBlock(main_func, "entry");
				this->builder->setInsertionPoint(entry_block);

				llvm::Value* entry_ret = llvmint::ptrcast<llvm::Value>(this->builder->createCall(func.llvm_func, {}, '\0'));
				this->builder->createRet(entry_ret);
			};




			EVO_NODISCARD auto printLLVMIR() const noexcept -> std::string {
				return this->module->print();
			};


			// return nullopt means target machine cannot output object file
			EVO_NODISCARD auto compileToObjectFile() noexcept -> std::optional< std::vector<evo::byte> > {
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
			inline auto lower_global_var(const Source& source, PIR::Var& var) noexcept -> void {
				const std::string mangled_name = PIRToLLVMIR::mangle_name(source, var);

				const SourceManager& source_manager = source.getSourceManager();
				const PIR::Type& type = source_manager.getType(var.type);

				llvm::Type* llvm_type = this->get_type(source_manager, type);
				llvm::Constant* value = nullptr;

				if(var.value.kind == PIR::Expr::Kind::ASTNode){
					if(source.getNode(var.value.ast_node).kind != AST::Kind::Uninit){
						value = this->get_const_value(source, var.value);
					}

				}else{
					EVO_DEBUG_ASSERT("Invalid kind of global var value");
				}

				llvm::GlobalVariable* global_val = this->builder->valueGlobal(*this->module, value, llvm_type, false, mangled_name.c_str());

				var.llvm.value = llvmint::ptrcast<llvm::Value>(global_val);
			};



			inline auto lower_stmt(Source& source, const PIR::Stmt& stmt) noexcept -> void {
				switch(stmt.kind){
					break; case PIR::Stmt::Kind::Var: this->lower_var(source, source.getVar(stmt.var));
					break; case PIR::Stmt::Kind::Conditional: this->lower_conditional(source, source.getConditional(stmt.conditional));
					break; case PIR::Stmt::Kind::Return: this->lower_return(source, source.getReturn(stmt.ret));
					break; case PIR::Stmt::Kind::Assignment: this->lower_assignment(source, source.getAssignment(stmt.assignment));
					break; case PIR::Stmt::Kind::FuncCall: this->lower_func_call(source, source.getFuncCall(stmt.func_call));
					break; default: EVO_FATAL_BREAK("Unknown stmt kind");
				};
			};



			inline auto lower_func(Source& source, PIR::Func& func) noexcept -> void {
				this->current_func = &func;
				const std::string mangled_name = PIRToLLVMIR::mangle_name(source, func);

				llvm::Type* return_type = this->builder->getTypeVoid();
				if(func.return_type.has_value()){
					const SourceManager& source_manager = source.getSourceManager();
					const PIR::Type& return_type_obj = source_manager.getType(*func.return_type);
					return_type = this->get_type(source_manager, return_type_obj);
				}



				llvm::FunctionType* prototype = this->builder->getFuncProto(return_type, {}, false);
				const auto linkage = func.is_export ? llvmint::LinkageTypes::ExternalLinkage : llvmint::LinkageTypes::InternalLinkage;
				const bool fast_call_conv = !func.is_export;
				llvm::Function* llvm_func = this->module->createFunction(mangled_name, prototype, linkage, true, fast_call_conv);

				func.llvm_func = llvm_func;


				llvm::BasicBlock* entry = this->builder->createBasicBlock(llvm_func, "entry");
				this->builder->setInsertionPoint(entry);


				for(const PIR::Stmt& stmt : func.stmts){
					this->lower_stmt(source, stmt);
				}


				if(func.has_return_stmt == false){
					this->builder->createRet();
				}

				this->current_func = nullptr;
			};



			inline auto lower_var(Source& source, PIR::Var& var) noexcept -> void {
				const std::string ident = std::string(source.getToken(var.ident).value.string);

				const SourceManager& source_manager = source.getSourceManager();
				const PIR::Type& type = source_manager.getType(var.type);

				llvm::Type* llvm_type = this->get_type(source_manager, type);

				llvm::AllocaInst* alloca_val = this->builder->createAlloca(llvm_type, ident);

				var.llvm.alloca = alloca_val;
				var.is_alloca = true;


				if(var.value.kind == PIR::Expr::Kind::ASTNode){
					const AST::Node& var_value_node = source.getNode(var.value.ast_node);
					if(var_value_node.kind != AST::Kind::Uninit){
						this->builder->createStore(alloca_val, this->get_value(source, var.value), false);
					}
					
				}else{
					this->builder->createStore(alloca_val, this->get_value(source, var.value), false);
				}

			};


			inline auto lower_conditional(Source& source, PIR::Conditional& cond) noexcept -> void {
				llvm::BasicBlock* then_block = this->builder->createBasicBlock(this->current_func->llvm_func, "if.then");
				llvm::BasicBlock* end_block = nullptr;

				llvm::Value* cond_value = this->get_value(source, cond.if_cond);

				if(cond.else_stmts.empty()){
					end_block = this->builder->createBasicBlock(this->current_func->llvm_func, "if.end");

					this->builder->createCondBranch(cond_value, then_block, end_block);

					this->builder->setInsertionPoint(then_block);
					for(const PIR::Stmt& stmt : cond.then_stmts){
						this->lower_stmt(source, stmt);
					}
					this->builder->createBranch(end_block);

				}else{
					llvm::BasicBlock* else_block = this->builder->createBasicBlock(this->current_func->llvm_func, "if.else");

					this->builder->createCondBranch(cond_value, then_block, else_block);

					// then block
					this->builder->setInsertionPoint(then_block);
					for(const PIR::Stmt& stmt : cond.then_stmts){
						this->lower_stmt(source, stmt);
					}

					// else block
					this->builder->setInsertionPoint(else_block);
					for(const PIR::Stmt& stmt : cond.else_stmts){
						this->lower_stmt(source, stmt);
					}

					// end block
					end_block = this->builder->createBasicBlock(this->current_func->llvm_func, "if.end");
					this->builder->createBranch(end_block);
					this->builder->setInsertionPoint(then_block);
					this->builder->createBranch(end_block);
				}

				this->builder->setInsertionPoint(end_block);
			};


			inline auto lower_return(Source& source, PIR::Return& ret) noexcept -> void {
				if(ret.value.has_value()){
					this->builder->createRet(this->get_value(source, *ret.value));

				}else{
					this->builder->createRet();
				}
			};



			inline auto lower_assignment(Source& source, PIR::Assignment& assignment) noexcept -> void {
				evo::debugAssert(
					source.getToken(assignment.op).kind == Token::get("="),
					"Only normal assignment (=) is supported for lowering at the moment"
				);


				llvm::Value* dst = this->get_concrete_value(source, assignment.dst);
				llvm::Value* value = this->get_value(source, assignment.value);

				this->builder->createStore(dst, value, false);
			};



			inline auto lower_func_call(Source& source, PIR::FuncCall& func_call) noexcept -> void {
				switch(func_call.kind){
					case PIR::FuncCall::Kind::Func: {
						const PIR::Func& func = source.getFunc(func_call.func);
						this->builder->createCall(func.llvm_func, {}, '\0');
					} break;


					case PIR::FuncCall::Kind::Intrinsic: {
						const PIR::Intrinsic& intrinsic = source.getSourceManager().getIntrinsic(func_call.intrinsic);

						switch(intrinsic.kind){
							case PIR::Intrinsic::Kind::__printHelloWorld: {
								evo::debugAssert(this->libc.printf != nullptr, "libc was not initialized");

								static llvm::GlobalVariable* hello_world_str = this->builder->valueString("Hello World, I'm Panther!\n", "hello_world_str");
								this->builder->createCall(this->libc.printf, { llvmint::ptrcast<llvm::Value>(hello_world_str) });
							} break;

							case PIR::Intrinsic::Kind::breakpoint: {
								this->builder->createIntrinsicCall(llvmint::IRBuilder::IntrinsicID::debugtrap, {});
							} break;

							default: {
								EVO_FATAL_BREAK("Unknown intrinsic");
							};
						};
					} break;


					default: EVO_FATAL_BREAK("Unknown func call kind");
				};
			};





			EVO_NODISCARD inline auto get_type(const SourceManager& source_manager, const PIR::Type& type) noexcept -> llvm::Type* {
				if(type.qualifiers.empty() == false){
					if(type.qualifiers.back().is_ptr){
						return llvmint::ptrcast<llvm::Type>(this->builder->getTypePtr());
					}else{
						EVO_FATAL_BREAK("Unsupported qualifiers");
					}
				}

				const PIR::BaseType& base_type = source_manager.getBaseType(type.base_type);


				if(base_type.builtin.kind == Token::TypeInt){
					// TODO: make sure is register sized
					return llvmint::ptrcast<llvm::Type>(this->builder->getTypeI64());

				}else if(base_type.builtin.kind == Token::TypeBool){
					return llvmint::ptrcast<llvm::Type>(this->builder->getTypeBool());
				}

				EVO_FATAL_BREAK("Unknown type");
			};



			EVO_NODISCARD inline auto get_const_value(const Source& source, PIR::Expr value) noexcept -> llvm::Constant* {
				evo::debugAssert(value.kind == PIR::Expr::Kind::ASTNode, "Value is not constant");

				const Token& token = source.getLiteral(value.ast_node);

				switch(token.kind){
					case Token::LiteralInt: {
						return llvmint::ptrcast<llvm::Constant>(this->builder->valueUI64(token.value.integer));
					} break;

					case Token::LiteralBool: {
						return llvmint::ptrcast<llvm::Constant>(this->builder->valueBool(token.value.boolean));
					} break;
				};

				EVO_FATAL_BREAK("Invalid value kind");
			};



			EVO_NODISCARD inline auto get_value(const Source& source, PIR::Expr value) noexcept -> llvm::Value* {
				switch(value.kind){
					case PIR::Expr::Kind::ASTNode: {
						const AST::Node& node = source.getNode(value.ast_node);

						switch(node.kind){
							case AST::Kind::Literal: {
								const Token& token = source.getLiteral(value.ast_node);

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
								EVO_FATAL_BREAK("Unknown AST::Kind");
							} break;
						};
					} break;

					case PIR::Expr::Kind::Var: {
						const PIR::Var& var = source.getVar(value.var);
						if(var.is_alloca){
							return llvmint::ptrcast<llvm::Value>(this->builder->createLoad(var.llvm.alloca));
						}else{
							const SourceManager& source_manager = source.getSourceManager();
							llvm::Type* var_type = this->get_type(source_manager, source_manager.getType(var.type));
							return llvmint::ptrcast<llvm::Value>(this->builder->createLoad(var.llvm.value, var_type));
						}
					} break;

					case PIR::Expr::Kind::FuncCall: {
						const PIR::FuncCall& func_call = source.getFuncCall(value.func_call);
						const PIR::Func& func = source.getFunc(func_call.func);

						return llvmint::ptrcast<llvm::Value>(this->builder->createCall(func.llvm_func, {}, '\0'));
					} break;

					case PIR::Expr::Kind::Prefix: {
						const PIR::Prefix& prefix = source.getPrefix(value.prefix);

						switch(source.getToken(prefix.op).kind){
							case Token::KeywordCopy: {
								return this->get_value(source, prefix.rhs);
							} break;

							case Token::KeywordAddr: {
								const PIR::Var& var = source.getVar(prefix.rhs.var);
								if(var.is_alloca){
									return llvmint::ptrcast<llvm::Value>(var.llvm.alloca);
								}else{
									return var.llvm.value;
								}

							} break;

							default: EVO_FATAL_BREAK("Invalid or unknown prefix operator");
						};
					} break;


					case PIR::Expr::Kind::Deref: {
						const PIR::Deref& deref = source.getDeref(value.deref);

						llvm::Value* lhs_value = this->get_value(source, deref.ptr);

						const SourceManager& source_manager = source.getSourceManager();
						llvm::Type* deref_type = this->get_type(source_manager, source_manager.getType(deref.type));
						return llvmint::ptrcast<llvm::Value>(this->builder->createLoad(lhs_value, deref_type));
					} break;

				};


				EVO_FATAL_BREAK("Invalid value kind");
			};


			EVO_NODISCARD inline auto get_concrete_value(const Source& source, const PIR::Expr& expr) noexcept -> llvm::Value* {
				switch(expr.kind){
					case PIR::Expr::Kind::Var: {
						const PIR::Var& var = source.getVar(expr.var);

						if(var.is_alloca){
							return llvmint::ptrcast<llvm::Value>(var.llvm.alloca);
						}else{
							return var.llvm.value;
						}
					} break;


					case PIR::Expr::Kind::Deref: {
						const PIR::Deref& deref = source.getDeref(expr.deref);
						
						return this->get_value(source, deref.ptr);
					} break;

					default: EVO_FATAL_BREAK("Unknown or unsupported concrete expr kind");
				};
			};




			EVO_NODISCARD inline static auto mangle_name(const Source& source, const PIR::Func& func) noexcept -> std::string {
				const std::string ident = std::string(source.getToken(func.ident).value.string);

				if(func.is_export){
					return ident;
				}else{
					return std::format("P.{}.{}", source.getID().id, ident);
				}
			};


			// should only be used for globals
			EVO_NODISCARD inline static auto mangle_name(const Source& source, const PIR::Var& var) noexcept -> std::string {
				evo::debugAssert(var.isGlobal(), "Variable name mangling should only be used on globals");

				const std::string ident = std::string(source.getToken(var.ident).value.string);
				
				return std::format("P.{}.{}", source.getID().id, ident);
			};



		private:
			llvmint::IRBuilder* builder = nullptr;
			llvmint::Module* module = nullptr;
			PIR::Func* current_func = nullptr;

			struct /* libc */ {
				llvm::Function* printf = nullptr;
			} libc;
	};


};