/*

Copyright 2019 Jonathan Scott
All rights reserved
You may not use this file without permission

*/

#include "xbg3File.h"

#include <iostream>
#include "Vector.h"
#include <stdlib.h>
#include "ResourceLoader.h"
#include "materialFile.h"
#include "xbtFile.h"
#include "xbt3File.h"
#include "glm/glm.hpp"
#include <SDL_log.h>
#include <SDL_rwops.h>
#include "FileHandler.h"
#include "IBinaryArchive.h"
#include "Serialization.h"
#include "HexBase64.h"
#include <DDRenderInterface.h>
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm\gtc\matrix_transform.hpp>
#include <Common.h>
#include <material3File.h>

static void serializeMat4(IBinaryArchive& fp, glm::mat4 &vec) {
	fp.serialize(vec);
}

static void serializeMat4Vec(IBinaryArchive& fp, Vector<glm::mat4>& vec) {
	uint32_t count = vec.size();
	fp.serialize(count);
	vec.resize(count);
	for (uint32_t i = 0; i < count; ++i)
		serializeMat4(fp, vec[i]);
}

void xbg3File::open(IBinaryArchive &fp) {
	header.read(fp);

	//CGeometryResource::GetCurrentVersion()
	//	which calls CPathID::GetCollisionTableVersion()
	//	which returns 0x38 (On WiiU)
	
	//ReadValue<T1>(T1 &, unsigned char *&) [with T1=CGeometryResource::SMemoryNeed]
	SDL_Log("SMemoryNeed: %u", fp.tell());
	memoryNeeded.read(fp);

	//.text:05A15178 Then Pads 4, loads float
	fp.serialize(unk1);

	fp.serialize(unk2);

	//CGeometryResource::ReadSceneGeometryParams((CSceneGeometry &,uchar const *&))
	SDL_Log("SceneGeometryParams: %u", fp.tell());
	//geomParams.read(fp);

	//materialResources.read(fp, geomParams.lods.size());

	SDL_RWseek(fp.fp, 0x34, RW_SEEK_SET); // TODO scan file for RMTL (0x20)
	// then go to next line + 4
	SDL_Log("MaterialResources: %u", fp.tell());
	fp.serializeNdVector(materialResources.materials);
	/*
	SDL_Log("MaterialSlotToIndex: %u", fp.tell());
	materialSlotToIndex.read(fp);

	SDL_Log("SkinNames: %u", fp.tell());
	skinNames.read(fp);

	SDL_Log("BonePalettes: %u", fp.tell());
	bonePalettes.read(fp);
	*/
	uint32_t SKEL;
	fp.serialize(SKEL);
	SDL_assert_release(SKEL == 0x534B454C);
	
	SDL_RWseek(fp.fp, 0x14, RW_SEEK_CUR);
	uint32_t NODE;
	fp.serialize(NODE);
	SDL_assert_release(NODE == 0x4E4F4445);

	SDL_RWseek(fp.fp, 0x10, RW_SEEK_CUR);
	skelResources.read(fp);

	uint32_t SKID;
	fp.serialize(SKID);
	SDL_assert_release(SKID == 0x534B4944);

	fp.scanTillConstant(0x534B4E44); // SKND
	SDL_RWseek(fp.fp, 0x10, RW_SEEK_CUR);

	uint32_t CLUS;
	fp.serialize(CLUS); // ClusterChunk
	SDL_assert_release(CLUS == 0x434C5553);
	
	SDL_RWseek(fp.fp, 0x10, RW_SEEK_CUR);
	fp.serializeNdVector(clusters);

	//SDL_RWseek(fp.fp, 0xACF, RW_SEEK_SET);
	fp.scanTillConstant(0x4C4F4453); // LODS
	lods.resize(1);
	lods[0].read(fp);

	// vertexStride: 28 00 00 00 or 40

	// CLUS: 03 00 00 00?
	// F4 02 28 00 (stride is 0x28 = 40, DrawIndexed 0x02F4 = 756)
	// 02 01 28 00 (0x0102 = 258)
	// 8A 03 28 00 (0x038A = 906)

	// length is evidently 28 69 00 00
	//SDL_RWseek(fp.fp, 0xB53, RW_SEEK_SET);
	SDL_Log("SGfxBuffers: %u", fp.tell());

	fp.scanTillConstant(0x01020304);
	SDL_RWseek(fp.fp, -13, RW_SEEK_CUR); // TODO verify for other cases
	xbg3File::SGfxBuffers buffer;
	fp.serialize(buffer);
	//fp.serializeNdVector(buffers);

	buffers.resize(1);
	buffers[0] = buffer;

	// TODO implement other LODS
	return;

	SDL_RWseek(fp.fp, 0x8400, RW_SEEK_SET);
	xbg3File::SGfxBuffers buffer2;
	//fp.serialize(buffer2);

	SDL_RWseek(fp.fp, 0xC15C, RW_SEEK_SET);
	xbg3File::SGfxBuffers buffer3;
	//fp.serialize(buffer3);

	SDL_RWseek(fp.fp, 0xDF50, RW_SEEK_SET);
	xbg3File::SGfxBuffers buffer4;
	fp.serialize(buffer4);

	// 4 LODs, only care about the buffer from the first one
	// might as well read the others anyway

	// for index buffer: length is 80 07 00 00
	// multiply by 2, skip 4 (04 03 02 01) then serialize indexData

	//SDL_assert_release(fp.tell() == fp.size());
}

