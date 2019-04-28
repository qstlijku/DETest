#pragma once

#include "NBCF.h"
#include <map>

class IBinaryArchive;

class CMoveResourceDataManager {
public:
	struct CMoveResourceData {

	};

	void open(IBinaryArchive& fp);

	std::map<uint32_t, Vector<uint8_t> > _f9706572;
	Node root;
};

