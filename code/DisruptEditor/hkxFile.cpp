#include "hkxFile.h"

#include "IBinaryArchive.h"
#include <SDL_log.h>

bool batchCollisionFile::open(IBinaryArchive& fp) {
	fp.serialize(head);
	if (head.size != 0) {
		fp.serialize(hkxSize);
		fp.pad(16);
		if (hkxSize != 0) {
			std::vector<uint8_t> data(hkxSize);
			fp.memBlock(data.data(), 1, hkxSize);
			hkx.read(data);
		}
	}
	return true;
}

template <typename T>
static T* ReadPtr(uint8_t* &ptr) {
	T* value = (T*)ptr;
	ptr += sizeof(T);
	return value;
}

void hkxFile::read(const std::vector<uint8_t>& hkxData) {
	//Read our havok file to the base
	data = hkxData;

	uint8_t* ptr = data.data();

	//Validate Header
	hkxHeader = ReadPtr<hkPackfileHeader>(ptr);
	SDL_assert_release(hkxHeader->m_magic[0] == 0x57e0e057);
	SDL_assert_release(hkxHeader->m_magic[1] == 0x10c0c010);
	SDL_assert_release(hkxHeader->m_fileVersion == 9);
	//This is for x64 pc
	SDL_assert_release(hkxHeader->m_layoutRules[0] == 8);//Trying to process a binary file with a different pointer size than this platform.
	SDL_assert_release(hkxHeader->m_layoutRules[1] == 1);//Trying to process a binary file with a different endian than this platform.
	SDL_assert_release(hkxHeader->m_layoutRules[2] == 0);//Trying to process a binary file with a different padding optimization than this platform.
	SDL_assert_release(hkxHeader->m_layoutRules[3] == 1);//Trying to process a binary file with a different empty base class optimization than this platform.
	SDL_assert_release(hkxHeader->m_numSections == sections.size());//hardcoded
	SDL_assert_release(hkxHeader->m_contentsSectionIndex == 2);
	SDL_assert_release(hkxHeader->m_contentsSectionOffset == 0);
	SDL_assert_release(hkxHeader->m_contentsClassNameSectionIndex == 0);
	SDL_assert_release(hkxHeader->m_contentsClassNameSectionOffset == 75);
	SDL_assert_release(strcmp(hkxHeader->m_contentsVersion, "hk_2012.2.0-r1") == 0);
	SDL_assert_release(hkxHeader->m_flags == 0);
	SDL_assert_release(hkxHeader->m_pad[0] == -1);
	//This is the end of the 64 byte header, hkPackfileHeader

	//Read Sections
	for(size_t i = 0; i < sections.size(); ++i)
		sections[i] = ReadPtr<hkPackfileSectionHeader>(ptr);

	//Make sure __types__ is empty
	SDL_assert_release(sections[TYPES]->m_localFixupsOffset == 0);
	SDL_assert_release(sections[TYPES]->m_globalFixupsOffset == 0);
	SDL_assert_release(sections[TYPES]->m_virtualFixupsOffset == 0);
	SDL_assert_release(sections[TYPES]->m_exportsOffset == 0);
	SDL_assert_release(sections[TYPES]->m_importsOffset == 0);
	SDL_assert_release(sections[TYPES]->m_endOffset == 0);

	{
		//Patch Local Fixups
		//Local fixup int32_t local[2]
		//Patch by *(void*)(local[0] + absoluteDataStart + filePtr) = local[1] + absoluteDataStart + filePtr;
		int* localFixups = (int*)(data.data() + sections[DATA]->m_absoluteDataStart + sections[DATA]->m_localFixupsOffset);
		int size = sections[DATA]->getLocalSize() / sizeof(int32_t);
		for (int i = 0; i < size; i += 2) {
			if (localFixups[i] == -1)
				break;

			uint64_t* toPatch = (uint64_t*)(localFixups[i] + (uint64_t)sections[DATA]->m_absoluteDataStart + data.data());
			uint64_t value = (uint64_t)(localFixups[i + 1] + (uint64_t)sections[DATA]->m_absoluteDataStart + data.data());
			*toPatch = value;
		}
	}

	{
		//Patch Global Fixups
		//int32_t offsetToPatch
		//int32_t section
		//int32_t value
		int* globalFixups = (int*)(data.data() + sections[DATA]->m_absoluteDataStart + sections[DATA]->m_globalFixupsOffset);
		int size = sections[DATA]->getGlobalSize() / sizeof(int32_t);
		for (int i = 0; i < size; i += 3) {
			if (globalFixups[i] == -1)
				break;

			int section = globalFixups[i + 1];
			SDL_assert_release(section >= 0 && section < sections.size());

			uint64_t value = 0;

			if (sections[section]->m_localFixupsOffset == 0) {
				value = 0;
			} else {
				value = sections[section]->m_absoluteDataStart + (uint64_t)globalFixups[i + 2] + (uint64_t)data.data();
			}

			uint64_t* toPatch = (uint64_t*)(globalFixups[i] + (uint64_t)sections[DATA]->m_absoluteDataStart + data.data());
			*toPatch = value;
		}
	}

	{
		//Patch Virtual Fixups
		//int32_t offsetToPatch
		//int32_t section, likely _classnames__
		//int32_t classNameOffset, offset from _classnames_ section
		int* virtualFixups = (int*)(data.data() + sections[DATA]->m_absoluteDataStart + sections[DATA]->m_virtualFixupsOffset);
		int size = sections[DATA]->getFinishSize() / sizeof(int32_t);
		for (int i = 0; i < size; i += 3) {
			if (virtualFixups[i] == -1)
				break;

			int section = virtualFixups[i + 1];
			SDL_assert_release(section >= 0 && section < sections.size());

			uint64_t value = 0;

			if (sections[section]->m_localFixupsOffset == 0) {
				value = 0;
			} else {
				value = sections[section]->m_absoluteDataStart + (uint64_t)virtualFixups[i + 2] + (uint64_t)data.data();
			}

			//Value will point to the String of the class in the classnames section
			const char* className = (const char*)value;

			//Calls hkClassNameRegistry::getClassByName()

			//TODO: I think it's ok to not patch the vtable for now
			uint64_t* toPatch = (uint64_t*)(virtualFixups[i] + (uint64_t)sections[DATA]->m_absoluteDataStart + data.data());
			//*toPatch = vtable;

		}
	}

	//Read __data__
	dataSection = (CHkPhysMergedBody*)(data.data() + sections[DATA]->m_absoluteDataStart);

	//DEBUG
	/*for (auto& it : dataSection->mergedResources) {
		SDL_Log("hkxMergedResource: %s", it.resourceId.getReverseFilename().c_str());
	}*/

	//__debugbreak();
}

bool physResourceFile::open(IBinaryArchive& fp) {
	fp.serializeConstant<uint32_t>(0x67);
	fp.serialize(unk2);
	fp.serialize(hkxSize);
	fp.serialize(switchCase);

	return false;
}
