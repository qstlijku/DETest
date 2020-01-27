#include "buildingBatchFile.h"

void buildingBatchFile::open(IBinaryArchive& reader) {
	reader.serialize(head);
	reader.markHeader();
	reader.markInPlaceOffset(reader.header.unk2);
	reader.serialize(unk1);
	reader.serialize(compoundParent);
	reader.serializeNdVector(resources);
	reader.serialize(facadeGfxModels);
	reader.serializeNdVectorExternal2(buildingData, "CHiResBuildingData");
	reader.finish();
}

void buildingBatchFile::FacadeGfxModels::read(IBinaryArchive& fp) {
	fp.serializeNdVectorExternal2(models, "SGfxModelInfo");

	fp.PreAllocateSizeOfType("CSceneGraphicObjectClusterHelper", models.size());
}

void buildingBatchFile::SGfxModelInfo::read(IBinaryArchive& fp) {
	fp.serialize(geomResource);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(materialSlots);
	fp.serialize(NbFacadeWindows);
}

void buildingBatchFile::CStaticFacadeCluster::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(data.format);
	fp.serialize(data);
}

void buildingBatchFile::CHiResBuildingData::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serializeNdVectorExternal2(facades, "CStaticFacadeCluster");
}
