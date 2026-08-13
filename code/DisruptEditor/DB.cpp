#include "DB.h"

#include "Hash.h"
#include "SDL.h"
#include "FileHandler.h"
#include <filesystem>
#include <Common.h>
#include <future>

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

	char buffer[20];
#if WD2 || WD3
	snprintf(buffer, sizeof(buffer), "_%16x", hash.id);
#else
	snprintf(buffer, sizeof(buffer), "_%08x", hash.id);
#endif
	return std::string(buffer);
}

std::string DB::getStrFromCRC(CStringID hash) {
	auto it = crcList.find(hash);
	if (it != crcList.end())
		return it->second;

	char buffer[12];
	snprintf(buffer, sizeof(buffer), "_%08x", hash.id);
	return std::string(buffer);
}

std::string DB::getStrFromDobbs(CDobbsID hash) {
	auto it = dobbsList.find(hash);
	if (it != dobbsList.end())
		return it->second;

	char buffer[12];
	snprintf(buffer, sizeof(buffer), "_%08x", hash.id);
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
	fileTypes.clear();
	dareBaoList.clear();
	root.clear();

	crcList.reserve(275000);
	fnvList.reserve(275000);
	dareBaoList.reserve(150000);

	std::future<void> futureFNV = std::async([&]() {
		//Fill with Known files
		handleFNVFile((base + "res/Watch Dogs.filelist").c_str());

		//Fill with known FNV
		handleFNVFile((base + "res/arches.txt").c_str());
		handleFNVFile((base + "res/archeBrute.txt").c_str());
	});

	std::future<void> futureCRC = std::async([&]() {
		handleCRCFile((base + "res/classNames.txt").c_str(), "ClassNames");
		handleCRCFile((base + "res/exeStrings.txt").c_str(), "etc");
		handleCRCFile((base + "res/strings.txt").c_str(), "etc");
		handleCRCFile((base + "res/materialNames.txt").c_str(), "Material");
		handleCRCFile((base + "res/bones1.txt").c_str(), "bones");
		handleCRCFile((base + "res/bones2.txt").c_str(), "bones");
		handleCRCFile((base + "res/bones3.txt").c_str(), "bones");
	});

	//Load Dare
	std::future<void> futureDARE = std::async([&]() {
		char line[64];
		FILE* fp = fopen((base + "res/dare.txt").c_str(), "r");
		while (fgets(line, sizeof(line), fp)) {
			line[strlen(line) - 1] = '\0';
			uint32_t spk, sbao;
			sscanf(line, "%u,%u", &spk, &sbao);
			dareBaoList[sbao] = spk;
		}
		fclose(fp);
	});

	//Load Dobbs
	{
		const char* dobbsNames[] = {
			"ResourceDescriptor",
			"PlayEventDescriptor",
			"MultiEventDescriptor",
			"PresetDescriptor",
			"PresetEventDescriptor",
			"StopEventDescriptor",
			"RemovePresetEventDescriptor",
			"ProjectDesc",
			"RolloffResourceDescriptor",
			"EmitterSpec",
			"ChangeVolumeEventDescriptor",
			"StopNGoEventDescriptor",
			"SwitchEventDescriptor",
			"SndData",
			"SampleResourceDescriptor",
			"RandomResourceDescriptor",
			"SilenceResourceDescriptor",
			"MultiLayerResourceDescriptor",
			"SequenceResourceDescriptor",
			"MultiTrackResourceDescriptor",
			"ThemeResourceDescriptor",
			"GranularResourceDescriptor",
			"SwitchResourceDescriptor",
			"ThemePartOutroDescriptor",
		};
		for (const char* name : dobbsNames) {
			CDobbsID id(name);
			dobbsList[id] = name;
		}
	}

	//Scan the Patch Dir
	try {
		for (auto& p : std::filesystem::recursive_directory_iterator(settings.patchDir)) {
			if (p.is_regular_file()) {
				std::string path = p.path().generic_string();
				std::replace(path.begin(), path.end(), '/', '\\');
				addFNVEntry(path.c_str() + settings.patchDir.size());
			}
		}
	} catch (...) {}

	futureFNV.get();
	futureCRC.get();
	futureDARE.get();
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
		if (line[0] == 0) continue;

		CStringID strID(line);
		crcList[strID] = line;
	}

	fclose(fp);
}

void DB::handleFNVFile(const char* file) {
	FILE* fp = fopen(file, "r");

	char line[512];
	while (fgets(line, sizeof(line), fp)) {
		line[strlen(line) - 1] = '\0';
		addFNVEntry(line);
	}

	fclose(fp);
}

void DB::addFNVEntry(const char* file) {
	CPathID hash(file);
	addFNVEntry(hash, file);
}

void DB::addFNVEntry(CPathID hash, const char* file) {
	if (FH::fileExists(file)) {
		fnvList[hash] = file;
		root.AddEntry(file);
	}
}
