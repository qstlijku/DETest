#include "buildingBatchFile.h"

void buildingBatchFile::open(IBinaryArchive& reader) {
	reader.serialize(head);
	reader.markHeader();
	reader.serialize(unk1);
	reader.serialize(compoundParent);
	reader.serializeNdVector(resources);
	reader.serialize(facadeGfxModels);
	reader.serializeNdVectorExternal(buildingData, CStringID("CHiResBuildingData").id, unk2);
}

void buildingBatchFile::FacadeGfxModels::read(IBinaryArchive& fp) {
	fp.serializeNdVectorExternal(models, CStringID("SGfxModelInfo").id, unk1);

	CStringID CSceneGraphicObjectClusterHelper("CSceneGraphicObjectClusterHelper");
	fp.serialize(CSceneGraphicObjectClusterHelper);
	SDL_assert_release(CSceneGraphicObjectClusterHelper == CStringID("CSceneGraphicObjectClusterHelper"));

	fp.serializeNdVector_pod(unk2);
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
	fp.serialize(data);
}

void buildingBatchFile::CHiResBuildingData::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serializeNdVectorExternal(facades, CStringID("CStaticFacadeCluster").id, unk2);
}
