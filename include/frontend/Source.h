#pragma once


#include <Evo.h>

#include "./SourceID.h"

#include "./Token.h"
#include "./AST.h"
#include "Message.h"
#include "PIR.h"

#include <filesystem>
#include <unordered_map>

namespace panther{


	class Source{
		public:
			using ID = SourceID;

			struct Config{
				bool allowStructMemberTypeInference = false;
			};

		public:
			// TODO: other permutations of refs
			Source(std::filesystem::path&& src_path, std::string&& src_data, const Config& _config, class SourceManager& src_manager, ID id) noexcept
				: src_location(std::move(src_path)), data(std::move(src_data)), config(_config), source_manager(src_manager), src_id(id) {};

			~Source() noexcept {
				for(std::string* str_ptr : this->string_literal_values){
					delete str_ptr;
				}
			};



			EVO_NODISCARD inline auto getLocation() const noexcept -> const std::filesystem::path& { return this->src_location; };
			EVO_NODISCARD inline auto getData() const noexcept -> const std::string& { return this->data; };
			EVO_NODISCARD inline auto getConfig() const noexcept -> const Config& { return this->config; };
			EVO_NODISCARD inline auto getSourceManager() noexcept -> SourceManager& { return this->source_manager; };
			EVO_NODISCARD inline auto getSourceManager() const noexcept -> const SourceManager& { return this->source_manager; };
			EVO_NODISCARD inline auto getID() const noexcept -> ID { return this->src_id; };

			// returns true if successful (no errors)
			EVO_NODISCARD auto tokenize() noexcept -> bool;

			// returns true if successful (no errors)
			EVO_NODISCARD auto parse() noexcept -> bool;

			// returns true if successful (no errors)
			EVO_NODISCARD auto semantic_analysis_global_idents_and_imports() noexcept -> bool;
			EVO_NODISCARD auto semantic_analysis_global_aliases() noexcept -> bool;
			EVO_NODISCARD auto semantic_analysis_global_types() noexcept -> bool;
			EVO_NODISCARD auto semantic_analysis_global_values() noexcept -> bool;
			EVO_NODISCARD auto semantic_analysis_runtime() noexcept -> bool;


			//////////////////////////////////////////////////////////////////////
			// getting / creating

			///////////////////////////////////
			// AST

			EVO_NODISCARD auto getNode(AST::Node::ID node_id) const noexcept -> const AST::Node&;
			EVO_NODISCARD auto getToken(Token::ID token_id) const noexcept -> const Token&;


			EVO_NODISCARD auto getVarDecl(AST::Node::ID node_id) const noexcept -> const AST::VarDecl&;
			EVO_NODISCARD auto getVarDecl(const AST::Node& node) const noexcept -> const AST::VarDecl&;

			EVO_NODISCARD auto getFunc(AST::Node::ID node_id) const noexcept -> const AST::Func&;
			EVO_NODISCARD auto getFunc(const AST::Node& node) const noexcept -> const AST::Func&;

			EVO_NODISCARD auto getStruct(AST::Node::ID node_id) const noexcept -> const AST::Struct&;
			EVO_NODISCARD auto getStruct(const AST::Node& node) const noexcept -> const AST::Struct&;

			EVO_NODISCARD auto getTemplatePack(AST::Node::ID node_id) const noexcept -> const AST::TemplatePack&;
			EVO_NODISCARD auto getTemplatePack(const AST::Node& node) const noexcept -> const AST::TemplatePack&;

			EVO_NODISCARD auto getFuncParams(AST::Node::ID node_id) const noexcept -> const AST::FuncParams&;
			EVO_NODISCARD auto getFuncParams(const AST::Node& node) const noexcept -> const AST::FuncParams&;

			EVO_NODISCARD auto getConditional(AST::Node::ID node_id) const noexcept -> const AST::Conditional&;
			EVO_NODISCARD auto getConditional(const AST::Node& node) const noexcept -> const AST::Conditional&;

			EVO_NODISCARD auto getReturn(AST::Node::ID node_id) const noexcept -> const AST::Return&;
			EVO_NODISCARD auto getReturn(const AST::Node& node) const noexcept -> const AST::Return&;

			EVO_NODISCARD auto getAlias(AST::Node::ID node_id) const noexcept -> const AST::Alias&;
			EVO_NODISCARD auto getAlias(const AST::Node& node) const noexcept -> const AST::Alias&;


			EVO_NODISCARD auto getType(AST::Node::ID node_id) const noexcept -> const AST::Type&;
			EVO_NODISCARD auto getType(const AST::Node& node) const noexcept -> const AST::Type&;

