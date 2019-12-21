#include "FileHandler.h"
#include "Common.h"
#include "Hash.h"
#include "DB.h"
#include "SDL_log.h"
#include <filesystem>
#include <DatFat.h>
#include <CPathID.h>

namespace FH {
	std::vector<DatFat> dats;
}

static std::string getExt(const std::filesystem::path& path) {
	const std::string name = path.filename().generic_string();
	return name.substr(name.find('.') + 1);
}

static std::string getName(const std::filesystem::path& path) {
	const std::string name = path.filename().generic_string();
	return name.substr(0, name.find('.'));
}

Vector<FileInfo> FH::getFileList(const std::string &dir, const std::string &extFilter) {
	std::unordered_map<std::string, FileInfo> files;

	//Search patch
	try {
		std::string fullPath = settings.patchDir + dir;
		for (auto& p : std::filesystem::directory_iterator(fullPath)) {
			std::string path = p.path().generic_string().substr(settings.patchDir.size());
			if (p.is_regular_file() && files.count(path) == 0) {
				std::string ext = getExt(path);
				if (extFilter.empty() || ext == extFilter) {
					FileInfo& fi = files[path];
					fi.fullPath = path;
					fi.ext = ext;
					fi.name = getName(path);
				}
			}
		}
	} catch (...) {
	}

	//Search DB
	for (auto& it : DB::instance().fnvList) {
		std::string ext = getExt(it.second);
		if (extFilter.empty() || ext == extFilter) {
			FileInfo& fi = files[it.second];
			fi.fullPath = it.second;
			fi.ext = ext;
			fi.name = getName(it.second);
		}
	}

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
	for (auto& it : dats) {
		SDL_RWops* fp = it.openRead(path);
		if (fp)
			return fp;
	}
	return NULL;
}

SDL_RWops * FH::openFileHash(CPathID path) {
	std::string dbPath = DB::instance().getFileByHash(path);
	char fullPath[512];
	snprintf(fullPath, sizeof(fullPath), "%s%s", settings.patchDir.c_str(), dbPath.c_str());
	if (std::filesystem::exists(fullPath))
		return SDL_RWFromFile(fullPath, "rb");

	for (auto& it : dats) {
		SDL_RWops* fp = it.openRead(path);
		if (fp)
			return fp;
	}
	return NULL;
}

bool FH::fileExists(const char* path) {
	//Check if file in patch dir exists
	char fullPath[512];
	snprintf(fullPath, sizeof(fullPath), "%s%s", settings.patchDir.c_str(), path);
	if (std::filesystem::exists(fullPath))
		return true;

	CPathID hash(path);

	//Check dats
	for (auto& it : dats) {
		if (it.files.count(hash))
			return true;
	}

	return false;
}

std::string FH::getFileLocations(const char* path) {
	std::string str;

	//Check if file in patch dir exists
	char fullPath[512];
	snprintf(fullPath, sizeof(fullPath), "%s%s", settings.patchDir.c_str(), path);
	if (std::filesystem::exists(fullPath))
		str += "patch\n";

	CPathID hash(path);

	//Check dats
	for (auto& it : dats) {
		if (it.files.count(hash))
			str += it.name + "\n";
	}

	return str;
}

SDL_RWops* FH::openFileWrite(const std::string& path) {
	std::string fullPath = settings.patchDir + path;
	//Create parent directories
	std::size_t found = fullPath.find_last_of("/\\");
	std::string parentDir = fullPath.substr(0, found);
	std::filesystem::create_directories(parentDir);

	return SDL_RWFromFile(fullPath.c_str(), "wb");
}

void FH::Init() {
	InitXCompress();
	dats.reserve(60);

	AddDatFat(settings.gameDir + "data_win64/patch1.fat");
	AddDatFat(settings.gameDir + "data_win64/patch.fat");

	AddDatFat(settings.gameDir + "data_win64/common.fat");
	//AddDatFat(settings.gameDir + "data_win64/shaders.fat");
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
	if(std::filesystem::exists(filename))
		dats.emplace_back(filename);
}

static bool startsWith(const char* str, const char* prefix) {
	size_t lenpre = strlen(prefix), lenstr = strlen(str);
	return lenstr < lenpre ? false : memcmp(prefix, str, lenpre) == 0;
}

