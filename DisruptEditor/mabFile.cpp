/*

Copyright 2019 Jonathan Scott
All rights reserved
You may not use this file without permission

*/
#include "mabFile.h"

#include "IBinaryArchive.h"

void mabFile::open(IBinaryArchive & fp) {
	//AnimDataHeader
	uint32_t magic = 12955;
	fp.serialize(magic);
	SDL_assert_release(magic == 12955);

	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);

	fp.serialize(unk4);
}
