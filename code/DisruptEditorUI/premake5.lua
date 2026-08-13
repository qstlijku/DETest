
project "DisruptEditorUI"
    language "C++"
	kind "WindowedApp"
	postbuildcommands "{COPY} $(SolutionDir)../code/vendor/SDL2/SDL2.dll %{cfg.targetdir}"
	postbuildcommands "{COPY} $(SolutionDir)../res/ %{cfg.targetdir}/res/"
	libdirs { 
		"$(SolutionDir)../code/vendor/SDL2",
	}
	filter "configurations:Debug"
		libdirs { "C:/Program Files/Autodesk/FBX/FBX SDK/2019.0/lib/vs2015/x64/debug" }
	filter "configurations:Release"
		libdirs { "C:/Program Files/Autodesk/FBX/FBX SDK/2019.0/lib/vs2015/x64/release" }
	filter {}
	debugdir "%{cfg.targetdir}"

	vpaths
    {
        ["Headers/*"] = { "**.hpp", "**.h" },
        ["Sources/*"] = "**.cpp",
        ["Resources/*"] = "**.rc",
        ["*"] = "premake5.lua"
	}
	
	includedirs
	{
		".",
		"../DisruptEditor",
		"../vendor/SDL2",
		"../vendor/tinyxml2",
		"../vendor/lz4",
		"../vendor/glm",
		"../vendor/imgui",
		"../vendor/ImGuizmo",
		"../vendor/portable-file-dialogs",
		"../vendor/stb",
		"../vendor/dr_libs",
		"../vendor/debug-draw",
		"../vendor/hk2012_2_0_r1/Source",

		"C:/Program Files/Autodesk/FBX/FBX SDK/2019.0/include",
	}

	links
	{
		--system
		"Dbghelp",
		"D3d11",
		"dxguid",

		"libfbxsdk-md",

		"tinyxml2",
		"lz4",
		"DDSTextureLoader",

		-- imgui
		"imgui",
		"ImGuizmo",

		-- SDL2
		"SDL2",
		"SDL2main",

		"DisruptEditor"
	}

    files
    {
		"premake5.lua",
		"**.natvis",
        "**.cpp",
        "**.hpp",        
        "**.h",      
        "**.c",
		"**.rc"
	}
