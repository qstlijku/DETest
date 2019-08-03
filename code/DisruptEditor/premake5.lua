
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
		"../vendor/glm",
		"../vendor/imgui",
		"../vendor/ImGuizmo",
		"../vendor/noc_file_dialog",
		"../vendor/tinyfiles",
		"../vendor/stb",
		"../vendor/dr_wav",
		"../vendor/debug_draw",
		"../vendor/sqlite_modern_cpp",
		"../vendor/sqlite3"
	}

	links
	{
		--system
		"Shlwapi",

		"tinyxml2",
		"sqlite3",

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
        "**.cpp",
        "**.hpp",        
        "**.h",      
        "**.c",
		"**.rc",

		"../vendor/Implementation.cpp"
    }
