#include "DisruptTypes.h"

#include "Serialization.h"
#include "SDL.h"
#include "FileHandler.h"
#include "IBinaryArchive.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_transform.hpp>

void CResourceContainer::read(IBinaryArchive& fp) {
	fp.serialize(type);
	fp.serialize(file);
}

void CResourceContainer::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(type);
	REGISTER_MEMBER(file);
}

void CArchetypeResource::read(IBinaryArchive& fp) {
	fp.serialize(file);
	if(file.id != -1)
		fp.serialize(type);
}

void CArchetypeResource::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(type);
	REGISTER_MEMBER(file);
}

void CGeometryResource::read(IBinaryArchive& fp) {
	fp.serialize(file);
	if (file.id != -1)
		fp.serialize(type.id);
}

void CGeometryResource::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(file);
	REGISTER_MEMBER(type);
}

void CMaterialSlotsMap::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);

	uint32_t count = slots.size();
	fp.serialize(count);
	slots.resize(count);
	for (uint32_t i = 0; i < count; ++i) {
		auto &it = slots[i];
		fp.serialize(it.first);
		it.second.read(fp);
	}
}

void CMaterialSlotsMap::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(slots);
}

void CMaterialSlotValue::read(IBinaryArchive& fp) {
	res.read(fp);
	fp.serialize(type.id);
}

void CMaterialSlotValue::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(res);
	REGISTER_MEMBER(type);
}

void CMaterialResource::read(IBinaryArchive& fp) {
	fp.serialize(file.id);
	if(file.id != -1)
		fp.serialize(type.id);
}

void CMaterialResource::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(file);
	REGISTER_MEMBER(type);
}

void CProjectedDecalInfo::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
}

void CProjectedDecalInfo::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
}

void SInstanceRange::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(unk9);
	fp.serialize(unk10);
	fp.serialize(unk11);
	fp.serialize(unk12);
}

void SInstanceRange::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
	REGISTER_MEMBER(unk9);
	REGISTER_MEMBER(unk10);
	REGISTER_MEMBER(unk11);
	REGISTER_MEMBER(unk12);
}

void CParticlesSystemParamResource::read(IBinaryArchive& fp) {
	fp.serialize(file.id);
	if (file.id != -1)
		fp.serialize(type.id);
}

void CParticlesSystemParamResource::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(file);
	REGISTER_MEMBER(type);
}

void CParticlesSystemHdl::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
}

void CParticlesSystemHdl::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
}

void CDynamicLightObject::read(IBinaryArchive& fp) {
	fp.serialize(bEnabled);
	fp.serialize(unk2);
	fp.serialize(vectorExtraPositionOffset);
	fp.serialize(fExtraDoubleSpecularOffset);
	settings.read(fp);
}

void CDynamicLightObject::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(bEnabled);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(vectorExtraPositionOffset);
	REGISTER_MEMBER(fExtraDoubleSpecularOffset);
	REGISTER_MEMBER(settings);
}

void CDynamicLightSettings::read(IBinaryArchive& fp) {
	fp.serialize(type);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	targets.read(fp);
	fp.serialize(unk9);
	fp.serialize(unk10);
	fp.serialize(unk11);
	fp.serialize(unk12);
	fp.serialize(unk13);
	fp.serialize(unk14);
	fp.serialize(unk15);
	fp.serialize(unk16);
	fp.serialize(unk17);
	fp.serialize(unk18);
	fp.serialize(unk19);
	fp.serialize(unk20);
	fp.serialize(unk21);
	fp.serialize(unk22);
	fp.serialize(unk23);
	fp.serialize(unk24);
	fp.serialize(unk25);
	fp.serialize(unk26);
	fp.serialize(unk27);
	fp.serialize(unk28);
	fp.serialize(unk29);
	fp.serialize(unk30);
	fp.serialize(unk31);
	fp.serialize(unk32);
	fp.serialize(unk33);
	fp.serialize(unk34);
	fp.serialize(unk35);
	fp.serialize(unk36);
	fp.serialize(unk37);
	fp.serialize(unk38);
	fp.serialize(unk39.id);
	fp.serialize(unk40.id);
	fp.serialize(unk41);
	fp.serialize(unk42);
	fp.serialize(unk43);
	fp.serialize(unk44);
	fp.serialize(unk45);
	fp.serialize(unk46);

	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVectorExternal<CSceneLightClipPlane, NoLock, ndVectorPropertiesWrapper<ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>, ndVectorAllowExternalCopyProperties>>]
	fp.serializeNdVector(clipPlanes);

	fp.serialize(unk47);
	fp.serialize(unk48);
}

