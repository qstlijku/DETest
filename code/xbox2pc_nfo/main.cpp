#include <stdio.h>
#include <string>
#include "SDL_rwops.h"
#include <SDL_assert.h>
#include "DatFat.h"
#include <tinyxml2.h>
#include <filesystem>
#include <unordered_set>
#include <future>
#include <Windows.h>

int main(int argc, char **argv) {
	printf("Disrupt Editor - Xbox NFO Extractor\nCreated by Jon : https://github.com/j301scott/DisruptEditor\n");

	if (argc < 2) {
		printf("Usage: xbox2pc_nfo.exe common.dat\n");
		return 0;
	}

	InitXCompress();

	for (int i = 1; i < argc; ++i) {
		printf("Converting file %s\n", argv[i]);

		std::string nfoFile(argv[i]);
		nfoFile[nfoFile.size() - 3] = 'n';
		nfoFile[nfoFile.size() - 2] = 'f';
		nfoFile[nfoFile.size() - 1] = 'o';
		tinyxml2::XMLDocument nfo;
		if (nfo.LoadFile(nfoFile.c_str()) == tinyxml2::XMLError::XML_SUCCESS) {
			std::unordered_set<CPathID> hasRead;
			DatFat dat(argv[i]);

			//Read NFO and extact each file
			for (tinyxml2::XMLElement* it = nfo.RootElement()->FirstChildElement("common")->FirstChildElement("File"); it; it = it->NextSiblingElement("File")) {
				CPathID fileID;
				fileID = it->UnsignedAttribute("Crc");
				std::string fileName = it->Attribute("Path");

				hasRead.emplace(fileID);

				std::filesystem::path outputfileName = std::string(argv[i]) + "_unpack/" + fileName;
				std::filesystem::path root = outputfileName.parent_path();
				std::filesystem::create_directories(root);

				uint64_t FileTimeU64 = it->Unsigned64Attribute("FileTime");
				FILETIME ft;
				ft.dwLowDateTime = (DWORD)(FileTimeU64 & 0xFFFFFFFF);
				ft.dwHighDateTime = (DWORD)(FileTimeU64 >> 32);

				if (std::filesystem::exists(outputfileName))
					continue;

				printf("Extracting %s %08x\n", fileName.c_str(), fileID.id);

				SDL_RWops* out = SDL_RWFromFile(outputfileName.generic_string().c_str(), "wb");
				if (out) {
					SDL_RWops* fp = dat.openRead(fileID);
					if (fp) {
						std::vector<uint8_t> data(SDL_RWsize(fp));
						SDL_RWread(fp, data.data(), 1, data.size());
						SDL_RWclose(fp);

						SDL_RWwrite(out, data.data(), 1, data.size());
					} else {
						printf("Failed opening file\n");
					}
					SDL_RWclose(out);

					HANDLE hFile = CreateFileW(outputfileName.generic_wstring().c_str(), FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, 0, 0);
					if (hFile) {
						SetFileTime(hFile, &ft, NULL, &ft);
						CloseHandle(hFile);
					} else {
						printf("Failed to set file attributes\n");
					}
				} else {
					printf("Failed opening output\n");
				}
			}

			//Check for files that are not in the nfo
			for (const auto& it : dat.files) {
				if (hasRead.count(it.first) == 0) {
					printf("File not in NFO %08x\n", it.first.id);
				}
			}
		} else {
			printf("Failed opening nfo file\n");
		}

		printf("\n");
	}

	return 0;
}