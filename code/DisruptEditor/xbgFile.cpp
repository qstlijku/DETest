/*

Copyright 2019 Jonathan Scott
All rights reserved
You may not use this file without permission

*/

#include "xbgFile.h"
#include "idle_frame0_rots.h"
#include "rest_pose_rots.h"
#include "wdl_idle_pose.h"
#include <iostream>
#include "Vector.h"
#include <stdlib.h>
#include "ResourceLoader.h"
#include "materialFile.h"
#include "xbtFile.h"
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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <Common.h>
#include <glm/gtc/quaternion.hpp>
#include <World.h>
#include <SDL_filesystem.h>

static void serializeMat4(IBinaryArchive& fp, glm::mat4 &vec) {
	fp.pad(16);
	fp.serialize(vec);
}

static void serializeMat4Vec(IBinaryArchive& fp, Vector<glm::mat4>& vec) {
	uint32_t count = vec.size();
	fp.serialize(count);
	vec.resize(count);
	for (uint32_t i = 0; i < count; ++i)
		serializeMat4(fp, vec[i]);
}

void xbgFile::open(IBinaryArchive &fp) {
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
	geomParams.read(fp);

	SDL_Log("MaterialResources: %u", fp.tell());
	materialResources.read(fp, geomParams.lods.size());

	SDL_Log("MaterialSlotToIndex: %u", fp.tell());
	materialSlotToIndex.read(fp);

	SDL_Log("SkinNames: %u", fp.tell());
	skinNames.read(fp);

	SDL_Log("BonePalettes: %u", fp.tell());
	bonePalettes.read(fp);

	SDL_Log("SkelResources: %u", fp.tell());
	skelResources.read(fp);

	SDL_Log("ReflexSystem: %u", fp.tell());
	reflexSystem.read(fp);

	SDL_Log("SecondaryMotionObjects: %u", fp.tell());
	secondaryMotionObjects.read(fp);

	SDL_Log("ProceduralNodes: %u", fp.tell());
	proceduralNodes.read(fp);

	//<unnamed>::ReadLOD(unsigned long, const unsigned char *&, CSceneGeometryLOD &, CPreAllocator &, ndVectorExternal<CSkelResource, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>> &, ndVectorExternal<CBonePalette, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>> &, bool);
	SDL_Log("ReadLOD: %u", fp.tell());
	lods.resize(geomParams.lods.size());
	for (uint32_t i = 0; i < geomParams.lods.size(); ++i)
		lods[i].read(fp);

	fp.serialize(unk3);
	SDL_Log("SGfxBuffers: %u", fp.tell());
	fp.serializeNdVector(buffers);

	SDL_Log("GeometryMips: %u", fp.tell());
	fp.serializeNdVector(mips);

	fp.serialize(clothWrinkleControlPatchBundles);
	SDL_assert_release(fp.tell() == fp.size());
}

void xbgFile::Header::read(IBinaryArchive & fp) {
	magic = 0x47454F4D;
	fp.serialize(magic);
	SDL_assert_release(magic == 0x47454F4D);

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

void xbgFile::Header::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(magic);
	REGISTER_MEMBER(majorVersion);
	REGISTER_MEMBER(minorVersion);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
}

void xbgFile::SMemoryNeed::read(IBinaryArchive & fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
}

void xbgFile::SMemoryNeed::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
}

void xbgFile::SceneGeometryParams::read(IBinaryArchive &fp) {
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

void xbgFile::SceneGeometryParams::registerMembers(MemberStructure & ms) {
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

void xbgFile::MaterialResources::read(IBinaryArchive & fp, uint32_t lods) {
	fp.serialize(unk0);
	uint32_t s = lods - unk0;
	unk1.resize(s);
	for (uint32_t i = 0; i < s; ++i)
		fp.serialize(unk1[i]);
	fp.serializeNdVector(materials);
}

void xbgFile::MaterialResources::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk0);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(materials);
}

void xbgFile::MaterialResources::MaterialFile::read(IBinaryArchive & fp) {
	CPathID hashed(file);
	fp.serialize(hashed);
	fp.serialize(file);
}

void xbgFile::MaterialResources::MaterialFile::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, file);
}

void xbgFile::MaterialSlotToIndex::read(IBinaryArchive & fp) {
	fp.serializeNdVector(slots);
}

void xbgFile::MaterialSlotToIndex::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, slots);
}

void xbgFile::MaterialSlotToIndex::Slot::read(IBinaryArchive & fp) {
	name.read(fp);
	fp.serialize(slot);
}

void xbgFile::MaterialSlotToIndex::Slot::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(slot);
}

void xbgFile::SkinNames::read(IBinaryArchive & fp) {
	fp.serializeNdVector(skins);
}

void xbgFile::SkinNames::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, skins);
}

void xbgFile::BonePalettes::read(IBinaryArchive & fp) {
	fp.serializeNdVector(pallets);
}

void xbgFile::BonePalettes::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, pallets);
}

void xbgFile::BonePalettes::BonesPallet::read(IBinaryArchive & fp) {
	fp.serializeNdVector(unk1);
}

void xbgFile::BonePalettes::BonesPallet::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, unk1);
}