void CDynamicLightSettings::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(type);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
	REGISTER_MEMBER(targets);
	REGISTER_MEMBER(unk9);
	REGISTER_MEMBER(unk10);
	REGISTER_MEMBER(unk11);
	REGISTER_MEMBER(unk12);
	REGISTER_MEMBER(unk13);
	REGISTER_MEMBER(unk14);
	REGISTER_MEMBER(unk15);
	REGISTER_MEMBER(unk16);
	REGISTER_MEMBER(unk17);
	REGISTER_MEMBER(unk18);
	REGISTER_MEMBER(unk19);
	REGISTER_MEMBER(unk20);
	REGISTER_MEMBER(unk21);
	REGISTER_MEMBER(unk22);
	REGISTER_MEMBER(unk23);
	REGISTER_MEMBER(unk24);
	REGISTER_MEMBER(unk25);
	REGISTER_MEMBER(unk26);
	REGISTER_MEMBER(unk27);
	REGISTER_MEMBER(unk28);
	REGISTER_MEMBER(unk29);
	REGISTER_MEMBER(unk30);
	REGISTER_MEMBER(unk31);
	REGISTER_MEMBER(unk32);
	REGISTER_MEMBER(unk33);
	REGISTER_MEMBER(unk34);
	REGISTER_MEMBER(unk35);
	REGISTER_MEMBER(unk36);
	REGISTER_MEMBER(unk37);
	REGISTER_MEMBER(unk38);
	REGISTER_MEMBER(unk39);
	REGISTER_MEMBER(unk40);
	REGISTER_MEMBER(unk41);
	REGISTER_MEMBER(unk42);
	REGISTER_MEMBER(unk43);
	REGISTER_MEMBER(unk44);
	REGISTER_MEMBER(unk45);
	REGISTER_MEMBER(unk46);
	REGISTER_MEMBER(clipPlanes);
	REGISTER_MEMBER(unk47);
	REGISTER_MEMBER(unk48);
}

void CSceneLightTargets::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(bGeometry);
	fp.serialize(bParticles);
	fp.serialize(bParticlesShadow);
	fp.serialize(bParticlesProjectedTexture);
	fp.serialize(bWater);
	fp.serialize(bGlobalIllumination);
}

void CSceneLightTargets::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(bGeometry);
	REGISTER_MEMBER(bParticles);
	REGISTER_MEMBER(bParticlesShadow);
	REGISTER_MEMBER(bParticlesProjectedTexture);
	REGISTER_MEMBER(bWater);
	REGISTER_MEMBER(bGlobalIllumination);
}

void CSceneLightClipPlane::read(IBinaryArchive& fp) {
	fp.serialize(angYaw);
	fp.serialize(angPitch);
	fp.serialize(fDistance);
	fp.serialize(fFadeDistance);
	fp.serialize(bOccludeBounce);
}

void CSceneLightClipPlane::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(angYaw);
	REGISTER_MEMBER(angPitch);
	REGISTER_MEMBER(fDistance);
	REGISTER_MEMBER(fFadeDistance);
	REGISTER_MEMBER(bOccludeBounce);
}

void CSceneLight::read(IBinaryArchive& fp) {
	fp.serializeNdVectorExternal_TOREMOVE(clipPlanes, 2461405956, u1);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
}

void CSceneLight::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(u1);
	REGISTER_MEMBER(clipPlanes);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
}

void CBatchedInstanceIDInPlace::read(IBinaryArchive& fp) {
	uint32_t size = (uint32_t)data.size();
	fp.serialize(size);
	fp.serializeInPlace(data, size, 4);
}

void CBatchedInstanceIDInPlace::registerMembers(MemberStructure& ms) {
	ms.registerMember(NULL, data);
}

void CTextureResource::read(IBinaryArchive& fp) {
	fp.serialize(file.id);
	if (file.id != -1)
		fp.serialize(type.id);
}

void CTextureResource::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(file);
	REGISTER_MEMBER(type);
}

