#include "CLODictionary.h"

static void TestSerialize(IBinaryArchive& fp) {
	uint8_t unused = 0;
	fp.serialize(unused);
}

void CLODataDictionaries::read(IBinaryArchive& fp) {
	fp.serialize(version);
	fp.serialize(enticerData);
	fp.serialize(enticerNPCData);
	fp.serialize(enticerAttractorData);
	fp.serialize(enticerVehicleData);
	fp.serialize(navigationHelperData);
	fp.serialize(vigilanteData);
	fp.serialize(cityLifeGroupData);
	fp.serialize(vehicleGroupData);
	fp.serialize(emergentEventData);
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
	fp.serialize(objectData);
	TestSerialize(fp);
	fp.serialize(unk1);
	//TODO
}

void CCityLifeObjectData::read(IBinaryArchive& fp) {
	TestSerialize(fp);
	fp.serialize(IsSimpleCLO);

	if (IsSimpleCLO) {//I Think this is always true CCityLifeObjectData::ShowActivationSettings
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
	uint32_t a;
	fp.serialize(a);
}

void CCityLifeDataAndStateHandler::read(IBinaryArchive& fp) {
	//Seek forward 36

	fp.serialize(unk1);
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
	fp.serializeNdVectorExternal(instances);
	fp.serialize(unk3);
	//Padded to 16?
}

void SCityLifeObjectInstanceDataArrays::read(IBinaryArchive& fp) {
	fp.serializeNdVectorExternal(unk1);
	fp.serializeNdVectorExternal(unk2);
	fp.serializeNdVectorExternal_pod(unk3);
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
