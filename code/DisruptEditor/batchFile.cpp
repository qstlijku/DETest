#include "batchFile.h"

#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_rwops.h>
#include <string.h>
#include <string>
#include "Hash.h"
#include "FileHandler.h"
#include "Serialization.h"
#include "IBinaryArchive.h"

//Game stores ptr in 0x20 of r3, size in 0x24
void crashFileHandler() {
	throw 0;
}

#define assert_file_crash(x) { SDL_assert_release(x); if(!(x)) crashFileHandler(); }

bool batchFile::open(IBinaryArchive &reader) {
	size_t size = reader.size();

	reader.serialize(head);

	if (head.type == 0) {
		//assert_file_crash(strstr(filename, "_compound.cbatch"));

		reader.markHeader();
		reader.markInPlaceOffset(reader.header.unk2);

		//assert_file_crash(compound.unk3 == 0);

		reader.serialize(srcFilename);

		//Resources
		reader.serializeNdVector(resources);

		reader.serialize(physicsFile);

		componentMBP.read(reader);
		buildingMBP.read(reader);
		quadtreeCollidableMBP.read(reader);
		debrisSpawnerMBP.read(reader);
		vegetationMBP.read(reader);

		reader.serialize(batchResource);

		reader.finish();

		//assert_file_crash(compound.bridgeSize == reader.tell() - 24);
	} else if (head.type == 1) {
		//assert_file_crash(strstr(filename, "_phys.cbatch"));
	}

	/*reader.pad(4);
	while (SDL_RWtell(reader.fp) < SDL_RWsize(reader.fp)) {
		uint32_t offset = SDL_RWtell(reader.fp);
		CStringID path;
		path.id = SDL_ReadLE32(reader.fp);
		std::string type = path.getReverseName();
		if (type[0] != '_' && !type.empty())
	}*/
	

	if (!reader.isReading()) {
		//Go back and write the size, head.size
		SDL_RWseek(reader.fp, 0, RW_SEEK_SET);
		head.size = SDL_RWsize(reader.fp) - sizeof(head);
		reader.serialize(head);
	}

	return true;
}

void batchFile::CComponentMultiBatchProcessor::read(IBinaryArchive& fp) {
	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBatchModelProcessorsAndResources *, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]

	bool unk1 = true;
	fp.serialize(unk1);
	assert_file_crash(unk1);//If this is zero it looks like we should just skip the rest of this code, however none of the files contain 0 so I won't bother

	uint32_t batchCount = batchProcessors.size();
	fp.serialize(batchCount);
	batchProcessors.resize(batchCount);

	//void SerializeArray<T1>(IBinaryArchive &, T1 *, unsigned long) [with T1=CBatchModelProcessorsAndResources *]
	for (uint32_t i = 0; i < batchCount; ++i) {
		CBatchModelProcessorsAndResources &batch = batchProcessors[i];

		uint32_t unk2 = 0;
		fp.serialize(unk2);
		assert_file_crash(unk2 == 0);

		CStringID typeID;
		typeID.id = 0xE85D5889;
		fp.serialize(typeID.id);
		assert_file_crash(typeID.id == 0xE85D5889);

		uint32_t unk3 = 0;
		fp.serialize(unk3);
		assert_file_crash(unk3 == 0);

		batch.read(fp);
	}
}

template <typename T, typename ... U>
void serializeAny(IBinaryArchive& fp, std::variant<U...>& ptr) {
	if (!std::holds_alternative<T>(ptr))
		ptr.emplace<T>();
	std::get<T>(ptr).read(fp);
}

template <typename T, typename ... U>
void serializeAny(MemberStructure& ms, std::variant<U...>& ptr) {
	if (!std::holds_alternative<T>(ptr))
		ptr.emplace<T>();
	ms.registerMember(NULL, std::get<T>(ptr));
}

