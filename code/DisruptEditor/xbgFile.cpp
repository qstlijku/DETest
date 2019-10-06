/*

Copyright 2019 Jonathan Scott
All rights reserved
You may not use this file without permission

*/

#include "xbgFile.h"

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
	relfexSystem.read(fp);

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
	fp.serializeNdVectorExternal(buffers);

	SDL_Log("GeometryMips: %u", fp.tell());
	fp.serializeNdVectorExternal(mips);

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

	fp.serializeNdVectorExternal_pod(lods);

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
	fp.serializeNdVectorExternal(materials);
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
	fp.serializeNdVectorExternal(slots);
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
	fp.serializeNdVectorExternal(skins);
}

void xbgFile::SkinNames::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, skins);
}

void xbgFile::BonePalettes::read(IBinaryArchive & fp) {
	fp.serializeNdVectorExternal(pallets);
}

void xbgFile::BonePalettes::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, pallets);
}

void xbgFile::BonePalettes::BonesPallet::read(IBinaryArchive & fp) {
	fp.serializeNdVectorExternal_pod(unk1);
}

void xbgFile::BonePalettes::BonesPallet::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, unk1);
}

void xbgFile::SkelResources::read(IBinaryArchive & fp) {
	//Can either read 1 or 0
	fp.serialize(unk1);
	if (unk1) {
		fp.serializeNdVectorExternal(resources);

		fp.serialize(unk2);

		serializeMat4Vec(fp, mats);

		SDL_assert_release(resources.size() == mats.size());
	}
}

void xbgFile::SkelResources::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(resources);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(mats);
}

void xbgFile::SkelResources::SRawNode::read(IBinaryArchive & fp) {
	fp.serialize(unk1);
	fp.serialize(pos);
	fp.serialize(rot);
	fp.serialize(unk9);
}

void xbgFile::SkelResources::SRawNode::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(pos);
	REGISTER_MEMBER(rot);
	REGISTER_MEMBER(unk9);
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
	fp.serializeNdVectorExternal(smos);
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
	fp.serializeNdVectorExternal(spheres);
	SDL_Log("Cylinders: %u", fp.tell());
	fp.serializeNdVectorExternal(cylinders);
	SDL_Log("Capsules: %u", fp.tell());
	fp.serializeNdVectorExternal(capsules);
	SDL_Log("Planes: %u", fp.tell());
	fp.serializeNdVectorExternal(planes);

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
	fp.serializeNdVectorExternal(spheres);
	SDL_Log("%u", fp.tell());
	fp.serializeNdVectorExternal(boxes);
	SDL_Log("%u", fp.tell());
	fp.serializeNdVectorExternal(cylinders);
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
	fp.serializeNdVectorExternal(particles);
	fp.serializeNdVectorExternal(meshes);
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
	fp.serializeNdVectorExternal(triangles);
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
	fp.serializeNdVectorExternal(connectivity);
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
	fp.serializeNdVectorExternal(springs);
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
	fp.serializeNdVectorExternal(nodes);
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
	fp.serializeNdVectorExternal(meshes);
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
	fp.serialize(unk1);
	fp.serialize(faceCount);
	fp.serialize(primitiveCount);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	//fp.serialize(unk7);
}

void xbgFile::CBasicDrawCallRange::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(faceCount);
	REGISTER_MEMBER(primitiveCount);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	//REGISTER_MEMBER(unk7);
}

enum EBufferType {
	VERTEX_BUFFER = 0,
	INDEX_BUFFER = 1,
	UNIFORM_BLOCK = 2,
	SHADER_PROGRAM = 3,
	STREAM_OUTPUT = 4,
	DISPLAY_LIST = 5
};

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
	index->size = indexData.size() / sizeof(short);
}

void xbgFile::SGfxBuffers::read(IBinaryArchive & fp) {
	//<unnamed>::ReadGfxBuffers(const unsigned char *&, SGfxBuffers &, unsigned long, bool)
	
	//CBufferRenderResource::Create(Device3D::EBufferType 0, const IRenderResourceCommandTrackerDecoratorFactory & (addi r4, r27, unk_107D7CDA@l), unsigned long, unsigned long 1, const void *, bool 0, bool 0, unsigned long 0, bool, bool)
	fp.pad(4);
	fp.serializeNdVectorExternal_pod(vertexData);
	
	//CBufferRenderResource::Create(Device3D::EBufferType 1, const IRenderResourceCommandTrackerDecoratorFactory &, unsigned long, unsigned long, const void *, bool, bool, unsigned long, bool, bool)
	fp.pad(4);
	fp.serializeNdVectorExternal_pod(indexData);

	//Device3D::CBuffer::Create(Device3D::EBufferType, Device3D::EBufferUsage, unsigned long elementSize, unsigned long elementCount, const void * ptr, bool)
}

void xbgFile::SGfxBuffers::registerMembers(MemberStructure & ms) {
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
	}
}

void xbgFile::registerMembers(MemberStructure & ms) {
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
	REGISTER_MEMBER(relfexSystem);
	REGISTER_MEMBER(secondaryMotionObjects);
	REGISTER_MEMBER(proceduralNodes);
	REGISTER_MEMBER(lods);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(buffers);
	REGISTER_MEMBER(mips);
	REGISTER_MEMBER(clothWrinkleControlPatchBundles);
}

void xbgFile::CSphere::read(IBinaryArchive & fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
}

void xbgFile::CSphere::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
}

void xbgFile::LOD::CSceneMesh::CDrawCallRange::read(IBinaryArchive & fp) {
	SDL_Log("CDrawCallRange: %u", fp.tell());
	drawCall.read(fp);
	sphere.read(fp);
	fp.serialize(unk1);
	fp.serialize(unk2);
	name.read(fp);
	fp.serialize(unk3);
	fp.serialize(unk4);
}

void xbgFile::LOD::CSceneMesh::CDrawCallRange::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(drawCall);
	REGISTER_MEMBER(sphere);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(name);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
}

void xbgFile::GeomMips::read(IBinaryArchive & fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	CPathID hashed(path);
	fp.serialize(hashed);
	fp.serialize(path);
}

void xbgFile::GeomMips::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(path);
}
