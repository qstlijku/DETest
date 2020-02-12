#include "xbtFile.h"

#include "Vector.h"
#include "FileHandler.h"
#include <SDL_rwops.h>
#include "DDSTextureLoader.h"

bool xbtFile::open(IBinaryArchive& reader) {
	reader.padding = reader.PADDING_NONE;
	reader.serializeConstant<uint32_t>(5784148);//magic
	reader.serializeConstant<uint32_t>(123);//version

	reader.serialize(offset);

	SDL_RWops* fp = reader.fp;
	SDL_RWseek(fp, offset, RW_SEEK_SET);

	std::vector<uint8_t> data(SDL_RWsize(fp) - offset);
	SDL_RWread(fp, data.data(), 1, data.size());

	HRESULT hr = DirectX::CreateDDSTextureFromMemory(RenderInterface::instance().g_pd3dDevice.Get(), data.data(), data.size(), &pTexture, &pResource);
	SDL_assert_release(hr == S_OK);

	return true;
}
