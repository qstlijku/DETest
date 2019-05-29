#pragma once

#include <vector>

class IBinaryArchive;

class WaterMeshes {
public:

	struct WaterMeshIndexBuffer {
		std::vector<uint16_t> data;
		void read(IBinaryArchive& fp);
	};

	struct WaterMesh {
		uint16_t unk1;
		uint32_t unk2;
		uint32_t unk3;
		uint32_t unk4;
		void read(IBinaryArchive& fp);
	};

	uint32_t unk1;
	uint32_t unk2;
	std::vector<WaterMeshIndexBuffer> indexes;
	std::vector<WaterMesh> meshes;

	void open(IBinaryArchive& fp);
};