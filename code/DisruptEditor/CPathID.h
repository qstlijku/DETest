#pragma once

#include <string>
#include "Compat.h"

class MemberStructure;
class IBinaryArchive;

#if WD2 || WD3
typedef uint64_t CPathIDType;
#else
typedef uint32_t CPathIDType;
#endif

class CPathID {
public:
	CPathID() {}

	CPathID(CPathIDType _id) : id(_id) {}
	CPathID(const std::string& filename);
	CPathID(const char *filename);

	std::string getReverseFilename();
	void read(IBinaryArchive& fp);
	void registerMembers(MemberStructure &ms);

	CPathIDType id = -1;
};

static bool operator==(const CPathID& lhs, const CPathID& rhs) { 
	return lhs.id == rhs.id;
}

namespace std {
	template<> struct hash<CPathID> {
		typedef CPathID argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const noexcept {
			return std::hash<CPathIDType>{}(s.id);
		}
	};
}
