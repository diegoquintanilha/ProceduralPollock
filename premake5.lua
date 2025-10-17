-- Solution
workspace "ProceduralPollock"
	architecture "x64"

	configurations
	{
		"Debug",
		"Profile",
		"Release"
	}
	
	startproject "ProceduralPollock"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Project
project "ProceduralPollock"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "On"
	floatingpoint "Fast"
	flags { "MultiProcessorCompile" }

	targetdir("bin/output/" .. outputdir .. "/%{prj.name}")
	objdir("bin/intermediates/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		-- Source files
		"src/**.h",
		"src/**.cpp",

		-- Misc.
		".gitignore",
		"LICENSE",
		"premake5.lua",
		"README.md"
	}
	
	includedirs
	{
		"src"
	}
	
	links
	{
		"d3d11",
		"d3dcompiler"
	}
	
	filter "system:windows"
		systemversion "latest"
		defines "UNICODE"
	
	filter "configurations:Debug"
		defines "_DEBUG"
		runtime "Debug"
		symbols "On"
	
	filter "configurations:Profile"
		defines "_PROFILE"
		runtime "Release"
		symbols "On"
		optimize "Speed"
		inlining ("Auto")
		linktimeoptimization "On"
		
	filter "configurations:Release"
		defines "_RELEASE"
		runtime "Release"
		symbols "Off"
		optimize "Speed"
		inlining ("Auto")
		linktimeoptimization "On"
