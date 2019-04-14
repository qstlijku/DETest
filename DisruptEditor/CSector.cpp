#include "CSector.h"

#include <stdio.h>
#include <stdint.h>
#include <SDL_assert.h>
#include "IBinaryArchive.h"
#include "Vector.h"
#include <SDL.h>
#include "FileHandler.h"
#include "debug_draw.hpp"

void CSectorHighRes::open(IBinaryArchive &fp) {
	uint32_t magic = 1397901394;
	fp.serialize(magic);
	SDL_assert_release(magic == 1397901394);

	uint32_t version = 4;
	fp.serialize(version);
	SDL_assert_release(version == 4);

	uint32_t size = maps.size();
	fp.serialize(size);
	maps.resize(size);

	fp.serialize(unk1);

	SDL_assert_release(size == 16);

	for (uint32_t i = 0; i < size; ++i)
		maps[i].read(fp);
}

CSectorHighRes::CSceneTerrainSectorPackedData& CSectorHighRes::getMap(int x, int y) {
	SDL_assert(x >= 0 && x < 4 && y >= 0 && y < 4);
	return maps[(y * 4) + x];
}

void CSectorHighRes::CSceneTerrainSectorPackedData::read(IBinaryArchive & fp) {
	//SDL_Log("STerrainSectorPackedData: %u", fp.tell());

	fp.serialize(unk1);
	if (unk1) {
		fp.serialize(unk2);

		//SDL_Log("STerrainSectorPackedData counter: %u", fp.tell());
		uint32_t size = elements.size();
		fp.serialize(size);
		elements.resize(size);

		//Seek
		Sint64 seek = ((fp.tell() - 4 + 19) & 0xFFFFFFFFFFFFFFF0ui64) - fp.tell();
		Vector<uint8_t> temp(seek);
		fp.memBlock(temp.data(), 1, seek);

		//SDL_Log("STerrainSectorPackedData data: %u", fp.tell());
		fp.memBlock(terrainSectorPackedData.data(), terrainSectorPackedData.size() * sizeof(uint16_t), 1);

		for (uint32_t i = 0; i < size; ++i)
			elements[i].read(fp);

		fp.serialize(unk4);
		fp.serialize(unk5);
		fp.serialize(unk6);
		fp.serialize(unk7);
	}
}

typedef uint32_t uint;
typedef uint16_t ushort;
typedef uint8_t byte;

double CONCAT44(uint a, uint b) {
	uint64_t c = a;
	c = c << (4 * 8);
	c |= b;
	return *(double*)&c;
}

const float _DAT_105070e0 = -1.0;
const double _DAT_105070f0 = 4.503599627370496e15;
const float _DAT_10507df0 = 0.0078125;

double CSectorHighRes::CSceneTerrainSectorPackedData::GetZ(long x, long y) {
	return DecompressHeight(terrainSectorPackedData[GetOffset(x, y)]);
}

void CSectorHighRes::CSceneTerrainSectorPackedData::SetZ(long x, long y, double height) {
	terrainSectorPackedData[GetOffset(x, y)] = CompressHeight(height);
}

int CSectorHighRes::CSceneTerrainSectorPackedData::GetOffset(long x, long y) {
	int offset = ((y * 0x41 + x) * 4 + 0x150) / 2;
	SDL_assert_release(offset <= terrainSectorPackedData.size());
	return offset;
}

uint16_t CSectorHighRes::CSceneTerrainSectorPackedData::CompressHeight(double height) {
	double step1 = height / _DAT_10507df0;
	double step2 = step1 + _DAT_105070f0;
	uint64_t step3 = *(uint64_t*)& step2;
	uint64_t step4 = step3 & 0x00000000FFFFFFFF;
	return step4;
}

double CSectorHighRes::CSceneTerrainSectorPackedData::DecompressHeight(uint16_t packed) {
	return (double)((float)((double)CONCAT44(0x43300000, packed) - _DAT_105070f0) * _DAT_10507df0);
}

std::shared_ptr<VertexBuffer> CSectorHighRes::CSceneTerrainSectorPackedData::getVertexBuffer() {
	if (!vertexBuffer || isDirty) {
		Vector<float> data;
		data.reserve(65 * 65 * 3);
		for (int x = 0; x < 65; ++x) {
			for (int y = 0; y < 65; ++y) {
				data.push_back(x);
				data.push_back(y);
				data.push_back(GetZ(x, y));
			}
		}

		vertexBuffer = createVertexBuffer(data.data(), data.size() * sizeof(float), VertexBufferOptions::BUFFER_STATIC);
		isDirty = false;
	}

	return vertexBuffer;
}

