#pragma once

#include "glad.h"
#include "IBinaryArchive.h"

class xbtFile {
public:
	bool open(IBinaryArchive &reader);
	void bind(int slot);
	bool loaded = false;
private:
	GLuint id;
};