void xbg3File::CClusterChunk::read(IBinaryArchive& fp) {
	/*
	* 		uint32_t numDrawCalls; // TODO vector count
		uint32_t unk1;
		uint16_t unk2;

		// start of draw call
		uint16_t indexCount;
		uint16_t vertexStride;
		uint16_t maxIndex;
		uint16_t unk3;

		uint16_t boneIndices[48];
	*/
	fp.serialize(matID);
	fp.serialize(unk1);

	fp.serialize(indexCount);
	fp.serialize(vertexStride);
	fp.serialize(maxIndex);
	fp.serialize(unk2);

	for (int i = 0; i < 48; i++)
	{
		fp.serialize(boneIndices[i]);
	}

	SDL_RWseek(fp.fp, 0x20, RW_SEEK_CUR); // maxIndex again?
	SDL_RWseek(fp.fp, 0x20, RW_SEEK_CUR); // numINdices again?
}

void xbg3File::Header::read(IBinaryArchive & fp) {
	magic = 0x47454F4D;
	fp.serialize(magic);
	if (magic != 0x47454F4D)
	{
		SDL_assert_release(magic == 0x4D455348);
		return; // for now
	}

	majorVersion = 97;
	fp.serialize(majorVersion);
	SDL_assert_release(majorVersion == 97);

	minorVersion = 50;
	fp.serialize(minorVersion);
	SDL_assert_release(minorVersion == 50);

	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
}

void xbg3File::Header::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(magic);
	REGISTER_MEMBER(majorVersion);
	REGISTER_MEMBER(minorVersion);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
}

void xbg3File::SMemoryNeed::read(IBinaryArchive & fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
}

void xbg3File::SMemoryNeed::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
}

void xbg3File::SceneGeometryParams::read(IBinaryArchive &fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);

	fp.serialize(unk5);
	fp.serialize(unk6);

	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(unk9);
	fp.serialize(unk10);

	fp.serialize(unk11);
	fp.serialize(unk12);
	fp.serialize(unk13);
	//fp.serialize(unk14);

	fp.serializeNdVector(lods);

	fp.serialize(unk15);
	fp.serialize(unk16);
	fp.serialize(unk17);
	fp.serialize(unk18);
}

void xbg3File::SceneGeometryParams::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
	REGISTER_MEMBER(unk9);
	REGISTER_MEMBER(unk10);
	REGISTER_MEMBER(unk11);
	REGISTER_MEMBER(unk12);
	REGISTER_MEMBER(unk13);
	REGISTER_MEMBER(lods);
	REGISTER_MEMBER(unk15);
	REGISTER_MEMBER(unk16);
	REGISTER_MEMBER(unk17);
	REGISTER_MEMBER(unk18);
}

void xbg3File::MaterialResources::read(IBinaryArchive & fp, uint32_t lods) {
	fp.serialize(unk0);
	uint32_t s = lods - unk0;
	unk1.resize(s);
	for (uint32_t i = 0; i < s; ++i)
		fp.serialize(unk1[i]);
	fp.serializeNdVector(materials);
}

void xbg3File::MaterialResources::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk0);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(materials);
}

void xbg3File::MaterialResources::MaterialFile::read(IBinaryArchive & fp) {
	if (fp.padding != IBinaryArchive::PaddingType::PADDING_ONE)
	{
		// hack for first one
		CPathID hashed(file);
		fp.serialize(hashed);
	}
	fp.padding = IBinaryArchive::PaddingType::PADDING_ONE;
	fp.serialize(file);
	fp.serialize(materialName);
}

