-- premake5


project "CLI"
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
		"Panther",
	}




project "*"