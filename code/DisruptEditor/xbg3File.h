/*

Copyright 2019 Jonathan Scott
All rights reserved
You may not use this file without permission

*/

#pragma once

#include <stdint.h>
#include "Vector.h"
#include <string>
#include "CPathID.h"
#include "CStringID.h"
#include "glm/glm.hpp"
#include "NBCF.h"
#include <list>
#include <memory>
#include "DDRenderInterface.h"
#include "DisruptTypes.h"

class IBinaryArchive;
class MemberStructure;

class xbg3File {
public:
	void open(IBinaryArchive &fp);

	//Header (20 bytes)
	struct Header {
		uint32_t magic;
		uint16_t majorVersion;
		uint16_t minorVersion;
		uint32_t unk1;
		uint32_t unk2;
		uint32_t unk3;
		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};
	Header header;

	struct SMemoryNeed {
		uint32_t unk1;
		uint32_t unk2;
		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};
	SMemoryNeed memoryNeeded;

	float unk1;
	bool unk2;//This bool is used in the first branch of SceneGeometryParams

	struct CMeshNameID {
		std::string name;
		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};

	// CSceneGeometry::SMeshDecompression struct (WDL):
	// positionMin, positionRange (floats)
	// meshLocalHeight, isBuildingFacade (both floats!)

	struct SceneGeometryParams {
		// preAllocatedMemory?
		// WDL meshDecompression: 4 floats
		uint32_t unk1;
		float unk2;
		float unk3;
		float unk4;

		//First Branch
		float unk5;
		float unk6;

		// WDL: glm::vec4 uvDecompression here

		glm::vec3 unk7; // bSphereCenter
		float unk8; // bSphereRadius
		glm::vec3 unk9; // bBoxMin
		glm::vec3 unk10; // bBoxMax

		//Game doesn't read this?
		uint32_t unk11;
		uint32_t unk12;
		uint32_t unk13;
		//uint32_t unk14;

		Vector<float> lods;

		float unk15; // killDistance
		bool unk16; // castShadowEnable, showInReflectionEnable (WDL)
		bool unk17; // isMeshWater (WDL)
		uint8_t unk18; // pcSkuLodFlags (no clue what this is)

		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};
	SceneGeometryParams geomParams;

	struct MaterialResources {
		uint32_t unk0;
		Vector<float> unk1;
		struct MaterialFile {
			std::string file;
			std::string materialName; // sometimes filename duplicated
			void read(IBinaryArchive &fp);
			void registerMembers(MemberStructure &ms);
		};
		Vector<MaterialFile> materials;

		void read(IBinaryArchive &fp, uint32_t lods);
		void registerMembers(MemberStructure &ms);
	};
	MaterialResources materialResources;

	struct MaterialSlotToIndex {
		struct Slot {
			CMeshNameID name;
			uint32_t slot;
			void read(IBinaryArchive &fp);
			void registerMembers(MemberStructure &ms);
		};
		Vector<Slot> slots;

		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};
	MaterialSlotToIndex materialSlotToIndex;

	struct SkinNames {
		Vector<CMeshNameID> skins;

		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};
	SkinNames skinNames;

	struct BonePalettes {
		struct BonesPallet {
			Vector<uint16_t> unk1;
			void read(IBinaryArchive &fp);
			void registerMembers(MemberStructure &ms);
		};
		Vector<BonesPallet> pallets;

		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};
	BonePalettes bonePalettes;

	struct SkelResources {
		struct SRawNode {
			uint32_t unkInd;
			uint32_t unkPad;
			glm::vec4 unkVec;
			glm::vec3 pos;
			glm::vec4 rot;
			uint32_t unk6;
			uint32_t unk7;
			uint32_t unk8;
			void read(IBinaryArchive &fp);
			void registerMembers(MemberStructure &ms);
		};

		struct SkelResource {
			SRawNode node;
			CMeshNameID name;
			void read(IBinaryArchive &fp);
			void registerMembers(MemberStructure &ms);
		};

