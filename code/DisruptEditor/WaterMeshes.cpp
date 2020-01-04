#include "WaterMeshes.h"

#include "IBinaryArchive.h"

void WaterMeshes::open(IBinaryArchive& fp) {
	fp.bigEndian = true;
	fp.padding = fp.PADDING_NONE;

	uint32_t magic = 3;//Big endian 3
	fp.serialize(magic);
	SDL_assert_release(magic == 3);

	fp.serialize(unk1);
	fp.serialize(unk2);

	fp.serializeNdVector(indexes);
	fp.serializeNdVector(meshes);
}

void WaterMeshes::WaterMeshIndexBuffer::read(IBinaryArchive& fp) {
	uint32_t size = data.size() * 2;
	fp.serialize(size);
	data.resize(size / 2);
	for (uint32_t i = 0; i < size / 2; ++i)
		fp.serialize(data[i]);
}

void WaterMeshes::WaterMesh::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
}
