#pragma once

#include "CPathID.h"
#include "CStringID.h"
#include "IBinaryArchive.h"
#include "Serialization.h"
#include <array>
#include <vector>
#include "DDRenderInterface.h"

class CSplineControlPoint {
public:
	glm::vec4 rotation; //ndQuat
	glm::vec3 position;
	float length;
	uint32_t cpIdx;
	float tangentIn; // WDL: union struct with morphFactor
	float tangentOut;
	void read(IBinaryArchive& fp);
};

class CSpline {
public:
	CStringID splineName; //CSplineNameID
	uint16_t id;
	// glm::vec2 uvMappingRotation (WDL)
	bool isLinear;
	bool needsExport;
	// float lengthOffsetUV (WDL)
	Vector<CSplineControlPoint> controlPoints;
	void read(IBinaryArchive& fp);
};

struct CSplineCoord {
	uint32_t cpIdx;
	float t;
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
	// Note: In WDL these appear at the end of the struct, assigned
	// at the beginning but MAY not be serialized like the others
	glm::vec2 regionOffset;
	float regionSize;
	uint32_t regionID;

	Vector<std::unique_ptr<CSpline>> splineSubsets;

	uint32_t splunk4;
	uint32_t splunk5;
	// WDL has no CSplineLoftMorphing below but has CSplineLoftElementDBInfo
	std::vector<SSplineRangeCoords> rangeCoords;
	std::vector<CRangeOffsets> rangeOffsets;
	std::vector<std::unique_ptr<CSplineLoftMorphing>> morphing;
	std::vector<CSplineLoftMeshDesc> meshDescs;
	std::vector<CSplineLoftMeshLODGFXDesc> meshLODGFXDescs;
	std::vector<float> lodDistances;
	std::vector<uint64_t> lods; // SBitMask<128,0> in WDL
	std::vector<CSplineLoftPrimitiveDrawCallDesc> primitiveDrawCallDesc;
	std::vector<CSplineLoftPrimitiveDrawCallGFXBuffers> primitiveDrawCallGFXBufDesc;
	// May want to assign vertex and index buffers here if convenient
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
	struct SSplineLoftVertexP {//32 bytes
		std::array<uint8_t, 32> unk1;
	};
	std::vector<SSplineLoftVertexP> unk5;
	std::vector<uint16_t> unk6;

	void read(IBinaryArchive& fp);
};

class CSplineNetworkRegionResourceEntry {
public:
	CSceneSplineLoftRegion loftRegion;
	std::vector<std::unique_ptr<CSplineLoftPrimitiveDesc>> primitiveDesc;
	std::vector<std::unique_ptr<SSplineLoftDrawCall>> drawCalls;//Unused?
	std::vector<CPathID> materials;
	std::vector<std::string> materialPaths; // for debugging
	std::vector<uint32_t> elementDBIDs; // actually CUniqueID in WDL

	void read(IBinaryArchive& fp);
};

class SplineLoftHiRes {
public:
	uint32_t unk1;
	
	struct UnkStr {//32 bytes
		glm::vec3 pos;
		glm::vec2 texcoord2;
		std::array<uint8_t, 4> normal;
		std::array<uint8_t, 4> texCoord1;
		std::array<uint8_t, 4> texCoord0;
		void read(IBinaryArchive& fp) {
			static_assert(sizeof(*this) == 32);
			fp.memBlock(this, 1, sizeof(*this));
		}
	};
	std::vector<UnkStr> vertexData;
	std::shared_ptr<VertexBuffer> vertex;

	std::vector<uint16_t> indexData;
	std::shared_ptr<IndexBuffer> index;

	std::vector<std::unique_ptr<CSplineNetworkRegionResourceEntry>> networkRegionResources;

	CPathID lowRes;
	std::string lowResPath;

	std::string thisPath;

	void open(IBinaryArchive& fp);
	void draw(ID3D11DeviceContext* context);
	void createBuffers();
};

///////Low Res (WD1), both (WDL)

struct SArrayRange {
	uint16_t start;
	uint16_t count;
	void read(IBinaryArchive& fp);
};

