#pragma once

#include "IBinaryArchive.h"
#include "CPathID.h"
#include "CStringID.h"

template <typename T>
struct CGameplayQuality {
	T Normal;
	T HighEnd;
	void read(IBinaryArchive& fp) {
		fp.serialize(Normal);
		fp.serialize(HighEnd);
	}
};

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

struct ECLOTriggeredBhvType {
	void read(IBinaryArchive& fp);
};

struct ECLOTriggeredBhv_RaycastCheckType {
	void read(IBinaryArchive& fp);
};

struct ECLOConversationRole {
	void read(IBinaryArchive& fp);
};

struct CCityLifeSingleObjectData {
	CCityLifeObjectData objectData;
	bool unk1;

	//if unk1
	uint32_t Layer;//0x80
	bool unk2;
	bool unk3;

	//if WTF?
	bool unk4;
	bool unk5;
	struct SCityLifeObjectProximityBhvSettings {
		float unk1;
		float unk2;
		glm::vec3 unk3;
		float unk4;
		float unk5;
		float unk6;
		float unk7;
		float unk8;
		float unk9;

		void read(IBinaryArchive& fp);
	};
	SCityLifeObjectProximityBhvSettings unk6;

	bool unk7;

	std::string unk8;

	bool unk9;

	ECLOTriggeredBhvType unk10;
	float unk11;
	float unk12;
	ECLOTriggeredBhv_RaycastCheckType unk13;

	bool unk14;
	struct SVigilanteSettings {
		uint32_t unk1;
		uint32_t unk2;
		uint32_t unk3;
		uint32_t unk4;
		ECLOConversationRole unk5;

		void read(IBinaryArchive& fp);
	};
	SVigilanteSettings unk15;

	void read(IBinaryArchive& fp);
};

struct CEnticerDescription {
	uint32_t unk1;
	uint32_t EnticerDescriptionRef;

	void read(IBinaryArchive& fp);
};

struct CEnticerAction {
	uint32_t unk1;
	uint32_t EnticerDescriptionRef;

	void read(IBinaryArchive& fp);
};

struct SVigilanteAttractedSettings {
	CStringID VigilanteType;
	CStringID VigilanteLayer;
	bool bTriggerOnProximity;
	float fTriggerOnProximityDistance;

	void read(IBinaryArchive& fp) {
		fp.serialize(VigilanteType);
		fp.serialize(VigilanteLayer);
		fp.serialize(bTriggerOnProximity);
		fp.serialize(fTriggerOnProximityDistance);
	}
};

struct CEnticerData {
	CCityLifeSingleObjectData singleObjectData;
	CEnticerDescription entdescDescription;
	CEnticerAction entactAction;

	uint32_t entcontContext;
	CGameplayQuality<bool> ActivationMandatory;
	uint32_t fActivationProba;

	SVigilanteAttractedSettings VigilanteAttractedSettings;//0xA0

	bool bIsPillsDealer;

	typedef uint32_t EMValueCLO_FLEE_DIRECTION;
	typedef uint32_t EMValue_ENTICER_SETUPBEHAVIORS;

	void read(IBinaryArchive& fp);
};

struct CEmergentEventDescription {
	uint32_t unk1;
	uint32_t EmergentEventDescriptionRef;

	void read(IBinaryArchive& fp) {
		fp.serialize(unk1);
		if (unk1 == 0)
			fp.serialize(EmergentEventDescriptionRef);
	}
};

struct CStrikingEventSettings {
	uint32_t unk1;
	uint32_t StrikingEventSettingsRef;

	void read(IBinaryArchive& fp) {
		fp.serialize(unk1);
		if (unk1 == 0)
			fp.serialize(StrikingEventSettingsRef);
	}
};

struct CEmergentEventData {
	CCityLifeSingleObjectData singleObjectData;
	CEmergentEventDescription eedescDescription;
	CStrikingEventSettings strikingeventStrikingEventDescription;

	void read(IBinaryArchive& fp) {
		fp.serialize(singleObjectData);
		fp.serialize(eedescDescription);
		fp.serialize(strikingeventStrikingEventDescription);
	}
};

