-- premake5

include "../libs/premake5_LLVM.lua"


project "Panther_Frontend"
	kind "StaticLib"
	-- staticruntime "On"
	

	targetdir(target.lib)
	objdir(target.obj)

	files {
		"./frontend/**.h",
		"./frontend/**.cpp",
	}

	

	includedirs{
		(config.location .. "/libs"),

		"../include/",
		"./",
	}

	links{
		"Evo",
	}


project "*"



project "Panther_Middleend"
	kind "StaticLib"
	-- staticruntime "On"
	

	targetdir(target.lib)
	objdir(target.obj)

	files {
		"./middleend/**.h",
		"./middleend/**.cpp",
	}

	

	includedirs{
		(config.location .. "/libs/LLVM/include"),
		(config.location .. "/libs"),

		"../include/",
		"./",
	}


	link_llvm()

	-- link_llvm_AArch64()
	-- link_llvm_AMDGPU()
	-- link_llvm_ARM()
	-- link_llvm_AVR()
	-- link_llvm_BPF()
	-- link_llvm_Exegesis()
	-- link_llvm_Hexagon()
	-- link_llvm_Lanai()
	-- link_llvm_LoongArch()
	-- link_llvm_Mips()
	-- link_llvm_MSP430()
	-- link_llvm_NVPTX()
	-- link_llvm_PowerPC()
	-- link_llvm_RISCV()
	-- link_llvm_Sparc()
	-- link_llvm_SystemZ()
	-- link_llvm_WASM()
	link_llvm_X86()
	-- link_llvm_XCore()

	links{
		"Evo",
	}


project "*"