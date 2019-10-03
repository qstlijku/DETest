#include "CDobbsID.h"

#include "Serialization.h"
#include "Hash.h"
#include "DB.h"

CDobbsID::CDobbsID(const std::string &filename) {
	id = Hash::gearDobbsHash((const uint8_t*)filename.c_str(), filename.size());
}

std::string CDobbsID::getReverseName() {
	return DB::instance().getStrFromDobbs(id);
}

void CDobbsID::registerMembers(MemberStructure & ms) {
	switch (ms.type) {
	case MemberStructure::TOXML: {
		std::string temp = getReverseName();
		ms.registerMember(NULL, temp);
		break;
	}
	case MemberStructure::FROMXML: {
		std::string temp;
		ms.registerMember(NULL, temp);
		if (temp[0] == '_')
			sscanf(temp.c_str(), "_%08x", &id);
		else
			id = Hash::gearDobbsHash((const uint8_t*)temp.c_str(), temp.size());
		break;
	}
	default:
		ms.registerMember(NULL, id);
	}
}
