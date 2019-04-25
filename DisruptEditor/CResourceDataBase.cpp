#include "CResourceDataBase.h"

void CResourceDataBase::open(IBinaryArchive& fp) {
	fp.padding = fp.PADDING_NONE;
	fp.serializeNdVectorExternal(unk1);
	fp.serializeNdVectorExternal(files);
	fp.serializeNdVectorExternal_pod(unk3);
	fp.serializeNdVectorExternal(types);
}

void CResourceDataBase::Unk1::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
}
