#include "CResourceDataBase.h"

#include "Serialization.h"

void CResourceDataBase::open(IBinaryArchive& fp) {
	fp.padding = fp.PADDING_NONE;
	fp.serializeNdVectorExternal(unk1);
	fp.serializeNdVectorExternal(files);
	fp.serializeNdVectorExternal_pod(unk3);
	fp.serializeNdVectorExternal(types);
}

void CResourceDataBase::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(files);
	REGISTER_MEMBER(unk3);
	REGISTER_MEMBER(types);
}

void CResourceDataBase::Unk1::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
}

void CResourceDataBase::Unk1::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(unk1);
	REGISTER_MEMBER(unk2);
	REGISTER_MEMBER(unk3);
}
