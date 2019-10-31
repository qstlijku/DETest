#pragma once

#include <string>

class IBinaryArchive;
class MemberStructure;

//SerializerLabel
class CDobbsID {
public:
	CDobbsID() {}
	CDobbsID(uint32_t _id) : id(_id) {}
	CDobbsID(const std::string& filename);
	CDobbsID(const char* filename);

	std::string getReverseName();
	void read(IBinaryArchive& fp);
	void registerMembers(MemberStructure& ms);

	uint32_t id = -1;
};

static bool operator==(const CDobbsID& lhs, const CDobbsID& rhs) {
	return lhs.id == rhs.id;
}

namespace std {
	template<> struct hash<CDobbsID> {
		typedef CDobbsID argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const noexcept {
			return std::hash<uint32_t>{}(s.id);
		}
	};
}

