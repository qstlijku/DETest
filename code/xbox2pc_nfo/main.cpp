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

struct NFOFile {
	std::string Path;
	CPathID Crc;
	uint64_t FileTime;
	uint64_t FileSize;
	uint64_t FilePosition;
};
bool SortByPosition(const NFOFile &i, const NFOFile &j) { return (i.FilePosition < j.FilePosition); }

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

			std::vector<NFOFile> files;

			for (tinyxml2::XMLElement* it = nfo.RootElement()->FirstChildElement("common")->FirstChildElement("File"); it; it = it->NextSiblingElement("File")) {
				NFOFile& file = files.emplace_back();
				file.Path = it->Attribute("Path");
				file.Crc = it->UnsignedAttribute("Crc");
				file.FileTime = it->Unsigned64Attribute("FileTime");
				file.FileSize = it->Unsigned64Attribute("FileSize");
				file.FilePosition = it->Unsigned64Attribute("FilePosition");

				//SDL_assert_release(CPathID(file.Path) == file.Crc);
			}

			std::sort(files.begin(), files.end(), SortByPosition);

			//Read NFO and extact each file
			for (auto &it : files) {
				hasRead.emplace(it.Crc);

				std::filesystem::path outputfileName = std::string(argv[i]) + "_unpack/" + it.Path;
				std::filesystem::path root = outputfileName.parent_path();
				std::filesystem::create_directories(root);

				FILETIME ft;
				ft.dwLowDateTime = (DWORD)(it.FileTime & 0xFFFFFFFF);
				ft.dwHighDateTime = (DWORD)(it.FileTime >> 32);

				if (std::filesystem::exists(outputfileName))
					continue;

				printf("Extracting %s %08x %p\n", it.Path.c_str(), it.Crc.id, it.FilePosition);

				SDL_RWops* fp = dat.openRead(it.Crc);
				std::vector<uint8_t> data;
				if (fp) {
					data.resize(SDL_RWsize(fp));
					SDL_RWread(fp, data.data(), 1, data.size());
					SDL_RWclose(fp);
				} else {
					printf("Failed opening file\n");
				}

				SDL_RWops* out = SDL_RWFromFile(outputfileName.generic_string().c_str(), "wb");
				if (out) {
					SDL_RWwrite(out, data.data(), 1, data.size());
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