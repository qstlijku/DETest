
project "xbox2pc_nfo"
    language "C++"
	kind "ConsoleApp"
	libdirs { "$(SolutionDir)../code/vendor/SDL2" }

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
		"../vendor/tinyfiles",
		"../vendor/portable-file-dialogs",
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
		"lz4",

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
        "**.cpp",
        "**.hpp",        
        "**.h",      
        "**.c",
		"**.rc"
    }
