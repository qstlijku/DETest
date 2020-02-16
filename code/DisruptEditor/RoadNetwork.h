#pragma once

#include "IBinaryArchive.h"
#include <memory>
#include "DisruptTypes.h"
#include "CStringID.h"

class RoadNetwork {
public:
	struct SRoadResourceLowResHeader {
		uint32_t unk1;
		uint32_t unk2;
		uint32_t size;
		uint32_t numRoadSegments;
		uint32_t numRoadIntersections;
		uint32_t hasHiRes;
		uint32_t unk5;
		uint32_t unk6;

		void read(IBinaryArchive& fp);
	};
	SRoadResourceLowResHeader header;

	//Unused
	struct CVehicleDensitySelector {
		uint32_t TrafficPatternSelectorRef;
		void read(IBinaryArchive& fp);
	};
	std::vector<std::unique_ptr<CVehicleDensitySelector>> densities;

	struct CRoadLane {
		uint32_t unk1;
		CStringID type;
		void read(IBinaryArchive& fp);
		void readHiRes(IBinaryArchive& fp);
	};

	struct IRoadSegment {
		CSphere sphere;
		uint32_t unk1;
		uint32_t unk2;

		//HiResData
		uint32_t unk3;
		CStringID type;

		float unk4;
		uint8_t unk5;
		uint8_t unk6;
		std::vector<CRoadLane*> lanes;

		void read(IBinaryArchive& fp);
		void readHiRes(IBinaryArchive& fp);
	};
	struct CRoadSegment : public IRoadSegment {//Inherits IRoadSegment
		void read(IBinaryArchive& fp);
		CRoadLane lane;
	};
	std::vector<CRoadSegment> segments;

	//RoadNetworkRoadGrid
	glm::vec2 unk1;
	glm::vec2 unk2;
	glm::vec2 unk3;
	glm::ivec2 unk4;
	glm::vec2 unk5;
	uint32_t unk6;


	void read(IBinaryArchive& fp);
};