		Vector<SkelResource> resources;
		uint32_t unk2;
		Vector<glm::mat4> offset2BoneMats;

		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};
	SkelResources skelResources;

	struct CBasicDrawCallRange {
		uint32_t vertexBufferByteOffset; // in WDL: int[3]?
		uint32_t primitiveCount;
		uint32_t indexCount;
		uint32_t indexBufferStartIndex; // TODO: why is this times 2 when used?
		uint16_t vertexCount;
		uint16_t minIndexValue;
		uint16_t maxIndexValue;
		uint16_t groupCount; // vertex smoothing group?

		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};

	struct CClusterChunk
	{
		uint16_t matID; // index of something, could be wrong
		uint32_t unk1;

		// start of draw call
		uint16_t indexCount;
		uint16_t vertexStride;
		uint16_t maxIndex;
		uint16_t unk2;

		uint16_t boneIndices[48];

		void read(IBinaryArchive& fp);
		void registerMembers(MemberStructure& ms);

		// cache
		std::shared_ptr<VertexBuffer> vertex;
	};

	Vector<CClusterChunk> clusters;

	struct LOD {
		struct CSceneMesh {
			glm::vec3 unk1;
			float unk2;
			float unk3;
			uint32_t primitiveType;//0, 1, 0x38

			uint16_t matID;//2, 0x3C
			uint16_t vertexFormat;//3, 0x3E

			uint8_t vertexStride;//4 Vertex Stride, 0x40
			uint8_t unk9;//0x41
			uint16_t unk10;//5, 0x46

			uint32_t boneMapID;//6, 7

			CBasicDrawCallRange drawCall;//0xC

			uint32_t unk12;
			uint32_t unk13;

			struct CDrawCallRange {
				CBasicDrawCallRange drawCall;
				CSphere sphere; // bounding sphere
				glm::vec3 unk1; // CAABBox boundingBox
				glm::vec3 unk2; // CAABBox boundingBox
				CMeshNameID name;
				uint16_t unk3; // visibilityBitIndex
				uint16_t unk4; // attachedBoneIndex

				void read(IBinaryArchive &fp);
				void registerMembers(MemberStructure &ms);
			};
			Vector<CDrawCallRange> drawCalls;

			void read(IBinaryArchive &fp);
			void registerMembers(MemberStructure &ms);

			// cache
			std::shared_ptr<VertexBuffer> vertex;
		};
		Vector<CSceneMesh> meshes;

		void read(IBinaryArchive &fp);
		void registerMembers(MemberStructure &ms);
	};
	Vector<LOD> lods;

	uint32_t unk3;

	struct SGfxBuffers {
		std::shared_ptr<VertexBuffer> vertex;
		std::shared_ptr<IndexBuffer> index;

		void createBuffers();

		std::vector<uint8_t> vertexData, indexData;

		void read(IBinaryArchive &fp);
		void extractNormals();
		void registerMembers(MemberStructure &ms);
	};
	Vector<SGfxBuffers> buffers;

	void registerMembers(MemberStructure &ms);
	void draw(ID3D11DeviceContext* context, int lodNum = 0);
	std::list<std::string> getDiffuseTexture(int lodNum = 0);

	struct VertexType
	{
		glm::vec4 pos;
		glm::vec2 uv;
		glm::vec4 blendWeights;
		glm::vec4 blendIndices;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 binormal;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
		float tx, ty, tz;
		float bx, by, bz;
	};

	struct TempVertexType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VectorType
	{
		float x, y, z;
	};
	std::shared_ptr<VertexBuffer> createVertexBuffer(std::vector<uint8_t> vertexData, int start, int count, int stride, glm::vec4 offset);
	ModelType *CalculateModelVectors(std::vector<VertexType> vertices, int vertexCount);
	void CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3, VectorType& tangent, VectorType& binormal);
	void draw(ID3D11DeviceContext* context, const std::vector<glm::mat4> &mats, int lodNum = 0);

	//Cache
	std::vector<ID3D11ShaderResourceView*> srvs;
};