void xbg3File::MaterialResources::MaterialFile::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, file);
	ms.registerMember(NULL, materialName);
}

void xbg3File::MaterialSlotToIndex::read(IBinaryArchive & fp) {
	fp.serializeNdVector(slots);
}

void xbg3File::MaterialSlotToIndex::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, slots);
}

void xbg3File::MaterialSlotToIndex::Slot::read(IBinaryArchive & fp) {
	name.read(fp);
	fp.serialize(slot);
}

void xbg3File::MaterialSlotToIndex::Slot::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(slot);
}

void xbg3File::SkinNames::read(IBinaryArchive & fp) {
	fp.serializeNdVector(skins);
}

void xbg3File::SkinNames::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, skins);
}

void xbg3File::BonePalettes::read(IBinaryArchive & fp) {
	fp.serializeNdVector(pallets);
}

void xbg3File::BonePalettes::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, pallets);
}

void xbg3File::BonePalettes::BonesPallet::read(IBinaryArchive & fp) {
	fp.serializeNdVector(unk1);
}

void xbg3File::BonePalettes::BonesPallet::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, unk1);
}

void xbg3File::SkelResources::read(IBinaryArchive & fp) {
	fp.serializeNdVector(resources);
	fp.serialize(unk2); // MB2O
	SDL_assert_release(unk2 == 0x4F32424D);

	SDL_RWseek(fp.fp, 0x10, RW_SEEK_CUR);
	serializeMat4Vec(fp, offset2BoneMats);

	//SDL_assert_release(resources.size() == mats.size());
}

void xbg3File::SkelResources::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(resources);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(offset2BoneMats);
}

void xbg3File::SkelResources::SRawNode::read(IBinaryArchive & fp) {
	fp.serialize(unkInd);
	fp.serialize(unkPad);
	fp.serialize(unkVec);
	fp.serialize(pos);
	fp.serialize(rot);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
}

void xbg3File::SkelResources::SRawNode::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unkInd);
	REGISTER_MEMBER(unkPad);
	REGISTER_MEMBER(unkVec);
	REGISTER_MEMBER(pos);
	REGISTER_MEMBER(rot);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
}

void xbg3File::SkelResources::SkelResource::read(IBinaryArchive & fp) {
	CStringID hashed;
	fp.serialize(hashed);
	node.read(fp);
	name.read(fp);
}

void xbg3File::SkelResources::SkelResource::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, node);
	REGISTER_MEMBER(name);
}

void xbg3File::CMeshNameID::read(IBinaryArchive & fp) {
	//CStringID hashed;
	//fp.serialize(hashed);
	fp.serialize(name);
}

void xbg3File::CMeshNameID::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, name);
}

void xbg3File::LOD::read(IBinaryArchive & fp) {
	fp.serializeNdVector(meshes);
}

void xbg3File::LOD::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, meshes);
}

void xbg3File::LOD::CSceneMesh::read(IBinaryArchive & fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(primitiveType);
	fp.serialize(matID);
	fp.serialize(vertexFormat);
	fp.serialize(unk3);
	fp.serialize(vertexStride);
	fp.serialize(unk9);
	fp.serialize(unk10);
	fp.serialize(boneMapID);
	return;
	drawCall.read(fp);

	uint32_t count = drawCalls.size();
	fp.serialize(count);
	drawCalls.resize(count);

	fp.serialize(unk12);
	fp.serialize(unk13);

	for (uint32_t i = 0; i < count; ++i)
		drawCalls[i].read(fp);
}

void xbg3File::LOD::CSceneMesh::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(primitiveType);
	REGISTER_MEMBER(matID);
	REGISTER_MEMBER(vertexFormat);
	REGISTER_MEMBER(vertexStride);
	REGISTER_MEMBER(unk9);
	REGISTER_MEMBER(unk10);
	REGISTER_MEMBER(boneMapID);
	REGISTER_MEMBER(drawCall);
	REGISTER_MEMBER(unk12);
	REGISTER_MEMBER(unk13);
	REGISTER_MEMBER(drawCalls);
}

void xbg3File::CBasicDrawCallRange::read(IBinaryArchive & fp) {
	fp.serialize(vertexBufferByteOffset);
	fp.serialize(primitiveCount);
	fp.serialize(indexCount);
	fp.serialize(indexBufferStartIndex);
	fp.serialize(vertexCount);
	fp.serialize(minIndexValue);
	fp.serialize(maxIndexValue);
	fp.serialize(groupCount);
}

