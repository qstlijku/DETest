#pragma once

#include <stdint.h>
#include <vector>
#include "CPathID.h"
#include <memory>
#include <array>
#include "xbtFile.h"
#include "DDRenderInterface.h"

class IBinaryArchive;
class MemberStructure;

class CSectorHighRes {
public:
	void open(IBinaryArchive &fp);

	struct STerrainSectorPackedElementInfo {
		uint64_t unk1;
		uint32_t unk2;
		void read(IBinaryArchive& fp);
	};

	struct CSceneTerrainSectorPackedData {
		bool unk1;
		uint32_t unk2;

		std::array<uint16_t, 0x5ab0 / 2> terrainSectorPackedData;//STerrainSectorPackedData

		std::vector<STerrainSectorPackedElementInfo> elements;

		uint8_t unk4;
		uint8_t unk5;
		uint8_t unk6;
		uint8_t unk7;

		void read(IBinaryArchive& fp);

		//CTerrain::GetZ(long, long)
		double GetZ(long x, long y);
		void SetZ(long x, long y, double height);
		static int GetOffset(long x, long y);
		static uint16_t CompressHeight(double height);
		static double DecompressHeight(uint16_t packed);

		std::shared_ptr<VertexBuffer> getVertexBuffer();
		static std::shared_ptr<IndexBuffer> getIndexBuffer();
	private:
		//DisruptEditorData
		std::shared_ptr<VertexBuffer> vertexBuffer;
		bool isDirty = false;
	};

	uint32_t unk1;

	std::vector<CSceneTerrainSectorPackedData> maps;
	CSceneTerrainSectorPackedData& getMap(int x, int y);
};


class CSector {
public:
	void open(IBinaryArchive &fp);
	void draw();
	void save();

	CPathID lowRes, hiRes;

	//Stored at 0x4 in CSector
	struct SSectorDataChunk {//Size is 0x38
		uint8_t unk1;//0
		std::array<uint8_t, 3> padding;
		uint32_t unk2;//4, 
		float minZ;//8
		float maxZ;//0xC, 
		uint32_t unk5;//0x10
		float unk6;//0x14
		float unk7;//0x18
		float unk8;//0x1C
		float unk9;//0x20
		float unk10;//0x24
		float unk11;//0x28
		float unk12;//0x2C
		float unk13;//0x30
		uint8_t unk14;//0x34
		uint8_t unk15;//0x35
		uint16_t padding2;
		
		//For SWaterSectorDatas?
		//COneBitGrid<(long)65>
		std::array<uint8_t, (65 * 8) + 9> bitGrid;

		void read(IBinaryArchive& fp);
	};

	std::vector<SSectorDataChunk> dataChunk;

	//DisruptEditorData
	int xPos, yPos, sectorID;
	std::shared_ptr<xbtFile> getColorTexture();
	std::shared_ptr<xbtFile> getDiffuseTexture();
	std::shared_ptr<xbtFile> getMaskTexture();
	std::shared_ptr<CSectorHighRes> getHiRes();
private:
	std::shared_ptr<CSectorHighRes> highRes;
	std::shared_ptr<xbtFile> color, diffuse, mask;
};
