#pragma once

#include <array>
#include <vector>
#include <glm/glm.hpp>
#include "CPathID.h"

//Havok
#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Common/Serialize/Packfile/Binary/hkPackfileHeader.h>
#include <Common/Serialize/Packfile/Binary/hkPackfileSectionHeader.h>
#include <Physics/Internal/Collide/StaticCompound/hkpStaticCompoundShape.h>

class IBinaryArchive;

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
	CPathID resourceId;//0x0, e.g graphics\buildings\facade\facade_plaza_base_store_6x8_01.hkx
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

//Size: 0x90, inherits: hkReferencedObject
//aka CPhysMergedStaticEntityPreLoad
struct CHkPhysMergedBody : public hkReferencedObject {
	hkpRigidBody *rigidBody;  //0x10, struct ptr
	hkArray<SHkChildSpawnableMergedInstanceInfos> childInstanceInfos;  //0x18, struct array
	hkArray<CHkPhysStaticMergedInstance> staticInstances; //0x28, struct array
	hkArray<CHkPhysDynamicMergedInstance> dynamicInstances; //0x38, struct array
	hkArray<CHkPhysBreakableMergedInstance> breakableInstances; //0x48, struct array
	hkArray<CHkPhysMergedResource> staticMergedResources; //0x58, struct array
	hkArray<CHkPhysDynamicMergedResource> dynamicMergedResources; //0x68, struct array
	hkArray<CHkPhysBreakableMergedResource> breakableMergedResources; //0x78, struct array
	uint32_t version; //0x88
};
static_assert(sizeof(CHkPhysMergedBody) == 0x90);

struct nomadStaticPhysResourceData : public hkpShapeBase {
	hkpShape *shape;
	uint16_t navmeshIndex;
	uint16_t highresIndex;
};

//root
struct nomadFacadePhysResourceData : public nomadStaticPhysResourceData {
	hkArray<uint16_t> facadeLeftVertices;
	hkArray<uint16_t> facadeRightVertices;
};

//root
struct nomadExtraShapes {
	hkArray<hkpShape*> shapes;
};

class hkxFile {
public:
	std::vector<uint8_t> data;
	hkPackfileHeader* hkxHeader = NULL;
	std::array<hkPackfileSectionHeader*, 3> sections;
	enum SectionTypes { CLASSNAMES = 0, TYPES = 1, DATA = 2 };

	uint8_t* dataSection = NULL;

	void read(const std::vector<uint8_t>& hkxData);
	std::vector<uint8_t> write();
};

#include <batchFile.h>

class batchCollisionFile {
public:
	batchFile::batchHeader head;
	uint32_t hkxSize;

	//Root is CHkPhysMergedBody
	hkxFile hkx;

	bool open(IBinaryArchive& fp);
};

class physResourceFile {
public:
	uint32_t unk2;
	uint32_t hkxSize;
	uint32_t switchCase;

	// 0 - nomadVehiclePhysResourceData
	// 1 - nomadRigidPhysResourceData
	// 2 - nomadStaticPhysResourceData
	// 3 - nomadBreakablePhysResourceData
	// 4 - nomadRagdollPhysResourceData
	// 5 - nomadKinematicPhysResourceData
	// 6 - nomadFacadePhysResourceData
	// 9 - nomadExtraShapes

	//Unknown, find a file with this
	//nomadMultipleBodiesPhysResourceData
	//nomadSingleBodyPhysResourceData
	//nomadPhysResourceData

	hkxFile hkx;

	bool open(IBinaryArchive& fp);
};

