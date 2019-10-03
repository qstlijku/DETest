#include "DatFat.h"

#include <SDL.h>

static void appDecompressLZX(byte* CompressedBuffer, int CompressedSize, byte* UncompressedBuffer, int UncompressedSize, int windowSize);

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
		fe.offset = (d << 3) & 0xffffffff;
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

SDL_RWops* DatFat::openRead(uint32_t hash) {
	auto it = files.find(hash);
	if (it != files.end()) {
		if (it->second.compression == FileEntry::Compression::None) {
			return SDL_RWFromConstMem(datPtr + it->second.offset, it->second.size);
		} else if (it->second.compression == FileEntry::Compression::Xbox) {
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

			uint8_t* uncompressedBytes = new uint8_t[largestUncompressedChunkSize];
			uint8_t* compressedBytes = new uint8_t[largestCompressedChunkSize];

			int64_t remaining = uncompressedSize;
			while (remaining > 0) {
				int32_t compressedChunkSize = SDL_ReadBE32(dat);
				SDL_RWread(dat, compressedBytes, 1, compressedChunkSize);
				appDecompressLZX(compressedBytes, compressedChunkSize, uncompressedBytes, largestUncompressedChunkSize, windowSize);
				//remaining -= actualUncompressedChunkSize;
			}

			uint8_t* data = (uint8_t*) malloc(it->second.realSize);

			SDL_RWops *fp = SDL_RWFromConstMem(data, it->second.realSize);
			fp->close = mem_close;
			return fp;
		} else {
			SDL_assert_release(false);
		}

	}

	return NULL;
}


////////////////////
// Compression
////////////////////


#include "mspack.h"
#include "lzx.h"

// https://github.com/gildor2/UModel/blob/master/Unreal/UnCoreCompression.cpp
typedef unsigned char byte;
struct appDecompressLZX_file {
	byte* buf;
	int			bufSize;
	int			pos;
	int			rest;
};

static int appDecompressLZX_read(struct appDecompressLZX_file* file, void* buffer, int bytes) {
	//guard(mspack_read);

	if (!file->rest) {
		// read block header
		if (file->buf[file->pos] == 0xFF) {
			// [0]   = FF
			// [1,2] = uncompressed block size
			// [3,4] = compressed block size
			file->rest = (file->buf[file->pos + 3] << 8) | file->buf[file->pos + 4];
			file->pos += 5;
		} else {
			// [0,1] = compressed size
			file->rest = (file->buf[file->pos + 0] << 8) | file->buf[file->pos + 1];
			file->pos += 2;
		}
		if (file->rest > file->bufSize - file->pos)
			file->rest = file->bufSize - file->pos;
	}
	if (bytes > file->rest) bytes = file->rest;
	if (!bytes) return 0;

	// copy block data
	memcpy(buffer, file->buf + file->pos, bytes);
	file->pos += bytes;
	file->rest -= bytes;

	return bytes;
	//unguard;
}

static int appDecompressLZX_write(struct appDecompressLZX_file* file, void* buffer, int bytes) {
	//guard(mspack_write);
	//assert(file->pos + bytes <= file->bufSize);
	memcpy(file->buf + file->pos, buffer, bytes);
	file->pos += bytes;
	return bytes;
	//unguard;
}

static void* appDecompressLZX_alloc(struct mspack_system* self, size_t bytes) {
	return /*appMalloc*/malloc(bytes);
}

static void appDecompressLZX_free(void* ptr) {
	/*appFree*/free(ptr);
}

static void appDecompressLZX_copy(void* src, void* dst, size_t bytes) {
	memcpy(dst, src, bytes);
}

static struct mspack_system lzxSys =
{
	NULL,				// open
	NULL,				// close
	(int (*)(mspack_file *, void *, int)) & appDecompressLZX_read,
	(int (*)(mspack_file*, void*, int)) & appDecompressLZX_write,
	NULL,				// seek
	NULL,				// tell
	NULL,				// message
	&appDecompressLZX_alloc,
	&appDecompressLZX_free,
	&appDecompressLZX_copy
};

static void appDecompressLZX(byte* CompressedBuffer, int CompressedSize, byte* UncompressedBuffer, int UncompressedSize, int windowSize) {
	//guard(appDecompressLZX);

	// setup streams
	struct appDecompressLZX_file src, dst;
	src.buf = CompressedBuffer;
	src.bufSize = CompressedSize;
	src.pos = 0;
	src.rest = 0;
	dst.buf = UncompressedBuffer;
	dst.bufSize = UncompressedSize;
	dst.pos = 0;
	// prepare decompressor
	struct lzxd_stream* lzxd = lzxd_init(&lzxSys, (mspack_file*)& src, (mspack_file*)& dst, 17, 0, 256 * 1024, UncompressedSize, 0);
	//assert(lzxd);
	// decompress
	int r = lzxd_decompress(lzxd, UncompressedSize);
	if (r != MSPACK_ERR_OK)
		return; //appError("lzxd_decompress(%d,%d) returned %d", CompressedSize, UncompressedSize, r);
	// free resources
	lzxd_free(lzxd);

	//unguard;
}
