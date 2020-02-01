#pragma once

#include "CPathID.h"
#include "CStringID.h"
#include "IBinaryArchive.h"
#include "Serialization.h"
#include <array>
#include "DDRenderInterface.h"

class CSplineControlPoint {
public:
	glm::vec4 unk2;//quat
	glm::vec3 unk1;
	float unk3;
	uint32_t unk4;
	uint32_t pad;
	float unk5;
	float unk6;
	void read(IBinaryArchive& fp);
};

class CSpline {
public:
	CStringID splineName;//CSplineNameID
	uint16_t unk1;
	bool unk2;
	bool unk3;
	Vector<CSplineControlPoint> controlPoints;
	void read(IBinaryArchive& fp);
};

struct CSplineCoord {
	uint32_t unk1;
	float unk2;
};

struct SSplineRangeCoords { //2^4
	CSplineCoord x1;
	CSplineCoord x2;
};

struct CRangeOffsets { //0x24
	glm::vec3 unk1;//0x0
private:
	float pack1;
public:
	glm::vec3 unk2;//0x10
private:
	float pack2;
public:
	uint8_t unk3;//0x20
};

struct CKnot {
	float unk1;
	float unk2;
	float unk3;
	float unk4;
	float unk5;
	float unk6;
	float unk7;

	typedef uint32_t EKnotType;
	EKnotType type;

	void read(IBinaryArchive& fp);
};

struct CSplineLoftMorphing {
	glm::vec2 unk1;
	std::vector<CKnot> knots;

	void read(IBinaryArchive& fp);
};

struct CSplineLoftMeshDesc { //2^3
	uint32_t unk1;
	uint32_t unk2;
};

struct CSplineLoftMeshLODGFXDesc { //0x2
	uint16_t unk1;
};

struct CSplineLoftPrimitiveDrawCallDesc { //0x4
	uint32_t unk1;
};

struct CSplineLoftPrimitiveDrawCallGFXBuffers { //2^4
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;
	uint32_t unk4;
};

class CSceneSplineLoftRegion {
public:
	glm::vec2 unk1;
	float unk2;
	uint32_t unk3;

	Vector<std::unique_ptr<CSpline>> splines;

	uint32_t unk4;
	uint32_t unk5;

	std::vector<SSplineRangeCoords> rangeCoords;
	std::vector<CRangeOffsets> rangeOffsets;
	std::vector<std::unique_ptr<CSplineLoftMorphing>> morphing;
	std::vector<CSplineLoftMeshDesc> meshDescs;
	std::vector<CSplineLoftMeshLODGFXDesc> meshLODGFXDescs;
	std::vector<float> unk6;
	std::vector<uint64_t> unk7;
	std::vector<CSplineLoftPrimitiveDrawCallDesc> primitiveDrawCallDesc;
	std::vector<CSplineLoftPrimitiveDrawCallGFXBuffers> primitiveDrawCallGFXBufDesc;

	void read(IBinaryArchive& fp);
};

struct CSceneSplineLoftPrimitive {
	uint8_t unk1;

	void read(IBinaryArchive& fp);
};

class CSplineLoftPrimitiveDesc {
public:
	CSceneSplineLoftPrimitive primitive;
	std::array<uint8_t, 0x50> unk2;

	void read(IBinaryArchive& fp);
};

class SSplineLoftDrawCall {
public:
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;
	uint32_t unk4;

	void read(IBinaryArchive& fp);
};

class CSplineNetworkRegionResourceEntry {
public:
	CSceneSplineLoftRegion loftReigon;
	std::vector<std::unique_ptr<CSplineLoftPrimitiveDesc>> primitiveDesc;
	std::vector<std::unique_ptr<SSplineLoftDrawCall>> drawCalls;
	std::vector<CPathID> unk2;
	std::vector<uint32_t> unk3;

	void read(IBinaryArchive& fp);
};

class SplineLoftHiRes {
public:
	uint32_t unk1;
	
	struct UnkStr {
		std::array<uint8_t, 32> unk;
		void read(IBinaryArchive& fp) {
			fp.memBlock(unk.data(), 1, unk.size());
		}
	};
	std::vector<UnkStr> vertexData;
	std::shared_ptr<VertexBuffer> vertex;

	std::vector<uint16_t> indexData;
	std::shared_ptr<IndexBuffer> index;

	uint32_t unk2;
	std::vector<std::unique_ptr<CSplineNetworkRegionResourceEntry>> networkRegionResources;

	CPathID lowRes;

	void open(IBinaryArchive& fp);
	void draw(ID3D11DeviceContext* context);
	void createBuffers();
};

class LoftShape {
public:
	//
	struct Loft {

	};

	struct Lods {

	};

	struct Lodd {

	};

	struct Side {

	};
	
	struct Mtrl {

	};

	void open(IBinaryArchive& fp);
	void draw();
};
