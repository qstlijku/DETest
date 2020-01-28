#pragma once

#include <stdint.h>
#include "DisruptTypes.h"
#include "Vector.h"
#include "Pair.h"
#include "IBinaryArchive.h"
#include <variant>
#include <NomadDB.h>

struct SDL_RWops;
class MemberStructure;
class IBinaryArchive;

class batchFile {
public:
	struct batchHeader {
		uint32_t magic;
		uint32_t unk1; //32, checks
		uint32_t type; //0 for compound, 1 for phys, 2 for building
		uint32_t size;
		uint32_t unk3;
		uint32_t unk4;//0
		uint32_t unk5;//0, Game Checks
		uint32_t unk6;//0
		void read(IBinaryArchive& fp);
	};

	//Component MultiBatch Classes
	struct CGraphicBatchProcessor {
		float unk1;
		bool unk2;
		bool unk3;
		uint32_t unk4;
		bool unk5;
		bool unk6;
		bool unk7;
		uint8_t unk8;
		bool hasBatchInstanceID;
		bool unk10;
		CGeometryResource xbg;
		//Data before this comment is assumed to be 28 bytes
		CMaterialSlotsMap materialSlots;
		Vector<CProjectedDecalInfo> decals;
		bool unk12;

		CBatchedInstanceIDInPlace batchedInstanceID;
		uint32_t unkc3;
		uint32_t unkc4;

		ClusterData data;
		Vector<SInstanceRange> ranges;

		void read(IBinaryArchive &fp);
	};

	//TODO: May be wrong
	struct CSoundPointBatchProcessor {
		bool isBreakable;
		//References library SoundPoint
		NomadDBRef libraryObject = "SoundPoint";

		struct SBatchedSoundPointTransform {
			float unk1;
			float unk2;
			float unk3;
			float unk4;
			float unk5;
		};
		std::vector<SBatchedSoundPointTransform> inPlaceData;
		std::vector<CBatchedInstanceID> batchedInstanceID;

		void read(IBinaryArchive& fp);
	};

	struct CBlackoutEffectBatchProcessor {
		struct SEffectPosAndAngle {
			glm::vec3 pos;
			glm::vec3 angle;
			void read(IBinaryArchive& fp);
		};
		glm::vec2 unk1;
		glm::vec2 unk2;
		Vector<SEffectPosAndAngle> posAndAngles;
		bool hasBatchInstanceIDs;
		CBatchedInstanceIDInPlace batchedInstanceID;
		uint32_t BlackoutEffectRef;

		void read(IBinaryArchive& fp);
	};

	struct CParticlesBatchProcessor {
		CParticlesSystemParamResource paramFile;
		bool hasBatchInstanceIDs;
		uint32_t unk2;
		CBatchedInstanceIDInPlace batchedInstanceID;
		Vector<CParticlesSystemHdl> hdls;

		void read(IBinaryArchive& fp);
	};

	struct CDynamicLightBatchProcessor {
		CDynamicLightObject lightObject;
		bool hasBatchInstanceIDs;
		CBatchedInstanceIDInPlace batchedInstanceID;

		Vector<CSceneLight> sceneLight;

		void read(IBinaryArchive& fp);
	};

	struct CLightEffectBatchProcessor {
		CLightEffectObject obj;

		bool hasBatchInstanceIDs;
		Vector<CBatchedInstanceID> batchedInstanceID;

		//bool unk2;

		Vector<CSceneLightEffectInstance> instances;

		void read(IBinaryArchive& fp);
	};

	struct CSecurityCameraBatchProcessor {
		bool unk1;
		bool unk2;
		CGeometryResource xbg;
		CMaterialSlotsMap materialSlots;
		uint32_t unk3;
		bool unk4;
		bool unk5;

		uint32_t hasUnk;
		uint32_t unk6;
		uint32_t unk7;
		CArchetypeResource arche;
		SSecurityCameraBatchArchetypeInformation info;
		Vector<std::unique_ptr<CSecurityCameraObjectBatched>> objects;

		void read(IBinaryArchive& fp);
	};

	struct CRealTreeBatchProcessor {
		bool unk1;
		uint32_t unk2;
		CRealtreeResource resource;//CRealtreeResource

		//data size and ranges can be different
		ClusterData data;
		Vector<SInstanceRange> ranges;

		void read(IBinaryArchive& fp);
	};

	struct CTrafficLightBatchProcessor {
		bool unk1;
		bool unk2;
		CGeometryResource geom;
		CMaterialSlotsMap materials;
		uint32_t unk3;
		bool unk4;
		bool unk5;
		uint32_t unk6;

		//If unk6
		uint32_t unk7;
		uint32_t unk8;
		Vector<std::unique_ptr<CTrafficLightObjectBatched>> trafficLights;

		void read(IBinaryArchive& fp);
	};

	struct CDynamicMediaBatchProcessor {
		bool unk1;
		bool unk2;
		CGeometryResource geom;
		CMaterialSlotsMap materials;
		uint32_t unk3;
		bool unk4;
		bool unk5;
		bool unk6;
		bool unk7;
		uint32_t unk8;

