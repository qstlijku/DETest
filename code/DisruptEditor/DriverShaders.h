#pragma once

#include <memory>
#include <vector>
#include <tinyxml2.h>
#include <d3d11.h>

class DriverShaders {
public:
    DriverShaders();
    static DriverShaders& instance();
    std::vector<std::unique_ptr<tinyxml2::XMLDocument>> materialDescriptors;
    std::unique_ptr<tinyxml2::XMLDocument> samplerStates;

	struct RenderState {
		D3D11_RASTERIZER_DESC rasterizerState;
		float ZCullForwardLimit;
		float ZCullBackLimit;
	};
	RenderState loadRasterizerState(const char* path);
};

/*
GetHashedName__24__N_9_blob1_cpp_b9a9929dFULUl
path is engine\shaders\obj\h%02llX\%s_%016llX.%s
%02llX
%s - type (pixel, vertex, renderstates)
%016llX - 
%s - extension (vso, pso)

									# "Vertex"
.data:10697E34                 .long aPixel_0          # "Pixel"
.data:10697E38                 .long aVertexdesc       # "VertexDesc"
.data:10697E3C                 .long aPixeldesc        # "PixelDesc"
.data:10697E40                 .long aGeometry_3       # "Geometry"
.data:10697E44                 .long aCompute          # "Compute"
.data:10697E48                 .long aRenderstates     # "RenderStates"
.data:10697E4C                 .long aDependencies_0   # "Dependencies"
*/

/*
index.* files
uint32_t fileSize
uint32_t head magic

Check if objects contain NULL
.rso = CRenderStatesLoader

*/
