#pragma once

#include "IBinaryArchive.h"
#include "DDRenderInterface.h"

class xbtFile : public Texture {
public:
	bool open(IBinaryArchive &reader);
	bool loaded = false;
};

