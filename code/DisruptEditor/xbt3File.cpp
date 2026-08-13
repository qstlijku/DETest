#include "xbt3File.h"

#include "Vector.h"
#include "FileHandler.h"
#include <SDL_rwops.h>
#include "DDSTextureLoader.h"

static void serializeString(IBinaryArchive& fp, std::string& str) {
	if (fp.isReading()) {
		str.clear();
		char it;
		do {
			fp.serialize(it);
			if (it != '\0')
				str.push_back(it);
		} while (it != '\0');
	} else {
		for (int i = 0; i < str.size(); ++i)
			fp.serialize(str[i]);
		fp.serializeConstant<char>('\0');
	}
	fp.pad(4);
}

bool xbt3File::open(IBinaryArchive& reader) {
	reader.padding = reader.PADDING_NONE;
	reader.serializeConstant<uint32_t>(5784148);//magic
	reader.serializeConstant<uint32_t>(104);//version

	uint32_t offset = 40;
	reader.serialize(offset);

	reader.serialize(unk);
	reader.serialize(unk4);
	reader.serialize(unk0);
	reader.serializeConstant<uint8_t>(0xFF);
	reader.serializeConstant<uint8_t>(0xFF);
	reader.serialize(unk1);
	reader.serialize(unk2);
	reader.serialize(unk3);

	serializeString(reader, mipFile);

	if (reader.isReading()) {
		//SDL_assert_release(offset == SDL_RWtell(reader.fp));
		SDL_RWseek(reader.fp, offset, RW_SEEK_SET);

		data.resize(SDL_RWsize(reader.fp) - offset);
		SDL_RWread(reader.fp, data.data(), 1, data.size());

		HRESULT hr = DirectX::CreateDDSTextureFromMemory(RenderInterface::instance().g_pd3dDevice.Get(), data.data(), data.size(), &pTexture, &pResource);
		SDL_assert_release(hr == S_OK);
	} else {
		offset = SDL_RWtell(reader.fp);
		reader.memBlock(data.data(), 1, data.size());

		//Go back and write offset
		SDL_RWseek(reader.fp, 8, RW_SEEK_SET);
		reader.serialize(offset);
		SDL_RWseek(reader.fp, 0, RW_SEEK_END);
	}

	return true;
}
