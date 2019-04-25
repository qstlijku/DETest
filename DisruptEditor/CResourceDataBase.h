#pragma once

#include "IBinaryArchive.h"
#include "CStringID.h"
#include "CPathID.h"

class CResourceDataBase {
public:
	struct Unk1 {
		uint32_t unk1;
		uint32_t unk2;
		uint32_t unk3;
		void read(IBinaryArchive& fp);
	};
	Vector<Unk1> unk1;
	Vector<CPathID> files;
	Vector<uint8_t> unk3;
	Vector<CStringID> types;
	void open(IBinaryArchive& fp);
};

