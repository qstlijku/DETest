#include "xbtFile.h"

#include "Vector.h"
#include "FileHandler.h"
#include <SDL_rwops.h>
#include "DDSTextureLoader.h"

bool xbtFile::open(IBinaryArchive& reader) {
	SDL_RWops* fp = reader.fp;
	//Seek past xbt header
	SDL_RWseek(fp, 8, RW_SEEK_CUR);
	int32_t ddsOffset = SDL_ReadLE32(fp);
	SDL_RWseek(fp, ddsOffset, RW_SEEK_SET);

	std::vector<uint8_t> data(SDL_RWsize(fp) - ddsOffset);
	SDL_RWread(fp, data.data(), 1, data.size());

	HRESULT hr = DirectX::CreateDDSTextureFromMemory(RenderInterface::instance().g_pd3dDevice, data.data(), data.size(), &pTexture, &pResource);
	SDL_assert_release(hr == S_OK);

	return true;
}
