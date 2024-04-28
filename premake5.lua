-- premake5


workspace "Panther"
	architecture "x64"

	configurations{
		"Debug",
		"Dev",
		"Optimize",
		"Release",
		"ReleaseDist",
	}

	platforms {
		"Windows",
		"Linux",
	}
	defaultplatform "Windows"


	flags{
		"MultiProcessorCompile",
		-- "NoPCH",
	}


	startproject "pthr"


	---------------------------------------
	-- platform

	filter "system:Windows"
		system "windows"
	filter {}


	filter "system:Linux"
		system "linux"
	filter {}




	------------------------------------------------------------------------------
	-- configs

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
		optimize "Off"

		defines{
			"_DEBUG",
		}
	filter {}


	filter "configurations:Dev"
		runtime "Debug"
		symbols "On"
		optimize "Off"

		defines{
			"_DEBUG",
		}
	filter {}


	filter "configurations:Optimize"
		runtime "Debug" -- TODO: figure out how to have LLVM build with release runtime
		symbols "On"
		optimize "On"

		defines{
			"NDEBUG",
		}

		flags{
			"LinkTimeOptimization",
		}
	filter {}


	filter "configurations:Release"
		runtime "Debug" -- TODO: figure out how to have LLVM build with release runtime
		symbols "Off"
		optimize "Full"

		defines{
			"NDEBUG",
		}

		flags{
			"LinkTimeOptimization",
		}
	filter {}



	filter "configurations:ReleaseDist"
		runtime "Debug" -- TODO: figure out how to have LLVM build with release runtime
		symbols "Off"
		optimize "Full"

		defines{
			"NDEBUG",
		}

		flags{
			"LinkTimeOptimization",
		}
	filter {}



------------------------------------------------------------------------------
-- global variables
	
config = {
	location = ("%{wks.location}"),
	platform = ("%{cfg.platform}"),
	build    = ("%{cfg.buildcfg}"),
	project  = ("%{prj.name}"),
}


target = {
	bin = string.format("%s/build/%s/%s/bin/",   config.location, config.platform, config.build),
	lib = string.format("%s/build/%s/%s/lib/%s", config.location, config.platform, config.build, config.project),
	obj = string.format("%s/build/%s/%s/obj/%s", config.location, config.platform, config.build, config.project),
}



------------------------------------------------------------------------------
-- extern lib projects


include "libs/premake5_Evo.lua"


------------------------------------------------------------------------------
-- project settings

language "C++"
cppdialect "C++20"
exceptionhandling "Off"
allmodulespublic "Off"



---------------------------------------
-- build

filter "configurations:Debug"
	warnings "High"
	debugdir(config.location .. "/testing")

	defines{
		"PANTHER_BUILD_DEBUG",
		"PANTHER_CONFIG_DEBUG",
		"PANTHER_CONFIG_TRACE",
	}

filter {}


filter "configurations:Dev"
	warnings "High"
	debugdir (config.location .. "/testing")

	defines{
		"PANTHER_BUILD_DEV",
		"PANTHER_CONFIG_DEBUG",
	}

filter {}


filter "configurations:Optimize"
	debugdir (config.location .. "/testing")

	defines{
		"PANTHER_BUILD_OPTIMIZE",
		"PANTHER_CONFIG_DEBUG",
	}

filter {}


filter "configurations:Release"
	debugdir (config.location .. "/testing")

	defines{
		"PANTHER_BUILD_RELEASE",
		"PANTHER_CONFIG_RELEASE",
	}
filter {}


filter "configurations:ReleaseDist"
	defines{
		"PANTHER_BUILD_DIST",
		"PANTHER_CONFIG_RELEASE",
	}
filter {}




------------------------------------------------------------------------------
-- projects

include "./libs/premake5_LLVM.lua"

include "./src/frontend/premake5_Panther_frontend.lua"
include "./src/LLVM_interface/premake5_LLVM_interface.lua"
include "./src/LLD_interface/premake5_LLD_interface.lua"
include "./pthr/premake5_pthr.lua"


------------------------------------------------------------------------------
-- grouping

project("Evo").group = "External Libs"

project("LLD_interface").group = "Interfaces"
project("LLVM_interface").group = "Interfaces"

project("Panther_frontend").group = "Panther Lib"

project("pthr").group = "Executables"


