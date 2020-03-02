
premake.path = premake.path .. ";build"

workspace "DisruptEditor"
    configurations { "Debug", "Release" }

	-- force latest cpp standard
    if os.istarget('windows') then
        buildoptions "/std:c++latest"
    else
        buildoptions "-std=c++17"
    end

    -- multi threaded compilation
    flags "MultiProcessorCompile"

    symbols "On"
	pic "On"
    targetprefix ""
    characterset "Unicode"
	architecture "x64"

    location "../build/"
    os.mkdir"../build/symbols"
	
	startproject "DisruptEditorUI"

	libdirs {
		 "../bin/%{cfg.buildcfg}/",
		 "../bin/vendor/%{cfg.buildcfg}/"
	}
	
    defines {
        "NOMINMAX",
        --"WIN32_LEAN_AND_MEAN"
    }

    filter "platforms:x64"
         architecture "x86_64"

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG", "GC_DBG" }
        optimize "Off"
        runtime "Debug"

    filter "configurations:Release"
        -- staticruntime "On"
        optimize "Speed"
		
	filter {"system:windows", "kind:StaticLib" }
		targetdir "../bin/vendor/%{cfg.buildcfg}/"
		
	filter {"system:windows", "kind:not StaticLib" }
		targetdir "../bin/%{cfg.buildcfg}/"
		

    filter {"system:windows", "kind:not StaticLib"}
        linkoptions { "/PDB:\"$(SolutionDir)\\symbols\\$(ProjectName)_%{cfg.buildcfg}.pdb\"" }

    filter { "system:windows", "kind:not StaticLib" }
        linkoptions "/manifestdependency:\"type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\""

    -- Disable deprecation warnings and errors
    -- disabling as less warnings as possible
    filter "action:vs*"
        defines {
            "_CRT_SECURE_NO_WARNINGS",
            -- "_CRT_SECURE_NO_DEPRECATE",
            -- "_CRT_NONSTDC_NO_WARNINGS",
            -- "_CRT_NONSTDC_NO_DEPRECATE",
            -- "_SCL_SECURE_NO_WARNINGS",
            -- "_SCL_SECURE_NO_DEPRECATE",

            "_WINSOCK_DEPRECATED_NO_WARNINGS",
        }

    defines {
        "WD1",
        --"WD2",

        "GLM_FORCE_DEPTH_ZERO_TO_ONE",
    }

	group "Editor"
	include "./DisruptEditor"
	include "./DisruptEditorUI"
    
    group "Tools"
    include "./ConvertMaterials"
    include "./ConvertXBG"
    include "./xbox2pc_nfo"

	group "Vendor"
	include "vendor/3rdparty.lua"

-- Cleanup
if _ACTION == "clean" then
    os.rmdir("../bin");
    os.rmdir("../build");
end
