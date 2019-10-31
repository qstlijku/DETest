
project "ConvertXBG"
    language "C++"
	kind "ConsoleApp"
	libdirs { "$(SolutionDir)../code/vendor/SDL2" }
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
		"../vendor/glm",
		"../vendor/imgui",
		"../vendor/ImGuizmo",
		"../vendor/tinyfiles",
		"../vendor/noc_file_dialog",
		"../vendor/stb",
		"../vendor/dr_wav",
		"../vendor/debug_draw",
	}

	links
	{
		--system
		"D3d11",
		"dxguid",

		"tinyxml2",

		-- imgui
		"imgui",
		"ImGuizmo",

		-- SDL2
		"SDL2",
		"SDL2main",

		"DisruptEditor"
	}

	defines
	{
		"NOC_FILE_DIALOG_WIN32"
	}

    files
    {
        "premake5.lua",
        "**.cpp",
        "**.hpp",        
        "**.h",      
        "**.c",
		"**.rc"
    }
