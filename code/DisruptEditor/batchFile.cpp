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
#include "NBCF.h"

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

		reader.serialize(hasProcessor[0]);
		if(hasProcessor[0])
			componentMBP.read(reader);

		reader.serialize(hasProcessor[1]);
		if (hasProcessor[1])
			buildingMBP.read(reader);

		reader.serialize(hasProcessor[2]);
		if (hasProcessor[2])
			quadtreeCollidableMBP.read(reader);

		reader.serialize(hasProcessor[3]);
		if (hasProcessor[3])
			debrisSpawnerMBP.read(reader);

		reader.serialize(hasProcessor[4]);
		if (hasProcessor[4])
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
	uint32_t batchCount = batchProcessors.size();
	fp.serialize(batchCount);
	batchProcessors.resize(batchCount);

	fp.PreAllocatePointers(batchCount);

	//void SerializeArray<T1>(IBinaryArchive &, T1 *, unsigned long) [with T1=CBatchModelProcessorsAndResources *]
	for (uint32_t i = 0; i < batchCount; ++i) {
		CBatchModelProcessorsAndResources &batch = batchProcessors[i];

		fp.serializeConstant<uint32_t>(0);//This can be handled if it's not

		fp.serializeConstant(CStringID("CBatchModelProcessorsAndResources"));

		//PreAllocateDynamicType(TypeAbove);
		fp.serializeConstant<uint32_t>(0);

		batch.read(fp);
	}
}

template <typename T, typename ... U>
void serializeAny(IBinaryArchive& fp, std::variant<U...>& ptr) {
	if (!std::holds_alternative<T>(ptr))
		ptr.emplace<T>();
	std::get<T>(ptr).read(fp);
}

void batchFile::CBatchModelProcessorsAndResources::read(IBinaryArchive& fp) {
	// void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=CSmartResourcePtr<CArchetypeResource>]
	arche.read(fp);
	//CResourceManager::GetResource((CPathID const &,CStringID const &))

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<IBatchProcessor *, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
	
	uint32_t batchProcessorCount = processors.size();
	fp.serialize(batchProcessorCount);
	processors.resize(batchProcessorCount);

	fp.PreAllocatePointers(batchProcessorCount);

	//void SerializeArray<T1>(IBinaryArchive &, T1 *, unsigned long) [with T1=IBatchProcessor *]
	for (uint32_t i = 0; i < batchProcessorCount; ++i) {
		IBatchProcessor &batch = processors[i];

		fp.serializeConstant<uint32_t>(0);//This can be handled

		fp.serialize(batch.batchProcessor);

		//TODO: PreAllocateDynamicType
		fp.serialize(batch.batchProcessorUnk1);

		if (batch.batchProcessor == "CGraphicBatchProcessor")
			serializeAny<CGraphicBatchProcessor>(fp, batch.data);
		else if (batch.batchProcessor == "CSoundPointBatchProcessor")
			serializeAny<CSoundPointBatchProcessor>(fp, batch.data);
		else if (batch.batchProcessor == "CBlackoutEffectBatchProcessor")
			serializeAny<CBlackoutEffectBatchProcessor>(fp, batch.data);
		else if (batch.batchProcessor == "CParticlesBatchProcessor")
			serializeAny<CParticlesBatchProcessor>(fp, batch.data);
		else if (batch.batchProcessor == "CDynamicLightBatchProcessor")
			serializeAny<CDynamicLightBatchProcessor>(fp, batch.data);
		else if (batch.batchProcessor == "CLightEffectBatchProcessor")
			serializeAny<CLightEffectBatchProcessor>(fp, batch.data);
		else if (batch.batchProcessor == "CSecurityCameraBatchProcessor")
			serializeAny<CSecurityCameraBatchProcessor>(fp, batch.data);
		else if (batch.batchProcessor == "CRealTreeBatchProcessor")
			serializeAny<CRealTreeBatchProcessor>(fp, batch.data);
		else if (batch.batchProcessor == "CTrafficLightBatchProcessor")
			serializeAny<CTrafficLightBatchProcessor>(fp, batch.data);
		else if (batch.batchProcessor == "CDynamicMediaBatchProcessor")
			serializeAny<CDynamicMediaBatchProcessor>(fp, batch.data);
		else if (batch.batchProcessor == "CBollardBatchProcessor")
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

	fp.serialize(data.format);

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CProjectedDecalInfo, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
	fp.serializeNdVector(decals);

	fp.serialize(unk12);

	//Count for CClusterHelper?
	uint32_t rangeCount = ranges.size();
	fp.serialize(rangeCount);
	ranges.resize(rangeCount);

	if (rangeCount) {
		fp.PreAllocatePointers(rangeCount);
		fp.PreAllocateSizeOfType("CClusterHelper", rangeCount);

		fp.serialize(data);
		//SDL_assert_release(data.data.size() == rangeCount);

		if (hasBatchInstanceID) {
			fp.serialize(batchedInstanceID);
		}

		fp.serialize(unkc3);
		fp.serialize(unkc4);

		for (uint32_t j = 0; j < rangeCount; ++j)
			ranges[j].read(fp);
	}
}

void batchFile::batchHeader::read(IBinaryArchive& fp) {
	fp.serialize(magic);
	fp.serialize(unk1);
	fp.serialize(type);
	fp.serialize(size);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);

	assert_file_crash(magic == 1112818504);
	assert_file_crash(unk1 == 32);
	assert_file_crash(type == 0 || type == 1 || type == 2);
	assert_file_crash(unk4 == 0);
	assert_file_crash(unk5 == 0);
	assert_file_crash(unk6 == 0);
}

