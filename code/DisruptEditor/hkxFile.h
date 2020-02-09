#pragma once

#include <array>
#include <vector>

class IBinaryArchive;

//Size 0xc
template <typename T>
struct hkArray {
	T* data;
	int size;
	int capacityAndFlags;// highest 2 bits indicate any special considerations about the allocation for the array

	enum {
		CAPACITY_MASK = int(0x3FFFFFFF),
		FLAG_MASK = int(0xC0000000),
		DONT_DEALLOCATE_FLAG = int(0x80000000), // Indicates that the storage is not the array's to delete
		ALLOCATED_FROM_SPU = int(0x40000000),	// Ps3 specific. Indicates that the array storage has been allocated as a result of a SPU request.
		FORCE_SIGNED = -1
	};
};

//Size: 0x4, inherits: none
struct hkBaseObject {
	//no members
private:
	uint32_t baseStub;//Just to pad to 0x4
};

//Size: 0x8, inherits: hkBaseObject
struct hkReferencedObject : public hkBaseObject {
	uint16_t memSizeAndFlags;//0x4
	int16_t referenceCount;//0x6
};

/*//Size: 0x8, inherits: none
struct hkpEntitySmallArraySerializeOverrideType {
	void* data;//0x0
	uint16_t size;//0x4
	uint16_t capacityAndFlags;//0x6
};

//Size: 0x38, inherits hkReferencedObject
struct hkpConstraintInstance : public hkReferencedObject {

};

//Size: 0x220, inherits: 
struct hkpEntity {
	hkpMaterial material;//0x88
	void* limitContactImpulseUtilAndFlag;//0x94, SERIALIZE_IGNORED
	float damageMultiplier;//0x98
	void* breakableBody;//0x9C, SERIALIZE_IGNORED
	uint32_t solverData;//0xA0, SERIALIZE_IGNORED
	uint16_t storageIndex;//0xA4
	uint16_t contactPointCallbackDelay;//0xA6
	hkpEntitySmallArraySerializeOverrideType constraintsMaster;//0xA8, SERIALIZE_IGNORED
	hkArray<hkConstraintInternal> constraintsSlave;//0xB0, SERIALIZE_IGNORED | NOT_OWNED
	hkArray<uint8_t> constraintRuntime;//0xBC, SERIALIZE_IGNORED
};

//Size: 0x220, inherits: hkpEntity
struct hkpRigidBody : public hkpEntity {
	//no members
};*/

//Size: 0x10, inherits: none
struct SHkChildSpawnableMergedInstanceInfos {
	uint32_t type;//0x0
	uint32_t batchedInstanceIdOrPhysInstanceHandle;//0x4
	uint32_t entityId;//0x8
};

//Size: 0x6, inherits: none
struct CHkPhysMergedInstance {
	uint16_t resourceIdx; //0x0
	uint16_t startCompoundInstanceIdx; //0x2
	uint16_t type; //0x4
};

//Size: 0x6, inherits: CHkPhysMergedInstance
struct CHkPhysStaticMergedInstance : public CHkPhysMergedInstance {
	//No Members
};

//Size: 0x1c, inherits: CHkPhysMergedInstance
struct CHkPhysSpawnableMergedInstance : public CHkPhysMergedInstance {
	bool removed;//0x6
	uint32_t instanceID;//0x8
	uint32_t linkedInstanceInfoStartIndex;//0xc
	uint32_t linkedInstanceInfoCount;//0x10
	float health;//0x14
	void* dynamicLinkedEntities;//0x18, SERIALIZE_IGNORED
};

//Size: 0x1c, inherits: CHkPhysSpawnableMergedInstance
struct CHkPhysDynamicMergedInstance : public CHkPhysSpawnableMergedInstance {
	//No Members
};

//Size: 0x1c, inherits: CHkPhysSpawnableMergedInstance
struct CHkPhysBreakableMergedInstance : public CHkPhysSpawnableMergedInstance {
	//No Members
};

//Size: 0xc, inherits: none
struct CHkPhysMergedResource {
	uint32_t resourceId;//0x0
	void* physResourceImpl;//0x4, SERIALIZE_IGNORED
	uint16_t typeFlags;//0x8
};

//Size: 0xC, inherits: CHkPhysMergedResource
struct CHkPhysSpawnableMergedResource : public CHkPhysMergedResource {
	//No Members
};

//Size: 0x30, inherits: CHkPhysSpawnableMergedResource
struct CHkPhysDynamicMergedResource : public CHkPhysSpawnableMergedResource {
	float damageScales[7];//0xC
	float overrideMass;//0x28
	uint8_t damageFlags;//0x2C
	bool staticFromStart;//0x2D
};

