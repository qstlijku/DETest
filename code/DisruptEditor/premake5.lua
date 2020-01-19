
project "DisruptEditor"
    language "C++"
	kind "StaticLib"
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
		"../vendor/SDL2",
		"../vendor/tinyxml2",
		"../vendor/lz4",
		"../vendor/glm",
		"../vendor/imgui",
		"../vendor/ImGuizmo",
		"../vendor/portable-file-dialogs",
		"../vendor/tinyfiles",
		"../vendor/stb",
		"../vendor/dr_wav",
		"../vendor/debug_draw",
		"../vendor/DirectXTex",
	}

	links
	{
		--system
		"Shlwapi",

		"tinyxml2",
		"lz4",

		-- imgui
		"imgui",
		"ImGuizmo",

		-- SDL2
		"SDL2",
		"SDL2main"
	}

    files
    {
        "premake5.lua",
        "**.natvis",
        "**.cpp",
        "**.hpp",        
        "**.h",      
        "**.c",
		"**.rc",

		"../vendor/Implementation.cpp"
    }