void xbgFile::SkelResources::read(IBinaryArchive & fp) {
	//Can either read 1 or 0
	fp.serialize(unk1);
	if (unk1) {
		fp.serializeNdVector(resources);

		fp.serialize(unk2);

		serializeMat4Vec(fp, mats);

		SDL_assert_release(resources.size() == mats.size());
	}

	std::string base = SDL_GetBasePath();

	FILE* fb = fopen((base + "res/bones.txt").c_str(), "w");

	for (auto skel : resources)
	{
		fputs((skel.name.name + "\n").c_str(), fb);
	}
	fclose(fb);
}

void xbgFile::SkelResources::registerMembers(MemberStructure & ms) {
	int i = 0;
	for (auto& skelRes : resources)
	{
		auto name = skelRes.name;
		uint16_t parent = skelRes.node.parentIndex;
		glm::quat rotQuat(skelRes.node.rot.w, skelRes.node.rot.x, skelRes.node.rot.y, skelRes.node.rot.z);
		glm::vec3 rotEuler = glm::degrees(glm::eulerAngles(rotQuat));
		REGISTER_MEMBER(rotQuat);
		REGISTER_MEMBER(rotEuler);
		//ms.registerMember(("name" + std::to_string(i)).c_str(), name);
		REGISTER_MEMBER(parent);
		i++;
	}
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(resources);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(mats);
}

void xbgFile::SkelResources::SRawNode::read(IBinaryArchive & fp) {
	fp.serialize(boneLOD);
	fp.serialize(unused[0]);
	fp.serialize(unused[1]);
	fp.serialize(unused[2]);
	fp.serialize(pos);
	fp.serialize(rot);
	fp.serialize(parentIndex);
	fp.serialize(obj2NodeMatInd);
}

void xbgFile::SkelResources::SRawNode::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(boneLOD);
	//REGISTER_MEMBER(unused);
	REGISTER_MEMBER(pos);
	REGISTER_MEMBER(rot);
	REGISTER_MEMBER(parentIndex);
	REGISTER_MEMBER(obj2NodeMatInd);
}

void xbgFile::SkelResources::SkelResource::read(IBinaryArchive & fp) {
	node.read(fp);
	name.read(fp);
}

void xbgFile::SkelResources::SkelResource::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, node);
	REGISTER_MEMBER(name);
}

void xbgFile::ReflexSystem::read(IBinaryArchive &fp) {
	fp.serialize(has);
	if (has) {
		if (fp.isReading()) {
			uint32_t size;
			fp.serialize(size);
			size_t offset = fp.tell();
			readFCB(fp, root);
			fp.pad(4);
			SDL_assert_release(offset + size == fp.tell());
		}
		else {
			Vector<uint8_t> data(4 * 1000 * 1000);
			SDL_RWops* temp = SDL_RWFromMem(data.data(), data.size());
			writeFCBA(temp, root);

			uint32_t size = SDL_RWtell(temp);
			fp.serialize(size);
			fp.memBlock(data.data(), 1, size);
			fp.pad(4);
		}
	}
}

void xbgFile::ReflexSystem::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(has);
	if (has) {
		if (ms.type == ms.TOXML) {
			root.serializeXML(*ms.printer);
		} else if (ms.type == ms.FROMXML) {
			root.deserializeXML(ms.it->LastChildElement());
		}
	}
}

void xbgFile::SecondaryMotionObjects::read(IBinaryArchive & fp) {
	fp.serializeNdVector(smos);
}

void xbgFile::SecondaryMotionObjects::registerMembers(MemberStructure& ms) {
	ms.registerMember(NULL, smos);
}

void xbgFile::SecondaryMotionObjects::SMO::read(IBinaryArchive & fp) {
	SDL_Log("simulationParams: %u", fp.tell());
	simulationParams.read(fp);
	SDL_Log("secondaryMotionUnitCollisionPrimitives: %u", fp.tell());
	secondaryMotionUnitCollisionPrimitives.read(fp);
	SDL_Log("secondaryMotionUnitLimits: %u", fp.tell());
	secondaryMotionUnitLimits.read(fp);
	SDL_Log("secondaryMotionUnitParticles: %u", fp.tell());
	secondaryMotionUnitParticles.read(fp);
	SDL_Log("secondaryMotionUnitTriangles: %u", fp.tell());
	secondaryMotionUnitTriangles.read(fp);
	SDL_Log("secondaryMotionUnitConnectivities: %u", fp.tell());
	secondaryMotionUnitConnectivities.read(fp);
	SDL_Log("secondaryMotionUnitSpringDescs: %u", fp.tell());
	secondaryMotionUnitSpringDescs.read(fp);
	fp.serialize(unk1);
	fp.serialize(unk2);
}

void xbgFile::SecondaryMotionObjects::SMO::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(simulationParams);
	REGISTER_MEMBER(secondaryMotionUnitCollisionPrimitives);
	REGISTER_MEMBER(secondaryMotionUnitLimits);
	REGISTER_MEMBER(secondaryMotionUnitParticles);
	REGISTER_MEMBER(secondaryMotionUnitTriangles);
	REGISTER_MEMBER(secondaryMotionUnitConnectivities);
	REGISTER_MEMBER(secondaryMotionUnitSpringDescs);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
}

void xbgFile::SecondaryMotionObjects::SMO::SSMSimulationParametersDesc::read(IBinaryArchive & fp) {
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
	fp.serialize(type);
	fp.serialize(unk13);
}

