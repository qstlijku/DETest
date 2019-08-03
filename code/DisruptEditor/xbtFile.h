#pragma once

#include "IBinaryArchive.h"

class xbtFile {
public:
	bool open(IBinaryArchive &reader);
	void bind(int slot);
	bool loaded = false;
};