void CLightEffectObject::read(IBinaryArchive& fp) {
	fileTexture.read(fp);
	fp.serialize(bEnable);
	fp.serialize(unk2);
	effect.read(fp);
}

void CLightEffectObject::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(fileTexture);
	REGISTER_MEMBER(bEnable);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(effect);
}

void CSceneLightEffect::read(IBinaryArchive& fp) {
	//void SerializeMember<T1>(IBinaryArchive &, T1 &) [with T1=ndVector<CLightEffectFlareElement, NoLock, ndVectorTracker<(unsigned long)18, (unsigned long)4, (unsigned long)9>, false>]
	fp.serializeNdVector(flares);

	fp.serialize(effectType);
	fp.serialize(sourceType);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(unk9);
	fp.serialize(unk10);
	fp.serialize(unk11);
	fp.serialize(unk12);
	fp.serialize(unk13);
	fp.serialize(unk14);
	fp.serialize(unk15);
	fp.serialize(unk16);
	fp.serialize(unk17);
	fp.serialize(unk18);
	fp.serialize(unk19);
	fp.serialize(unk20);
	fp.serialize(unk21);
	fp.serialize(unk22);
	fp.serialize(unk23);
	fp.serialize(unk24);
	fp.serialize(unk25);
	fp.serialize(unk26);
	fp.serialize(unk27);
	fp.serialize(unk28);
	fp.serialize(unk29);
	fp.serialize(unk30);
	fp.serialize(unk31);
	fp.serialize(unk32);
	fp.serialize(ang1);
	fp.serialize(unk33);
	fp.serialize(unk34);
	fp.serialize(unk35);
	fp.serialize(unk36);
	fp.serialize(unk37);
	fp.serialize(unk38);
	fp.serialize(unk39);
	fp.serialize(unk40);
	fp.serialize(unk41);
}

void CSceneLightEffect::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(flares);
	REGISTER_MEMBER(effectType);
	REGISTER_MEMBER(sourceType);
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
	REGISTER_MEMBER(unk9);
	REGISTER_MEMBER(unk10);
	REGISTER_MEMBER(unk11);
	REGISTER_MEMBER(unk12);
	REGISTER_MEMBER(unk13);
	REGISTER_MEMBER(unk14);
	REGISTER_MEMBER(unk15);
	REGISTER_MEMBER(unk16);
	REGISTER_MEMBER(unk17);
	REGISTER_MEMBER(unk18);
	REGISTER_MEMBER(unk19);
	REGISTER_MEMBER(unk20);
	REGISTER_MEMBER(unk21);
	REGISTER_MEMBER(unk22);
	REGISTER_MEMBER(unk23);
	REGISTER_MEMBER(unk24);
	REGISTER_MEMBER(unk25);
	REGISTER_MEMBER(unk26);
	REGISTER_MEMBER(unk27);
	REGISTER_MEMBER(unk28);
	REGISTER_MEMBER(unk29);
	REGISTER_MEMBER(unk30);
	REGISTER_MEMBER(unk31);
	REGISTER_MEMBER(unk32);
	REGISTER_MEMBER(ang1);
	REGISTER_MEMBER(unk33);
	REGISTER_MEMBER(unk34);
	REGISTER_MEMBER(unk35);
	REGISTER_MEMBER(unk36);
	REGISTER_MEMBER(unk37);
	REGISTER_MEMBER(unk38);
	REGISTER_MEMBER(unk39);
	REGISTER_MEMBER(unk40);
	REGISTER_MEMBER(unk41);
}

void CLightEffectFlareElement::read(IBinaryArchive& fp) {
	fp.serialize(fScale);
	fp.serialize(fScaleMin);
	fp.serialize(fScaleMinThreshold);
	fp.serialize(fOffset);
	fp.serialize(iTextureSliceIndex);
	fp.serialize(vectorColor);
	fp.serialize(fRotationXAmount);
	fp.serialize(fRotationYAmount);
	fp.serialize(fFadeAngle);
}

void CLightEffectFlareElement::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(fScale);
	REGISTER_MEMBER(fScaleMin);
	REGISTER_MEMBER(fScaleMinThreshold);
	REGISTER_MEMBER(fOffset);
	REGISTER_MEMBER(iTextureSliceIndex);
	REGISTER_MEMBER(vectorColor);
	REGISTER_MEMBER(fRotationXAmount);
	REGISTER_MEMBER(fRotationYAmount);
	REGISTER_MEMBER(fFadeAngle);
}

