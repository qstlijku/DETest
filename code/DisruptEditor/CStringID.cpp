#include "CStringID.h"

#include "Hash.h"
#include "DB.h"
#include "IBinaryArchive.h"
#include "Serialization.h"

CStringID::CStringID(const std::string &filename) {
	if (filename.empty())
		id = -1;
	else
		id = Hash::getHash(filename.c_str());
}

std::string CStringID::getReverseName() {
	return DB::instance().getStrFromCRC(id);
}

void CStringID::read(IBinaryArchive& fp) {
	fp.serialize(id);
}

void CStringID::registerMembers(MemberStructure & ms) {
	switch (ms.type) {
	case MemberStructure::TOXML:
		ms.registerMember(NULL, getReverseName());
		break;
	case MemberStructure::FROMXML: {
		std::string temp;
		ms.registerMember(NULL, temp);
		if (temp[0] == '_')
			sscanf(temp.c_str(), "_%08x", &id);
		else
			id = Hash::getHash(temp.c_str());
		break;
	}
	default:
		ms.registerMember(NULL, id);
	}
}
