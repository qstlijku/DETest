#pragma once

#include <memory>
#include "CPathID.h"

class xbgFile;
class materialFile;
class xbtFile;
class xbt3File;
class SplineLoftHiRes;
class SplineLoftLowRes;
class batchFile;
class buildingBatchFile;
class realTreeFile;

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

std::shared_ptr<xbt3File> loadTexture3(const char *path);

std::shared_ptr<SplineLoftHiRes> loadHiResSplineLoft(CPathID path);

std::shared_ptr<SplineLoftLowRes> loadLowResSplineLoft(CPathID path);

std::shared_ptr<batchFile> loadBatchFile(CPathID path);

std::shared_ptr<buildingBatchFile> loadBuildingBatchFile(CPathID path);

std::shared_ptr<realTreeFile> loadRealTree(CPathID path);
