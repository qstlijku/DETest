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
		CPathID unk3;
		void read(IBinaryArchive& fp);
		void registerMembers(MemberStructure& ms);
	};
	Vector<Unk1> unk1;
	struct ResourceFile {
		CPathID file;
		uint8_t refType = 0;
		void registerMembers(MemberStructure& ms);
	};
	Vector<ResourceFile> files;
	Vector<CStringID> types;
	void open(IBinaryArchive& fp);
	void registerMembers(MemberStructure& ms);
};

