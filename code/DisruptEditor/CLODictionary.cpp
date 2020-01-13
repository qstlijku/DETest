#include "CLODictionary.h"
#include <SDL_log.h>
#include "FileHandler.h"

static void TestSerialize(IBinaryArchive& fp) {
	uint8_t unused = 0;
	fp.serialize(unused);
}

CLODataDictionaries::CLODataDictionaries() {
	SDL_RWops* fp = FH::openFile("worlds\\windy_city\\generated\\citylifedatadict.dat");

	/*{
		SDL_RWops* fpo = FH::openFileWrite("worlds\\windy_city\\generated\\citylifedatadict.dat");
		for (int i = 0; i < SDL_RWsize(fp); ++i)
			SDL_WriteU8(fpo, SDL_ReadU8(fp));
		SDL_RWclose(fpo);
		SDL_RWseek(fp, 0, RW_SEEK_SET);
	}*/

	CBinaryArchiveReader reader(fp);
	read(reader);

	SDL_RWclose(fp);
}

void CLODataDictionaries::read(IBinaryArchive& fp) {
	fp.markHeader();
	fp.serialize(version);
	//fp.serialize(enticerData);
	SDL_RWseek(fp.fp, 520280, RW_SEEK_SET);
	fp.serialize(enticerNPCData);
	fp.serialize(enticerAttractorData);
	fp.serialize(enticerVehicleData);
	fp.serialize(navigationHelperData);
	fp.serialize(vigilanteData);
	fp.serialize(cityLifeGroupData);
	fp.serialize(vehicleGroupData);
	fp.serialize(emergentEventData);
	fp.finish();
}

CLODataDictionaries& CLODataDictionaries::instance() {
	static CLODataDictionaries dict;
	return dict;
}

void CEnticerData::read(IBinaryArchive& fp) {
	fp.serialize(singleObjectData);
	TestSerialize(fp);
	fp.serialize(entdescDescription);
	
	if (entdescDescription.unk1 == 0) {
		TestSerialize(fp);
		fp.serialize(entactAction);
	}
	TestSerialize(fp);

	TestSerialize(fp);
	fp.serialize(entcontContext);
	TestSerialize(fp);
	fp.serialize(ActivationMandatory);
	fp.serialize(fActivationProba);
	TestSerialize(fp);
}

void CCityLifeSingleObjectData::read(IBinaryArchive& fp) {
#if 0
	fp.serialize(objectData);
	TestSerialize(fp);
	fp.serialize(unk1);
	if (unk1) {
		TestSerialize(fp);
		fp.serialize(Layer);
		fp.serialize(unk2);
		fp.serialize(unk3);
	}
	if (0x38/*TODO*/) {
		TestSerialize(fp);
		fp.serialize(unk4);
		fp.serialize(unk5);
		if () {
			TestSerialize(fp);
			fp.serialize(unk6);
		}
		TestSerialize(fp);
		if () {
			TestSerialize(fp);
			fp.serialize(unk7);
			if () {
				TestSerialize(fp);
				fp.serialize(unk8);
			}
		}
		TestSerialize(fp);
		fp.serialize(unk9);

		if () {
			TestSerialize(fp);
			fp.serialize(unk10);
			fp.serialize(unk11);
			fp.serialize(unk12);
			fp.serialize(unk13);
		}
		TestSerialize(fp);

		fp.serialize(unk14);

		if (unk14) {
			TestSerialize(fp);
			fp.serialize(unk15);
		}
	}
#endif
}

void CCityLifeObjectData::read(IBinaryArchive& fp) {
	TestSerialize(fp);
	fp.serialize(IsSimpleCLO);

	if (IsSimpleCLO) {//Calls it's own vtable, I Think this is always true CCityLifeObjectData::ShowActivationSettings
		TestSerialize(fp);
		fp.serialize(ActivationSettings);
	}
}

void SCityLifeObjectActivationSettings::read(IBinaryArchive& fp) {
	TestSerialize(fp);
	fp.serialize(selActivationType);
	fp.serialize(selActivationZoneType);
	if (selActivationZoneType == 1) {
		TestSerialize(fp);
		fp.serialize(ActivationRadius);
		fp.serialize(ActivationMinRadius);
		fp.serialize(ActivationOffset);
	}

	if (selActivationType == 0) {
		TestSerialize(fp);
		fp.serialize(IsActiveByDefault);
	}

	TestSerialize(fp);
	fp.serialize(NeverReactivate);

	if (!NeverReactivate) {
		TestSerialize(fp);
		fp.serialize(timeBeforeReactivate);
	}
}

void CEnticerDescription::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	if(unk1 == 0)
		fp.serialize(EnticerDescriptionRef);
}


void CEnticerAction::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	if (unk1 == 0)
		fp.serialize(EnticerDescriptionRef);
}


