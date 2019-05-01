
project "DisruptEditorUI"
    language "C++"
	kind "WindowedApp"
	postbuildcommands "{COPY} $(SolutionDir)../code/vendor/SDL2/SDL2.dll %{cfg.targetdir}"
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
		"../vendor/glad",
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
		"../vendor/sqlite_modern_cpp",
		"../vendor/sqlite3",
	}

	links
	{
		--system
		"Dbghelp",

		"glad",
		"tinyxml2",
		"sqlite3",

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
