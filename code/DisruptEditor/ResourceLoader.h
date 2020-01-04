#pragma once

#include <memory>
#include "CPathID.h"

class xbgFile;
class materialFile;
class xbtFile;
class SplineLoftHiRes;
class batchFile;
class buildingBatchFile;

std::shared_ptr<xbgFile> loadXBG(CPathID path);

std::shared_ptr<materialFile> loadMaterial(CPathID path);

std::shared_ptr<xbtFile> loadTexture(CPathID path);

std::shared_ptr<SplineLoftHiRes> loadHiResSplineLoft(CPathID path);

std::shared_ptr<batchFile> loadbatchFile(CPathID path);

std::shared_ptr<buildingBatchFile> loadBuildingBatchFile(CPathID path);
