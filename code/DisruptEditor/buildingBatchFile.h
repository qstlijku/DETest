#pragma once

#include "batchFile.h"

class buildingBatchFile {
public:
	struct SGfxModelInfo {
		CGeometryResource geomResource;
		uint32_t unk1;
		uint32_t unk2;
		CMaterialSlotsMap materialSlots;
		uint16_t NbFacadeWindows;
		void read(IBinaryArchive& fp);
	};

	struct FacadeGfxModels {
		uint32_t unk1;
		std::vector<SGfxModelInfo> models;
		std::vector<uint8_t> unk2;
		void read(IBinaryArchive& fp);
	};

	struct CStaticFacadeCluster {
		glm::vec2 unk1;
		glm::vec3 unk2;
		glm::vec3 unk3;
		glm::vec3 unk4;
		float unk5;
		uint32_t unk6;
		ClusterData data;

		void read(IBinaryArchive& fp);
	};

	struct CHiResBuildingData {
		uint32_t unk1;
		uint32_t unk2;
		std::vector<CStaticFacadeCluster> facades;
		void read(IBinaryArchive& fp);
	};

	batchFile::batchHeader head;
	uint32_t unk1;
	CPathID compoundParent;
	std::vector<CResourceContainer> resources;
	FacadeGfxModels facadeGfxModels;
	uint32_t unk2;
	std::vector<CHiResBuildingData> buildingData;

	void open(IBinaryArchive& reader);
};