std::shared_ptr<VertexBuffer> CSectorHighRes::CSceneTerrainSectorPackedData::getIndexBuffer() {
	static std::shared_ptr<VertexBuffer> indexBuffer;
	if (!indexBuffer) {
		Vector<uint16_t> data;

		indexBuffer = createVertexBuffer(data.data(), data.size() * sizeof(uint16_t), VertexBufferOptions::BUFFER_STATIC);
	}
	return indexBuffer;
}

void CSectorHighRes::STerrainSectorPackedElementInfo::read(IBinaryArchive & fp) {
	//SDL_Log("STerrainSectorPackedElementInfo: %u", fp.tell());
	fp.pad(4);
	fp.padding = IBinaryArchive::PADDING_NONE;
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.padding = IBinaryArchive::PADDING_IBINARYARCHIVE;
}

void CSector::open(IBinaryArchive & fp) {
	uint32_t magic = 1396921426;
	fp.serialize(magic);
	SDL_assert_release(magic == 1396921426);

	uint32_t version = 6;
	fp.serialize(version);
	SDL_assert_release(version == 6);

	lowRes.read(fp);
	hiRes.read(fp);

	fp.serializeNdVectorExternal(dataChunk);
	SDL_assert_release(dataChunk.size() == 16);

	size_t size = fp.size();
	SDL_assert_release(fp.tell() == fp.size());
}

void CSector::draw() {
	std::shared_ptr<CSectorHighRes> hiRes = getHiRes();
	float xOffset = xPos * 64;
	float yOffset = yPos * 64;

	for (int x = 0; x < 64 * 2; ++x) {
		for (int y = 0; y < 64 * 2; ++y) {
			int xSectorOffset = x / 32;
			int ySectorOffset = y / 32;
			int xSectorInnerOffset = x % 32;
			int ySectorInnerOffset = y % 32;

			auto map = hiRes->getMap(xSectorOffset, ySectorOffset);
			for (int sx = 0; sx < 32; ++sx) {
				for (int sy = 0; sy < 32; ++sy) {
					/*glm::vec3 pos((sx / 2.f) + xOffset, (sy / 2.f) + yOffset, map.GetZ(sx, sy));
					dd::cross(&pos.x, 0.25f);*/
				}
			}

			//SDL_Log("%i %i %i %i", xSectorOffset, ySectorOffset, xSectorInnerOffset, ySectorInnerOffset);
		}
	}

	int a = 1;
}

void CSector::save() {
	//TODO: Calculate min and max z
	/*for (auto &it : dataChunk) {
		it.minZ = 60;
		it.maxZ = 130;
	}*/

	char filename[80];
	snprintf(filename, sizeof(filename), "worlds/windy_city/generated/sdat/sd%u.sdat", sectorID);
	SDL_RWops* fp = FH::openFileWrite(filename);
	open(CBinaryArchiveWriter(fp));
	SDL_RWclose(fp);

	if (highRes) {
		snprintf(filename, sizeof(filename), "worlds/windy_city/generated/sdat/sd%u.sdhr", sectorID);
		fp = FH::openFileWrite(filename);
		highRes->open(CBinaryArchiveWriter(fp));
		SDL_RWclose(fp);
	}
}

std::shared_ptr<CSectorHighRes> CSector::getHiRes() {
	if (!highRes) {
		highRes = std::make_shared<CSectorHighRes>();
		char filename[80];
		snprintf(filename, sizeof(filename), "worlds/windy_city/generated/sdat/sd%u.sdhr", sectorID);
		SDL_RWops* fp = FH::openFile(filename);
		SDL_assert_release(fp);
		CBinaryArchiveReader reader(fp);
		highRes->open(reader);
		SDL_RWclose(fp);
	}
	return highRes;
}

void CSector::SSectorDataChunk::read(IBinaryArchive & fp) {
	fp.serialize(unk1);
	for (int i = 0; i < 3; ++i)
		fp.serialize(padding[i]);
	fp.serialize(unk2);
	fp.serialize(minZ);
	fp.serialize(maxZ);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(unk9);
	fp.serialize(unk10);
	fp.serialize(unk11);
	fp.serialize(unk12);
	fp.serialize(unk13);
	fp.serialize(unk14);
	fp.serialize(unk15);
	fp.serialize(padding2);

	if (unk6 > 0.f)
		fp.memBlock(bitGrid.data(), 1, bitGrid.size());
	else
		SDL_Log("No bit grid");
}
