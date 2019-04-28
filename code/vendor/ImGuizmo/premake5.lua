
project "ImGuizmo"
    language "C++"
    kind "StaticLib"
	
    vpaths { ["*"] = "*" }

    includedirs
    {
        ".",
		"../glm",
		"../glad",
		"../SDL2",
		"../imgui"
    }

    files
    {
        "premake5.lua",
        "**.h",
        "**.cpp"
    }