//Size: 0x80, inherits: none
struct SHkPhysBreakableInitialMotionParams {
	glm::vec4 towardsCameraTargets;//0x0
	glm::vec4 directionalMinVel;//0x10
	glm::vec4 directionalMaxVel;//0x20
	glm::vec4 rotationalMinVel;//0x30
	glm::vec4 rotationalMaxVel;//0x40
	glm::vec4 radialCenterOffset;//0x50
	float radialMinSpeed;//0x60
	float radialMaxSpeed;//0x64
	float towardsCameraMinSpeed;//0x68
	float towardsCameraMaxSpeed;//0x6C
	uint8_t flags;//0x70
	uint8_t breakerTypeFlags;//0x71
	bool directionalLocalSpace;//0x72
	bool rotationalLocalSpace;//0x73
};

//Size: 0x80, inherits: none
struct SHkPhysBreakableLayerParamsData {
	float health;//0x0
	float autoDestructionTriggerHealthFactor;//0x4
	float needUpdateHealthFactor;//0x8
	float destructionTimeMin;//0xC
	float destructionTimeMax;//0x10
	float linearDamping;//0x14
	float angularDamping;//0x18
	float constraintFrictionFactor;//0x1C
	float constraintHealthFactor;//0x20
	float constraintBreachImpulseFactor;//0x24
	float collisionVelocityTransferFactor;//0x28
	uint16_t onLeafDetachBehaviour;//0x2C
	uint8_t damageFlags;//0x2E
	uint8_t flags;//0x2F
	float damageScales[7];//0x30
	float constraintDamageScales[7];//0x4C
	hkArray<uint16_t> parts;//0x68
	hkArray<SHkPhysBreakableInitialMotionParams> initialMotionParams;//0x74
};

//Size: 0x2c, inherits: CHkPhysSpawnableMergedResource
struct CHkPhysBreakableMergedResource : public CHkPhysSpawnableMergedResource {
	float density;//0xc
	float explosionImpulseFactor;//0x10
	float explosionRandomFactor;//0x14
	float linkedChildsDamageOnSpawn;//0x18
	bool linkedChildsSwitchStateOnSpawn;//0x1C
	hkArray<SHkPhysBreakableLayerParamsData> layerParams;//0x20
};

class hkpRigidBody;

//Size: 0x90, inherits: hkReferencedObject
struct CHkPhysMergedBody : public hkReferencedObject {
	hkpRigidBody *rigidBody;  //0x8, struct ptr
	hkArray<SHkChildSpawnableMergedInstanceInfos> childInstanceInfos;  //0xC, struct array
	hkArray<CHkPhysStaticMergedInstance> staticInstances; //0x18, struct array
	hkArray<CHkPhysDynamicMergedInstance> dynamicInstances; //0x24, struct array
	hkArray<CHkPhysBreakableMergedInstance> breakableInstances; //0x30, struct array
	hkArray<CHkPhysMergedResource> staticMergedResources; //0x3C, struct array
	hkArray<CHkPhysDynamicMergedResource> dynamicMergedResources; //0x48, struct array
	hkArray<CHkPhysBreakableMergedResource> breakableMergedResources; //0x54, struct array
	uint32_t version; //0x60
};

class hkxFile {
public:
	int32_t userTag;

	struct Section {
		std::array<char, 19> sectionTag;

		int32_t absoluteDataStart;

		//Add absoluteDataStart to these
		int32_t localFixupsOffset;
		int32_t globalFixupsOffset;
		int32_t virtualFixupsOffset;
		int32_t exportsOffset;
		int32_t importsOffset;
		int32_t endOffset;

		/// Size in bytes of data part
		int getDataSize() const {
			return localFixupsOffset;
		}
		/// Size in bytes of intra section pointer patches
		int getLocalSize() const {
			return globalFixupsOffset - localFixupsOffset;
		}
		/// Size in bytes of inter section pointer patches
		int getGlobalSize() const {
			return virtualFixupsOffset - globalFixupsOffset;
		}
		/// Size in bytes of finishing table.
		int getFinishSize() const {
			return exportsOffset - virtualFixupsOffset;
		}
		/// Size in bytes of exports table.
		int getExportsSize() const {
			return importsOffset - exportsOffset;
		}
		/// Size in bytes of imports table.
		int getImportsSize() const {
			return endOffset - importsOffset;
		}

		void read(IBinaryArchive& fp);
	};
	std::array<Section, 3> sections;
	enum SectionTypes { CLASSNAMES = 0, TYPES = 1, DATA = 2 };

	struct ClassNames {
		uint32_t signature;
		std::string name;
		void read(IBinaryArchive& fp);
	};
	std::vector<ClassNames> classNames;

	void read(IBinaryArchive& fp);
};

#include <batchFile.h>

class batchCollisionFile {
public:
	batchFile::batchHeader head;
	uint32_t hkxSize;
	hkxFile hkx;

	bool open(IBinaryArchive& fp);
};

class physResourceFile {
public:
	uint32_t unk2;
	uint32_t hkxSize;
	uint32_t switchCase;

	hkxFile hkx;

	bool open(IBinaryArchive& fp);
};