			EVO_NODISCARD auto getBlock(AST::Node::ID node_id) const noexcept -> const AST::Block&;
			EVO_NODISCARD auto getBlock(const AST::Node& node) const noexcept -> const AST::Block&;

			EVO_NODISCARD auto getPrefix(AST::Node::ID node_id) const noexcept -> const AST::Prefix&;
			EVO_NODISCARD auto getPrefix(const AST::Node& node) const noexcept -> const AST::Prefix&;

			EVO_NODISCARD auto getInfix(AST::Node::ID node_id) const noexcept -> const AST::Infix&;
			EVO_NODISCARD auto getInfix(const AST::Node& node) const noexcept -> const AST::Infix&;

			EVO_NODISCARD auto getPostfix(AST::Node::ID node_id) const noexcept -> const AST::Postfix&;
			EVO_NODISCARD auto getPostfix(const AST::Node& node) const noexcept -> const AST::Postfix&;

			EVO_NODISCARD auto getFuncCall(AST::Node::ID node_id) const noexcept -> const AST::FuncCall&;
			EVO_NODISCARD auto getFuncCall(const AST::Node& node) const noexcept -> const AST::FuncCall&;

			EVO_NODISCARD auto getInitializer(AST::Node::ID node_id) const noexcept -> const AST::Initializer&;
			EVO_NODISCARD auto getInitializer(const AST::Node& node) const noexcept -> const AST::Initializer&;


