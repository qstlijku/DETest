#pragma once

#include <string>
#include "Compat.h"

class MemberStructure;
class IBinaryArchive;

class CStringID {
public:
	CStringID() {}
	CStringID(uint32_t _id) : id(_id) {}
	CStringID(const std::string &filename);
	CStringID(const char* filename);

	std::string getReverseName();
	void read(IBinaryArchive& fp);
	void registerMembers(MemberStructure& ms);

	uint32_t id = -1;
};

static bool operator==(const CStringID& lhs, const CStringID& rhs) {
	return lhs.id == rhs.id;
}

namespace std {
	template<> struct hash<CStringID> {
		typedef CStringID argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const noexcept {
			return std::hash<uint32_t>{}(s.id);
		}
	};
}