void CSceneLightEffectInstance::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
}

void CSceneLightEffectInstance::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
}

void SSecurityCameraBatchArchetypeInformation::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serializeNdVector(unk5);
}

void SSecurityCameraBatchArchetypeInformation::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
}

void CSecurityCameraObjectBatched::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(instance);
}

void CSecurityCameraObjectBatched::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(instance);
}

void CRealtreeResource::read(IBinaryArchive& fp) {
	fp.serialize(file.id);
	if (file.id != -1)
		fp.serialize(type.id);
}

void CRealtreeResource::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(file);
	REGISTER_MEMBER(type);
}

void CTrafficLightObjectBatched::read(IBinaryArchive& fp) {
	fp.serialize(offset);
	fp.serialize(instance);
}

void CTrafficLightObjectBatched::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(offset);
	REGISTER_MEMBER(instance);
}

void CBatchedDynamicMediaSystemObject::read(IBinaryArchive& fp) {
	fp.serialize(offset);
	fp.serialize(instance);
}

void CBatchedDynamicMediaSystemObject::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(offset);
	REGISTER_MEMBER(instance);
}

void CSceneBuilding::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);

	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(unk9);
	fp.serialize(unk10);
	fp.serialize(unk11);
	fp.serialize(unk12);

	uint32_t count = batchData.size();
	fp.serialize(count);
	batchData.resize(count);

	fp.PreAllocateSizeOfType("CSceneBuildingBatchData", 1);

	fp.PreAllocateMemory((count << 3) + 4, 4);

	for (uint32_t i = 0; i < count; ++i)
		fp.serialize(batchData[i]);
}

void CSceneBuilding::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(unk4);
	REGISTER_MEMBER(unk5);
	REGISTER_MEMBER(unk6);
	REGISTER_MEMBER(unk7);
	REGISTER_MEMBER(unk8);
	REGISTER_MEMBER(unk9);
	REGISTER_MEMBER(unk10);
	REGISTER_MEMBER(unk11);
	REGISTER_MEMBER(unk12);
	REGISTER_MEMBER(batchData);
}

void CBuildingMaterialPaletteResource::read(IBinaryArchive& fp) {
	fp.serialize(file.id);
	if (file.id != -1)
		fp.serialize(type.id);
}

void CBuildingMaterialPaletteResource::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(file);
	REGISTER_MEMBER(type);
}

void ClusterData::read(IBinaryArchive& fp) {
	uint32_t count = data.size();
	fp.serialize(count);
	data.resize(count);

	//T1 *SerializeInPlace<T1>(IBinaryArchive &, T1 *, count * ComputeStride(format), 0, 0) [with T1=unsigned char]
	uint32_t byteSize = count * ComputeStride(format);
	std::vector<uint8_t> byteData(byteSize);

	//Convert to compressed hell
	{
		uint8_t* ptr = byteData.data();
		for (uint32_t i = 0; i < count; ++i) {
			if ((1 & format) != 0) {
				memcpy(ptr, data[i].matrix, 32);
				ptr += 32;
			}
			if ((2 & format) != 0) {//SwapPositionRotZTransform, float3 short2
				memcpy(ptr, &data[i].pos, 16);
				ptr += 16;
			}
			if ((4 & format) != 0) {//SwapFacadeInfo, 2 ushorts
				memcpy(ptr, data[i].facade, 4);
				ptr += 4;
			}
			if ((8 & format) != 0) {//U32
				memcpy(ptr, &data[i].unk1, 4);
				ptr += 4;
			}
			if ((64 & format) != 0) {//Unknown
				memcpy(ptr, &data[i].unk2, 4);
				ptr += 4;
			}
		}
	}

	fp.serializeInPlace(byteData, byteSize, 1);

	//Convert to You know what
	{
		uint8_t* ptr = byteData.data();
		for (uint32_t i = 0; i < count; ++i) {
			if ((1 & format) != 0) {
				memcpy(data[i].matrix, ptr, 32);
				ptr += 32;
			}
			if ((2 & format) != 0) {//SwapPositionRotZTransform, float3 short2
				memcpy(&data[i].pos, ptr, 16);
				ptr += 16;
			}
			if ((4 & format) != 0) {//SwapFacadeInfo, 2 ushorts
				memcpy(data[i].facade, ptr, 4);
				ptr += 4;
			}
			if ((8 & format) != 0) {//U32
				memcpy(&data[i].unk1, ptr, 4);
				ptr += 4;
			}
			if ((64 & format) != 0) {//Unknown
				memcpy(&data[i].unk2, ptr, 4);
				ptr += 4;
			}
		}
	}
}