void batchFile::CBatchModelProcessorsAndResources::read(IBinaryArchive& fp) {
	// void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=CSmartResourcePtr<CArchetypeResource>]
	arche.read(fp);
	//CResourceManager::GetResource((CPathID const &,CStringID const &))

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<IBatchProcessor *, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
	//unk1 = SDL_ReadLE32(fp);
	//assert_file_crash(unk1 == 1);
	
	uint32_t batchProcessorCount = processors.size();
	fp.serialize(batchProcessorCount);
	processors.resize(batchProcessorCount);

	//void SerializeArray<T1>(IBinaryArchive &, T1 *, unsigned long) [with T1=IBatchProcessor *]
	for (uint32_t i = 0; i < batchProcessorCount; ++i) {
		IBatchProcessor &batch = processors[i];

		uint32_t unk1 = 0;
		fp.serialize(unk1);
		assert_file_crash(unk1 == 0);

		fp.serialize(batch.batchProcessor.id);
		fp.serialize(batch.batchProcessorUnk1);

		std::string typeName = batch.batchProcessor.getReverseName();

		if (typeName == "CGraphicBatchProcessor")
			serializeAny<CGraphicBatchProcessor>(fp, batch.data);
		else if (typeName == "CSoundPointBatchProcessor")
			serializeAny<CSoundPointBatchProcessor>(fp, batch.data);
		else if (typeName == "CBlackoutEffectBatchProcessor")
			serializeAny<CBlackoutEffectBatchProcessor>(fp, batch.data);
		else if (typeName == "CParticlesBatchProcessor")
			serializeAny<CParticlesBatchProcessor>(fp, batch.data);
		else if (typeName == "CDynamicLightBatchProcessor")
			serializeAny<CDynamicLightBatchProcessor>(fp, batch.data);
		else if (typeName == "CLightEffectBatchProcessor")
			serializeAny<CLightEffectBatchProcessor>(fp, batch.data);
		else if (typeName == "CSecurityCameraBatchProcessor")
			serializeAny<CSecurityCameraBatchProcessor>(fp, batch.data);
		else if (typeName == "CRealTreeBatchProcessor")
			serializeAny<CRealTreeBatchProcessor>(fp, batch.data);
		else if (typeName == "CTrafficLightBatchProcessor")
			serializeAny<CTrafficLightBatchProcessor>(fp, batch.data);
		else if (typeName == "CDynamicMediaBatchProcessor")
			serializeAny<CDynamicMediaBatchProcessor>(fp, batch.data);
		else if (typeName == "CBollardBatchProcessor")
			serializeAny<CBollardBatchProcessor>(fp, batch.data);
		else {
			assert_file_crash(false && "IBatchProcessor not implemented");
			return;
		}


		//Calls some vptr in IBinaryArchive (PreAllocateDynamicType)
		//Calls CFactoryBase::PlacementCreateObjectImpl(const CStringID &, void *)

	}
}

void batchFile::CGraphicBatchProcessor::read(IBinaryArchive& fp) {
	//CGraphicBatchProcessor::Serialize((IBinaryArchive &))
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(hasBatchInstanceID);
	fp.serialize(unk10);
	xbg.read(fp);


	materialSlots.read(fp);

	fp.serialize(stride);

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CProjectedDecalInfo, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
	fp.serializeNdVector(decals);

	fp.serialize(unk12);

	//Count for CClusterHelper?
	uint32_t rangeCount = ranges.size();
	fp.serialize(rangeCount);
	ranges.resize(rangeCount);

	if (rangeCount) {
		//CClusterHelper
		CStringID type;
		fp.serialize(type.id);
		assert_file_crash(type.id == 0x2C9D950A);

		//Ptr to data
		//Calls ClusterDataSwapBytes(ptr, stride, type);

		fp.serialize(unkc1);
		fp.serialize(unkc2);
		SDL_assert_release(unkc2 == rangeCount);

		if (hasBatchInstanceID) {
			fp.serialize(batchedInstanceID);
		}

		fp.serialize(unkc3);
		fp.serialize(unkc4);
		fp.serialize(unkc5);


		for (uint32_t j = 0; j < rangeCount; ++j)
			ranges[j].read(fp);

	}
}