void batchFile::CSoundPointBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(isBreakable);
	fp.serialize(libraryObject);

	//CNomadDb::GenRecoverLibraryObject(const(0x2E69D575, unk2))
	//0x2E69D575 = SoundPoint is CStringID
	//Node *soundPointObj = NomadDB::GetLibraryObject(libraryObject);
	//int iNumMaxPlaying = soundPointObj->getAttrValue<int32_t>("iNumMaxPlaying");

	uint16_t count = inPlaceData.size();
	fp.serialize(count);
	inPlaceData.resize(count);
	fp.memBlockInPlace(inPlaceData.data(), sizeof(inPlaceData[0]), count);

	if (isBreakable) {
		batchedInstanceID.resize(count);
		fp.memBlockInPlace(batchedInstanceID.data(), sizeof(batchedInstanceID[0]), count);
	}

	//fp.PreAllocateSizeOfType("ndSoundHandle", (iNumMaxPlaying >> 0xe) * unk3);
	//TODO: replace
	fp.serializeConstant<CStringID>("ndSoundHandle");
	uint32_t unk11 = 0;
	fp.serialize(unk11);
	fp.serialize(unk11);
	//SDL_Log("Sound %08x: %u %u %u %u", libraryObject.libID.id, unk11, count, isBreakable, iNumMaxPlaying);
	
	fp.PreAllocatePointers(count);

	//Could be SBatchedSoundPoint or 0xDF63D1E
	if(isBreakable)
		fp.PreAllocateSizeOfType(0xDF63D1E, count);//SBatchedSoundPointBreakable
	else
		fp.PreAllocateSizeOfType("SBatchedSoundPoint", count);
}

void batchFile::CBlackoutEffectBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBlackoutEffectBatchProcessor::SEffectPosAndAngle, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
	fp.serializeNdVectorExternal(posAndAngles, 495023964);

	fp.serialize(hasBatchInstanceIDs);
	if (hasBatchInstanceIDs) {
		//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBatchedInstanceID, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
		fp.serialize(batchedInstanceID);
	}

	fp.serialize(BlackoutEffectRef);
	//CNomadDb::GenRecoverLibraryObject(const(CStringID,libraryObject))
	//CStringID = 0xC9A01639 = BlackoutEffect
}

void batchFile::CBlackoutEffectBatchProcessor::SEffectPosAndAngle::read(IBinaryArchive& fp) {
	fp.serialize(pos);
	fp.serialize(angle);
}

void batchFile::CParticlesBatchProcessor::read(IBinaryArchive& fp) {
	paramFile.read(fp);
	fp.serialize(hasBatchInstanceIDs);
	fp.serialize(unk2);

	if (hasBatchInstanceIDs) {
		//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBatchedInstanceID, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
		fp.serialize(batchedInstanceID);
	}

	fp.serializeNdVectorExternal(hdls, 0x16BB23DD);

}

void batchFile::CDynamicLightBatchProcessor::read(IBinaryArchive& fp) {
	lightObject.read(fp);
	fp.serialize(hasBatchInstanceIDs);
	if (hasBatchInstanceIDs) {
		//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBatchedInstanceID, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
		fp.serialize(batchedInstanceID);
	}

	fp.serializeNdVectorExternal(sceneLight, 0xFB4B8BEB);
}

void batchFile::CLightEffectBatchProcessor::read(IBinaryArchive& fp) {
	obj.read(fp);
	fp.serialize(hasBatchInstanceIDs);
	if (hasBatchInstanceIDs) {
		//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBatchedInstanceID, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
		fp.serializeNdVector(batchedInstanceID);
	}

	//fp.serialize(unk2);

	//CSceneLightEffectInstance = 0xEB07AAAC
	fp.serializeNdVectorExternal(instances, 0xEB07AAAC, true);
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
		fp.serializeNdVectorExternal(objects, 0x91B64372);
	}
}

