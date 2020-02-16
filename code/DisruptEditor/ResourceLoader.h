#pragma once

#include <memory>
#include "CPathID.h"

class xbgFile;
class materialFile;
class xbtFile;
class SplineLoftHiRes;
class batchFile;
class buildingBatchFile;
class realTreeFile;
class batchCollisionFile;
class physResourceFile;

class ResourceFile {
public:
	virtual bool read(IBinaryArchive& fp) = 0;

	void SetPath(CPathID p);
	void SetPathStr(const std::string &p);
	CPathID GetPath();
	std::string GetPathStr();
private:
	CPathID path = -1;
	std::string pathStr;
};

std::shared_ptr<xbgFile> loadXBG(CPathID path);

std::shared_ptr<materialFile> loadMaterial(CPathID path);

std::shared_ptr<xbtFile> loadTexture(CPathID path);

std::shared_ptr<SplineLoftHiRes> loadHiResSplineLoft(CPathID path);

std::shared_ptr<batchFile> loadbatchFile(CPathID path);

std::shared_ptr<buildingBatchFile> loadBuildingBatchFile(CPathID path);

std::shared_ptr<batchCollisionFile> loadCollisionBatchFile(CPathID path);

std::shared_ptr<physResourceFile> loadPhysResourceFile(CPathID path);

std::shared_ptr<realTreeFile> loadRealTree(CPathID path);
