#pragma once

#include "IBinaryArchive.h"
#include "Serialization.h"
#include <array>
#include "DDRenderInterface.h"

class SplineLoftHiRes {
public:
	uint32_t unk1;
	std::shared_ptr<VertexBuffer> vertexBuffer;
	std::shared_ptr<IndexBuffer> indexBuffer;

	void open(IBinaryArchive& fp);
	void draw();
};

class LoftShape {
public:
	//
	struct Loft {

	};

	struct Lods {

	};

	struct Lodd {

	};

	struct Side {

	};
	
	struct Mtrl {

	};

	void open(IBinaryArchive& fp);
	void draw();
};