void batchFile::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(head);
	REGISTER_MEMBER(srcFilename);
	REGISTER_MEMBER(resources);
	REGISTER_MEMBER(physicsFile);
	REGISTER_MEMBER(componentMBP);
	REGISTER_MEMBER(buildingMBP);
	REGISTER_MEMBER(quadtreeCollidableMBP);
	REGISTER_MEMBER(debrisSpawnerMBP);
	REGISTER_MEMBER(vegetationMBP);
	REGISTER_MEMBER(batchResource);
}

void batchFile::CBatchModelProcessorsAndResources::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(arche);
	REGISTER_MEMBER(processors);
}

void batchFile::batchHeader::read(IBinaryArchive& fp) {
	fp.memBlock(this, sizeof(*this), 1);
	assert_file_crash(magic == 1112818504);
	assert_file_crash(unk1 == 32);
	assert_file_crash(type == 0 || type == 1 || type == 2);
	assert_file_crash(unk4 == 0);
	assert_file_crash(unk5 == 0);
	assert_file_crash(unk6 == 0);
	//assert_file_crash(head.unk7 == 0);
	//assert_file_crash(head.unk8 == 0);
	//assert_file_crash(head.unk9 == 0);
	//assert_file_crash(head.unk10 == 0);
}

void batchFile::batchHeader::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(magic);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(type);
	REGISTER_MEMBER(size);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
}

void batchFile::CComponentMultiBatchProcessor::registerMembers(MemberStructure & ms) {
	ms.registerMember(NULL, batchProcessors);
}

void batchFile::IBatchProcessor::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(batchProcessor);
	REGISTER_MEMBER(batchProcessorUnk1);

	std::string typeName = batchProcessor.getReverseName();
	if (typeName == "CGraphicBatchProcessor")
		serializeAny<CGraphicBatchProcessor>(ms, data);
	else if (typeName == "CSoundPointBatchProcessor")
		serializeAny<CSoundPointBatchProcessor>(ms, data);
	else if (typeName == "CBlackoutEffectBatchProcessor")
		serializeAny<CBlackoutEffectBatchProcessor>(ms, data);
	else if (typeName == "CParticlesBatchProcessor")
		serializeAny<CParticlesBatchProcessor>(ms, data);
	else if (typeName == "CDynamicLightBatchProcessor")
		serializeAny<CDynamicLightBatchProcessor>(ms, data);
	else if (typeName == "CLightEffectBatchProcessor")
		serializeAny<CLightEffectBatchProcessor>(ms, data);
	else if (typeName == "CSecurityCameraBatchProcessor")
		serializeAny<CSecurityCameraBatchProcessor>(ms, data);
	else if (typeName == "CRealTreeBatchProcessor")
		serializeAny<CRealTreeBatchProcessor>(ms, data);
	else if (typeName == "CTrafficLightBatchProcessor")
		serializeAny<CTrafficLightBatchProcessor>(ms, data);
	else if (typeName == "CDynamicMediaBatchProcessor")
		serializeAny<CDynamicMediaBatchProcessor>(ms, data);
	else if (typeName == "CBollardBatchProcessor")
		serializeAny<CBollardBatchProcessor>(ms, data);
}

void batchFile::CGraphicBatchProcessor::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
	REGISTER_MEMBER(hasBatchInstanceID);
	REGISTER_MEMBER(unk10);
	REGISTER_MEMBER(xbg);
	REGISTER_MEMBER(materialSlots);
	REGISTER_MEMBER(stride);
	REGISTER_MEMBER(decals);
	REGISTER_MEMBER(unk12);
	REGISTER_MEMBER(ranges);
}

void batchFile::CSoundPointBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(libraryObject);

	//CNomadDb::GenRecoverLibraryObject(const(0x2E69D575, unk2))
	//0x2E69D575 = SoundPoint is CStringID

	fp.serialize(unk3);

	uint32_t ndSoundHandleType;
	fp.serialize(ndSoundHandleType);
	assert_file_crash(ndSoundHandleType == 0x36C7FB6A);

	fp.serialize(unk4);
	fp.serialize(unk5);

	//Could be SBatchedSoundPoint or 0xDF63D1E
	fp.serialize(type.id);
	if (type.id == 0xB29388DD) {//SBatchedSoundPoint
		fp.serialize(unk6);
		fp.serialize(unk7);
	} else if (type.id == 0xDF63D1E) {//SBatchedSoundPointBreakable
		fp.serialize(unk6);
		fp.serialize(unk7);
	} else {
		assert_file_crash(false);
	}

}