struct CCityLifeBaseGroupData {
	CCityLifeObjectData objData;
	uint32_t LayerCount;
	std::vector<uint32_t> Weights;
	bool bSynchronizeTriggeredBhv;
	bool bIsShowcase;
	bool bHasEmergentEventLayer;
	uint32_t iEmergentEventLayerMask;
	std::vector<uint32_t> EmergentEventSettingsKeys;

	void read(IBinaryArchive& fp) {
		fp.serialize(objData);
		fp.serialize(LayerCount);
		fp.serializeNdVector_pod(Weights);
		fp.serialize(bSynchronizeTriggeredBhv);
		fp.serialize(bIsShowcase);
		fp.serialize(bHasEmergentEventLayer);
		fp.serialize(iEmergentEventLayerMask);
		fp.serializeNdVector_pod(EmergentEventSettingsKeys);
	}
};

struct CVehicleGroupData {
	CCityLifeBaseGroupData groupData;
	uint32_t fVehiculeToSpawnMin;
	uint32_t fVehiculeToSpawnMax;

	void read(IBinaryArchive& fp) {
		fp.serialize(groupData);
		fp.serialize(fVehiculeToSpawnMin);
		fp.serialize(fVehiculeToSpawnMax);
	}
};

struct CCityLifeGroupData {
	CCityLifeBaseGroupData groupData;
	std::vector<uint32_t> Contexts;
	std::vector<uint32_t> AnimTemplates;
	bool bIsManagedByEntity;
	bool unk1;//Unused?
	std::vector<uint32_t> PlatformConfigs;
	CGameplayQuality<bool> SpawningMandatory;

	void read(IBinaryArchive& fp) {
		fp.serialize(groupData);
		fp.serializeNdVector_pod(Contexts);
		fp.serializeNdVector_pod(AnimTemplates);
		fp.serialize(bIsManagedByEntity);
		fp.serialize(unk1);
		fp.serializeNdVector_pod(PlatformConfigs);
		fp.serialize(SpawningMandatory);
	}
};

struct CVigilanteData {
	CCityLifeBaseGroupData groupData;
	CStringID VigilanteType;
	std::string VigilanteGroupName;
	std::vector<CStringID> LayerTypes;
	std::vector<std::string> VigilanteBoxPath;
	glm::vec3 vectorPreventSpawningBoxSize;
	float fDistToReactionClose;
	float fDistToReactionFar;
	uint32_t iVigilanteRewardList;
	uint32_t iVigilanteRewardListNonLethalTakedown;
	uint32_t entcontContext;
	uint32_t iDefaultLayer;
	//ArrayConsistencyCheck

	void read(IBinaryArchive& fp) {
		fp.serialize(groupData);
		fp.serialize(VigilanteType);
		fp.serialize(VigilanteGroupName);
		fp.serializeNdVector(LayerTypes);
		fp.serializeNdVector_pod(VigilanteBoxPath);
		fp.serialize(vectorPreventSpawningBoxSize);
		fp.serialize(fDistToReactionClose);
		fp.serialize(fDistToReactionFar);
		fp.serialize(iVigilanteRewardList);
		fp.serialize(iVigilanteRewardListNonLethalTakedown);
		fp.serialize(entcontContext);
		fp.serialize(iDefaultLayer);
	}
};

struct CNavigationHelperData {
	CCityLifeSingleObjectData singleObj;
	uint32_t selNavType;//SNavHelperType
	float fDistanceForlane;

	void read(IBinaryArchive& fp) {
		fp.serialize(singleObj);
		fp.serialize(selNavType);
		fp.serialize(fDistanceForlane);
	}
};

struct CVehicleSpawnInfo {
	uint32_t unk1;
	uint32_t VehicleSpawnInfoRef;

	void read(IBinaryArchive& fp) {
		fp.serialize(unk1);
		if (unk1 == 0)
			fp.serialize(VehicleSpawnInfoRef);
	}
};

struct CVehiclesBank {
	uint32_t unk1;
	uint32_t VehiclesBankRef;

	void read(IBinaryArchive& fp) {
		fp.serialize(unk1);
		if (unk1 == 0)
			fp.serialize(VehiclesBankRef);
	}
};

typedef uint32_t EVehicleSpawningAlignment;

