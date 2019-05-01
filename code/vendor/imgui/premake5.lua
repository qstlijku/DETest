
project "imgui"
    language "C++"
    kind "StaticLib"
	
    vpaths { ["*"] = "*" }

    includedirs
    {
        ".",
		"../glm",
		"../glad",
		"../SDL2"
    }

    files
    {
        "premake5.lua",
        "**.h",
        "**.cpp"
    }