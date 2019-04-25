#pragma once

#include "CStringID.h"
#include "CPathID.h"
#include "IBinaryArchive.h"

class MemberStructure;

class CResourceDataBase {
public:
	struct Unk1 {
		uint32_t unk1;
		uint32_t unk2;
		uint32_t unk3;
		void read(IBinaryArchive& fp);
		void registerMembers(MemberStructure& ms);
	};
	Vector<Unk1> unk1;
	Vector<CPathID> files;
	Vector<uint8_t> unk3;
	Vector<CStringID> types;
	void open(IBinaryArchive& fp);
	void registerMembers(MemberStructure& ms);
};

