#include "DatFat.h"

#include <SDL.h>
#include <Windows.h>

//XCompress
typedef enum _XMEMCODEC_TYPE {
	XMEMCODEC_DEFAULT = 0,
	XMEMCODEC_LZX = 1
} XMEMCODEC_TYPE;
typedef void* XMEMDECOMPRESSION_CONTEXT;
typedef struct _XMEMCODEC_PARAMETERS_LZX {
	DWORD Flags;
	DWORD WindowSize;
	DWORD CompressionPartitionSize;
} XMEMCODEC_PARAMETERS_LZX;
typedef HRESULT (*XMemCreateDecompressionContext)(
	XMEMCODEC_TYPE                  CodecType,
	CONST VOID* pCodecParams,
	DWORD                           Flags,
	XMEMDECOMPRESSION_CONTEXT* pContext
);
static XMemCreateDecompressionContext _XMemCreateDecompressionContext;
typedef HRESULT (*XMemDecompress)(
	XMEMDECOMPRESSION_CONTEXT       Context,
	VOID* pDestination,
	SIZE_T* pDestSize,
	CONST VOID* pSource,
	SIZE_T                          SrcSize
);
static XMemDecompress _XMemDecompress;
typedef VOID (*XMemDestroyDecompressionContext)(
	XMEMDECOMPRESSION_CONTEXT       Context
);
static XMemDestroyDecompressionContext _XMemDestroyDecompressionContext;

void InitXCompress() {
	HMODULE hModule = LoadLibrary(L"res/xcompress64.dll");
	if (!hModule) {
		SDL_ShowSimpleMessageBox(0, "Error", "xcompress64.dll is missing from res/", NULL);
		exit(0);
	}

	_XMemCreateDecompressionContext = (XMemCreateDecompressionContext)GetProcAddress(hModule, "XMemCreateDecompressionContext");
	SDL_assert_release(_XMemCreateDecompressionContext);
	_XMemDecompress = (XMemDecompress)GetProcAddress(hModule, "XMemDecompress");
	SDL_assert_release(_XMemDecompress);
	_XMemDestroyDecompressionContext = (XMemDestroyDecompressionContext)GetProcAddress(hModule, "XMemDestroyDecompressionContext");
	SDL_assert_release(_XMemDestroyDecompressionContext);
}

DatFat::DatFat(const std::string &filename) {
	std::string fatFile = filename;
	fatFile[fatFile.size() - 3] = 'f';

	std::string datFile = filename;
	datFile[datFile.size() - 3] = 'd';

	FILE *fat = fopen(fatFile.c_str(), "rb");
	SDL_assert_release(fat);

	uint32_t magic;
	fread(&magic, sizeof(magic), 1, fat);
	SDL_assert_release(magic == 1178686515);

	int32_t version;
	fread(&version, sizeof(version), 1, fat);
	SDL_assert_release(version == 8);

	uint32_t flags;
	fread(&flags, sizeof(flags), 1, fat);
	SDL_assert_release((flags & ~0xFFFFFF) == 0);

	uint32_t entries;
	fread(&entries, sizeof(entries), 1, fat);

	for (uint32_t i = 0; i < entries; ++i) {
		uint32_t a, b, c, d;
		fread(&a, sizeof(uint32_t), 1, fat);
		fread(&b, sizeof(uint32_t), 1, fat);
		fread(&c, sizeof(uint32_t), 1, fat);
		fread(&d, sizeof(uint32_t), 1, fat);

		FileEntry &fe = files[a];
		fe.realSize = (b >> 3) & 0x1FFFFFFFu;
		fe.compression = (FileEntry::Compression) ((b >> 0) & 0x00000007u);
		fe.offset = ((uint64_t)d) << 3;
		fe.offset |= (c >> 29) & 0x00000007u;
		fe.size = (c >> 0) & 0x1FFFFFFFu;
	}

	fclose(fat);

	//Init Dat mmap
	file = CreateFileA(datFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	SDL_assert_release(file);
	fileMapping = CreateFileMappingA(file, NULL, PAGE_READONLY, 0, 0, NULL);
	SDL_assert_release(fileMapping);
	datPtr = (const uint8_t*)MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);
	SDL_assert_release(datPtr);
}

DatFat::~DatFat() {
	if (datPtr)
		UnmapViewOfFile(datPtr);
	if (fileMapping)
		CloseHandle(fileMapping);
	if(file)
		CloseHandle(file);
}

static int mem_close(SDL_RWops* context) {
	if (context) {
		free(context->hidden.mem.base);
		SDL_FreeRW(context);
	}
	return 0;
}

SDL_RWops* DatFat::openRead(CPathID hash) {
	auto it = files.find(hash);
	if (it != files.end()) {
		if (it->second.compression == FileEntry::Compression::None) {
			return SDL_RWFromConstMem(datPtr + it->second.offset, it->second.size);
		} else if (it->second.compression == FileEntry::Compression::Xbox) {
			uint8_t* data = (uint8_t*)malloc(it->second.realSize);
			uint8_t* dataIt = data;

			SDL_RWops *dat = SDL_RWFromConstMem(datPtr + it->second.offset, it->second.size);
			uint32_t magic = SDL_ReadBE32(dat);
			SDL_assert_release(magic == 0x0FF512EE);

			uint32_t version = SDL_ReadBE32(dat);
			SDL_assert_release(version == 0x01030000);

			uint32_t unknown08 = SDL_ReadBE32(dat);
			SDL_assert_release(unknown08 == 0);

			uint32_t unknown0C = SDL_ReadBE32(dat);
			SDL_assert_release(unknown0C == 0);

			uint32_t windowSize = SDL_ReadBE32(dat);

			uint32_t chunkSize = SDL_ReadBE32(dat);

			int64_t uncompressedSize = SDL_ReadBE64(dat);

			int64_t compressedSize = SDL_ReadBE64(dat);

			int32_t largestUncompressedChunkSize = SDL_ReadBE32(dat);

			int32_t largestCompressedChunkSize = SDL_ReadBE32(dat);

			uint8_t* compressedBytes = new uint8_t[largestCompressedChunkSize];

			int64_t remaining = uncompressedSize;
			while (remaining > 0) {
				XMEMDECOMPRESSION_CONTEXT context;
				XMEMCODEC_PARAMETERS_LZX param;
				param.Flags = 0;
				param.WindowSize = windowSize;
				param.CompressionPartitionSize = chunkSize;
				HRESULT ret = _XMemCreateDecompressionContext(XMEMCODEC_LZX, &param, 1, &context);
				SDL_assert_release(ret == S_OK);

				int32_t compressedChunkSize = SDL_ReadBE32(dat);
				SDL_RWread(dat, compressedBytes, 1, compressedChunkSize);
				
				SIZE_T uncompressedChunkSize = largestUncompressedChunkSize;
				ret = _XMemDecompress(context, dataIt, &uncompressedChunkSize, compressedBytes, compressedChunkSize);
				SDL_assert_release(ret == S_OK);

				dataIt += uncompressedChunkSize;

				_XMemDestroyDecompressionContext(context);
				remaining -= uncompressedChunkSize;
			}

			SDL_assert_release(dataIt == data + it->second.realSize);

			delete[] compressedBytes;

			SDL_RWops *fp = SDL_RWFromConstMem(data, it->second.realSize);
			fp->close = mem_close;
			return fp;
		} else {
			SDL_assert_release(false);
		}

	}

	return NULL;
}