void batchFile::CSoundPointBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(libraryObject);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
}

void batchFile::CBlackoutEffectBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBlackoutEffectBatchProcessor::SEffectPosAndAngle, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
	fp.serializeNdVectorExternal(posAndAngles, 495023964, unk3);

	fp.serialize(hasBatchInstanceIDs);
	if (hasBatchInstanceIDs) {
		//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBatchedInstanceID, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
		batchedInstanceID.read(fp);
	}

	fp.serialize(BlackoutEffectRef);
	//CNomadDb::GenRecoverLibraryObject(const(CStringID,libraryObject))
	//CStringID = 0xC9A01639 = BlackoutEffect
}

void batchFile::CBlackoutEffectBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(posAndAngles);
	REGISTER_MEMBER(hasBatchInstanceIDs);
}

void batchFile::CBlackoutEffectBatchProcessor::SEffectPosAndAngle::read(IBinaryArchive& fp) {
	fp.serialize(pos);
	fp.serialize(angle);
}

void batchFile::CBlackoutEffectBatchProcessor::SEffectPosAndAngle::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(pos);
	REGISTER_MEMBER(angle);
}

void batchFile::CParticlesBatchProcessor::read(IBinaryArchive& fp) {
	paramFile.read(fp);
	fp.serialize(hasBatchInstanceIDs);
	fp.serialize(unk2);

	if (hasBatchInstanceIDs) {
		//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBatchedInstanceID, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
		batchedInstanceID.read(fp);
	}


	fp.serializeNdVectorExternal(hdls, 0x16BB23DD, unk3);

}

void batchFile::CParticlesBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(paramFile);
	REGISTER_MEMBER(hasBatchInstanceIDs);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(batchedInstanceID);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(hdls);
}

void batchFile::CDynamicLightBatchProcessor::read(IBinaryArchive& fp) {
	lightObject.read(fp);
	fp.serialize(hasBatchInstanceIDs);
	if (hasBatchInstanceIDs) {
		//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBatchedInstanceID, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
		batchedInstanceID.read(fp);
	}

	fp.serializeNdVectorExternal(sceneLight, 0xFB4B8BEB, unk1);
}

void batchFile::CDynamicLightBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(lightObject);
	REGISTER_MEMBER(hasBatchInstanceIDs);
	REGISTER_MEMBER(batchedInstanceID);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(sceneLight);
}

void batchFile::CLightEffectBatchProcessor::read(IBinaryArchive& fp) {
	obj.read(fp);
	fp.serialize(hasBatchInstanceIDs);
	if (hasBatchInstanceIDs) {
		//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBatchedInstanceID, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
		fp.serializeNdVector(batchedInstanceID);
	}


	//CSceneLightEffectInstance = 0xEB07AAAC
	fp.serializeNdVectorExternal(instances, 0xEB07AAAC, unk2);
}

void batchFile::CLightEffectBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(obj);
	REGISTER_MEMBER(hasBatchInstanceIDs);
	REGISTER_MEMBER(batchedInstanceID);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(instances);
}

void batchFile::CSecurityCameraBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	xbg.read(fp);
	materialSlots.read(fp);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);

	fp.serialize(hasUnk);
	if (hasUnk) {
		fp.serialize(unk6);
		fp.serialize(unk7);
		arche.read(fp);
		info.read(fp);
		fp.serialize(unk8);
		fp.serializeNdVectorExternal(objects, 0x91B64372, unk9);
	}
}

void batchFile::CSecurityCameraBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(xbg);
	REGISTER_MEMBER(materialSlots);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(hasUnk);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(arche);
	REGISTER_MEMBER(info);
	REGISTER_MEMBER(unk8);
	REGISTER_MEMBER(unk9);
	REGISTER_MEMBER(objects);
}

