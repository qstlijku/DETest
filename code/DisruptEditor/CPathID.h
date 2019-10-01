#pragma once

#include <string>

class MemberStructure;
class IBinaryArchive;

class CPathID {
public:
	CPathID() {}
	CPathID(uint32_t _id) : id(_id) {}
	CPathID(const std::string& filename);
	CPathID(const char *filename);

	std::string getReverseFilename();
	void read(IBinaryArchive& fp);
	void registerMembers(MemberStructure &ms);

	uint32_t id = -1;
};

static bool operator==(const CPathID& lhs, const CPathID& rhs) { 
	return lhs.id == rhs.id;
}

namespace std {
	template<> struct hash<CPathID> {
		typedef CPathID argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const noexcept {
			return std::hash<uint32_t>{}(s.id);
		}
	};
}
