#include "LLD_interface/LLDInterface.h"

#include <LLVM.h>
#include <lld/common/Driver.h>

LLD_HAS_DRIVER(coff)
LLD_HAS_DRIVER(elf)
LLD_HAS_DRIVER(mingw)
LLD_HAS_DRIVER(macho)
LLD_HAS_DRIVER(wasm)




namespace panther{


    class MessageOStream : public llvm::raw_ostream {
        public:
            MessageOStream(){
                this->SetUnbuffered();
            };
            ~MessageOStream() = default;

    
        public:
            std::vector<std::string> messages{};

        private:
            void write_impl(const char* ptr, size_t size) noexcept override {
                this->messages.emplace_back(ptr, size);
                this->current_size += size;
            };

            uint64_t current_pos() const override { return this->current_size; }

        private:
            size_t current_size = 0;
    };




    // static auto all_lld_drivers = std::array<lld::DriverDef, 5>{
    //     lld::DriverDef{ lld::WinLink, &lld::coff::link  },
    //     lld::DriverDef{ lld::Gnu,     &lld::elf::link   },
    //     lld::DriverDef{ lld::MinGW,   &lld::mingw::link },
    //     lld::DriverDef{ lld::Darwin,  &lld::macho::link },
    //     lld::DriverDef{ lld::Wasm,    &lld::wasm::link  },
    // };
   


    auto LLDInterface::link(const std::string& input_file_path, const std::string& target_output, Linker linker) noexcept -> LinkerOutput {
        auto args = std::vector<const char*>{};
        auto str_alloc = std::vector<std::string>{};
        auto driver = std::optional<lld::DriverDef>();

        switch(linker){
            case Linker::WinLink: {
                args.emplace_back("lld-link");

                std::string& output_arg = str_alloc.emplace_back("-out:");
                output_arg += target_output;
                args.emplace_back(output_arg.c_str());

                args.emplace_back("-WX"); // treat warnings as errors
                args.emplace_back("-nologo"); // suppress copyright banner (doesn't seem to do anything, but just in case)

                args.emplace_back("-defaultlib:libcmt");
                args.emplace_back("-defaultlib:oldnames");
                // args.emplace_back("-libpath:C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.33.31629\\lib\\x64");
                // args.emplace_back("-libpath:C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.33.31629\\atlmfc\\lib\\x64");
                // args.emplace_back("-libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.19041.0\\ucrt\\x64");
                // args.emplace_back("-libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.19041.0\\um\\x64");
                // args.emplace_back("-libpath:C:\\Program Files\\LLVM\\lib\\clang\\17\\lib\\windows");


                driver = lld::DriverDef(lld::WinLink, &lld::coff::link);
            } break;

            case Linker::Gnu: {
                // TODO: better messaging
                EVO_FATAL_BREAK("Gnu Not supported");
            } break;

            case Linker::MinGW: {
                // TODO: better messaging
                EVO_FATAL_BREAK("MinGW Not supported");
            } break;

            case Linker::Darwin: {
                // TODO: better messaging
                EVO_FATAL_BREAK("Darwin Not supported");
            } break;

            case Linker::Wasm: {
                // TODO: better messaging
                EVO_FATAL_BREAK("Wasm Not supported");
            } break;
        };

        args.emplace_back(input_file_path.c_str());

        
        evo::debugAssert(driver.has_value(), "linker driver not set");

        auto stdout_msgs = MessageOStream();
        auto stderr_msgs = MessageOStream();

        const lld::Result result = lld::lldMain(args, stdout_msgs, stderr_msgs, {*driver});

        evo::debugAssert(stdout_msgs.messages.empty(), "lld linker had messages on stdout");

        return {result.retCode, result.canRunAgain, std::move(stderr_msgs.messages)};
    };


};