#include "RoadNetwork.h"


void RoadNetwork::read(IBinaryArchive& fp) {
	fp.serialize(header);

	//CRoadNetworkManager::SerializeRoadNetworkLowRes
	//SerializeRoadNetworkVehicleDensitySelectors
	fp.serializeNdVector(densities);
}

void RoadNetwork::CVehicleDensitySelector::read(IBinaryArchive& fp) {
	fp.serialize(TrafficPatternSelectorRef);
}

void RoadNetwork::SRoadResourceLowResHeader::read(IBinaryArchive& fp) {
	fp.serialize(magic);
	fp.serialize(version);
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
}