void batchFile::CRealTreeBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(resource);

	uint32_t rangeCount = ranges.size();
	fp.serialize(rangeCount);
	ranges.resize(rangeCount);

	if (rangeCount) {
		fp.serialize(unk4);

		CStringID clusterType("CSceneRealTreeClusterHelper");
		fp.serialize(clusterType);
		SDL_assert_release(clusterType == CStringID("CSceneRealTreeClusterHelper"));

		fp.serialize(unk5);
		fp.serialize(unk6);//same as rangeCount?
		SDL_assert_release(rangeCount == unk6);

		uint32_t a = 0x1C89E6B5;
		fp.serialize(a);
		SDL_assert_release(a == 0x1C89E6B5);

		fp.serialize(unk7);
		fp.serialize(unk8);

		for (uint32_t j = 0; j < rangeCount; ++j)
			ranges[j].read(fp);
	}
}

void batchFile::CRealTreeBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(resource);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(ranges);
}

void batchFile::CBuildingMultiBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(has);
	if (!has) return;

	fp.serialize(unk1);

	uint32_t count = buildings.size();
	fp.serialize(count);
	if (count == 0) return;

	fp.serializeNdVector(buildingResources);

	CStringID buildingType("CBuilding");
	fp.serialize(buildingType);
	assert_file_crash(buildingType == CStringID("CBuilding"));

	fp.serialize(unk2);

	uint32_t count2 = count;
	fp.serialize(count2);
	assert_file_crash(count2 == count);

	buildings.resize(count);
	for (uint32_t i = 0; i < count; ++i) {
		buildings[i].read(fp);
	}

	fp.serialize(unk3);
	fp.serialize(lowGeom);
	fp.serialize(palette);
	fp.serialize(material);
	fp.serialize(roofGeom);

	CStringID UnkType(0xA0E2DE5C);
	fp.serialize(UnkType);
	assert_file_crash(UnkType == CStringID(0xA0E2DE5C));

	fp.serialize(cunk1);
	fp.serialize(cunk2);

	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(unk9);
}

void batchFile::CBuildingMultiBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(has);
	REGISTER_MEMBER(unk1);

	REGISTER_MEMBER(buildingResources);

	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(buildings);

	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(lowGeom);
	REGISTER_MEMBER(palette);
	REGISTER_MEMBER(material);
	REGISTER_MEMBER(roofGeom);

	REGISTER_MEMBER(cunk2);

	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
	REGISTER_MEMBER(unk9);
}

void batchFile::CQuadtreeCollidableMultiBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(has);
	if (!has) 
		return;

	//void SerializeMember<T1>(IBinaryArchive&, T1&)[with T1 = IQuadtreeCollidableBatchProcessor * [3]]
	//Serializes array of size 3

	//Types:
	//CQuadtreeCollidableBatchProcessor::SDeepEllipse
	//CQuadtreeCollidableBatchProcessor::SRoadObjectQuadtreeElement

	for (int i = 0; i < 3; ++i) {
		uint32_t count2 = 0;
		fp.serialize(count2);
		assert_file_crash(count2 == 0);

		CStringID type;
		fp.serialize(type.id);
		std::string typeName = type.getReverseName();

		auto& var = quadTrees[i];
		if (typeName == "CQuadtreeCollidableBatchProcessorSDeepEllipse") {
			var = CQuadtreeCollidableBatchProcessor<SDeepEllipse>();
			std::get<CQuadtreeCollidableBatchProcessor<SDeepEllipse>>(var).read(fp);
		} else if(typeName == "CQuadtreeCollidableBatchProcessorSRoadObjectQuadtreeElement") {
			var = CQuadtreeCollidableBatchProcessor<SRoadObjectQuadtreeElement>();
			std::get<CQuadtreeCollidableBatchProcessor<SRoadObjectQuadtreeElement>>(var).read(fp);
		} else {
			assert_file_crash(false);
		}
	}

}

void batchFile::CQuadtreeCollidableMultiBatchProcessor::registerMembers(MemberStructure& ms) {
}

void batchFile::SDeepEllipse::read(IBinaryArchive &fp) {
	fp.serialize(offset);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(decalCollisionType);
}