			EVO_NODISCARD auto getLiteral(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getLiteral(const AST::Node& node) const noexcept -> const Token&;

			EVO_NODISCARD auto getIdent(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getIdent(const AST::Node& node) const noexcept -> const Token&;

			EVO_NODISCARD auto getIntrinsic(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getIntrinsic(const AST::Node& node) const noexcept -> const Token&;

			EVO_NODISCARD auto getUninit(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getUninit(const AST::Node& node) const noexcept -> const Token&;

			EVO_NODISCARD auto getUnreachable(AST::Node::ID node_id) const noexcept -> const Token&;
			EVO_NODISCARD auto getUnreachable(const AST::Node& node) const noexcept -> const Token&;




			///////////////////////////////////
			// PIR

			EVO_NODISCARD inline auto createVar(auto&&... args) noexcept -> PIR::Var::ID {
				this->pir.vars.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Var::ID(*this, uint32_t(this->pir.vars.size() - 1));
			};

			// EVO_NODISCARD static inline auto getVar(const PIR::Var::ID& id) noexcept -> const PIR::Var& {
			// 	return id.source.pir.vars[size_t(id.id)];
			// };
			EVO_NODISCARD static inline auto getVar(PIR::Var::ID id) noexcept -> PIR::Var& {
				return id.source.pir.vars[size_t(id.id)];
			};


			EVO_NODISCARD inline auto createFunc(auto&&... args) noexcept -> PIR::Func::ID {
				this->pir.funcs.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Func::ID(*this, uint32_t(this->pir.funcs.size() - 1));
			};

			// EVO_NODISCARD static inline auto getFunc(const PIR::Func::ID& id) noexcept -> const PIR::Func& {
			// 	return id.source.pir.funcs[size_t(id.id)];
			// };
			EVO_NODISCARD static inline auto getFunc(PIR::Func::ID id) noexcept -> PIR::Func& {
				return id.source.pir.funcs[size_t(id.id)];
			};


			EVO_NODISCARD inline auto createStruct(auto&&... args) noexcept -> PIR::Struct::ID {
				this->pir.structs.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Struct::ID(*this, uint32_t(this->pir.structs.size() - 1));
			};

			// EVO_NODISCARD static inline auto getStruct(const PIR::Struct::ID& id) noexcept -> const PIR::Struct& {
			// 	return id.source.pir.structs[size_t(id.id)];
			// };
			EVO_NODISCARD static inline auto getStruct(PIR::Struct::ID id) noexcept -> PIR::Struct& {
				return id.source.pir.structs[size_t(id.id)];
			};
			


			EVO_NODISCARD inline auto createParam(auto&&... args) noexcept -> PIR::Param::ID {
				this->pir.params.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Param::ID( uint32_t(this->pir.params.size() - 1) );
			};

			EVO_NODISCARD inline auto getParam(PIR::Param::ID id) const noexcept -> const PIR::Param& {
				return this->pir.params[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getParam(PIR::Param::ID id) noexcept -> PIR::Param& {
				return this->pir.params[size_t(id.id)];
			};


			EVO_NODISCARD inline auto createConditional(auto&&... args) noexcept -> PIR::Conditional::ID {
				this->pir.conditionals.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Conditional::ID( uint32_t(this->pir.conditionals.size() - 1) );
			};

			EVO_NODISCARD inline auto getConditional(PIR::Conditional::ID id) const noexcept -> const PIR::Conditional& {
				return this->pir.conditionals[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getConditional(PIR::Conditional::ID id) noexcept -> PIR::Conditional& {
				return this->pir.conditionals[size_t(id.id)];
			};



			EVO_NODISCARD inline auto createReturn(auto&&... args) noexcept -> PIR::Return::ID {
				this->pir.returns.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Return::ID( uint32_t(this->pir.returns.size() - 1) );
			};

			EVO_NODISCARD inline auto getReturn(PIR::Return::ID id) const noexcept -> const PIR::Return& {
				return this->pir.returns[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getReturn(PIR::Return::ID id) noexcept -> PIR::Return& {
				return this->pir.returns[size_t(id.id)];
			};



			EVO_NODISCARD inline auto createAssignment(auto&&... args) noexcept -> PIR::Assignment::ID {
				this->pir.assignments.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Assignment::ID( uint32_t(this->pir.assignments.size() - 1) );
			};

			EVO_NODISCARD inline auto getAssignment(PIR::Assignment::ID id) const noexcept -> const PIR::Assignment& {
				return this->pir.assignments[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getAssignment(PIR::Assignment::ID id) noexcept -> PIR::Assignment& {
				return this->pir.assignments[size_t(id.id)];
			};



			EVO_NODISCARD inline auto createFuncCall(auto&&... args) noexcept -> PIR::FuncCall::ID {
				this->pir.func_calls.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::FuncCall::ID( uint32_t(this->pir.func_calls.size() - 1) );
			};

			EVO_NODISCARD inline auto getFuncCall(PIR::FuncCall::ID id) const noexcept -> const PIR::FuncCall& {
				return this->pir.func_calls[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getFuncCall(PIR::FuncCall::ID id) noexcept -> PIR::FuncCall& {
				return this->pir.func_calls[size_t(id.id)];
			};

			EVO_NODISCARD inline auto createInitializer(auto&&... args) noexcept -> PIR::Initializer::ID {
				this->pir.initializers.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Initializer::ID( uint32_t(this->pir.initializers.size() - 1) );
			};

			EVO_NODISCARD inline auto getInitializer(PIR::Initializer::ID id) const noexcept -> const PIR::Initializer& {
				return this->pir.initializers[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getInitializer(PIR::Initializer::ID id) noexcept -> PIR::Initializer& {
				return this->pir.initializers[size_t(id.id)];
			};



			EVO_NODISCARD inline auto createPrefix(auto&&... args) noexcept -> PIR::Prefix::ID {
				this->pir.prefixes.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Prefix::ID( uint32_t(this->pir.prefixes.size() - 1) );
			};

			EVO_NODISCARD inline auto getPrefix(PIR::Prefix::ID id) const noexcept -> const PIR::Prefix& {
				return this->pir.prefixes[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getPrefix(PIR::Prefix::ID id) noexcept -> PIR::Prefix& {
				return this->pir.prefixes[size_t(id.id)];
			};


			EVO_NODISCARD inline auto createDeref(auto&&... args) noexcept -> PIR::Deref::ID {
				this->pir.derefs.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Deref::ID( uint32_t(this->pir.derefs.size() - 1) );
			};

			EVO_NODISCARD inline auto getDeref(PIR::Deref::ID id) const noexcept -> const PIR::Deref& {
				return this->pir.derefs[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getDeref(PIR::Deref::ID id) noexcept -> PIR::Deref& {
				return this->pir.derefs[size_t(id.id)];
			};


			EVO_NODISCARD inline auto createAccessor(auto&&... args) noexcept -> PIR::Accessor::ID {
				this->pir.accessors.emplace_back(std::forward<decltype(args)>(args)...);
				return PIR::Accessor::ID( uint32_t(this->pir.accessors.size() - 1) );
			};

			EVO_NODISCARD inline auto getAccessor(PIR::Accessor::ID id) const noexcept -> const PIR::Accessor& {
				return this->pir.accessors[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getAccessor(PIR::Accessor::ID id) noexcept -> PIR::Accessor& {
				return this->pir.accessors[size_t(id.id)];
			};



			EVO_NODISCARD inline auto getGlobalVar(PIR::Var::ID id) const noexcept -> const PIR::Var& {
				return this->pir.vars[size_t(id.id)];
			};
			EVO_NODISCARD inline auto getGlobalVar(PIR::Var::ID id) noexcept -> PIR::Var& {
				return this->pir.vars[size_t(id.id)];
			};


			inline auto addPublicFunc(std::string_view ident, PIR::Func::ID id) noexcept -> void {
				using PubFuncListIter = std::unordered_map<std::string_view, std::vector<PIR::Func::ID>>::iterator;
				PubFuncListIter pub_func_list_iter = this->pir.pub_funcs.find(ident);
				if(pub_func_list_iter != this->pir.pub_funcs.end()){
					// add to existing list
					pub_func_list_iter->second.emplace_back(id);
				}else{
					// create new list
					auto new_func_list = std::vector<PIR::Func::ID>{id};
					this->pir.pub_funcs.emplace(ident, std::move(new_func_list));
				}
			};
			inline auto addPublicVar(std::string_view ident, PIR::Var::ID id) noexcept -> void {
				this->pir.pub_vars.emplace(ident, id);
			};
			inline auto addPublicStruct(std::string_view ident, PIR::Struct::ID id) noexcept -> void {
				this->pir.pub_structs.emplace(ident, id);
			};
			inline auto addPublicImport(std::string_view ident, Source::ID id) noexcept -> void {
				this->pir.pub_imports.emplace(ident, id);
			};




			//////////////////////////////////////////////////////////////////////
			// messaging / errors

			auto fatal(const std::string& msg, uint32_t line, uint32_t collumn) noexcept -> void;
			auto fatal(const std::string& msg, uint32_t line, uint32_t collumn_start, uint32_t collumn_end) noexcept -> void;


			auto error(const std::string& msg, uint32_t line, uint32_t collumn, std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, Token::ID token_id,              std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, const Token& token,              std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, AST::Node::ID node_id,           std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, const AST::Node& node,           std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto error(const std::string& msg, Location location,               std::vector<Message::Info>&& infos = {}) noexcept -> void;


			auto warning(const std::string& msg, uint32_t line, uint32_t collumn, std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto warning(const std::string& msg, Token::ID token_id,              std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto warning(const std::string& msg, const Token& token,              std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto warning(const std::string& msg, AST::Node::ID node_id,           std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto warning(const std::string& msg, const AST::Node& node,           std::vector<Message::Info>&& infos = {}) noexcept -> void;
			auto warning(const std::string& msg, Location location,               std::vector<Message::Info>&& infos = {}) noexcept -> void;



			EVO_NODISCARD inline auto hasErrored() const noexcept -> bool { return this->has_errored; };

		public:
			std::vector<Token> tokens{};
			std::vector<std::string*> string_literal_values{}; // using raw pointers because for some reason it causes an error

			std::vector<AST::Node::ID> global_stmts{};
			std::vector<AST::Node> nodes{};
			std::vector<AST::VarDecl> var_decls{};
			std::vector<AST::Func> funcs{};
			std::vector<AST::Struct> structs{};
			std::vector<AST::TemplatePack> template_packs{};
			std::vector<AST::FuncParams> func_params{};
			std::vector<AST::Conditional> conditionals{};
			std::vector<AST::Alias> aliases{};
			std::vector<AST::Return> returns{};
			std::vector<AST::Prefix> prefixes{};
			std::vector<AST::Infix> infixes{};
			std::vector<AST::Postfix> postfixes{};
			std::vector<AST::FuncCall> func_calls{};
			std::vector<AST::Initializer> initializers{};
			std::vector<AST::Type> types{};
			std::vector<AST::Block> blocks{};



			struct /* pir */ {
				std::vector<PIR::Var> vars{};
				std::vector<PIR::Param> params{};
				std::vector<PIR::Func> funcs{};
				std::vector<PIR::Struct> structs{};
				std::vector<PIR::Conditional> conditionals{};
				std::vector<PIR::Return> returns{};
				std::vector<PIR::Assignment> assignments{};
				std::vector<PIR::FuncCall> func_calls{};
				std::vector<PIR::Initializer> initializers{};
				std::vector<PIR::Prefix> prefixes{};
				std::vector<PIR::Deref> derefs{};
				std::vector<PIR::Accessor> accessors{};

				std::vector<PIR::Var::ID> global_vars{};

				std::unordered_map<std::string_view, std::vector<PIR::Func::ID>> pub_funcs{};
				std::unordered_map<std::string_view, PIR::Var::ID> pub_vars{};
				std::unordered_map<std::string_view, PIR::Struct::ID> pub_structs{};
				std::unordered_map<std::string_view, Source::ID> pub_imports{};
				std::unordered_map<std::string_view, PIR::Type::VoidableID> pub_aliases{};
			} pir;


		private:
			EVO_NODISCARD auto get_node_location(AST::Node::ID node_id) const noexcept -> Location;
			EVO_NODISCARD auto get_node_location(const AST::Node& node) const noexcept -> Location;


		private:
			std::filesystem::path src_location;
			std::string data;
			const Config& config;
			ID src_id;
			class SourceManager& source_manager;

			bool has_errored = false;

			class SemanticAnalyzer* semantic_analyzer = nullptr;
	};


};