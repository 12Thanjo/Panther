#include "LLVM_interface/Module.h"

#include <LLVM.h>


namespace panther{
	namespace llvmint{
		
		Module::Module(std::string_view name, llvm::LLVMContext& context){
			this->module = new llvm::Module(llvm::StringRef(name), context);
		}
		
		Module::~Module(){
			delete this->module;
			this->module = nullptr;
		}
		


		auto Module::createFunction(
			evo::CStrProxy name, llvm::FunctionType* prototype, llvmint::LinkageTypes linkage, bool nothrow, bool fast_call_conv
		) noexcept -> llvm::Function* {
			llvm::Function* func = llvm::Function::Create(
				prototype, static_cast<llvm::GlobalValue::LinkageTypes>(linkage), name.c_str(), this->module
			);

			if(nothrow){ func->setDoesNotThrow(); }
			if(fast_call_conv){ func->setCallingConv(llvm::CallingConv::Fast); }

			return func; 
		};


		auto Module::createStructType(evo::ArrayProxy<llvm::Type*> elements, bool is_packed, evo::CStrProxy name) noexcept -> llvm::StructType* {
		    return llvm::StructType::create(this->module->getContext(), {elements.data(), elements.size()}, name.c_str(), is_packed);
		};

		auto Module::createStructType(evo::CStrProxy name) noexcept -> llvm::StructType* {
		    return llvm::StructType::create(this->module->getContext(), name.c_str());
		};

		auto Module::setStructBody(llvm::StructType* struct_type, evo::ArrayProxy<llvm::Type*> elements, bool is_packed) noexcept -> void {
		    struct_type->setBody({elements.data(), elements.size()}, is_packed);
		};

		auto Module::createStructLiteral(evo::ArrayProxy<llvm::Type*> elements, bool is_packed) noexcept -> llvm::StructType* {
		    return llvm::StructType::get(this->module->getContext(), {elements.data(), elements.size()}, is_packed);
		};




		auto Module::getPointerSize() const noexcept -> unsigned {
			return this->module->getDataLayout().getPointerSize();
		};


		auto Module::getTypeSize(llvm::Type* type) const noexcept -> uint64_t {
		    return this->module->getDataLayout().getTypeAllocSize(type).getFixedValue();
		};
		



		auto Module::print() const noexcept -> std::string {
			auto data = llvm::SmallVector<char>();
			auto stream = llvm::raw_svector_ostream(data);

			this->module->print(stream, nullptr);

			const llvm::StringRef str_ref = stream.str(); 
			return str_ref.str();
		};



		auto Module::getDefaultTargetTriple() noexcept -> std::string {
			return llvm::sys::getDefaultTargetTriple();
		};


		auto Module::setTargetTriple(const std::string& target_triple) noexcept -> void {
			this->module->setTargetTriple(target_triple);
		};


		auto Module::setDataLayout(const std::string& target_triple, std::string_view cpu, std::string_view features) noexcept -> std::string {
			auto error_msg = std::string();
			const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, error_msg);

			if(target == nullptr){
				return error_msg;
			}


			auto opt = llvm::TargetOptions();
			this->target_machine = target->createTargetMachine(target_triple, cpu, features, opt, llvm::Reloc::PIC_);


			this->module->setDataLayout(target_machine->createDataLayout());

			return error_msg;
		};



		auto Module::compileToObjectFile() noexcept -> evo::Result<std::vector<evo::byte>> {
			auto data = llvm::SmallVector<char>();
			auto stream = llvm::raw_svector_ostream(data);

			auto pass = llvm::legacy::PassManager();
			auto file_type = llvm::CodeGenFileType::ObjectFile;

			if(this->target_machine->addPassesToEmitFile(pass, stream, nullptr, file_type)){
				return evo::resultError;
			}

			pass.run(*this->module);


			auto output = std::vector<evo::byte>();
			output.resize(data.size());

			std::memcpy(output.data(), data.data(), data.size());

			return output;
		};




		auto Module::get_clone() const noexcept -> std::unique_ptr<llvm::Module> {
			return llvm::CloneModule(*this->module);
		};

	
	};
};