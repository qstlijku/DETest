#include "CLODictionary.h"

static void TestSerialize(IBinaryArchive& fp) {
	uint8_t unused = 0;
	fp.serialize(unused);
}

void CLODataDictionaries::read(IBinaryArchive& fp) {
	fp.serialize(version);
	fp.serialize(enticerData);
}

void CEnticerData::read(IBinaryArchive& fp) {
	fp.serialize(singleObjectData);
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

	if (true) {//I Think this is always true CCityLifeObjectData::ShowActivationSettings
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
}
