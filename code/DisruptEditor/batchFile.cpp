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
		reader.markInPlaceOffset(reader.header.unk2 + sizeof(IBinaryArchive::Header));

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

		fp.serializeConstant<uint32_t>(0);

		fp.serializeConstant(CStringID("CBatchModelProcessorsAndResources"));

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

		fp.serializeConstant<uint32_t>(0);

		fp.serialize(batch.batchProcessor);
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
		//CClusterHelper
		fp.serializeConstant<CStringID>("CClusterHelper");

		fp.serialize(unkc2);

		fp.serialize(data);
		SDL_assert(data.data.size() == rangeCount);

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
	fp.serialize(unk1);
	fp.serialize(libraryObject);

	//CNomadDb::GenRecoverLibraryObject(const(0x2E69D575, unk2))
	//0x2E69D575 = SoundPoint is CStringID

	uint16_t unk3 = inPlaceData.size();
	fp.serialize(unk3);
	inPlaceData.resize(unk3);
	batchedInstanceID.resize(unk3);
	fp.memBlockInPlace(inPlaceData.data(), sizeof(inPlaceData[0]), unk3);
	fp.memBlockInPlace(batchedInstanceID.data(), sizeof(batchedInstanceID[0]), unk3);

	fp.serializeConstant<CStringID>("ndSoundHandle");

	fp.serialize(unk4);
	fp.serialize(unk5);

	//Could be SBatchedSoundPoint or 0xDF63D1E
	fp.serialize(type);
	if (type == "SBatchedSoundPoint") {//SBatchedSoundPoint
		fp.serialize(unk6);
		fp.serialize(unk7);
	} else if (type == 0xDF63D1E) {//SBatchedSoundPointBreakable
		fp.serialize(unk6);
		fp.serialize(unk7);
	} else {
		assert_file_crash(false);
	}

}

void batchFile::CBlackoutEffectBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBlackoutEffectBatchProcessor::SEffectPosAndAngle, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
	fp.serializeNdVectorExternal(posAndAngles, 495023964, unk3);

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


	fp.serializeNdVectorExternal(hdls, 0x16BB23DD, unk3);

}

void batchFile::CDynamicLightBatchProcessor::read(IBinaryArchive& fp) {
	lightObject.read(fp);
	fp.serialize(hasBatchInstanceIDs);
	if (hasBatchInstanceIDs) {
		//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CBatchedInstanceID, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>>]
		fp.serialize(batchedInstanceID);
	}

	fp.serializeNdVectorExternal(sceneLight, 0xFB4B8BEB, unk1);
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

void batchFile::CRealTreeBatchProcessor::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(resource);

	data.format = 2;

	uint32_t rangeCount = ranges.size();
	fp.serialize(rangeCount);
	ranges.resize(rangeCount);

	if (rangeCount) {
		fp.serialize(unk4);

		fp.serializeConstant<CStringID>("CSceneRealTreeClusterHelper");

		fp.serialize(unk5);
		fp.serialize(data);
		SDL_assert_release(data.data.size() == rangeCount);

		fp.serializeConstant<CStringID>(0x1C89E6B5);

		fp.serialize(unk7);
		fp.serialize(unk8);

		for (uint32_t j = 0; j < rangeCount; ++j)
			ranges[j].read(fp);
	}
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
		fp.serializeConstant<uint32_t>(0);

		fp.serialize(type);

		auto& var = quadTrees[i];
		if (type == "CQuadtreeCollidableBatchProcessorSDeepEllipse") {
			var = CQuadtreeCollidableBatchProcessor<SDeepEllipse>();
			std::get<CQuadtreeCollidableBatchProcessor<SDeepEllipse>>(var).read(fp);
		} else if(type == "CQuadtreeCollidableBatchProcessorSRoadObjectQuadtreeElement") {
			var = CQuadtreeCollidableBatchProcessor<SRoadObjectQuadtreeElement>();
			std::get<CQuadtreeCollidableBatchProcessor<SRoadObjectQuadtreeElement>>(var).read(fp);
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

void batchFile::SDebrisSpawnerBatchInstance::read(IBinaryArchive & fp) {
	fp.serialize(DebrisSpawnerDbObjectRef);
	//GenRecoverLibraryObject (CDebrisSpawnerDbObject, objectID);

	fp.serialize(offset);
}

void batchFile::CVegetationMultiBatchProcessor::read(IBinaryArchive & fp) {
	fp.serialize(has);
	if (!has)
		return;
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

void batchFile::CBollardBatchProcessor::read(IBinaryArchive& fp) {
	uint32_t count = (uint32_t)mats.size();
	fp.serialize(count);
	mats.resize(count);
	fp.memBlockInPlace(mats.data(), sizeof(glm::mat4), count);

	fp.serialize(batchedInstanceID);
}