void batchFile::SDeepEllipse::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(offset);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(decalCollisionType);
}

void batchFile::SRoadObjectQuadtreeElement::read(IBinaryArchive & fp) {
	fp.serialize(offset);
	fp.serialize(unk1);
	fp.serialize(decalCollisionType);
	fp.serialize(unk2);
}

void batchFile::SRoadObjectQuadtreeElement::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(offset);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(decalCollisionType);
	REGISTER_MEMBER(unk2);
}

void batchFile::CDebrisSpawnerMultiBatchProcessor::read(IBinaryArchive & fp) {
	fp.serialize(has);
	if (!has)
		return;

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<SDebrisSpawnerBatchInstance, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
	uint32_t counter = batchInstances.size();
	fp.serialize(counter);
	batchInstances.resize(counter);
	if (counter) {
		CStringID SDebrisSpawnerBatchInstanceType("SDebrisSpawnerBatchInstance");
		fp.serialize(SDebrisSpawnerBatchInstanceType);
		assert_file_crash(SDebrisSpawnerBatchInstanceType == CStringID("SDebrisSpawnerBatchInstance"));

		fp.serialize(unk1);
		
		uint32_t counter2 = counter;
		fp.serialize(counter2);
		assert_file_crash(counter2 == counter);

		for (uint32_t i = 0; i < counter; ++i)
			batchInstances[i].read(fp);
	}

	fp.serialize(unk2);
	fp.serialize(unk3);
}

void batchFile::CDebrisSpawnerMultiBatchProcessor::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(has);
	REGISTER_MEMBER(batchInstances);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
}

void batchFile::SDebrisSpawnerBatchInstance::read(IBinaryArchive & fp) {
	fp.serialize(DebrisSpawnerDbObjectRef);
	//GenRecoverLibraryObject (CDebrisSpawnerDbObject, objectID);

	fp.serialize(offset);
}

void batchFile::SDebrisSpawnerBatchInstance::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(DebrisSpawnerDbObjectRef);
	REGISTER_MEMBER(offset);
}

void batchFile::CVegetationMultiBatchProcessor::read(IBinaryArchive & fp) {
	fp.serialize(has);
	if (!has)
		return;
}

void batchFile::CVegetationMultiBatchProcessor::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(has);
}

void batchFile::CTrafficLightBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(geom);
	fp.serialize(materials);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	if (unk6) {
		fp.serialize(unk7);
		fp.serialize(unk8);
		fp.serialize(unk9);
		fp.serializeNdVectorExternal(trafficLights, 0x60E4849E, unk10);
	}
}

void batchFile::CTrafficLightBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(geom);
	REGISTER_MEMBER(materials);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
	REGISTER_MEMBER(unk9);
	REGISTER_MEMBER(unk10);
	REGISTER_MEMBER(trafficLights);
}

void batchFile::CDynamicMediaBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(geom);
	fp.serialize(materials);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	if (unk8) {
		fp.serialize(unk9);
		fp.serialize(unk10);
		fp.serialize(unk11);
		fp.serializeNdVectorExternal(mediaObjects, 0x5193828E, mediaObjects_unk);
		fp.serialize(what);
		if(what == 0)
			fp.serialize(SDynamicIngredientPresetRef);
		fp.serialize(EBroadcastChannel);
		fp.serialize(unk12);
		fp.serialize(unk13);
	}
}

void batchFile::CDynamicMediaBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(geom);
	REGISTER_MEMBER(materials);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
	REGISTER_MEMBER(unk9);
	REGISTER_MEMBER(unk10);
	REGISTER_MEMBER(unk11);
	REGISTER_MEMBER(mediaObjects_unk);
	REGISTER_MEMBER(mediaObjects);
	REGISTER_MEMBER(what);
	REGISTER_MEMBER(SDynamicIngredientPresetRef);
	REGISTER_MEMBER(EBroadcastChannel);
	REGISTER_MEMBER(unk12);
	REGISTER_MEMBER(unk13);
}

void batchFile::CBollardBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
}

void batchFile::CBollardBatchProcessor::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
}
