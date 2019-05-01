#pragma once

#include <unordered_map>
#include "sbaoFile.h"
#include "spkFile.h"

class DARE {
public:
	void addSoundResource(uint32_t res);
	void addAtomicObject(uint32_t res);

	std::unordered_map<uint32_t, sbaoFile> atomicObjects;
	std::unordered_map<uint32_t, spkFile> spks;

	void reset();
	sbaoFile& loadAtomicObject(uint32_t res);
	bool isAtomicObjectLoaded(uint32_t res);

	void addAODependency(uint32_t res);
	void addSndDataDependency(uint32_t res);

	static DARE& instance();
};

