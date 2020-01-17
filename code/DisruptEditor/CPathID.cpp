#include "CPathID.h"

#include "FileHandler.h"
#include "Hash.h"
#include "Serialization.h"
#include "IBinaryArchive.h"
#include "DB.h"

CPathID::CPathID(const std::string &filename) : CPathID(filename.c_str()) { }

CPathID::CPathID(const char *filename) {
#if WD2 || WD3
	id = Hash::getFilenameHash64(filename);
#else
	id = Hash::getFilenameHash(filename);
#endif
}

std::string CPathID::getReverseFilename() {
	return DB::instance().getFileByHash(*this);
}

void CPathID::read(IBinaryArchive& fp) {
	fp.serialize(id);
}

void CPathID::registerMembers(MemberStructure & ms) {
	switch (ms.type) {
	case MemberStructure::TOXML: {
		std::string temp = getReverseFilename();
		ms.registerMember(NULL, temp);
		break;
	}
	case MemberStructure::FROMXML: {
		std::string temp;
		ms.registerMember(NULL, temp);
		if (temp[0] == '_')
#if WD2 || WD3
			sscanf(temp.c_str(), "_%16x", &id);
#else
			sscanf(temp.c_str(), "_%08x", &id);
#endif
		else
			id = Hash::getFilenameHash(temp);
		break;
	}
	default:
		ms.registerMember(NULL, id);
	}
}