struct CEnticerVehicleData {
	CCityLifeSingleObjectData singleObj;
	CPathID archArchetype;
	bool unk1;
	bool bIsCarOnDemand;

	//0xD0 - 

	//if !bIsCarOnDemand && unk1
	bool unk2;
	
	//if !bIsCarOnDemand
	CVehicleSpawnInfo vehicleinfoVehicleSpawnInfo;
	CVehiclesBank vehiclesbankVehiclesBank;
	uint32_t entcontContext;
	uint32_t uiSpawningProba;
	bool unk3;
	bool bIsShowCase;

	EVehicleSpawningAlignment selAlignment;
	uint32_t uiFlipProba;
	bool unk5;

	float RandomAngle;

	bool unk6;
	bool unk7;
	bool unk8;

	bool bPlaceWithSimulation;

	void read(IBinaryArchive& fp) {
		fp.serialize(singleObj);
		fp.serialize(archArchetype);
		fp.serialize(unk1);
		fp.serialize(bIsCarOnDemand);

		if (!bIsCarOnDemand) {
			if(unk1)
				fp.serialize(unk2);

			if (unk2) {
				fp.serialize(vehicleinfoVehicleSpawnInfo);
				fp.serialize(vehiclesbankVehiclesBank);
			}

			fp.serialize(entcontContext);
			fp.serialize(uiSpawningProba);
			fp.serialize(unk3);
			fp.serialize(bIsShowCase);
		}

		fp.serialize(selAlignment);
		fp.serialize(uiFlipProba);
		fp.serialize(unk5);

		if(unk5)
			fp.serialize(RandomAngle);

		if (!bIsCarOnDemand) {
			fp.serialize(unk6);
			fp.serialize(unk7);
			fp.serialize(unk8);
		}

		fp.serialize(bPlaceWithSimulation);
	}
};

typedef uint32_t ELookAtHDLoopType;

struct SAttractorVigilanteSettings {
	float fTriggerDelay;
	float fTriggerOnDelayDelay;
	bool bTriggerOnProximity;
	float fTriggerOnProximityDistance;

	void read(IBinaryArchive& fp) {
		fp.serialize(fTriggerDelay);
		fp.serialize(fTriggerOnDelayDelay);
		fp.serialize(bTriggerOnProximity);
		fp.serialize(fTriggerOnProximityDistance);
	}
};

struct CCityLifeObjectUsageMaxAltitudeRestriction {
	float fMaxAltitude;

	void read(IBinaryArchive& fp) {
		fp.serialize(fMaxAltitude);
	}
};

struct SCityLifeObjectUsageConeRestrictionForComponent {
	float fAngleDegree;
	float fDirection;
	float fMinDist;
	void read(IBinaryArchive& fp) {
		fp.serialize(fAngleDegree);
		fp.serialize(fDirection);
		fp.serialize(fMinDist);
	}
};

struct CCityLifeObjectUsageConeRestriction {
	float fAngle;
	glm::vec3 vectorDir;
	float fMinDist;
	float fAngleFromDir;
	void read(IBinaryArchive& fp) {
		fp.serialize(fAngle);
		fp.serialize(vectorDir);
		fp.serialize(fMinDist);
		fp.serialize(fAngleFromDir);
	}
};

struct CAttractorData {
	CCityLifeSingleObjectData singleObj;
	uint32_t attdescDescription;
	uint32_t attactActionKey;//If attdescDescription == 0 || attdescDescription == -1

	typedef uint32_t EAttractorType;
	EAttractorType selAttractorType;
	
	bool unk1;
	bool unk2;

	//If selAttractorType == 2
	float fLookAtDuration;
	ELookAtHDLoopType selLookAtType;
	bool unk3;

	uint32_t entdescContextKey;
	uint32_t entdescReactionContextKey;

	//If selAttractorType == 0 || selAttractorType == 1
	bool unk4;

	//If selAttractorType == 0
	bool unk5;

	bool unk6;

	//If unk6
	float fStimRadius;
	glm::vec3 vectorStimOffset;
	float fStimFrequency;
	float fDisableAttractorOnEntityDuration;

	typedef uint32_t EAttractorTriggerMethod;
	EAttractorTriggerMethod selAttractorTriggerMethod;

