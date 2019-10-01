#include "FileHandler.h"
#include "Common.h"
#include "Hash.h"
#include "DB.h"
#include "SDL_log.h"
#include <filesystem>
#include <DatFat.h>
#include <CPathID.h>

static std::vector<DatFat> dats;

static std::string getExt(const std::filesystem::path& path) {
	const std::string name = path.filename().generic_string();
	return name.substr(name.find('.') + 1);
}

Vector<FileInfo> FH::getFileList(const std::string &dir, const std::string &extFilter) {
	std::unordered_map<std::string, FileInfo> files;

	/*for (const std::string &base : settings.searchPaths) {
		std::string fullPath = base + dir;
		if (!std::filesystem::exists(fullPath.c_str())) continue;

		for (auto& p : std::filesystem::directory_iterator(fullPath)) {
			std::string path = p.path().generic_string().substr(base.size());
			if (p.is_regular_file() && files.count(path) == 0) {
				std::string ext = getExt(p);
				if (extFilter.empty() || ext == extFilter) {
					FileInfo& fi = files[path];
					fi.fullPath = path;
					fi.ext = ext;
					fi.name = p.path().filename().generic_string();
				}
			}
		}
	}*/

	Vector<FileInfo> outFiles;
	for (auto &file : files)
		outFiles.push_back(file.second);
	return outFiles;
}

Vector<FileInfo> FH::getFileListFromAbsDir(const std::string & fullDir, const std::string & extFilter) {
	Vector<FileInfo> outFiles;
	if (!std::filesystem::exists(fullDir.c_str())) return outFiles;

	for (auto& p : std::filesystem::directory_iterator(fullDir)) {
		if (p.is_regular_file()) {
			std::string ext = getExt(p);
			if (extFilter.empty() || ext == extFilter) {
				FileInfo& fi = outFiles.emplace_back();
				fi.fullPath = p.path().generic_string();
				fi.ext = ext;
				fi.name = p.path().filename().generic_string();
			}
		}
	}

	return outFiles;
}

SDL_RWops * FH::openFile(const char *path) {
	//Check if file in patch dir exists
	char fullPath[512];
	snprintf(fullPath, sizeof(fullPath), "%s%s", settings.patchDir.c_str(), path);
	if (std::filesystem::exists(fullPath))
		return SDL_RWFromFile(fullPath, "rb");

	//Return by hash
	CPathID pid(path);
	return openFile(pid.id);
}

SDL_RWops * FH::openFile(uint32_t path) {
	for (auto& it : dats) {
		SDL_RWops* fp = it.openRead(path);
		if (fp)
			return fp;
	}
	return NULL;
}

SDL_RWops* FH::openFileWrite(const std::string& path) {
	std::string fullPath = settings.patchDir + path;
	//Create parent directories
	std::size_t found = fullPath.find_last_of("/\\");
	std::string parentDir = fullPath.substr(0, found);
	bool ret = std::filesystem::create_directories(parentDir);
	SDL_assert_release(ret);

	return SDL_RWFromFile(fullPath.c_str(), "wb");
}

void FH::Init() {
	InitXCompress();
	dats.reserve(60);

	AddDatFat(settings.gameDir + "data_win64/patch1.fat");
	AddDatFat(settings.gameDir + "data_win64/patch.fat");

	AddDatFat(settings.gameDir + "data_win64/common.fat");
	AddDatFat(settings.gameDir + "data_win64/shaders.fat");
	AddDatFat(settings.gameDir + "data_win64/shadersobj.fat");
	AddDatFat(settings.gameDir + "data_win64/sound.fat");
	AddDatFat(settings.gameDir + "data_win64/sound_" + settings.soundLang + ".fat");

	AddDatFat(settings.gameDir + "data_win64/worlds/windy_city/windy_city.fat");
	AddDatFat(settings.gameDir + "data_win64/worlds/windy_city/windy_city_" + settings.soundLang + ".fat");

	AddDatFat(settings.gameDir + "data_win64/dlc/dlc_exclusive/dlc_exclusive.fat");
	AddDatFat(settings.gameDir + "data_win64/dlc/dlc_exclusive/dlc_exclusive_" + settings.soundLang + ".fat");

	AddDatFat(settings.gameDir + "data_win64/dlc/dlc_pill_people/dlc_pill_people.fat");

	AddDatFat(settings.gameDir + "data_win64/dlc/dlc_solo/dlc_solo.fat");
	AddDatFat(settings.gameDir + "data_win64/dlc/dlc_solo/dlc_solo_" + settings.soundLang + ".fat");

	AddDatFat(settings.gameDir + "data_win64/dlc/dlc_exclusive/dlc_exclusive.fat");
	AddDatFat(settings.gameDir + "data_win64/dlc/dlc_exclusive/dlc_exclusive_" + settings.soundLang + ".fat");
}

void FH::AddDatFat(const std::string& filename) {
	dats.emplace_back(filename);
}
