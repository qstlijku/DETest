#include "DB.h"

#include "Hash.h"
#include "SDL.h"
#include "FileHandler.h"

void NodeEntryCollection::AddEntry(std::string sEntry, int wBegIndex) {
	if (wBegIndex < sEntry.size()) {
		int wEndIndex = sEntry.find("\\", wBegIndex);
		if (wEndIndex == std::string::npos)
			wEndIndex = sEntry.size();
		std::string sKey = sEntry.substr(wBegIndex, wEndIndex - wBegIndex);
		if (!sKey.empty()) {
			NodeEntry& oItem = (*this)[sKey];

			if (oItem.Key.empty())
				oItem.Key = sKey;

			// Now add the rest to the new item's children
			oItem.Children.AddEntry(sEntry, wEndIndex + 1);
		}
	}
}

std::string base = SDL_GetBasePath();

DB::DB() {
	reinit();
}

DB::~DB() {
}

std::string DB::getFileByHash(CPathID hash) {
	auto it = fnvList.find(hash);
	if (it != fnvList.end())
		return it->second;

	char buffer[12];
	snprintf(buffer, sizeof(buffer), "_%08x", hash);
	return std::string(buffer);
}

std::string DB::getStrFromCRC(CStringID hash) {
	auto it = crcList.find(hash);
	if (it != crcList.end())
		return it->second;

	char buffer[12];
	snprintf(buffer, sizeof(buffer), "_%08x", hash);
	return std::string(buffer);
}

std::string DB::getStrFromDobbs(CDobbsID hash) {
	auto it = dobbsList.find(hash);
	if (it != dobbsList.end())
		return it->second;

	char buffer[12];
	snprintf(buffer, sizeof(buffer), "_%08x", hash);
	return std::string(buffer);
}

uint32_t DB::getSpkFromSBAO(uint32_t resID) {
	auto it = dareBaoList.find(resID);
	if (it != dareBaoList.end())
		return it->second;
	return -1;
}

void DB::reinit() {
	//Reset lists
	dobbsList.clear();
	crcList.clear();
	fnvList.clear();
	dareBaoList.clear();
	root.clear();

	//Fill with Known files
	handleFNVFile((base + "res/Watch Dogs.filelist").c_str());

	//Fill with known FNV
	handleFNVFile((base + "res/arches.txt").c_str());

	handleFNVFile((base + "res/archeBrute.txt").c_str());

	handleCRCFile((base + "res/classNames.txt").c_str(), "ClassNames");
	handleCRCFile((base + "res/exeStrings.txt").c_str(), "etc");
	handleCRCFile((base + "res/strings.txt").c_str(), "etc");
	handleCRCFile((base + "res/materialNames.txt").c_str(), "Material");

	char line[64];
	FILE *fp = fopen((base + "res/dare.txt").c_str(), "r");
	while (fgets(line, sizeof(line), fp)) {
		line[strlen(line) - 1] = '\0';
		uint32_t spk, sbao;
		sscanf(line, "%u,%u", &spk, &sbao);
		dareBaoList[sbao] = spk;
	}
	fclose(fp);
}

DB & DB::instance() {
	static DB db;
	return db;
}

void DB::handleCRCFile(const char *file, const char* type) {
	FILE *fp = fopen(file, "r");

	char line[512];
	while (fgets(line, sizeof(line), fp)) {
		line[strlen(line) - 1] = '\0';

		CStringID strID(line);
		crcList[strID] = line;

		CDobbsID dobbsID(line);
		dobbsList[dobbsID] = line;
	}

	fclose(fp);
}

void DB::handleFNVFile(const char* file) {
	FILE* fp = fopen(file, "r");

	char line[512];
	while (fgets(line, sizeof(line), fp)) {
		line[strlen(line) - 1] = '\0';

		CPathID path(line);
		fnvList[path] = line;

		if(FH::fileExists(line))
			root.AddEntry(line);
	}

	fclose(fp);
}
