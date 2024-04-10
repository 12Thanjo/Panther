-- premake5



project "LLD_interface"
	kind "StaticLib"
	-- staticruntime "On"
	

	targetdir(target.lib)
	objdir(target.obj)

	files {
		"./**.h",
		"./**.cpp",
	}

	

	includedirs{
		(config.location .. "/libs/LLVM/include"),
		(config.location .. "/libs"),

		"../../include/",
		"../",
	}



	LLVM.link.all_platforms()


	links{
		"Evo",

		LLD.libs.COFF,
		LLD.libs.Common,
		LLD.libs.ELF,
		LLD.libs.MachO,
		LLD.libs.MinGW,
		LLD.libs.Wasm,

		LLVM.libs.FrontendOpenMP,
		LLVM.libs.ipo,
		LLVM.libs.FrontendOffloading,
	}


project "*"