void xbg3File::CBasicDrawCallRange::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(vertexBufferByteOffset);
	REGISTER_MEMBER(primitiveCount);
	REGISTER_MEMBER(indexCount);
	REGISTER_MEMBER(indexBufferStartIndex);
	REGISTER_MEMBER(vertexCount);
	REGISTER_MEMBER(minIndexValue);
	REGISTER_MEMBER(maxIndexValue);
	REGISTER_MEMBER(groupCount);
}

void xbg3File::LOD::CSceneMesh::CDrawCallRange::read(IBinaryArchive& fp) {
	SDL_Log("CDrawCallRange: %u", fp.tell());
	drawCall.read(fp);
	sphere.read(fp);
	fp.serialize(unk1);
	fp.serialize(unk2);
	name.read(fp);
	fp.serialize(unk3);
	fp.serialize(unk4);
}

void xbg3File::LOD::CSceneMesh::CDrawCallRange::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(drawCall);
	REGISTER_MEMBER(sphere);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
}

void xbg3File::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(header);
	REGISTER_MEMBER(memoryNeeded);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(geomParams);
	REGISTER_MEMBER(materialResources);
	REGISTER_MEMBER(materialSlotToIndex);
	REGISTER_MEMBER(skinNames);
	REGISTER_MEMBER(bonePalettes);
	REGISTER_MEMBER(skelResources);
	REGISTER_MEMBER(lods);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(buffers);
}

// TODO: do we need multiple matrices?
void xbg3File::draw(ID3D11DeviceContext* context, int lodNum) {
	std::vector<glm::mat4> mats(1);
	mats[0] = RenderInterface::instance().objectCB.Model;
	draw(context, mats, lodNum);
}

std::list<std::string> xbg3File::getDiffuseTexture(int lodNum)
{
	std::list<std::string> textures;
	if (lodNum >= lods.size())
	{
		return textures;
	}
	auto& lod = lods[lodNum];
	for (auto& mesh : lod.meshes) {
		auto materialFile = materialResources.materials[mesh.matID].file;
		auto material = loadMaterial(materialFile.c_str());
		auto diffuseFile = material->getCommandPath("DiffuseTexture1");
		if (diffuseFile != "")
			textures.push_back(diffuseFile);
	}
	return textures;
}

enum EBufferType {
	VERTEX_BUFFER = 0,
	INDEX_BUFFER = 1,
	UNIFORM_BLOCK = 2,
	SHADER_PROGRAM = 3,
	STREAM_OUTPUT = 4,
	DISPLAY_LIST = 5
};

void xbg3File::SGfxBuffers::read(IBinaryArchive & fp) {
	//<unnamed>::ReadGfxBuffers(const unsigned char *&, SGfxBuffers &, unsigned long, bool)
	
	//CBufferRenderResource::Create(Device3D::EBufferType 0, const IRenderResourceCommandTrackerDecoratorFactory & (addi r4, r27, unk_107D7CDA@l), unsigned long, unsigned long 1, const void *, bool 0, bool 0, unsigned long 0, bool, bool)
	//fp.pad(4);
	//fp.serializeNdVector(vertexData);

	uint32_t count;
	fp.serialize(count);
	vertexData.resize(count);
	uint8_t size;
	fp.serialize(size);
	for (int i = 1; i < size; i++)
	{
		uint8_t temp;
		fp.serialize(temp);
	}
	fp.memBlock(vertexData.data(), 1, count);

	fp.serialize(count);
	indexData.resize(2 * count);
	fp.serialize(size);
	for (int i = 1; i < size; i++)
	{
		uint8_t temp;
		fp.serialize(temp);
	}

	fp.memBlock(indexData.data(), 1, 2 * count);
	//SDL_RWseek(fp.fp, 0xEA28, RW_SEEK_SET);
	//xbg3File::SGfxBuffers bufferInd;
	//fp.serialize(bufferInd);

	//extractNormals();
	
	//CBufferRenderResource::Create(Device3D::EBufferType 1, const IRenderResourceCommandTrackerDecoratorFactory &, unsigned long, unsigned long, const void *, bool, bool, unsigned long, bool, bool)
	//fp.pad(4);
	//uint16_t unk;
	//fp.serialize(unk);
	//fp.serializeNdVector(indexData);

	//Device3D::CBuffer::Create(Device3D::EBufferType, Device3D::EBufferUsage, unsigned long elementSize, unsigned long elementCount, const void * ptr, bool)
}

