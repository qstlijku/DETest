
project "tinyxml2"
    language "C++"
    kind "StaticLib"
	
    vpaths { ["*"] = "*" }

    includedirs
    {
        "."
    }

    files
    {
        "premake5.lua",
        "**.h",
        "**.cpp"
    }