void batchFile::CRealTreeBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(resource);

	data.format = 2;

	uint32_t rangeCount = ranges.size();
	fp.serialize(rangeCount);
	ranges.resize(rangeCount);

	if (rangeCount) {
		fp.serialize(data);

		fp.PreAllocateSizeOfType("CSceneRealTreeClusterHelper", rangeCount);
		
		fp.PreAllocateSizeOfType(0x1C89E6B5, data.data.size()); //CSceneRealTreeClusterHelperSInstanceData

		for (uint32_t j = 0; j < rangeCount; ++j)
			ranges[j].read(fp);
	}
}

void batchFile::CBuildingMultiBatchProcessor::read(IBinaryArchive& fp) {
	fp.PauseInPlace();

	fp.serialize(unk1);

	uint32_t count = buildings.size();
	fp.serialize(count);
	if (count) {
		fp.serializeNdVector(buildingResources);

		fp.PreAllocatePointers(count);
		fp.PreAllocateSizeOfType("CBuilding", count);

		buildings.resize(count);
		for (uint32_t i = 0; i < count; ++i) {
			buildings[i].read(fp);
		}

		fp.serialize(unk3);
		fp.serialize(lowGeom);
		fp.serialize(palette);
		fp.serialize(material);
		fp.serialize(roofGeom);

		fp.PreAllocateSizeOfType(0xA0E2DE5C, count);

		fp.serialize(unk4);
		fp.serialize(unk5);
		fp.serialize(unk6);
		fp.serialize(unk7);
		fp.serialize(unk8);
		fp.serialize(unk9);
	}

	fp.ResumeInPlace();
}

void batchFile::CQuadtreeCollidableMultiBatchProcessor::read(IBinaryArchive& fp) {
	//void SerializeMember<T1>(IBinaryArchive&, T1&)[with T1 = IQuadtreeCollidableBatchProcessor * [3]]
	//Serializes array of size 3

	//Types:
	//CQuadtreeCollidableBatchProcessor::SDeepEllipse
	//CQuadtreeCollidableBatchProcessor::SRoadObjectQuadtreeElement

	for (int i = 0; i < 3; ++i) {
		fp.serializeConstant<uint32_t>(0);

		fp.serialize(type);

		auto& var = quadTrees[i];
		if (type == "CQuadtreeCollidableBatchProcessorSDeepEllipse") {
			serializeAny<CQuadtreeCollidableBatchProcessor<SDeepEllipse>>(fp, var);
		} else if(type == "CQuadtreeCollidableBatchProcessorSRoadObjectQuadtreeElement") {
			serializeAny<CQuadtreeCollidableBatchProcessor<SRoadObjectQuadtreeElement>>(fp, var);
		} else {
			assert_file_crash(false);
		}
	}

}

void batchFile::SDeepEllipse::read(IBinaryArchive &fp) {
	fp.serialize(offset);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(decalCollisionType);
}

void batchFile::SRoadObjectQuadtreeElement::read(IBinaryArchive & fp) {
	fp.serialize(offset);
	fp.serialize(unk1);
	fp.serialize(decalCollisionType);
	fp.serialize(unk2);
}

void batchFile::CDebrisSpawnerMultiBatchProcessor::read(IBinaryArchive & fp) {
	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<SDebrisSpawnerBatchInstance, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
	fp.serializeNdVectorExternal(batchInstances, "SDebrisSpawnerBatchInstance");

	fp.serialize(unk2);
	fp.serialize(unk3);
}

void batchFile::SDebrisSpawnerBatchInstance::read(IBinaryArchive & fp) {
	fp.serialize(DebrisSpawnerDbObjectRef);
	//GenRecoverLibraryObject (CDebrisSpawnerDbObject, objectID);

	fp.serialize(offset);
}

void batchFile::CVegetationMultiBatchProcessor::read(IBinaryArchive & fp) {
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
		fp.serializeNdVectorExternal(trafficLights, 0x60E4849E);
	}
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
		fp.serializeNdVectorExternal(mediaObjects, 0x5193828E);
		fp.serialize(ingredientPreset);
		fp.serialize(EBroadcastChannel);
		fp.serialize(unk12);
		fp.serialize(unk13);
	}
}

void batchFile::CBollardBatchProcessor::read(IBinaryArchive& fp) {
	uint32_t count = (uint32_t)mats.size();
	fp.serialize(count);
	mats.resize(count);
	fp.memBlockInPlace(mats.data(), sizeof(glm::mat4), count);

	fp.serialize(batchedInstanceID);
}
