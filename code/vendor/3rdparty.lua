project "DDSTextureLoader"
    language "C++"
    kind "StaticLib"
	
    vpaths { ["*"] = "*" }

    includedirs
    {
        "DirectXTex/DDSTextureLoader",
    }

    files
    {
        "DirectXTex/DDSTextureLoader/DDSTextureLoader.h",
        "DirectXTex/DDSTextureLoader/DDSTextureLoader.cpp"
    }

project "imgui"
    language "C++"
    kind "StaticLib"
	
    vpaths { ["*"] = "*" }

    includedirs
    {
        "imgui",
        "glm",
        "SDL2",
    }

    files
    {
        "imgui/**.h",
        "imgui/**.cpp"
    }

project "ImGuizmo"
    language "C++"
    kind "StaticLib"
    
    vpaths { ["*"] = "*" }

    includedirs
    {
        "glm",
        "imgui",
        "ImGuizmo",
    }

    files
    {
        "ImGuizmo/**.h",
        "ImGuizmo/**.cpp"
    }

    
project "lz4"
    language "C++"
    kind "StaticLib"
	
    vpaths { ["*"] = "*" }

    includedirs
    {
        "lz4"
    }

    files
    {
        "lz4/**.h",
        "lz4/**.c"
    }

project "tinyxml2"
    language "C++"
    kind "StaticLib"
    
    vpaths { ["*"] = "*" }

    includedirs
    {
        "tinyxml2"
    }

    files
    {
        "tinyxml2/tinyxml2.h",
        "tinyxml2/tinyxml2.cpp"
    }