static int endsWith(const char* str, const char* suffix) {
	int str_len = strlen(str);
	int suffix_len = strlen(suffix);

	return (str_len >= suffix_len) && (0 == strcmp(str + (str_len - suffix_len), suffix));
}

const char* FH::getTypeFromExtension(const char* name) {
	if (name[0] == '{')
		return "CArchetypeResource";

	if (startsWith(name, "wdfx_"))
		return "CParticlesSystemParamResource";

	//File Types
	if (endsWith(name, ".xml.data.fcb"))
		return "CWorldUnitDataResource";

	if (endsWith(name, ".ano"))
		return "CAnnotationResource";
	if (endsWith(name, ".bik"))
		return "CBinkResource";
	if (endsWith(name, ".bfd"))
		return "bfd";
	if (endsWith(name, ".bundle"))
		return "CBundleResource";
	if (endsWith(name, ".cbatch"))
		return "CBatchResource";
	if (endsWith(name, ".ci"))
		return "CCoverDefinitionResource";
	if (endsWith(name, ".cseq"))
		return "CSequenceResource";
	if (endsWith(name, ".dpax"))
		return "CPoseAnimationResource";
	if (endsWith(name, ".dpdx"))
		return "CPoseDefinitionResource";
	if (endsWith(name, ".embed"))
		return "CEmbeddedResourceContainer";
	if (endsWith(name, ".fcb"))
		return "CBinaryResource";
	if (endsWith(name, ".feu"))
		return "CFireUiBlobResource";
	if (endsWith(name, ".fso.bin"))
		return "CFluidSimulationResource";
	if (endsWith(name, ".hkx"))
		return "CPhysResource";
	if (endsWith(name, ".hgfx"))
		return "CSplineLoftHiResGfxResource";
	if (endsWith(name, ".lgfx"))
		return "CSplineLoftLowResGfxResource";
	if (endsWith(name, ".lib"))
		return "lib";
	if (endsWith(name, ".lipr.bin"))
		return "CLightProbesResource";
	if (endsWith(name, ".loc"))
		return "CLocStringsCompiledData";
	if (endsWith(name, ".lua"))
		return "CDominoBoxResource";
	if (endsWith(name, ".mab"))
		return "CAnimationResource";
	if (endsWith(name, ".material.bin"))
		return "CMaterialResource";
	if (endsWith(name, ".move.bin"))
		return "CMoveResource-fake";
	if (endsWith(name, ".obj"))
		return "obj";
	if (endsWith(name, ".phys"))
		return "CSplineLoftPhysicsResource";
	if (endsWith(name, ".pnm"))
		return "CPilotNavMeshResource";
	if (endsWith(name, ".rml"))
		return "rml";
	if (endsWith(name, ".sbao"))
		return "BinaryAudioObject";
	if (endsWith(name, ".sdat"))
		return "CSectorResource";
	if (endsWith(name, ".sdlr"))
		return "CSectorResourceLowRes";
	if (endsWith(name, ".sdhr"))
		return "CSectorResourceHiRes";
	if (endsWith(name, ".skeleton"))
		return "CSkeletonResource";
	if (endsWith(name, ".stimuli.dsc.pack"))
		return "CDialogStimuliResource";
	if (endsWith(name, ".spk"))
		return "CSoundResource";
	if (endsWith(name, ".srl"))
		return "CSRLResource";
	if (endsWith(name, ".xbg"))
		return "CGeometryResource";
	if (endsWith(name, ".xbgmip"))
		return "CGeometryMipResource";
	if (endsWith(name, ".xbt"))
		return "CTextureResource";
	if (endsWith(name, ".xlf"))
		return "xlf";
	if (endsWith(name, ".xml"))
		return "CXmlResource";

	if (startsWith(name, "engine\\shaders\\"))
		return "shaders";

	return "";
}

char* RWgets(SDL_RWops* rw, char* s, int size) {
	int num_read = 0;
	int newline = 0;

	while (num_read < size && !newline) {
		if (SDL_RWread(rw, &s[num_read], 1, 1) != 1)
			break;

		/* Unlike fgets(), don't store newline. Under Windows/DOS we'll
			* probably get an extra blank line for every line that's being
			* read, but that should be ok.
			*/
		if (s[num_read] == '\n' || s[num_read] == '\r') {
			s[num_read] = '\0';
			newline = 1;
		}

		num_read++;
	}

	s[num_read] = '\0';

	return (num_read != 0) ? s : NULL;
}