		//If unk8
		uint32_t unk9;
		uint32_t unk10;
		Vector<std::unique_ptr<CBatchedDynamicMediaSystemObject>> mediaObjects;
		SDynamicIngredientPreset ingredientPreset;
		uint32_t EBroadcastChannel;
		bool unk12;
		CStringID unk13;

		void read(IBinaryArchive& fp);
	};

	struct CBollardBatchProcessor {
		std::vector<glm::mat4> mats;
		CBatchedInstanceIDInPlace batchedInstanceID;

		void read(IBinaryArchive& fp);
	};

	struct IBatchProcessor {
		CStringID batchProcessor;
		uint32_t batchProcessorUnk1;

		std::variant<CGraphicBatchProcessor,
			CSoundPointBatchProcessor,
			CBlackoutEffectBatchProcessor,
			CParticlesBatchProcessor,
			CDynamicLightBatchProcessor,
			CLightEffectBatchProcessor,
			CSecurityCameraBatchProcessor,
			CRealTreeBatchProcessor,
			CTrafficLightBatchProcessor,
			CDynamicMediaBatchProcessor,
			CBollardBatchProcessor> data;
	};

	struct CBatchModelProcessorsAndResources {
		CArchetypeResource arche;
		Vector<IBatchProcessor> processors;
		void read(IBinaryArchive& fp);
	};

	struct CComponentMultiBatchProcessor {
		Vector<CBatchModelProcessorsAndResources> batchProcessors;
		void read(IBinaryArchive& fp);
	};

	//

	struct CBuildingMultiBatchProcessor {
		bool unk1;

		Vector<CPathID> buildingResources;//CBuildingBatchResourceHiRes, also a cbatch file

		uint32_t unk2;
		Vector<CSceneBuilding> buildings;

		glm::vec3 unk3;
		CPathID lowGeom;
		CBuildingMaterialPaletteResource palette;
		CMaterialResource material;
		CPathID roofGeom;

		uint32_t cunk1, cunk2;

		uint32_t unk4;
		uint32_t unk5;
		glm::vec3 unk6;
		glm::vec3 unk7;
		glm::vec3 unk8;
		float unk9;

		void read(IBinaryArchive& fp);
	};

	//template <typename T>
	//CQuadtreeCollidableBatchProcessor

	typedef uint32_t EDecalCollisionType;

	struct SDeepEllipse {
		glm::mat4 offset;
		glm::vec2 unk1;
		float unk2;
		EDecalCollisionType decalCollisionType;

		void read(IBinaryArchive& fp);
	};

	struct SRoadObjectQuadtreeElement {
		glm::mat4 offset;
		glm::vec2 unk1;
		EDecalCollisionType decalCollisionType;
		uint8_t unk2;

		void read(IBinaryArchive& fp);
	};

	template<typename T>
	struct CQuadtreeCollidableBatchProcessor {
		void read(IBinaryArchive& fp);

		uint32_t unk0;
		bool notHas;
		glm::vec2 unk1;
		glm::vec2 unk2;
		EDecalCollisionType decalCollisionType;

		Vector< T > tree;
	};

	struct CQuadtreeCollidableMultiBatchProcessor {
		CStringID type;
		std::array< std::variant<CQuadtreeCollidableBatchProcessor<SDeepEllipse>, CQuadtreeCollidableBatchProcessor<SRoadObjectQuadtreeElement> > , 3> quadTrees;

		void read(IBinaryArchive& fp);
	};

	struct SDebrisSpawnerBatchInstance {
		uint32_t DebrisSpawnerDbObjectRef;
		glm::mat4 offset;

		void read(IBinaryArchive& fp);
	};

	struct CDebrisSpawnerMultiBatchProcessor {
		Vector<SDebrisSpawnerBatchInstance> batchInstances;
		uint32_t unk1;

		glm::vec3 unk2;
		glm::vec3 unk3;

		void read(IBinaryArchive& fp);
	};

	struct CVegetationMultiBatchProcessor {
		void read(IBinaryArchive& fp);
	};
	
	batchHeader head;
	std::string srcFilename;
	Vector<CResourceContainer> resources;
	CPathID physicsFile;

	bool hasProcessor[5];
	CComponentMultiBatchProcessor componentMBP;
	CBuildingMultiBatchProcessor buildingMBP;
	CQuadtreeCollidableMultiBatchProcessor quadtreeCollidableMBP;
	CDebrisSpawnerMultiBatchProcessor debrisSpawnerMBP;
	CVegetationMultiBatchProcessor vegetationMBP;

	CPathID batchResource;//Path to this

	bool open(IBinaryArchive& reader);
};

template<typename T>
inline void batchFile::CQuadtreeCollidableBatchProcessor<T>::read(IBinaryArchive & fp) {
	fp.serialize(unk0);

	fp.serialize(notHas);
	if (notHas) return;

	fp.serialize(unk1);
	fp.serialize(unk2);

	uint32_t count = tree.size();
	fp.serialize(count);
	tree.resize(count);

	fp.serialize(decalCollisionType);

	//Construct Quadtree
	for (uint32_t i = 0; i < count; ++i)
		tree[i].read(fp);
}
