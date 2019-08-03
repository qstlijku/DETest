#include "xbtFile.h"

#include "Vector.h"
#include "FileHandler.h"
#include <SDL_rwops.h>

bool xbtFile::open(IBinaryArchive& reader) {
	SDL_RWops* fp = reader.fp;
	//Seek past xbt header
	SDL_RWseek(fp, 8, RW_SEEK_CUR);
	int32_t ddsOffset = SDL_ReadLE32(fp);
	SDL_RWseek(fp, ddsOffset, RW_SEEK_SET);



	return true;
}

void xbtFile::bind(int slot) {
	if (!loaded) return;
}