void xbg3File::SGfxBuffers::createBuffers() {
	vertex = std::make_shared<VertexBuffer>();
	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = vertexData.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(vertexData.size(), D3D11_BIND_VERTEX_BUFFER);
	RenderInterface::instance().g_pd3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertex->pVertexBuffer);

	index = std::make_shared<IndexBuffer>();
	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = indexData.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(indexData.size(), D3D11_BIND_INDEX_BUFFER);
	RenderInterface::instance().g_pd3dDevice->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&index->pIndexBuffer);
	index->size = indexData.size();
}

// Convert normalized float to signed 16 bit integer
// signed 8 bit: -128 to 127 (uint8_t from 0 to 255)
static float charsToFloat(uint8_t x, uint8_t y)
{
	int result = x + 256 * y;
	SDL_assert_release(result >= 0);
	if (result < 32768)
		return result;
	return result - 65536;
}

// Technically this shouldn't be x - 1
// but we want it to match zmodeler export
static float charToFloat(uint8_t x)
{
	return (x - 1) / 127.0f - 1;
}

static float charsToFloat2(uint8_t x, uint8_t y)
{
	return (x + 256 * y) / 32767.0f - 1;
}

void xbg3File::SGfxBuffers::registerMembers(MemberStructure& ms) {
	std::vector<xbg3File::VertexType> newData;
	int num = 0;
	int start = 0;
	int stride = 40;
	float tempCount = vertexData.size() / stride; // replacing count for now
	//offset.y = 3.051851e-05; // temp for now
	for (int i = 0; i < tempCount * stride; i += stride)
	{
		int k = start + i;
		// 4 position followed by 2 uv
		std::vector<float> posTemp, posTemp2, posTemp3, posTemp4, posTemp5;
		std::vector<float> blendTemp, blendTemp2;
		for (int j = 0; j < 8; j += 2)
		{
			float n = charsToFloat(vertexData[k], vertexData[k + 1]);
			posTemp.push_back(n);
			k += 2;
		}
		glm::vec4 modelPos(posTemp[0], posTemp[1], posTemp[2], posTemp[3]);
		//modelPos *= offset.y;
		//modelPos += offset.x;
		//modelPos.w = 1.0f;
		/*newData.push_back(modelPos.x);
		newData.push_back(modelPos.y);
		newData.push_back(modelPos.z);
		newData.push_back(modelPos.w);*/
		for (int j = 8; j < 12; j += 2)
		{
			float n = charsToFloat(vertexData[k], vertexData[k + 1]);
			posTemp2.push_back(n);
			k += 2;
		}
		glm::vec2 uvPos(posTemp2[0], posTemp2[1]);
		//uvPos *= offset.w;
		//uvPos += offset.z;
		if (stride <= 12)
		{
			SDL_assert_release(stride == 12);
			continue;
		}
		k += 4; // skip idk what
		for (int n = 0; n < 4; n++)
		{
			float blend = vertexData[k] / 255.0f;
			blendTemp.push_back(blend);
			k++;
		}
		for (int n = 0; n < 4; n++)
		{
			float blend2 = vertexData[k];
			blendTemp2.push_back(blend2);
			k++;
		}
		//k = normalOffset + start + i;
		//SDL_assert_release(k == normalOffset + start + i);
		for (int n = 0; n < 4; n++)
		{
			float normal = charToFloat(vertexData[k]);
			posTemp3.push_back(normal);
			k++;
		}
		k += 4; // skip color for now
		if (stride <= 20)
		{
			continue;
		}
		for (int n = 0; n < 4; n++)
		{
			float tangent = charToFloat(vertexData[k]);
			posTemp4.push_back(tangent);
			k++;
		}
		if (stride <= 24)
		{
			continue;
		}
		for (int n = 0; n < 4; n++)
		{
			float binormal = charToFloat(vertexData[k]);
			posTemp5.push_back(binormal);
			k++;
		}
		glm::vec3 normal(posTemp3[0], posTemp3[1], posTemp3[2]);
		glm::vec3 tangent(posTemp4[0], posTemp4[1], posTemp4[2]);
		glm::vec3 binormal(posTemp5[0], posTemp5[1], posTemp5[2]);
		glm::vec4 blendWeights(blendTemp[0], blendTemp[1], blendTemp[2], blendTemp[3]);
		glm::vec4 blendIndices(blendTemp2[0], blendTemp2[1], blendTemp2[2], blendTemp2[3]);

		xbg3File::VertexType vt;
		vt.pos = modelPos;
		vt.uv = uvPos;
		vt.blendWeights = blendWeights;
		vt.blendIndices = blendIndices;
		vt.normal = normal;
		vt.tangent = tangent;
		vt.binormal = binormal;
		newData.push_back(vt);
		num++;
	}

	std::vector<glm::vec4> blends;
	int i = 0;
	for (auto vs : newData)
	{
		//blends.push_back(vs.pos);
		ms.registerMember(("pos" + std::to_string(i)).c_str(), vs.pos);
		i++;
	}
	//REGISTER_MEMBER(blends);
}

