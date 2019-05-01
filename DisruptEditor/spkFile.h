#pragma once

#include "sbaoFile.h"
#include <memory>

class IBinaryArchive;
class MemberStructure;

class spkFile {
public:
	void open(IBinaryArchive &fp);
	void registerMembers(MemberStructure &ms);

	sbaoFile& getSbao(uint32_t resId);

	Vector<sbaoFile> objs;
};

