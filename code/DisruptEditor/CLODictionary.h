#pragma once

#include "IBinaryArchive.h"

typedef uint32_t ECityLifeObjectActivationType;
typedef uint32_t ECityLifeObjectActivationZoneType;

struct SCityLifeObjectActivationSettings {
	ECityLifeObjectActivationType selActivationType;
	ECityLifeObjectActivationZoneType selActivationZoneType;

	//This is for activationZoneType == 1
	float ActivationRadius;
	float ActivationMinRadius;
	glm::vec3 ActivationOffset;

	//This is for activationType == 0
	bool IsActiveByDefault;

	bool NeverReactivate;

	//If unk5 is false
	float timeBeforeReactivate;

	void read(IBinaryArchive& fp);
};

struct CCityLifeObjectData {
	bool IsSimpleCLO;//0x38
	SCityLifeObjectActivationSettings ActivationSettings;
	void read(IBinaryArchive& fp);
};

struct CCityLifeSingleObjectData {
	CCityLifeObjectData objectData;
	bool unk1;

	void read(IBinaryArchive& fp);
};

struct CEnticerDescription {

	void read(IBinaryArchive& fp);
};

struct CEnticerData {
	CCityLifeSingleObjectData singleObjectData;
	CEnticerDescription entdescDescription;

	void read(IBinaryArchive& fp);
};

template <typename T>
struct CCityLifeObjectDataDictionary {
	uint32_t version;
	Vector<uint32_t> unk1;
	Vector<uint8_t> unk2;
	Vector<T> elements;
	void read(IBinaryArchive& fp) {
		uint32_t elementCount = elements.size();
		fp.serialize(elementCount);
		elements.resize(elementCount);

		fp.serialize(version);
		fp.serializeNdVectorExternal_pod(unk1);

		uint8_t count = unk2.size();
		fp.serialize(count);
		unk2.resize(count);
		unk2.resize(count);
		for (uint8_t i = 0; i < count; ++i)
			fp.serialize(unk2[i]);

		for (uint32_t i = 0; i < elementCount; ++i)
			fp.serialize(elements[i]);
	}
};

class CLODataDictionaries {
public:
	//CLODataDictionaries::SerializeDictionaries((IBinaryArchive &))
	void read(IBinaryArchive& fp);

	uint32_t version;
	CCityLifeObjectDataDictionary<CEnticerData> enticerData;
};

