#pragma once

#include "CResourceDataBase.h"

class IBinaryArchive;
class MemberStructure;

class embedFile {
public:
	void open(IBinaryArchive &fp);
	void registerMembers(MemberStructure& ms);

	struct Unk1 {
		uint32_t unk1;
		uint32_t unk2;
		uint32_t unk3;
		uint32_t unk4;
		uint16_t unk5;
		void read(IBinaryArchive& fp);
		void registerMembers(MemberStructure& ms);
	};
	Vector<Unk1> unk1;
	CResourceDataBase db;
};

