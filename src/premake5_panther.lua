-- premake5


project "Panther"
	kind "StaticLib"
	-- staticruntime "On"
	

	targetdir(target.lib)
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
	}


project "*"