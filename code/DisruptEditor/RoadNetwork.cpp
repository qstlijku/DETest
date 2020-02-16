#include "RoadNetwork.h"


void RoadNetwork::read(IBinaryArchive& fp) {
	header.numRoadSegments = segments.size();

	fp.serialize(header);

	fp.markHeader();
	fp.PauseInPlace();

	//CRoadNetworkManager::SerializeRoadNetworkLowRes
	//SerializeRoadNetworkVehicleDensitySelectors
	//fp.serializeNdVectorExternal(densities); //Only on WiiU Version

	fp.PreAllocateSizeOfType("CRoadSegment", header.numRoadSegments);//0xc
	fp.PreAllocateSizeOfType("CRoadIntersection", header.numRoadIntersections);//0x10

	segments.resize(header.numRoadSegments);
	for (uint32_t i = 0; i < header.numRoadSegments; ++i) {
		fp.serialize(segments[i]);
		if (header.hasHiRes) {
			segments[i].readHiRes(fp);
		}
	}


	//CRoadNetworkManager::SerializeRoadNetworkRoadGrid
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.PreAllocateSizeOfType(0xc88a15ce, unk6);

	fp.finish();
}

void RoadNetwork::CVehicleDensitySelector::read(IBinaryArchive& fp) {
	fp.serialize(TrafficPatternSelectorRef);
}

void RoadNetwork::SRoadResourceLowResHeader::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(size);
	fp.serialize(numRoadSegments);
	fp.serialize(numRoadIntersections);
	fp.serialize(hasHiRes);
	fp.serialize(unk5);
	fp.serialize(unk6);
}

void RoadNetwork::CRoadSegment::read(IBinaryArchive& fp) {
	fp.serialize(*(IRoadSegment*)this);
	fp.serialize(lane);
}

void RoadNetwork::IRoadSegment::read(IBinaryArchive& fp) {
	fp.serialize(sphere);
	fp.serialize(unk1);
	fp.serialize(unk2);
}

void RoadNetwork::IRoadSegment::readHiRes(IBinaryArchive& fp) {
	/*fp.serialize(unk3);
	if (unk3 == 0) {
		fp.serialize(type);
		fp.PreAllocateDynamicType(type);

		fp.serialize(unk4);
		fp.serialize(unk5);
		fp.serialize(unk6);
		fp.serializeNdVectorExternal(lanes);
	}*/
}

void RoadNetwork::CRoadLane::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	if (unk1 == 0)
		fp.serialize(type);
}