struct CSceneSplineLoftBatch {
	uint32_t unk1; // unsigned int vertexCount
	struct SRangeDesc {
		/*
		*    unsigned int m_indexStart;
00000004     unsigned int m_indexEnd;
00000008     unsigned __int8 m_materialIdx;
00000009     bool m_castsShadow;
0000000A     bool m_hasAlphaTest;
0000000B     unsigned __int8 m_lodIdx;
		*/
		uint16_t unk1;
		uint16_t unk2;
		uint8_t unk3;
		bool unk4;
		bool unk5;
		uint8_t unk6;

		void read(IBinaryArchive& fp);
	};
	std::vector<SRangeDesc> unk2; // rangeDescs
	struct SPassDrawCallRanges {
		std::array<SArrayRange, 5> unk1; // SArrayRange shadow[2], normal[2], reflection
		void read(IBinaryArchive& fp);
	};
	SPassDrawCallRanges unk3; // passRanges
	// CSphere boundingSphere, CAABBox boundingBox (TODO create these types)
	glm::vec3 unk4; // center
	float unk5; // radius
	glm::vec3 unk6; // boundingBoxMin
	glm::vec3 unk7; // boundingBoxMax
	glm::vec2 unk8; // posDecompress
	glm::vec4 unk9; // uvDecompress
	glm::vec3 unk10; // position
	std::vector<float> unk11; // ndStaticVector<float,2,4> lodDistances???
	struct SPrimitiveData {
		uint32_t unk1; // unsigned int m_primitiveKey
		std::array< std::array<uint64_t, 8>, 3> unk2; // SBitMask<2048,0> m_drawCalls (sizeof 0x100)
		void read(IBinaryArchive& fp);
	};
	std::vector<SPrimitiveData> unk12; // primitives
	std::vector<SRangeDesc> unk13; // primitiveDrawCallRanges
	// WDL: CSplineLoftShaderProvider scoped ptr (not applicable to WD1?)
	void read(IBinaryArchive& fp);
};

class SplineLoftLowRes {
public:
	uint32_t unk1;
	std::vector<CPathID> materials;
	std::vector<std::string> materialPaths; // for debugging
	CSceneSplineLoftBatch unk3;
	CPathID hiRes; //CSplineLoftHiResGfxResource

	std::shared_ptr<VertexBuffer> vertex;
	std::shared_ptr<IndexBuffer> index;

	std::vector<uint8_t> vertexData;
	std::vector<uint8_t> indexData;

	void open(IBinaryArchive& fp);
	void draw(ID3D11DeviceContext* context);
	void createBuffers();
};

struct SLoftShapeVertex
{
	glm::vec4 pos;
	glm::vec4 normal;
	glm::vec2 uv;
	float occlusion;
};

class LoftShape {
public:
	//uint32_t unk1;
	//uint32_t size;
	//uint32_t count;
	/*
	* CLoftShapeRenderResource *m_renderResource;
    * CSmartResourcePtr<CMaterialResource> m_materialResource;
	*/

	struct SLoftShapeEdge
	{
		uint16_t pointIdx[2];
	};

	struct SLoftShapeEdgeWelding
	{
		uint16_t welding[2]; // Not sure what this is
	};

	struct CLoftShapeLODSide
	{
		SLoftShapeVertex *edgeVertices;
		unsigned int nbrEdgeVertices;
		SLoftShapeEdgeWelding *edgeWelding;
		SLoftShapeVertex *capVertices;
		unsigned int nbrCapVertices;
		Vector<uint16_t> capIndices;
		unsigned int nbrCapIndices;
		glm::vec3 offsetLeft;
		glm::vec3 offsetRight;
		glm::vec3 offsetTop;
		glm::vec3 offsetBottom;
	};

	// SChunkHeader types: 0x4C4F4453, 0x4D54524C

	struct CLoftShapeLOD {
		CLoftShapeLODSide sides[2];
		float maxDistance;
		SLoftShapeEdge edges;
		unsigned int nbrEdges;
		glm::vec2 texCoordRatio;
		unsigned int useShapeVCoord;
		float lodDistance;
	};

	void open(IBinaryArchive& fp);
	void draw();
};