void ClusterData::getMatrix(int index, glm::mat4& mat) {
	if (index < 0 || index >= data.size()) 
		return;

	float* matrix = &mat[0][0];
	Data& entry = data[index];

	if ((format & CompressedMatrix) == 0 && format & PositionRotZTransform) {
		float Rot = entry.rot / 32767.f;
		float Z =   entry.z   / 32767.f;
		mat = glm::translate(glm::mat4(1), entry.pos);
		mat = glm::rotate(mat, atan2f(Rot, Z), glm::vec3(0, 0, 1));
		return;
	}

	if ((format & CompressedMatrix) == 0) {
		mat = glm::mat4(1);
		SDL_assert_release(false);
		return;
	}

	mat = glm::mat4(1);
	int16_t* dataPtr = entry.matrix;

	float dVar7 = 1.f / 32767.f;
	*matrix = dataPtr[0] * dVar7;
	matrix[1] = dataPtr[4] * dVar7;
	matrix[2] = dataPtr[8] * dVar7;
	matrix[3] = 0.f;
	matrix[4] = dataPtr[1] * dVar7;
	matrix[5] = dataPtr[5] * dVar7;
	matrix[6] = dataPtr[9] * dVar7;
	matrix[7] = 0.f;
	matrix[8] = dataPtr[2] * dVar7;
	matrix[9] = dataPtr[6] * dVar7;
	matrix[10] = dataPtr[10] * dVar7;
	matrix[0xb] = 0.f;
	
	matrix[0xc] = dataPtr[3] * dVar7 + dataPtr[0xc];
	matrix[0xd] = dataPtr[7] * dVar7 + dataPtr[0xd];
	matrix[0xe] = dataPtr[0xb] * dVar7 + dataPtr[0xe];

	matrix[0xf] = 1.0f;
	float fVar13 = (dataPtr[0xf] / 32767.f) * 32.f + 32.f;
	matrix[4] = matrix[4] * fVar13;
	matrix[5] = matrix[5] * fVar13;
	*matrix = *matrix * fVar13;
	matrix[1] = matrix[1] * fVar13;
	matrix[10] = matrix[10] * fVar13;
	matrix[0xb] = matrix[0xb];
	matrix[6] = matrix[6] * fVar13;
	matrix[7] = matrix[7];
	matrix[8] = matrix[8] * fVar13;
	matrix[9] = matrix[9] * fVar13;
	matrix[2] = matrix[2] * fVar13;
	matrix[3] = matrix[3];
}

uint32_t ClusterData::ComputeStride(uint16_t format) {
	uint32_t uVar1 = 0;
	if ((1 & format) != 0) {//SwapCompressedMatrix, 16 shorts
		uVar1 = 32;
	}
	if ((2 & format) != 0) {//SwapPositionRotZTransform, float3 ushort2
		uVar1 = uVar1 + 16;
	}
	if ((4 & format) != 0) {//SwapFacadeInfo, 2 ushorts
		uVar1 = uVar1 + 4;
	}
	if ((8 & format) != 0) {//U32
		uVar1 = uVar1 + 4;
	}
	if ((16 & format) != 0) {//NOP
		uVar1 = uVar1 + 0;
	}
	if ((32 & format) != 0) {//NOP
		uVar1 = uVar1 + 0;
	}
	if ((64 & format) != 0) {//Unknown
		uVar1 = uVar1 + 4;
	}
	return uVar1;
}

void CResourceContainerSwap::read(IBinaryArchive& fp) {
	fp.serialize(file);
	fp.serialize(type);
}

void SDynamicIngredientPreset::read(IBinaryArchive& fp) {
	fp.serialize(what);
	if (what == 0)
		fp.serialize(SDynamicIngredientPresetRef);
}

void CSphere::read(IBinaryArchive& fp) {
	fp.serialize(pos);
	fp.serialize(radius);
}

void CSphere::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(pos);
	REGISTER_MEMBER(radius);
}
