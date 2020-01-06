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

class CSceneSplineLoftRegion {
public:
	glm::vec2 unk1;
	float unk2;
	uint32_t unk3;

	Vector<CSpline> splines;
	uint32_t splineUnk;

	uint32_t unk4;

	void read(IBinaryArchive& fp);
};

class CSplineLoftPrimitiveDesc {
public:
	void read(IBinaryArchive& fp);
};

class SSplineLoftDrawCall {
public:
	void read(IBinaryArchive& fp);
};

class CSplineNetworkRegionResourceEntry {
public:
	bool unk1;

	CSceneSplineLoftRegion loftReigon;
	Vector<CSplineLoftPrimitiveDesc> primitiveDesc;
	Vector<SSplineLoftDrawCall> drawCalls;
	Vector<CPathID> unk2;
	Vector<uint32_t> unk3;

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
	Vector<UnkStr> vertexData;
	std::shared_ptr<VertexBuffer> vertex;

	Vector<uint16_t> indexData;
	std::shared_ptr<IndexBuffer> index;

	uint32_t unk2;
	Vector<CSplineNetworkRegionResourceEntry> networkRegionResources;

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
