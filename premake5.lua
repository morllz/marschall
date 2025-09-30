include "./vendor/premake/custom/solution_items.lua"

workspace "marschall"
	architecture "x64"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

	solution_items {
		"premake5.lua",
		".gitignore",
		".gitmodules"
	}

	startproject "marschall-test"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "marschall"
	kind "StaticLib"
	location "marschall"
	language "C++"
	cppdialect "C++23"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.location}/include/**.hpp",
	}

	includedirs {
	}


	filter "system:windows"
		systemversion "latest"


	filter "configurations:Debug"
		defines "MARSCHALL_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "MARSCHALL_RELEASE"
		optimize "on"

	filter "configurations:Dist"
		defines "MARSCHALL_DIST"
		optimize "on"

project "marschall-test"
	kind "ConsoleApp"
	location "marschall-test"
	language "C++"
	cppdialect "C++23"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.location}/src/**.hpp",
		"%{prj.location}/src/**.cpp",
		"%{wks.location}/vendor/googletest/googletest/src/gtest-all.cc"
	}

	includedirs {
		"%{wks.location}/marschall/include",
		"%{wks.location}/vendor/googletest/googletest/include",
		"%{wks.location}/vendor/googletest/googletest"
	}


	filter "system:windows"
		systemversion "latest"


	filter "configurations:Debug"
		defines "MARSCHALL_TEST_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "MARSCHALL_TEST_RELEASE"
		optimize "on"

	filter "configurations:Dist"
		defines "MARSCHALL_TEST_DIST"
		optimize "on"