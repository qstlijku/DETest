#include "DatFat.h"

#include <SDL.h>
#include <Windows.h>
#include "lz4.h"
#include "Common.h"

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
typedef HRESULT(*XMemCreateDecompressionContext)(
	XMEMCODEC_TYPE                  CodecType,
	CONST VOID* pCodecParams,
	DWORD                           Flags,
	XMEMDECOMPRESSION_CONTEXT* pContext
	);
static XMemCreateDecompressionContext _XMemCreateDecompressionContext;
typedef HRESULT(*XMemDecompress)(
	XMEMDECOMPRESSION_CONTEXT       Context,
	VOID* pDestination,
	SIZE_T* pDestSize,
	CONST VOID* pSource,
	SIZE_T                          SrcSize
	);
static XMemDecompress _XMemDecompress;
typedef VOID(*XMemDestroyDecompressionContext)(
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

static int mem_close(SDL_RWops* context) {
	if (context) {
		delete [] context->hidden.mem.base;
		SDL_FreeRW(context);
	}
	return 0;
}

DatFat::~DatFat() {
	if (datPtr)
		UnmapViewOfFile(datPtr);
	if (fileMapping)
		CloseHandle(fileMapping);
	if (file)
		CloseHandle(file);
}

#if WD2
DatFat::DatFat(const std::string& filename) {
	name = filename.substr(filename.find(PLATFORM_FOLDER) + strlen(PLATFORM_FOLDER) + 1);

	std::string fatFile = filename;
	fatFile[fatFile.size() - 3] = 'f';

	std::string datFile = filename;
	datFile[datFile.size() - 3] = 'd';

	FILE* fat = fopen(fatFile.c_str(), "rb");
	SDL_assert_release(fat);

	uint32_t magic;
	fread(&magic, sizeof(magic), 1, fat);
	SDL_assert_release(magic == 1178686517);

	int32_t version;
	fread(&version, sizeof(version), 1, fat);
	SDL_assert_release(version == 11);

	uint32_t flags;
	fread(&flags, sizeof(flags), 1, fat);

	uint32_t unk[3];
	fread(unk, sizeof(unk), 1, fat);
	SDL_assert_release(unk[0] == 0xFFFFFFFF);
	SDL_assert_release(unk[1] == 0xFFFFFFFF);
	SDL_assert_release(unk[2] == 0);

	uint32_t entries;
	fread(&entries, sizeof(entries), 1, fat);

	for (uint32_t i = 0; i < entries; ++i) {
		CPathID id;
		fread(&id.id, sizeof(id.id), 1, fat);

		uint32_t entry[3];
		fread(entry, sizeof(entry), 1, fat);

		FileEntry& fe = files[id];
		fe.offset = entry[1] << 2;
		fe.offset |= (entry[0] & 0xC0000000) >> 30;
		fe.realSize = (entry[2] & 0xFFFFFFFC) >> 2;
		fe.size = entry[0] & 0x3FFFFFFF;
		fe.compression = (FileEntry::Compression)(entry[2] & 0x00000003);
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

typedef enum { endOnOutputSize = 0, endOnInputSize = 1 } endCondition_directive;
typedef enum { decode_full_block = 0, partial_decode = 1 } earlyEnd_directive;
typedef enum { noDict = 0, withPrefix64k, usingExtDict, usingDictCtx } dict_directive;
typedef enum { noDictIssue = 0, dictSmall } dictIssue_directive;
extern "C" LZ4LIB_API int LZ4_decompress_generic(const char* const src,
	char* const dst,
	int srcSize,
	int outputSize,         /* If endOnInput==endOnInputSize, this value is `dstCapacity` */

	endCondition_directive endOnInput,   /* endOnOutputSize, endOnInputSize */
	earlyEnd_directive partialDecoding,  /* full, partial */
	dict_directive dict,                 /* noDict, withPrefix64k, usingExtDict */
	const BYTE* const lowPrefix,  /* always <= dst, == dst when no prefix */
	const BYTE* const dictStart,  /* only if dict==usingExtDict */
	const size_t dictSize         /* note : = 0 if noDict */);

typedef char _BYTE;
typedef WORD _WORD;
typedef DWORD _DWORD;
typedef uint64_t _QWORD;
#define LODWORD(x)  (*((_DWORD*)&(x)))
__int64 sub_140004880(void* Src, void* Dst, int a3, int a4, int a5) {
    char* v5; // r11@1
    char* v6; // rbp@1
    char* v7; // r13@1
    char* v8; // r12@1
    _BYTE* v9; // rbx@1
    char* v10; // rsi@1
    unsigned __int64 v11; // r10@2
    unsigned int v12; // eax@3
    _BYTE* v13; // rbx@3
    char v14; // r9@3
    unsigned __int64 v15; // rdi@3
    __int64 v16; // rcx@4
    char* v17; // rdx@5
    __int64 v18; // rax@6
    int v19; // ecx@7
    char* v20; // r8@9
    __int64 v21; // r9@10
    __int64 v22; // rcx@11
    signed __int64 v23; // rcx@12
    char v24; // al@13
    signed __int64 v25; // r8@13
    signed __int64 v26; // rax@13
    char* v27; // r8@13
    unsigned __int64 v28; // rcx@13
    __int64 v29; // rax@15
    __int64 v30; // rax@21
    _BYTE* v31; // r8@23
    char v32; // al@24
    unsigned int v33; // eax@31
    char* v34; // rbx@31
    char v35; // bp@31
    unsigned __int64 v36; // rdi@31
    __int64 v37; // rcx@32
    char* v38; // r14@33
    unsigned int v39; // ecx@37
    char* v40; // r14@39
    __int64 v41; // rbp@40
    __int64 v42; // rcx@41
    unsigned __int64 v43; // rcx@42
    char v44; // al@44
    __int64 v46; // [sp+20h] [bp-A8h]@1
    __int64 v47; // [sp+28h] [bp-A0h]@1
    __int64 v48; // [sp+30h] [bp-98h]@1
    __int64 v49; // [sp+38h] [bp-90h]@1
    __int64 v50; // [sp+40h] [bp-88h]@1
    __int64 v51; // [sp+48h] [bp-80h]@1
    __int64 v52; // [sp+50h] [bp-78h]@1
    __int64 v53; // [sp+58h] [bp-70h]@1
    __int64 v54; // [sp+60h] [bp-68h]@1
    __int64 v55; // [sp+68h] [bp-60h]@1
    __int64 v56; // [sp+70h] [bp-58h]@1
    __int64 v57; // [sp+78h] [bp-50h]@1
    __int64 v58; // [sp+80h] [bp-48h]@1
    __int64 v59; // [sp+88h] [bp-40h]@1
    __int64 v60; // [sp+90h] [bp-38h]@1
    __int64 v61; // [sp+98h] [bp-30h]@1

    v5 = (char*)Dst + a5;
    v6 = (char*)Src + a3;
    v7 = (char*)Dst + a4;
    v8 = (char*)Dst;
    v9 = (_BYTE*)Src;
    v10 = (char*)Dst;
    v46 = 0i64;
    v47 = 3i64;
    v48 = 2i64;
    v49 = 3i64;
    v50 = 0i64;
    v51 = 0i64;
    v52 = 0i64;
    v53 = 0i64;
    v54 = 0i64;
    v55 = 0i64;
    v56 = 0i64;
    v57 = -1i64;
    v58 = 0i64;
    v59 = 1i64;
    v60 = 2i64;
    v61 = 3i64;
    if (Dst < v5) {
        v11 = (unsigned __int64)(v7 - 8);
        while (1) {
            v12 = *v9;
            v13 = v9 + 1;
            v14 = v12;
            v15 = (unsigned __int64)v12 >> 4;
            if (v15 == 15) {
                do {
                    v16 = *v13++;
                    v15 += v16;
                } while ((_DWORD)v16 == 255);
            }
            v17 = &v10[v15];
            if ((unsigned __int64)&v10[v15] > v11) {
                if (v17 == v7) {
                    memmove(v10, v13, v15);
                    v9 = &v13[v15];
                    v10 += v15;
                    goto LABEL_28;
                }
                return 0xFFFFFFFFi64;
            }
            do {
                v18 = *(_QWORD*)v13;
                v10 += 8;
                v13 += 8;
                *((_QWORD*)v10 - 1) = v18;
            } while (v10 < v17);
            v9 = &v13[v17 - v10 + 2];
            v19 = *((_WORD*)v9 - 1);
            if ((unsigned int)v19 >= 0xE000) {
                v19 = (*v9 << 13) + *((_WORD*)v9 - 1);
                ++v9;
            }
            v20 = &v17[-v19];
            if (v20 < v8)
                return 0xFFFFFFFFi64;
            v21 = v14 & 0xF;
            if (v21 == 15) {
                do {
                    v22 = *v9++;
                    v21 += v22;
                } while ((_DWORD)v22 == 255);
            }
            v23 = v17 - v20;
            if (v17 - v20 >= 8)
                break;
            v24 = *v20;
            v25 = (signed __int64)(v20 + 4);
            *v17 = v24;
            v17[1] = *(_BYTE*)(v25 - 3);
            v17[2] = *(_BYTE*)(v25 - 2);
            v17[3] = *(_BYTE*)(v25 - 1);
            v26 = (signed __int64)(v17 + 4);
            v17 += 8;
            v27 = (char*)(v25 - *(&v46 + v26 - v25));
            LODWORD(v26) = *(_DWORD*)v27;
            v20 = &v27[-*(&v54 + v23)];
            *((_DWORD*)v17 - 1) = v26;
            v28 = (unsigned __int64)&v17[v21 - 4];
            if (v28 > (unsigned __int64)(v7 - 12))
                goto LABEL_19;
            if ((unsigned __int64)v17 < v28)
                goto LABEL_15;
        LABEL_16:
            v10 = (char*)v28;
            if (v28 >= (unsigned __int64)v5)
                goto LABEL_28;
        }
        v28 = (unsigned __int64)&v17[v21 + 4];
        if (v28 <= (unsigned __int64)(v7 - 12)) {
            do {
            LABEL_15:
                v29 = *(_QWORD*)v20;
                v17 += 8;
                v20 += 8;
                *((_QWORD*)v17 - 1) = v29;
            } while ((unsigned __int64)v17 < v28);
            goto LABEL_16;
        }
    LABEL_19:
        if (v28 > (unsigned __int64)(v7 - 5))
            return 0xFFFFFFFFi64;
        for (; (unsigned __int64)v17 < v11; *((_QWORD*)v17 - 1) = v30) {
            v30 = *(_QWORD*)v20;
            v17 += 8;
            v20 += 8;
        }
        if ((unsigned __int64)v17 < v28) {
            v31 = (_BYTE*)(v20 - v17);
            do {
                v32 = v31[(_QWORD)v17++];
                *(v17 - 1) = v32;
            } while ((unsigned __int64)v17 < v28);
        }
        goto LABEL_16;
    }
LABEL_28:
    if (v7 != v6) {
        memcpy(&v9[v7 - v6], v9, v6 - v9);
        v9 += v7 - v6;
    }
    if (v10 < v9) {
        while (1) {
            v33 = *v9;
            v34 = v9 + 1;
            v35 = v33;
            v36 = (unsigned __int64)v33 >> 4;
            if (v36 == 15) {
                do {
                    v37 = (unsigned __int8)*v34++;
                    v36 += v37;
                } while ((_DWORD)v37 == 255);
            }
            v38 = &v10[v36];
            if (&v10[v36] > v7 - 8)
                break;
            if (v10 < v38) {
                memcpy(v10, v34, v36);
                v10 += v36;
                v34 += v36;
            }
            if (v10 >= v34)
                return (unsigned int)((_DWORD)v10 - (_DWORD)v8);
            v39 = *(_WORD*)v34;
            v9 = v34 + 2;
            if (v39 >= 0xE000)
                v39 += *v9++ << 13;
            v40 = &v38[-v39];
            if (v40 < v8)
                return 0xFFFFFFFFi64;
            v41 = v35 & 0xF;
            if (v41 == 15) {
                do {
                    v42 = *v9++;
                    v41 += v42;
                } while ((_DWORD)v42 == 255);
            }
            v43 = (unsigned __int64)&v10[v41 + 4];
            if (v43 > (unsigned __int64)(v7 - 5))
                return 0xFFFFFFFFi64;
            for (; (unsigned __int64)v10 < v43; *(v10 - 1) = v44) {
                v44 = *v40;
                ++v10;
                ++v40;
            }
            if (v10 >= v9)
                return (unsigned int)((_DWORD)v10 - (_DWORD)v8);
        }
        if (v38 != v7)
            return 0xFFFFFFFFi64;
        memmove(v10, v34, v36);
        LODWORD(v10) = v36 + (_DWORD)v10;
    }
    return (unsigned int)((_DWORD)v10 - (_DWORD)v8);
}

SDL_RWops* DatFat::openRead(CPathID hash) {
	auto it = files.find(hash);
	if (it != files.end()) {
		if (it->second.compression == FileEntry::Compression::None) {
			return SDL_RWFromConstMem(datPtr + it->second.offset, it->second.size);
		} else if (it->second.compression == FileEntry::Compression::LZ4) {
			uint8_t* data = new uint8_t[it->second.realSize];
			uint8_t* _Src = (uint8_t*)datPtr + it->second.offset;

			uint8_t head = *_Src;
			uint32_t preFixSize = head;
			uint8_t* pbVar3 = _Src + 1;
			if (0x7f < head) {
				uint8_t head2 = *pbVar3;
				pbVar3 = _Src + 2;
				uint32_t uVar2 = ((uint32_t)head - 0x80) + (uint32_t)head2 * 0x80;
				preFixSize = uVar2;
				if (0x3fff < uVar2) {
					head = *pbVar3;
					pbVar3 = _Src + 3;
					uVar2 = (uVar2 - 0x4000) + (uint32_t)head * 0x4000;
					preFixSize = uVar2;
					if (0x1fffff < uVar2) {
						preFixSize = (uint64_t)((uVar2 - 0x200000) + (uint32_t)*pbVar3 * 0x200000);
						pbVar3 = _Src + 4;
					}
				}
			}
			memcpy(data + it->second.realSize - it->second.size, _Src, it->second.size);
            int ret = sub_140004880((char*)pbVar3, (char*)data, it->second.size - ((uint64_t)pbVar3 - (uint64_t)_Src), it->second.realSize, preFixSize);
			//int ret = sub_140004880((char*)(pbVar3 + (uint64_t)data + it->second.realSize - it->second.size - _Src), (char*)data, it->second.size - (uint64_t)pbVar3 + (uint64_t)_Src, it->second.realSize, preFixSize);
			//int ret = LZ4_decompress_generic((char*)(pbVar3 + (uint64_t)data + it->second.realSize - it->second.size - _Src), (char*)data, it->second.size - (uint64_t)pbVar3 + (uint64_t)_Src, it->second.realSize, endOnInputSize, decode_full_block, noDict, (BYTE*)data - preFixSize, NULL, 0);
			SDL_assert_release(ret == it->second.realSize);

			SDL_RWops* fp = SDL_RWFromConstMem(data, it->second.realSize);
			fp->close = mem_close;
			return fp;
		} else {
			SDL_assert_release(false);
		}

	}

	return NULL;
}

#else

DatFat::DatFat(const std::string &filename) {
	name = filename.substr(filename.find(settings.platformFolder) + settings.platformFolder.size() + 1);

	std::string fatFile = filename;
    fatFile[fatFile.size() - 3] = 'f';
    fatFile[fatFile.size() - 2] = 'a';
    fatFile[fatFile.size() - 1] = 't';

	std::string datFile = filename;
	datFile[datFile.size() - 3] = 'd';
    datFile[datFile.size() - 2] = 'a';
    datFile[datFile.size() - 1] = 't';

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

SDL_RWops* DatFat::openRead(CPathID hash) {
	auto it = files.find(hash);
	if (it != files.end()) {
		if (it->second.compression == FileEntry::Compression::None) {
			return SDL_RWFromConstMem(datPtr + it->second.offset, it->second.size);
		} else if (it->second.compression == FileEntry::Compression::Xbox) {
			uint8_t* data = new uint8_t[it->second.realSize];
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
			SDL_RWclose(dat);

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

#endif