void xbg3File::SGfxBuffers::extractNormals()
{
	Vector<float> vertices;
	Vector<float> normals;

	for (int i = 0; i < 72; i += 24)
	{
		int k = i;
		for (int j = 0; j < 12; j += 2)
		{
			float n = charsToFloat2(vertexData[k], vertexData[k + 1]);
			vertices.push_back(n);
			k += 2;
		}
		for (int n = 0; n < 3; n++)
		{
			//normals.push_back(charToFloat(vertexData[k]));
			k++;
		}
		// padding? tangents???
	}
	// TODO: Normals start at vertexStride - 16
	for (int i = 0; i < vertexData.size(); i++)
	{
		float n = charToFloat(vertexData[i]);
		normals.push_back(n);
	}
	// TODO: Put normals into the vertex buffer
	// calculate tangent and binormal and store them here as well
}

/* The order goes:
Position
UVs
BlendWeight
BlendIndices
Normal
Color
Tangent
Binormal
*/

std::shared_ptr<VertexBuffer> xbg3File::createVertexBuffer(std::vector<uint8_t> vertexData, int start, int count, int stride, glm::vec4 offset) {
	auto vertex = std::make_shared<VertexBuffer>();
	std::vector<xbg3File::VertexType> newData;

	// aidenhead: tangents start ~12 after head (stride - 4?)
	// TODO verify with more examples
	UINT normalOffset = stride - 16;
	if (normalOffset < 12)
	{
		// TODO: What about stride = 28?
		normalOffset = 12;
	}
	int num = 0;
	float tempCount = vertexData.size() / stride - start; // replacing count for now
	offset.y = 3.051851e-05; // temp for now
	offset.w = 0.000656147953; // temp
	offset.z = -16.5;
	for (int i = 0; i < tempCount * stride; i += stride)
	{
		int k = start * stride + i;
		// 4 position followed by 2 uv

		std::vector<float> posTemp, posTemp2, posTemp3, posTemp4, posTemp5;
		std::vector<float> blendTemp, blendTemp2;
		for (int j = 0; j < 8; j += 2)
		{
			float n = charsToFloat(vertexData[k], vertexData[k + 1]);
			posTemp.push_back(n);
			k += 2;
		}
		glm::vec4 modelPos(posTemp[0], posTemp[1], posTemp[2], posTemp[3]);
		modelPos *= offset.y;
		modelPos += offset.x;
		modelPos.w = 1.0f;

		for (int j = 8; j < 12; j += 2)
		{
			float n = charsToFloat(vertexData[k], vertexData[k + 1]);
			posTemp2.push_back(n);
			k += 2;
		}
		glm::vec2 uvPos(posTemp2[0], posTemp2[1]);
		uvPos *= offset.w;
		uvPos += offset.z;
		if (stride <= 12)
		{
			SDL_assert_release(stride == 12);
			continue;
		}
		k += 4; // skip idk what
		for (int n = 0; n < 4; n++)
		{
			float blend = vertexData[k] / 255.0f;
			blendTemp.push_back(blend);
			k++;
		}
		for (int n = 0; n < 4; n++)
		{
			float blend2 = vertexData[k];
			blendTemp2.push_back(blend2);
			k++;
		}
		//k = normalOffset + start + i;
		//SDL_assert_release(k == normalOffset + start + i);
		for (int n = 0; n < 4; n++)
		{
			float normal = charToFloat(vertexData[k]);
			posTemp3.push_back(normal);
			k++;
		}
		k += 4; // skip color for now
		if (stride <= 20)
		{
			continue;
		}
		for (int n = 0; n < 4; n++)
		{
			float tangent = charToFloat(vertexData[k]);
			posTemp4.push_back(tangent);
			k++;
		}
		if (stride <= 24)
		{
			continue;
		}
		for (int n = 0; n < 4; n++)
		{
			float binormal = charToFloat(vertexData[k]);
			posTemp5.push_back(binormal);
			k++;
		}
		glm::vec3 normal(posTemp3[0], posTemp3[1], posTemp3[2]);
		glm::vec3 tangent(posTemp4[0], posTemp4[1], posTemp4[2]);
		glm::vec3 binormal(posTemp5[0], posTemp5[1], posTemp5[2]);
		glm::vec4 blendWeights(blendTemp[0], blendTemp[1], blendTemp[2], blendTemp[3]);
		glm::vec4 blendIndices(blendTemp2[0], blendTemp2[1], blendTemp2[2], blendTemp2[3]);

		xbg3File::VertexType vt;
		vt.pos = modelPos;
		vt.uv = uvPos;
		vt.blendWeights = blendWeights;
		vt.blendIndices = blendIndices;
		vt.normal = normal;
		vt.tangent = tangent;
		vt.binormal = binormal;
		newData.push_back(vt);
		num++;
	}

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = newData.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	SDL_assert_release(newData.size() == tempCount);
	// This is the total number of bytes, NOT newData.size()!
	CD3D11_BUFFER_DESC vertexBufferDesc(tempCount * sizeof(VertexType), D3D11_BIND_VERTEX_BUFFER);
	RenderInterface::instance().g_pd3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertex->pVertexBuffer);

	return vertex;
}