void xbgFile::SecondaryMotionObjects::SMO::SSMSimulationParametersDesc::registerMembers(MemberStructure& ms) {
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
	REGISTER_MEMBER(type);
	REGISTER_MEMBER(unk13);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitCollisionPrimitives::read(IBinaryArchive & fp) {
	SDL_Log("Spheres: %u", fp.tell());
	fp.serializeNdVector(spheres);
	SDL_Log("Cylinders: %u", fp.tell());
	fp.serializeNdVector(cylinders);
	SDL_Log("Capsules: %u", fp.tell());
	fp.serializeNdVector(capsules);
	SDL_Log("Planes: %u", fp.tell());
	fp.serializeNdVector(planes);

	SDL_Log("%u", fp.tell());
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitCollisionPrimitives::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(spheres);
	REGISTER_MEMBER(cylinders);
	REGISTER_MEMBER(capsules);
	REGISTER_MEMBER(planes);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitCollisionPrimitives::SSphereDesc::read(IBinaryArchive & fp) {
	name.read(fp);
	serializeMat4(fp, unk1);
	fp.serialize(fRadius);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitCollisionPrimitives::SSphereDesc::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(fRadius);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitCollisionPrimitives::SCylinderDesc::read(IBinaryArchive & fp) {
	name.read(fp);
	serializeMat4(fp, unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitCollisionPrimitives::SCylinderDesc::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitCollisionPrimitives::SCapsuleDesc::read(IBinaryArchive & fp) {
	SDL_Log("Capsule: %u", fp.tell());
	name.read(fp);
	serializeMat4(fp, unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitCollisionPrimitives::SCapsuleDesc::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitCollisionPrimitives::SInfinitePlaneDesc::read(IBinaryArchive & fp) {
	name.read(fp);
	serializeMat4(fp, unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitCollisionPrimitives::SInfinitePlaneDesc::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitLimits::read(IBinaryArchive & fp) {
	SDL_Log("%u", fp.tell());
	fp.serializeNdVector(spheres);
	SDL_Log("%u", fp.tell());
	fp.serializeNdVector(boxes);
	SDL_Log("%u", fp.tell());
	fp.serializeNdVector(cylinders);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitLimits::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(spheres);
	REGISTER_MEMBER(boxes);
	REGISTER_MEMBER(cylinders);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitLimits::SSphereLimitDesc::read(IBinaryArchive & fp) {
	name.read(fp);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitLimits::SSphereLimitDesc::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitLimits::SBoxLimitDesc::read(IBinaryArchive & fp) {
	name.read(fp);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitLimits::SBoxLimitDesc::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitLimits::SCylinderLimitDesc::read(IBinaryArchive & fp) {
	name.read(fp);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitLimits::SCylinderLimitDesc::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitParticles::read(IBinaryArchive & fp) {
	fp.serializeNdVector(particles);
	fp.serializeNdVector(meshes);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitParticles::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(particles);
	REGISTER_MEMBER(meshes);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitParticles::SSMParticleDesc::read(IBinaryArchive & fp) {
	name.read(fp);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitParticles::SSMParticleDesc::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitTriangles::read(IBinaryArchive & fp) {
	fp.serializeNdVector(triangles);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitTriangles::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(triangles);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitTriangles::Triangle::read(IBinaryArchive & fp) {
	fp.serialize(p1);
	fp.serialize(p2);
	fp.serialize(p3);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitTriangles::Triangle::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(p1);
	REGISTER_MEMBER(p2);
	REGISTER_MEMBER(p3);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitConnectivities::read(IBinaryArchive & fp) {
	//TODO:!
	fp.serializeNdVector(connectivity);
	SDL_assert_release(connectivity.empty());
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitConnectivities::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(connectivity);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitConnectivities::SSMParticleConnectivity::read(IBinaryArchive & fp) {
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitConnectivities::SSMParticleConnectivity::registerMembers(MemberStructure& ms) {
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitSpringDescs::read(IBinaryArchive & fp) {
	fp.serializeNdVector(springs);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitSpringDescs::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(springs);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitSpringDescs::Spring::read(IBinaryArchive & fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
}

void xbgFile::SecondaryMotionObjects::SMO::SecondaryMotionUnitSpringDescs::Spring::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
}

void xbgFile::CMeshNameID::read(IBinaryArchive & fp) {
	CStringID hashed(name);
	fp.serialize(hashed);
	fp.serialize(name);
}

void xbgFile::CMeshNameID::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, name);
}

void xbgFile::ProceduralNodes::read(IBinaryArchive & fp) {
	fp.serializeNdVector(nodes);
}

void xbgFile::ProceduralNodes::registerMembers(MemberStructure& ms) {
	ms.registerMember(NULL, nodes);
}

void xbgFile::ProceduralNodes::SProceduralNode::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(type);
	switch (type) {
	case 1:
		fp.serialize(t1_unk1);
		fp.serialize(t1_unk2);
		fp.serialize(t1_unk3);
		fp.serialize(t1_unk4);
		break;
	case 2:
		fp.serialize(t2_unk1);
		fp.serialize(t2_unk2);
		break;
	case 3:
		fp.serialize(t3_unk1);
		fp.serialize(t3_unk2);
		fp.serialize(t3_unk3);
		break;
	case 5:
		fp.serialize(t5_unk1);
		fp.serialize(t5_unk2);
		fp.serialize(t5_unk3);
		fp.serialize(t5_unk4);
		fp.serialize(t5_unk5);
		fp.serialize(t5_unk6);
		fp.serialize(t5_unk7);
		fp.serialize(t5_unk8);
		fp.serialize(t5_unk9);
		break;
	case 6:
		fp.serialize(t6_unk1);
		fp.serialize(t6_unk2);
		fp.serialize(t6_unk3);
		fp.serialize(t6_unk4);
		fp.serialize(t6_unk5);
		break;
	}
}

void xbgFile::ProceduralNodes::SProceduralNode::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(type);
	switch (type) {
	case 1:
		REGISTER_MEMBER(t1_unk1);
		REGISTER_MEMBER(t1_unk2);
		REGISTER_MEMBER(t1_unk3);
		REGISTER_MEMBER(t1_unk4);
		break;
	case 2:
		REGISTER_MEMBER(t2_unk1);
		REGISTER_MEMBER(t2_unk2);
		break;
	case 3:
		REGISTER_MEMBER(t3_unk1);
		REGISTER_MEMBER(t3_unk2);
		REGISTER_MEMBER(t3_unk3);
		break;
	case 5:
		REGISTER_MEMBER(t5_unk1);
		REGISTER_MEMBER(t5_unk2);
		REGISTER_MEMBER(t5_unk3);
		REGISTER_MEMBER(t5_unk4);
		REGISTER_MEMBER(t5_unk5);
		REGISTER_MEMBER(t5_unk6);
		REGISTER_MEMBER(t5_unk7);
		REGISTER_MEMBER(t5_unk8);
		REGISTER_MEMBER(t5_unk9);
		break;
	case 6:
		REGISTER_MEMBER(t6_unk1);
		REGISTER_MEMBER(t6_unk2);
		REGISTER_MEMBER(t6_unk3);
		REGISTER_MEMBER(t6_unk4);
		REGISTER_MEMBER(t6_unk5);
		break;
	}
}

void xbgFile::LOD::read(IBinaryArchive & fp) {
	fp.serializeNdVector(meshes);
}

void xbgFile::LOD::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, meshes);
}

void xbgFile::LOD::CSceneMesh::read(IBinaryArchive & fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(primitiveType);
	fp.serialize(matID);
	fp.serialize(vertexFormat);
	fp.serialize(vertexStride);
	fp.serialize(unk9);
	fp.serialize(unk10);
	fp.serialize(boneMapID);
	drawCall.read(fp);

	uint32_t count = drawCalls.size();
	fp.serialize(count);
	drawCalls.resize(count);

	fp.serialize(unk12);
	fp.serialize(unk13);

	for (uint32_t i = 0; i < count; ++i)
		drawCalls[i].read(fp);
}

void xbgFile::LOD::CSceneMesh::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
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

void xbgFile::CBasicDrawCallRange::read(IBinaryArchive & fp) {
	fp.serialize(vertexBufferByteOffset);
	fp.serialize(primitiveCount);
	fp.serialize(indexCount);
	fp.serialize(indexBufferStartIndex);
	fp.serialize(vertexCount);
	fp.serialize(minIndexValue);
	fp.serialize(maxIndexValue);
	fp.serialize(groupCount);
}

void xbgFile::CBasicDrawCallRange::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(vertexBufferByteOffset);
	REGISTER_MEMBER(primitiveCount);
	REGISTER_MEMBER(indexCount);
	REGISTER_MEMBER(indexBufferStartIndex);
	REGISTER_MEMBER(vertexCount);
	REGISTER_MEMBER(minIndexValue);
	REGISTER_MEMBER(maxIndexValue);
	REGISTER_MEMBER(groupCount);
}

void xbgFile::LOD::CSceneMesh::CDrawCallRange::read(IBinaryArchive& fp) {
	SDL_Log("CDrawCallRange: %u", fp.tell());
	drawCall.read(fp);
	sphere.read(fp);
	fp.serialize(unk1);
	fp.serialize(unk2);
	name.read(fp);
	fp.serialize(unk3);
	fp.serialize(unk4);
}

void xbgFile::LOD::CSceneMesh::CDrawCallRange::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(drawCall);
	REGISTER_MEMBER(sphere);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
}

void xbgFile::GeomMips::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	CPathID hashed(path);
	fp.serialize(hashed);
	fp.serialize(path);
}

void xbgFile::GeomMips::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(path);
}

void xbgFile::registerMembers(MemberStructure& ms) {
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
	REGISTER_MEMBER(reflexSystem);
	REGISTER_MEMBER(secondaryMotionObjects);
	REGISTER_MEMBER(proceduralNodes);
	REGISTER_MEMBER(lods);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(buffers);
	REGISTER_MEMBER(mips);
	REGISTER_MEMBER(clothWrinkleControlPatchBundles);
}

// TODO: do we need multiple matrices?
void xbgFile::draw(ID3D11DeviceContext* context, int lodNum) {
	std::vector<glm::mat4> mats(1);
	mats[0] = RenderInterface::instance().objectCB.Model;
	draw(context, mats, lodNum);
}

std::list<std::string> xbgFile::getDiffuseTexture(int lodNum)
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

void xbgFile::SGfxBuffers::read(IBinaryArchive & fp) {
	//<unnamed>::ReadGfxBuffers(const unsigned char *&, SGfxBuffers &, unsigned long, bool)
	
	//CBufferRenderResource::Create(Device3D::EBufferType 0, const IRenderResourceCommandTrackerDecoratorFactory & (addi r4, r27, unk_107D7CDA@l), unsigned long, unsigned long 1, const void *, bool 0, bool 0, unsigned long 0, bool, bool)
	fp.pad(4);
	fp.serializeNdVector(vertexData);

	//extractNormals();
	
	//CBufferRenderResource::Create(Device3D::EBufferType 1, const IRenderResourceCommandTrackerDecoratorFactory &, unsigned long, unsigned long, const void *, bool, bool, unsigned long, bool, bool)
	fp.pad(4);
	fp.serializeNdVector(indexData);

	//Device3D::CBuffer::Create(Device3D::EBufferType, Device3D::EBufferUsage, unsigned long elementSize, unsigned long elementCount, const void * ptr, bool)
}

void xbgFile::SGfxBuffers::createBuffers() {
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
float charsToFloat(uint8_t x, uint8_t y)
{
	int result = x + 256 * y;
	SDL_assert_release(result >= 0);
	if (result < 32768)
		return result;
	return result - 65536;
}

// Technically this shouldn't be x - 1
// but we want it to match zmodeler export
float charToFloat(uint8_t x)
{
	return (x - 1) / 127.0f - 1;
}

float charsToFloat2(uint8_t x, uint8_t y)
{
	return (x + 256 * y) / 32767.0f - 1;
}

void xbgFile::SGfxBuffers::registerMembers(MemberStructure& ms) {
	std::vector<float> vertices;
	std::vector<float> normals;
	for (int i = 0; i < vertexData.size() - 1; i += 2)
	{
		float x = charsToFloat(vertexData[i], vertexData[i + 1]);
		float n = x * 5.6111734e-05;
		vertices.push_back(n);
	}
	for (int i = 0; i < 60; i++)
	{
		float n = charToFloat(vertexData[i]);
		normals.push_back(n);
	}
	REGISTER_MEMBER(normals);
	REGISTER_MEMBER(vertices);
	REGISTER_MEMBER(vertexData);
	REGISTER_MEMBER(indexData);
	/*
	if (ms.type == ms.TOXML) {
		std::string v = toBase64String(vertexData.data(), vertexData.size());
		ms.registerMember("vertex", v);

		v = toBase64String(indexData.data(), indexData.size());
		ms.registerMember("index", v);
	} else if(ms.type == ms.FROMXML) {
		std::string v;
		ms.registerMember("vertex", v);
		vertexData = fromBase64String(v);

		ms.registerMember("index", v);
		indexData = fromBase64String(v);
	}*/
}

void xbgFile::SGfxBuffers::extractNormals()
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

/*
	MeshFVF_Point = 0x1,
	MeshFVF_PointComp = 0x2,
	MeshFVF_UV = 0x4,
	MeshFVF_UVComp = 0x8,
	MeshFVF_UVComp1 = 0x8,
	MeshFVF_Skin = 0x10,
	MeshFVF_SkinExtra = 0x20,
	MeshFVF_SkinRigid = 0x40,
	MeshFVF_NormalComp = 0x80,
	MeshFVF_Color = 0x100,
	MeshFVF_TangentComp = 0x200,
	MeshFVF_BinormalComp = 0x400,
	MeshFVF_PackedFirstUV = 0x800,
	MeshFVF_UVComp2 = 0x1000,
	MeshFVF_UVComp3 = 0x2000,
	MeshFVF_Normal = 0x4000,
	MeshFVF_NormalModifiedComp = 0x8000,
*/

std::shared_ptr<VertexBuffer> xbgFile::createVertexBuffer(std::vector<uint8_t> vertexData, int start, int count, int stride, int format, glm::vec4 offset) {
	auto vertex = std::make_shared<VertexBuffer>();
	std::vector<xbgFile::VertexType> newData;

	// aidenhead: tangents start ~12 after head (stride - 4?)
	// TODO verify with more examples
	UINT normalOffset = stride - 16;
	if (normalOffset < 12)
	{
		// TODO: What about stride = 28?
		normalOffset = 12;
	}
	int num = 0;

	auto point = format & 0x1;
	auto pointComp = format & 0x2;
	auto uv = format & 0x4;
	auto uvComp = format & 0x8;
	auto skin = format & 0x10;
	auto skinExtra = format & 0x20;
	auto skinRigid = format & 0x40;
	auto normalComp = format & 0x80;
	auto color = format & 0x100;
	auto tangentComp = format & 0x200;
	auto binormalComp = format & 0x400;
	auto packedFirstUV = format & 0x800;
	auto uvComp2 = format & 0x1000;
	auto uvComp3 = format & 0x2000;
	auto normal4 = format & 0x4000;
	auto normalModifiedComp = format & 0x8000;

	// Assert expected pos and uv formats
	SDL_assert_release(!point);
	SDL_assert_release(pointComp);
	SDL_assert_release(!uv);
	SDL_assert_release(uvComp);
	SDL_assert_release(skin);
	//SDL_assert_release(!skinExtra);
	SDL_assert_release(!skinRigid);
	SDL_assert_release(normalComp);
	SDL_assert_release(color);
	if (stride > 32)
	{
		SDL_assert_release(tangentComp);
		SDL_assert_release(binormalComp);
		//SDL_assert_release(!uvComp2);
	}
	else
	{
		SDL_assert_release(!tangentComp);
		SDL_assert_release(!binormalComp);
		SDL_assert_release(uvComp2);
	}

	SDL_assert_release(!packedFirstUV);
	SDL_assert_release(!uvComp3);
	SDL_assert_release(!normal4);
	SDL_assert_release(!normalModifiedComp);

	for (int i = 0; i < count * stride; i += stride)
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
		uvPos[0] = std::fmod(uvPos[0], 1.0);
		uvPos[1] = std::fmod(uvPos[1], 1.0);
		if (stride <= 12)
		{
			SDL_assert_release(stride == 12);
			continue;
		}

		if (uvComp2)
			k += 4; // skip second UV for now

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

		if (skinExtra)
		{
			k += 4; // TODO
		}
		// TODO: case for non-skinned models
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

		xbgFile::VertexType vt;
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
	//vertexBufferData.pSysMem = newData;
	vertexBufferData.pSysMem = newData.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	//SDL_assert_release(newData.size() == count * 12);
	// This is the total number of bytes, NOT newData.size()!
	CD3D11_BUFFER_DESC vertexBufferDesc(count * sizeof(VertexType), D3D11_BIND_VERTEX_BUFFER);
	RenderInterface::instance().g_pd3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertex->pVertexBuffer);

	return vertex;
}

xbgFile::SkelResources::SkelResource xbgFile::getSkelResAtIndex(uint16_t index)
{
	return skelResources.resources[index];
}

glm::mat4 xbgFile::getMatAtIndex(uint16_t index)
{
	return glm::inverse(skelResources.mats[index]);
}

glm::mat4 xbgFile::calculateRotationTransform(glm::mat4 transform, char axis, glm::vec3 rot)
{
	if (axis == 'X')
	    return glm::rotate(transform, rot.x, glm::vec3(1, 0, 0));
	if (axis == 'Y')
		return glm::rotate(transform, rot.y, glm::vec3(0, 1, 0));
	if (axis == 'Z')
		return glm::rotate(transform, rot.z, glm::vec3(0, 0, 1));
	return transform;
}

void xbgFile::updatePose(xbgFile::SkelResources::SkelResource &skelRes)
{
	auto name = skelRes.name;
	for (const SkelBone& bone : wdlAnimPose)
	{
		if (name.name == bone.name)
		{
			skelRes.node.rot.x = bone.x;
			skelRes.node.rot.y = bone.y;
			skelRes.node.rot.z = bone.z;
			skelRes.node.rot.w = bone.w;
			skelRes.node.pos.x = bone.px;
			skelRes.node.pos.y = bone.py;
			skelRes.node.pos.z = bone.pz;
			return;
		}
	}
	// Fall back to WD1 rest pose if bone not found in WDL data
	/*
	for (AnimBoneRot rot : restPose)
	{
		if (name.name == rot.name)
		{
			skelRes.node.rot.x = rot.x;
			skelRes.node.rot.y = rot.y;
			skelRes.node.rot.z = rot.z;
			skelRes.node.rot.w = rot.w;
		}
	}*/
}

glm::mat4 xbgFile::calculateLocalMatrix(xbgFile::SkelResources::SkelResource skelRes)
{
	glm::quat rotQuat(skelRes.node.rot.w, skelRes.node.rot.x, skelRes.node.rot.y, skelRes.node.rot.z);
	glm::vec3 rotEuler = glm::eulerAngles(rotQuat);

	glm::vec3 posToTranslate = skelRes.node.pos;
	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate(transform, posToTranslate);

	char x = world.rotationOrder[0];
	char y = world.rotationOrder[1];
	char z = world.rotationOrder[2];

	transform = calculateRotationTransform(transform, x, rotEuler);
	transform = calculateRotationTransform(transform, y, rotEuler);
	transform = calculateRotationTransform(transform, z, rotEuler);
	//transform = glm::rotate(transform, rotEuler.x, glm::vec3(1, 0, 0));
	//transform = glm::rotate(transform, rotEuler.y, glm::vec3(0, 1, 0));
	//transform = glm::rotate(transform, rotEuler.z, glm::vec3(0, 0, 1));

	return transform;

	/*
	auto currBone = skelRes;
	std::vector<glm::mat4> boneMats;
	// while currBone is not the root bone
	while (currBone.node.parentIndex != 0xFFFF)
	{
		auto parent = xbgFile::getSkelResAtIndex(currBone.node.parentIndex);
		boneMats.push_back(xbgFile::getMatAtIndex(currBone.node.obj2NodeMatInd));
		currBone = parent;
	}
	
	for (int i = 0; i < boneMats.size(); i++)
	{
		auto boneMat = boneMats[i];
		worldPos = worldPos * boneMat;
	}
	return worldPos;
	*/
}

// unk1 is always 0? Tried changing unk1 and unk3 for Aiden but nothing in game
// unk4 and unk5 are definitely UV offsets, unk2 is position multiplier
// scaled float uv = uvPos * unk5 + unk4 (e.g. unk5 = 1 / 32766, unk4 = 0)
// scaled float4 pos = input.Pos * unk2 + unk1 or just try * unk2

void xbgFile::draw(ID3D11DeviceContext* context, const std::vector<glm::mat4>& mats, int lodNum) {
	if (lodNum >= lods.size())
		return;

	auto& lod = lods[lodNum];

	srvs.resize(lod.meshes.size());
	int srvI = 0;

	glm::vec3 pelvis = glm::vec3(0, 0.922560, 0);
	glm::vec3 lThigh = glm::vec3(-0.028176, 0.085120, -0.018186);
	glm::vec3 lCalf = glm::vec3(0.395222, 0, 0);
	glm::vec3 lFoot = glm::vec3(0.413850, 0, 0);
	glm::vec3 lToe = glm::vec3(0.127968, 0, 0);

	glm::vec3 rThigh = glm::vec3(-0.028182, -0.085070, -0.018234);
	glm::vec3 rCalf = glm::vec3(0.395222, 0, 0);
	glm::vec3 rFoot = glm::vec3(0.413850, 0, 0);
	glm::vec3 rToe = glm::vec3(0.127968, 0, 0);

	pelvis = glm::rotate(pelvis, glm::radians(90.0f), glm::vec3(0, 0, 1));
	lThigh = glm::rotate(lThigh, glm::radians(175.0f), glm::vec3(0, 0, 1));
	rThigh = glm::rotate(rThigh, glm::radians(-175.0f), glm::vec3(0, 0, 1));
	lCalf = glm::rotate(lCalf, glm::radians(-4.7f), glm::vec3(0, 1, 0));
	rCalf = glm::rotate(rCalf, glm::radians(4.7f), glm::vec3(0, 1, 0));

	//dd::line(&pelvis.x, &lThigh.x, magenta);
	//dd::line(&lThigh.x, &lCalf.x, magenta);
	//dd::line(&lCalf.x, &lFoot.x, magenta);
	//dd::line(&lFoot.x, &lToe.x, magenta);
	//dd::line(&pelvis.x, &rThigh.x, green);
	//dd::line(&rThigh.x, &rCalf.x, green);
	//dd::line(&rCalf.x, &rFoot.x, green);
	//dd::line(&rFoot.x, &rToe.x, green);
	//return;

		/*
	// Build name->wdlModelPose index lookup once per session
	static std::unordered_map<std::string, size_t> s_modelPoseIdx;
	if (s_modelPoseIdx.empty()) {
		size_t count = sizeof(wdlModelPose) / sizeof(wdlModelPose[0]);
		for (size_t pi = 0; pi < count; pi++) {
			const char* n = wdlModelPose[pi].name;
			if (strcmp(n, "unknown") != 0)
				s_modelPoseIdx[n] = pi;
		}
	}

	// Set worldMatrix: directly from model-space data where available,
	// otherwise fall back to local-to-parent chain.
	// Bones must be in topological order (parents before children) for the fallback to work.
	for (auto& bone : skelResources.resources) {
		auto it = s_modelPoseIdx.find(bone.name.name);
		if (it != s_modelPoseIdx.end()) {
			const SkelBone& mb = wdlModelPose[it->second];
			glm::quat q(mb.w, mb.x, mb.y, mb.z);
			bone.worldMatrix = glm::mat4_cast(q);
			bone.worldMatrix[3] = glm::vec4(mb.px, mb.py, mb.pz, 1.0f);
		} else {
			xbgFile::updatePose(bone);
			glm::mat4 localMatrix = xbgFile::calculateLocalMatrix(bone);
			if (bone.node.parentIndex == 0xFFFF)
				bone.worldMatrix = localMatrix;
			else {
				auto parent = xbgFile::getSkelResAtIndex(bone.node.parentIndex);
				bone.worldMatrix = parent.worldMatrix * localMatrix;
			}
		}
		i++;
	}
	*/

	// Direct model-space assignment from wdlAnimPose (no FK)
	static std::unordered_map<std::string, size_t> s_animPoseIdx;
	if (s_animPoseIdx.empty()) {
		size_t count = sizeof(wdlAnimPose) / sizeof(wdlAnimPose[0]);
		for (size_t pi = 0; pi < count; pi++) {
			const char* n = wdlAnimPose[pi].name;
			if (strcmp(n, "Unknown") != 0)
				s_animPoseIdx[n] = pi;
		}
	}
	/*
	int i = 0;
	
	for (auto& bone : skelResources.resources) {
		auto it = s_animPoseIdx.find(bone.name.name);
		if (it != s_animPoseIdx.end()) {
			const SkelBone& mb = wdlAnimPose[it->second];
			glm::quat q(mb.w, mb.x, mb.y, mb.z);
			bone.worldMatrix = glm::mat4_cast(q);
			bone.worldMatrix[3] = glm::vec4(mb.px, mb.py, mb.pz, 1.0f);
		} else {
			xbgFile::updatePose(bone);
			glm::mat4 localMatrix = xbgFile::calculateLocalMatrix(bone);
			if (bone.node.parentIndex == 0xFFFF)
				bone.worldMatrix = localMatrix;
			else {
				auto parent = xbgFile::getSkelResAtIndex(bone.node.parentIndex);
				bone.worldMatrix = parent.worldMatrix * localMatrix;
			}
		}
		i++;
	}*/
	
	// FK loop
	int i = 0;
	for (auto& bone : skelResources.resources) {
		if (RenderInterface::instance().cameraCB.angle > 180)
		    xbgFile::updatePose(bone);
		glm::mat4 localMatrix = xbgFile::calculateLocalMatrix(bone);
		if (bone.node.parentIndex == 0xFFFF) // root bone
			bone.worldMatrix = localMatrix;
		else
		{
			auto parent = xbgFile::getSkelResAtIndex(bone.node.parentIndex);
			bone.worldMatrix = parent.worldMatrix * localMatrix;
		}
		i++;
	}

	i = 0;
	for (auto& skelRes : skelResources.resources) {
		if (i == 0)
		{
			i++;
			continue;
		}
		auto startPos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		//auto localMat = xbgFile::calculateLocalMatrix(skelRes);
		auto worldPos = skelRes.worldMatrix * startPos;
		auto parent = xbgFile::getSkelResAtIndex(skelRes.node.parentIndex);
		//auto parentMat = xbgFile::calculateLocalMatrix(parent);
		auto parentPos = parent.worldMatrix * startPos;
		//if (parent.node.parentIndex != 0xFFFF)
		dd::line(&worldPos.x, &parentPos.x, magenta);
		std::string msg = "Line from " + skelRes.name.name + " to " + parent.name.name;
		if (firstTime)
		{
			SDL_Log(msg.c_str());
			SDL_Log(("worldPos: " + glm::to_string(worldPos)).c_str());
			SDL_Log(("parentPos: " + glm::to_string(parentPos)).c_str());
		}
		i++;
	}
	firstTime = false;
	if (!settings.drawModels)
	    return;
	
	i = 0;
	// TODO: is global inverse transform applicable?
	for (auto& bone : skelResources.resources)
	{
		RenderInterface::instance().offsetsCB.offsets[i] = 
			skelResources.mats[bone.node.obj2NodeMatInd] * bone.worldMatrix;
		i++;
	}
	for (auto& mesh : lod.meshes) {
		auto offset = glm::vec4(geomParams.unk1, geomParams.unk2, geomParams.unk4, geomParams.unk5);
		RenderInterface::instance().objectCB.Offset = offset;
		/*
		<unk1>0</unk1><!--mesh.x compression?-->
		<unk2>0.16827907</unk2><!--mesh.y compression?-->
		<unk3>273.27335</unk3><!--mesh.z compression?-->
		<unk4>-1.5</unk4><!--UV.xy compression?-->
		<unk5>0.0001678518</unk5><!--UV.zw compression?-->
		*/
		// example: unk1 = 0, unk2 = 5.61117340e-05, unk4 = 0, unk5 = 3.05194408e-05 = 1 / 32766
		context->UpdateSubresource(RenderInterface::instance().objectCBB, 0, NULL, &RenderInterface::instance().objectCB, 0, 0);

		D3D_PRIMITIVE_TOPOLOGY pType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		// v is vertexCount, p is primitiveCount
		// Primitive type list (Device3D::EPrimitiveType)
		// type = 0: v = 3 * p (triangle list)
		// type = 1: v = p + 2 (triangle strip)
		// type = 3: v = 2 * p (line list)
		// type = 4: v = p + 1 (line strip)
		// type = 7: v = p (point list)
		switch (mesh.primitiveType) {
		case 0:
			pType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case 7:
			pType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			break;
		default:
			SDL_assert_release(false && "Unhandled Primitive Type");
		}
		context->IASetPrimitiveTopology(pType);

		auto materialFile = materialResources.materials[mesh.matID].file;
		auto material = loadMaterial(materialFile.c_str());
		auto paths = material->getTexturePaths();
		int numViews = paths.size();

		auto diffuseFile = material->getCommandPath("DiffuseTexture1");
		auto diffuse = loadTexture(diffuseFile.c_str());
		auto diffuseFile2 = material->getCommandPath("DiffuseTexture2");
		auto diffuse2 = loadTexture(diffuseFile2.c_str());
		auto normalFile = material->getCommandPath("NormalTexture1");
		auto normal = loadTexture(normalFile.c_str());
		auto specularFile = material->getCommandPath("SpecularTexture1");
		auto specular = loadTexture(specularFile.c_str());
		auto maskFile = material->getCommandPath("MaskTexture1");
		auto mask = loadTexture(maskFile.c_str());

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
			SpecularPower.xyzw:
* x is the only value used if MaskRedChannelMode is disabled. Range is [1,8192]
* z and w are used if MaskRedChannelMode is enabled. z is max glossiness and w is min glossiness, both in the range of [0,1]
* y is unused
* x can be remapped to z/w with `log2(x)/13`, and likewise z/w can be remapped to x with `2^(13 * z)` (or w)
WD is not very PBR so it darkens the specular if you reduce specularpower, instead of only making the reflection blurrier
*/

		UINT start = mesh.drawCall.vertexBufferByteOffset;
		UINT stride = mesh.vertexStride;
		UINT format = mesh.vertexFormat;
		UINT count = mesh.drawCall.vertexCount;
		if (mesh.vertex == NULL)
		{
			mesh.vertex = createVertexBuffer(buffers[lodNum].vertexData, start, count, stride, format, offset);
		}
		UINT stride2 = sizeof(VertexType);
		UINT offset2 = 0;
		context->IASetVertexBuffers(0, 1, &mesh.vertex->pVertexBuffer, &stride2, &offset2);
		context->IASetIndexBuffer(buffers[lodNum].index->pIndexBuffer, DXGI_FORMAT_R16_UINT, mesh.drawCall.indexBufferStartIndex * 2);
		SDL_assert_release(buffers[lodNum].index->size % sizeof(short) == 0);

		static std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3D11InputLayout> > layouts;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>& layout = layouts[mesh.vertexFormat];
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
				RenderInterface::instance().model.vShaderBlob->GetBufferPointer(),
				RenderInterface::instance().model.vShaderBlob->GetBufferSize(),
				&layout);
			SDL_assert_release(ret == S_OK);
		}
		context->IASetInputLayout(layout.Get());

		// Draw the triangles
		for (auto& mat : mats) {
			RenderInterface::instance().objectCB.Model = mat;
			context->UpdateSubresource(RenderInterface::instance().objectCBB, 0, NULL, &RenderInterface::instance().objectCB, 0, 0);
			context->DrawIndexed(mesh.drawCall.indexCount, 0, 0);
		}

		++srvI;
	}
}
