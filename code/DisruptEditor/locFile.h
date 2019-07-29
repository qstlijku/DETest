#pragma once

#include <map>
#include <string>

class IBinaryArchive;
class MemberStructure;

class locFile {
public:
	void open(IBinaryArchive &fp);

	void registerMembers(MemberStructure& ms);
	int16_t language;
	std::map<uint32_t, std::wstring> uncompiledStrings;
};