///////////////

void CCityLifeDataAndStateSerializationHandler_SerializeDataTemp(IBinaryArchive& fp) {
	uint32_t stateCount;
	fp.serialize(stateCount);
	SDL_Log("State Count: %u", stateCount);
}

void CCityLifeDataAndStateHandler::read(IBinaryArchive& fp) {
	fp.pad(8);//huh?
	fp.serialize(wlu);
	//void CCityLifeDataAndStateSerializationHandler<T1, T2, N3>::SerializeData(IBinaryArchive &) [with T1=CEnticerData, T2=CEnticerState, N3=(ECityLifeExportedType)0]
	CCityLifeDataAndStateSerializationHandler_SerializeDataTemp(fp);
	//void CCityLifeDataAndStateSerializationHandler<T1, T2, N3>::SerializeData(IBinaryArchive &) [with T1=CEnticerNPCData, T2=CEnticerNPCState, N3=(ECityLifeExportedType)1]
	CCityLifeDataAndStateSerializationHandler_SerializeDataTemp(fp);
	//void CCityLifeDataAndStateSerializationHandler<T1, T2, N3>::SerializeData(IBinaryArchive &) [with T1=CAttractorData, T2=CAttractorState, N3=(ECityLifeExportedType)2]
	CCityLifeDataAndStateSerializationHandler_SerializeDataTemp(fp);
	//void CCityLifeDataAndStateSerializationHandler<T1, T2, N3>::SerializeData(IBinaryArchive &) [with T1=CEnticerVehicleData, T2=CEnticerVehicleState, N3=(ECityLifeExportedType)3]
	CCityLifeDataAndStateSerializationHandler_SerializeDataTemp(fp);
	//void CCityLifeDataAndStateSerializationHandler<T1, T2, N3>::SerializeData(IBinaryArchive &) [with T1=CNavigationHelperData, T2=CNavigationHelperState, N3=(ECityLifeExportedType)4]
	CCityLifeDataAndStateSerializationHandler_SerializeDataTemp(fp);
	//void CCityLifeDataAndStateSerializationHandler<T1, T2, N3>::SerializeData(IBinaryArchive &) [with T1=CVigilanteData, T2=CVigilanteState, N3=(ECityLifeExportedType)5]
	CCityLifeDataAndStateSerializationHandler_SerializeDataTemp(fp);
	//void CCityLifeDataAndStateSerializationHandler<T1, T2, N3>::SerializeData(IBinaryArchive &) [with T1=CCityLifeGroupData, T2=CCityLifeGroupObjectState, N3=(ECityLifeExportedType)6]
	CCityLifeDataAndStateSerializationHandler_SerializeDataTemp(fp);
	//void CCityLifeDataAndStateSerializationHandler<T1, T2, N3>::SerializeData(IBinaryArchive &) [with T1=CVehicleGroupData, T2=CVehicleGroupState, N3=(ECityLifeExportedType)7]
	CCityLifeDataAndStateSerializationHandler_SerializeDataTemp(fp);
	//void CCityLifeDataAndStateSerializationHandler<T1, T2, N3>::SerializeData(IBinaryArchive &) [with T1=CEmergentEventData, T2=CEmergentEventState, N3=(ECityLifeExportedType)8]
	CCityLifeDataAndStateSerializationHandler_SerializeDataTemp(fp);

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVector<SCityLifeObjectInstanceDataArrays, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>, false>]
	//fp.serializeNdVector(instances);

	return;

	SDL_assert_release(sizeof(InPlaceData) == 0x50);

	uint32_t size = inPlaceData.size();
	fp.serialize(size);
	inPlaceData.resize(size);
	fp.memBlockInPlace(inPlaceData.data(), sizeof(InPlaceData), inPlaceData.size());
	fp.finish();
}

void SCityLifeObjectInstanceDataArrays::read(IBinaryArchive& fp) {
	fp.serializeNdVector(unk1);
	fp.serializeNdVector(unk2);
	fp.serializeNdVector_pod(unk3);
}

void SCityLifeObjectId::read(IBinaryArchive& fp) {
	TestSerialize(fp);
	fp.serialize(unk1);
	TestSerialize(fp);
	fp.serialize(unk2);
	TestSerialize(fp);
	fp.serialize(unk3);
	TestSerialize(fp);
	fp.serialize(unk4);
}

void CCityLifeSingleObjectData::SCityLifeObjectProximityBhvSettings::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(unk9);
}

void CCityLifeSingleObjectData::SVigilanteSettings::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
}

void ECLOTriggeredBhvType::read(IBinaryArchive& fp) {
}

void ECLOTriggeredBhv_RaycastCheckType::read(IBinaryArchive& fp) {
}

void ECLOConversationRole::read(IBinaryArchive& fp) {
}
