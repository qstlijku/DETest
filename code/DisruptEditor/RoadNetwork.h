#pragma once

#include "IBinaryArchive.h"

class RoadNetwork {
public:
	struct SRoadResourceLowResHeader {
		uint32_t magic;//12
		uint32_t version;

		uint32_t unk1;//Must not be 0
		uint32_t unk2;
		uint32_t unk3;
		uint32_t unk4;
		uint32_t unk5;
		uint32_t unk6;

		void read(IBinaryArchive& fp);
	};
	SRoadResourceLowResHeader header;

	struct CVehicleDensitySelector {
		uint32_t TrafficPatternSelectorRef;
		void read(IBinaryArchive& fp);
	};

	std::vector<CVehicleDensitySelector> densities;

	void read(IBinaryArchive& fp);
};

