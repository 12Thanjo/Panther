-- premake5


project "Main"
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

		"./",
	}

	links{
		"Evo",
	}




project "*"