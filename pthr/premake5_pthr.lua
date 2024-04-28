-- premake5


project "pthr"
	kind "ConsoleApp"
	-- staticruntime "On"
	

	targetdir(target.bin)
	objdir(target.obj)

	files {
		"./**.h",
		"./**.cpp",
	}

	

	includedirs{
		(config.location .. "/libs"),

		"../include/",
		"./",
	}

	links{
		"Evo",
		"Panther_frontend",
		"LLVM_interface",
		"LLD_interface",
	}




project "*"