	//If selAttractorTriggerMethod
	SAttractorVigilanteSettings AttractorTriggerSetting;

	bool bUseMaxAltRestriction;

	//If bUseMaxAltRestriction
	CCityLifeObjectUsageMaxAltitudeRestriction MaxAltitudeRestriction;

	std::vector<SCityLifeObjectUsageConeRestrictionForComponent> ConeRestrictionsComponents;

	std::vector<CCityLifeObjectUsageConeRestriction> ConeRestrictions;

	bool unk7;
	bool unk8;
	bool unk9;
	bool unk10;
	bool unk11;
	bool unk12;
	bool unk13;
	bool unk14;
	uint32_t unk142;
	bool unk15;
	float fAIRunningSpeed;
	typedef uint32_t EWalkType;
	EWalkType selAIWalkType;

	float unk16;
	float unk17;
	float unk18;
	float unk19;
	bool unk20;
	float unk21;

	bool unk22;
	uint32_t unk23;

	void read(IBinaryArchive& fp) {
		fp.serialize(singleObj);
		fp.serialize(attdescDescription);
		if (attdescDescription == 0 || attdescDescription == -1)
			fp.serialize(attactActionKey);

		fp.serialize(selAttractorType);

		fp.serialize(unk1);
		fp.serialize(unk2);

		if (selAttractorType == 2) {
			fp.serialize(fLookAtDuration);
			fp.serialize(selLookAtType);
			fp.serialize(unk3);
		}

		fp.serialize(entdescContextKey);
		fp.serialize(entdescReactionContextKey);

		if (selAttractorType == 0 || selAttractorType == 1) {
			fp.serialize(unk4);

			if (selAttractorType == 0)
				fp.serialize(unk5);
		}

		fp.serialize(unk6);

		if (unk6) {
			fp.serialize(fStimRadius);
			fp.serialize(vectorStimOffset);
			fp.serialize(fStimFrequency);
			fp.serialize(fDisableAttractorOnEntityDuration);
		}

		fp.serialize(selAttractorTriggerMethod);

		if (selAttractorTriggerMethod)
			fp.serialize(AttractorTriggerSetting);

		fp.serialize(bUseMaxAltRestriction);

		if (bUseMaxAltRestriction)
			fp.serialize(MaxAltitudeRestriction);

		if (false) {//TODO
			fp.serializeNdVector(ConeRestrictionsComponents);
		} else {
			fp.serializeNdVector(ConeRestrictions);
		}

		if (unk2) {
			fp.serialize(unk7);
			fp.serialize(unk8);
			fp.serialize(unk9);
			fp.serialize(unk10);
			fp.serialize(unk11);
			fp.serialize(unk12);
			fp.serialize(unk13);
			fp.serialize(unk14);
			fp.serialize(unk142);
			fp.serialize(unk15);
			fp.serialize(fAIRunningSpeed);
			fp.serialize(selAIWalkType);

			if (unk14) {
				fp.serialize(unk16);
				fp.serialize(unk17);
				fp.serialize(unk18);
				fp.serialize(unk19);
				fp.serialize(unk20);
				fp.serialize(unk21);
			}
		}

		if (false) {//TODO
			fp.serialize(unk22);
			fp.serialize(unk23);
		}
	}
};

struct CEnticerNPCData {
	CCityLifeSingleObjectData singleObj;
	uint32_t entnpcdescDescription;

	//If entnpcdescDescription == 0 || entnpcdescDescription == -1
	uint32_t entnpcactActionKey;

	std::string OverrideLKPGroupName;
	bool bIsVigilanteAttractedCandidate;

	//If bIsVigilanteAttractedCandidate
	SVigilanteAttractedSettings VigilanteAttractedSettings;

	void read(IBinaryArchive& fp) {
		fp.serialize(singleObj);
		fp.serialize(entnpcdescDescription);
		if (entnpcdescDescription == 0 || entnpcdescDescription == -1)
			fp.serialize(entnpcactActionKey);
		fp.serialize(OverrideLKPGroupName);
		fp.serialize(bIsVigilanteAttractedCandidate);
		if (bIsVigilanteAttractedCandidate)
			fp.serialize(VigilanteAttractedSettings);
	}
};

