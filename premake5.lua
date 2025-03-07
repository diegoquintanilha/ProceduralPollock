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
	staticruntime "on"
	
	targetdir("bin/output/" .. outputdir .. "/%{prj.name}")
	objdir("bin/intermediates/" .. outputdir .. "/%{prj.name}")
	
	-- Set the working directory as the output executable directory
	debugdir("bin/output/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"src/**.h",
		"src/**.cpp"
	}
	
	includedirs
	{
		"src"
	}
	
	floatingpoint "Fast"
	
	filter "system:windows"
		defines "_WINDOWS"
		systemversion "latest"
	
	filter "configurations:Debug"
		defines "_DEBUG"
		runtime "Debug"
		symbols "On"
		flags { "MultiProcessorCompile" }
	
	filter "configurations:Profile"
		defines "_PROFILE"
		runtime "Release"
		symbols "On"
		optimize "Speed"
		inlining ("Auto")
		flags { "MultiProcessorCompile", "LinkTimeOptimization" }
		
	filter "configurations:Release"
		defines "_RELEASE"
		runtime "Release"
		symbols "Off" -- This disables PDB generation in the final output folder
		optimize "Speed"
		inlining ("Auto")
		flags { "MultiProcessorCompile", "LinkTimeOptimization" }
