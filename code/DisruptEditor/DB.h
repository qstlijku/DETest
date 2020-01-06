#pragma once

#include <string>
#include <memory>
#include <map>
#include <unordered_map>

#include "CDobbsID.h"
#include "CPathID.h"
#include "CStringID.h"

struct NodeEntry;

class NodeEntryCollection : public std::map<std::string, NodeEntry> {
public:
	void AddEntry(std::string sEntry, int wBegIndex = 0);
};

struct NodeEntry {
	std::string Key;
	NodeEntryCollection Children;
};

class DB {
public:
	DB();
	~DB();

	std::string getFileByHash(CPathID hash);
	std::string getStrFromCRC(CStringID hash);
	std::string getStrFromDobbs(CDobbsID hash);

	uint32_t getSpkFromSBAO(uint32_t resID);

	void reinit();
	static DB& instance();

	NodeEntryCollection root;
	std::unordered_map<CStringID, std::string> crcList;
	std::unordered_map<CDobbsID, std::string> dobbsList;
	std::unordered_map<CPathID, std::string> fnvList;
	std::unordered_map<uint32_t, uint32_t> dareBaoList;
	void handleCRCFile(const char* file, const char* type);
	void handleFNVFile(const char* file);
	void addFNVEntry(const char* file);
};