template <typename T>
struct CCityLifeObjectDataDictionary {
	uint32_t unk2;
	Vector<uint32_t> unk3;
	uint8_t count = 0;
	Vector<uint8_t> unkData;
	Vector<T> elements;
	void read(IBinaryArchive& fp) {
		uint32_t elementCount = elements.size();
		fp.serialize(elementCount);
		elements.resize(elementCount);

		fp.serialize(unk2);
		fp.serializeNdVector_pod(unk3);

		fp.serialize(count);
		//Counts down from count
		uint8_t i = count;
		while (i) {
			fp.serialize(i);
			i--;
			i &= 0xFF;
		}

		//What?
		unkData.resize(elementCount);
		fp.memBlock(unkData.data(), elementCount, 1);

		for (uint32_t i = 0; i < elementCount; ++i)
			fp.serialize(elements[i]);
	}
};

class CLODataDictionaries {
public:
	CLODataDictionaries();

	//CLODataDictionaries::SerializeDictionaries((IBinaryArchive &))
	void read(IBinaryArchive& fp);

	static CLODataDictionaries& instance();

	uint32_t version;
	CCityLifeObjectDataDictionary<CEnticerData> enticerData;//TODO
	CCityLifeObjectDataDictionary<CEnticerNPCData> enticerNPCData;//TODO
	CCityLifeObjectDataDictionary<CAttractorData> enticerAttractorData;//TODO
	CCityLifeObjectDataDictionary<CEnticerVehicleData> enticerVehicleData;//Might be wrong
	CCityLifeObjectDataDictionary<CNavigationHelperData> navigationHelperData;
	CCityLifeObjectDataDictionary<CVigilanteData> vigilanteData;
	CCityLifeObjectDataDictionary<CCityLifeGroupData> cityLifeGroupData;
	CCityLifeObjectDataDictionary<CVehicleGroupData> vehicleGroupData;
	CCityLifeObjectDataDictionary<CEmergentEventData> emergentEventData;
};


//State

struct SCityLifeObjectId {
	void read(IBinaryArchive& fp);

	uint64_t unk1;
	uint64_t unk2;
	glm::vec3 unk3;
	glm::vec3 unk4;
};

struct SCityLifeObjectInstanceDataArrays {
	void read(IBinaryArchive& fp);

	std::vector<SCityLifeObjectId> unk1;
	std::vector<SCityLifeObjectId> unk2;
	std::vector<uint64_t> unk3;
};

//from CCityLifeObjectManagerData + InPlaceOffset
//CCityLifeDataAndStateHandler::SerializeData((IBinaryArchive &, CCityLifeObjectManager *, IWorldUnit *, ICityLifeManagerDebugInfo *))
class CCityLifeDataAndStateHandler {
public:
	void read(IBinaryArchive& fp);

	CPathID wlu; // eg. worlds\windy_city\generated\wlu\wlu_data_near205.xml.data.fcb
	std::vector<SCityLifeObjectInstanceDataArrays> instances;
	struct InPlaceData {
		uint32_t field_0x0;
		uint32_t field_0x4;
		uint32_t field_0x8;
		uint32_t field_0xc;
		uint32_t field_0x10;
		uint8_t unk_0x14;
		uint8_t unk_0x15;
		uint8_t unk_0x16;
		uint8_t unk_0x17;
		uint32_t unk_0x18;
		uint32_t unk_0x1c;
		uint32_t field_0x20;
		uint32_t field_0x24;
		uint32_t field_0x28;
		uint8_t unk_0x2c;
		uint8_t unk_0x2d;
		uint8_t unk_0x2e;
		uint8_t unk_0x2f;
		uint32_t field_0x30;
		uint32_t field_0x34;
		uint32_t field_0x38;
		uint32_t unk_0x3c;
		uint16_t field_0x40;
		uint16_t field_0x42;
		uint16_t field_0x44;
		uint8_t field_0x46;
		uint8_t unk_0x47;
		uint32_t field_0x48;
		uint8_t unk_0x4c;
		uint8_t unk_0x4d;
		uint8_t unk_0x4e;
		uint8_t unk_0x4f;
	};
	std::vector<InPlaceData> inPlaceData;
};