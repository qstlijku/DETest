#pragma once

#include "IBinaryArchive.h"
#include "DDRenderInterface.h"

class xbtFile : public Texture {
public:
	uint32_t offset;

	bool open(IBinaryArchive &reader);
};