// unk1 is always 0? Tried changing unk1 and unk3 for Aiden but nothing in game
// unk4 and unk5 are definitely UV offsets, unk2 is position multiplier
// scaled float uv = uvPos * unk5 + unk4 (e.g. unk5 = 1 / 32766, unk4 = 0)
// scaled float4 pos = input.Pos * unk2 + unk1 or just try * unk2

void xbg3File::draw(ID3D11DeviceContext* context, const std::vector<glm::mat4>& mats, int lodNum) {
	if (lodNum >= lods.size())
		return;

	//auto& lod = lods[lodNum];

	srvs.resize(clusters.size());
	int srvI = 0;
	int startIndex = 0;
	for (auto& chunk : clusters) {
	//for (auto& mesh : lod.meshes) {
		// TODO: extract PCMP and UCMP sections
		auto offset = glm::vec4(geomParams.unk1, geomParams.unk2, geomParams.unk4, geomParams.unk5);
		//RenderInterface::instance().objectCB.Offset = offset;
		// example for WD1: unk1 = 0, unk2 = 5.61117340e-05, unk4 = 0, unk5 = 3.05194408e-05 = 1 / 32766
		context->UpdateSubresource(RenderInterface::instance().objectCBB, 0, NULL, &RenderInterface::instance().objectCB, 0, 0);

		D3D_PRIMITIVE_TOPOLOGY pType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		// v is vertexCount, p is primitiveCount
		// Primitive type list (Device3D::EPrimitiveType)
		// type = 0: v = 3 * p (triangle list)
		// type = 1: v = p + 2 (triangle strip)
		// type = 3: v = 2 * p (line list)
		// type = 4: v = p + 1 (line strip)
		// type = 7: v = p (point list)
		/*
		switch (mesh.primitiveType) {
		case 0:
			pType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case 7:
			pType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			break;
		default:
			SDL_assert_release(false && "Unhandled Primitive Type");
		}*/
		context->IASetPrimitiveTopology(pType);

		// Assume bones are in order
		// root bone has identity blend matrix
		// TODO consider changing offset to blend matrix

		RenderInterface::instance().offsetsCB.offsets[0] = glm::mat4(1.0f);
		
		for (int i = 0; i < skelResources.offset2BoneMats.size(); i++)
		{
			RenderInterface::instance().offsetsCB.offsets[i + 1] =
				skelResources.offset2BoneMats[i];
		}

		uint32_t boneIndices[48];

		for (int i = 0; i < 48; i++)
		{
			boneIndices[i] = chunk.boneIndices[i];
		}

		for (int i = 0; i < 12; i++)
		{
			RenderInterface::instance().offsetsCB.boneIndices[i] =
				glm::ivec4(boneIndices[4 * i], boneIndices[4 * i + 1],
					boneIndices[4 * i + 2], boneIndices[4 * i + 3]);
		}

		context->UpdateSubresource(RenderInterface::instance().offsetsCBB, 0, NULL, &RenderInterface::instance().offsetsCB, 0, 0);
		
		auto materialFile = materialResources.materials[chunk.matID].file;
		auto fp = FH::openFile(materialFile.c_str());
		CBinaryArchiveReader reader(fp);
		auto material = std::make_shared<material3File>();
		material->open(reader);

		auto diffuseFile = material->getCommandPath("DiffuseTexture1");
		auto diffuse = loadTexture3(diffuseFile.c_str());
		auto diffuseFile2 = material->getCommandPath("DiffuseTexture2");
		auto diffuse2 = loadTexture3(diffuseFile2.c_str());
		auto normalFile = material->getCommandPath("NormalTexture1");
		auto normal = loadTexture3(normalFile.c_str());
		auto specularFile = material->getCommandPath("SpecularTexture1");
		auto specular = loadTexture3(specularFile.c_str());
		auto maskFile = material->getCommandPath("MaskTexture1");
		auto mask = loadTexture3(maskFile.c_str());

		/*
		long x = diffuseFile.rfind("_m.xbt");
		if (x > 0)
		{
			srvs[srvI] = mask->pResource;
		}
		else
			srvs[srvI] = diffuse->pResource;

		if (settings.drawDetails)
		{
			ID3D11ShaderResourceView* views[] = {
			srvs[srvI],
			normal->pResource,
			specular->pResource,
			diffuse2->pResource,
			};
			context->PSSetShaderResources(0, 4, views);
			RenderInterface::instance().lightCB.detail = 1.0f;
		}
		else
		{
			ID3D11ShaderResourceView* views[] = {
			srvs[srvI],
			normal->pResource,
			specular->pResource,
			};
			context->PSSetShaderResources(0, 3, views);
			RenderInterface::instance().lightCB.detail = 0.0f;
		}
		*/
		context->PSSetShaderResources(0, 1, &diffuse->pResource);
		auto specPowerCmd = material->findCommand("SpecularPower");
		// Pass the specular power float value to the shader
		if (specPowerCmd != NULL)
		{
			SDL_assert_release(specPowerCmd->type == 4); // float4
			glm::vec4 specPower = specPowerCmd->unks4;
			RenderInterface::instance().lightCB.specularPower = specPower.x;
		}
		context->UpdateSubresource(RenderInterface::instance().lightCBB, 0, NULL, &RenderInterface::instance().lightCB, 0, 0);
		
			/*
			* 		//1: float
		float unks1;

		//2: float2,
		glm::vec2 unks2;

		//3: float3, color3
		glm::vec3 unks3;

		//4: float4, color4
		glm::vec4 unks4;

		//5: int
		int32_t unks5;

		//6: bool
		bool unks6;

		//7: samplerState
		CStringID unks7;

		//For 8-10
		//8: sampler2D
		std::string path;
*/

		//UINT start = mesh.drawCall.vertexBufferByteOffset;
		UINT start = 0;
		UINT stride = chunk.vertexStride;
		UINT count = 0;
		//UINT count = mesh.drawCall.vertexCount;
		if (chunk.vertex == NULL)
		{
			chunk.vertex = createVertexBuffer(buffers[lodNum].vertexData, start, count, stride, offset);
		}
		UINT stride2 = sizeof(VertexType);
		UINT offset2 = 0;

		context->IASetVertexBuffers(0, 1, &chunk.vertex->pVertexBuffer, &stride2, &offset2);
		//context->IASetVertexBuffers(0, 1, &buffers[lodNum].vertex->pVertexBuffer, &stride, &start);
		context->IASetIndexBuffer(buffers[lodNum].index->pIndexBuffer, DXGI_FORMAT_R16_UINT, startIndex * 2);
		//context->IASetIndexBuffer(buffers[lodNum].index->pIndexBuffer, DXGI_FORMAT_R16_UINT, mesh.drawCall.indexBufferStartIndex * 2);
		SDL_assert_release(buffers[lodNum].index->size % sizeof(short) == 0);

		static std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3D11InputLayout> > layouts;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>& layout = layouts[chunk.unk1];
		if (!layout.Get()) {
			const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			HRESULT ret = RenderInterface::instance().g_pd3dDevice->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				RenderInterface::instance().model3.vShaderBlob->GetBufferPointer(),
				RenderInterface::instance().model3.vShaderBlob->GetBufferSize(),
				&layout);
			SDL_assert_release(ret == S_OK);
		}
		context->IASetInputLayout(layout.Get());
		// Draw the triangles
		context->UpdateSubresource(RenderInterface::instance().objectCBB, 0, NULL, &RenderInterface::instance().objectCB, 0, 0);
		//context->DrawIndexed(buffers[lodNum].indexData.size(), 0, 0);
		context->DrawIndexed(chunk.indexCount, 0, 0);
		++srvI;
		startIndex += chunk.indexCount;
	}
}
