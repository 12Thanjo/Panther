-- premake5

project "Panther_frontend"
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

		"../../include/",
		"../",
	}

	links{
		"Evo",
	}


project "*"
