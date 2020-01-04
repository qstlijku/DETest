#include "CSector.h"

#include <stdio.h>
#include <stdint.h>
#include <SDL_assert.h>
#include "IBinaryArchive.h"
#include "Vector.h"
#include <SDL.h>
#include "FileHandler.h"
#include "debug_draw.hpp"
#include "DDRenderInterface.h"
#include "glm/gtc/matrix_transform.hpp"
#include "ResourceLoader.h"

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
	SDL_assert_release(offset <= 0x5ab0 / 2);
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

		// Fill in a buffer description.
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(float) * data.size();
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = data.data();
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		// Create the vertex buffer.
		vertexBuffer = std::make_shared<VertexBuffer>();
		HRESULT hr = RenderInterface::instance().g_pd3dDevice->CreateBuffer(&bufferDesc, &InitData, &vertexBuffer->pVertexBuffer);
		vertexBuffer->stride = sizeof(float) * 3;

		isDirty = false;
	}

	return vertexBuffer;
}

std::shared_ptr<IndexBuffer> CSectorHighRes::CSceneTerrainSectorPackedData::getIndexBuffer() {
	static std::shared_ptr<IndexBuffer> indexBuffer;
	if (!indexBuffer) {
		Vector<uint16_t> data;
		
		unsigned int x = 0, y = 0;
		bool yadvance = true, xforward = true;
		unsigned char xadvance = 0;
		do {
			data.push_back((y * 65) + x);

			if (yadvance)
				++y;
			else
				--y;
			yadvance = !yadvance;

			++xadvance;

			if (xadvance == 2) {
				xadvance = 0;
				if (xforward)
					++x;
				else
					--x;
			}

			if (x == 65) {
				x -= 1;
				y += 2;
				yadvance = false;
				xforward = false;
				xadvance = 1;
			} else if (x == -1) {
				x = 0;
				y += 2;
				yadvance = false;
				xforward = true;
				xadvance = 1;
			}
		} while (y < 65);

		indexBuffer = std::make_shared<IndexBuffer>();
		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = data.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(short) * data.size(), D3D11_BIND_INDEX_BUFFER);
		RenderInterface::instance().g_pd3dDevice->CreateBuffer(
			&indexBufferDesc,
			&indexBufferData,
			&indexBuffer->pIndexBuffer);
		indexBuffer->size = data.size();
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

	fp.serializeNdVector(dataChunk);
	SDL_assert_release(dataChunk.size() == 16);

	size_t size = fp.size();
	SDL_assert_release(fp.tell() == fp.size());
}

void CSector::save() {
	//TODO: Calculate min and max z
	/*for (auto &it : dataChunk) {
		it.minZ = 60;
		it.maxZ = 130;
	}*/
	auto it = getHiRes();
	for (auto& a : it->maps) {
		/*for (int x = 0; x < 65; ++x) {
			for (int y = 0; y < 65; ++y) {
				int offset = a.GetOffset(x, y) + 0x21aa;
				float t = a.DecompressHeight(a.terrainSectorPackedData[offset]);
				float w = a.DecompressHeight(a.terrainSectorPackedData[offset+1]);

				//a.terrainSectorPackedData[offset + 1] = 0;// a.CompressHeight(70);
				//SDL_Log("%u %u", a.terrainSectorPackedData[offset], a.terrainSectorPackedData[offset+1]);
			}
		}*/

		int maxOffset = a.GetOffset(64, 64) + 2;
		/*for (int i = 0; i < (65 * 65) / 2; ++i)//This does banding shadows on the terrain
			a.terrainSectorPackedData[(18296 / 2) + i] &= 0xFF00;*/

		/*for (int i = 0; i < 674 / 2; ++i)//This appears to do nothing
			a.terrainSectorPackedData[(22524 / 2) + i] |= 0xFFFF;*/

		/*for (int i = 0; i < 1030 / 2; i += 2)//Causes weird floating triangles far away
			a.terrainSectorPackedData[(17236 / 2) + 5 + i] &= 0;*/

		/*for (int i = 0; i < 0x150 / 2; i += 1)//Causes weird floating triangles far away
			a.terrainSectorPackedData[i] &= 0;*/
	}

	for (auto& a : dataChunk) {
		for (auto& it : a.bitGrid)
			it = 0;
	}

	//Extra interesting data at 18296 bytes

	char filename[80];
	snprintf(filename, sizeof(filename), "worlds/windy_city/generated/sdat/sd%u.sdat", sectorID);
	SDL_RWops* fp = FH::openFileWrite(filename);
	CBinaryArchiveWriter writer(fp);
	open(writer);
	SDL_RWclose(fp);

	if (highRes) {
		snprintf(filename, sizeof(filename), "worlds/windy_city/generated/sdat/sd%u.sdhr", sectorID);
		fp = FH::openFileWrite(filename);
		CBinaryArchiveWriter writer(fp);
		highRes->open(writer);
		SDL_RWclose(fp);
	}
}

std::shared_ptr<xbtFile> CSector::getColorTexture() {
	if (!color) {
		char filename[80];
		snprintf(filename, sizeof(filename), "worlds/windy_city/generated/sdat/atlas%u_color.xbt", sectorID);
		color = loadTexture(filename);
	}
	return color;
}

std::shared_ptr<xbtFile> CSector::getDiffuseTexture() {
	if (!diffuse) {
		char filename[80];
		snprintf(filename, sizeof(filename), "worlds/windy_city/generated/sdat/atlas%u_diffuse_high.xbt", sectorID);
		diffuse = loadTexture(filename);
	}
	return diffuse;
}

std::shared_ptr<xbtFile> CSector::getMaskTexture() {
	if (!mask) {
		char filename[80];
		snprintf(filename, sizeof(filename), "worlds/windy_city/generated/sdat/atlas%u_mask_high.xbt", sectorID);
		mask = loadTexture(filename);
	}
	return mask;
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
}
