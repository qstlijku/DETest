#include "Common.h"

#include "Vector.h"
#include "Hash.h"
#include <unordered_map>
#include <SDL_assert.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <SDL_log.h>
#include <fstream>
#include "NBCF.h"
#include "xbgFile.h"
#include "materialFile.h"
#include "xbtFile.h"
#include "tinyfiles.h"
#include "stb_image.h"
#include "FileHandler.h"
#include "Serialization.h"
#include <SDL_messagebox.h>
#include "IBinaryArchive.h"
#include <future>
#include "xbgMipFile.h"
#include "DDRenderInterface.h"

Settings settings;
std::unordered_map<std::string, materialFile> materials;

void reloadSettings() {
	std::string contents = readFile("settings.xml");
	unserializeFromXML(settings, contents.c_str());
	if (settings.searchPaths.empty() || settings.patchDir.empty()) {
		//Fill with sample files
		saveSettings();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Disrupt Editor is not configured", "You must first setup Disrupt Editor by editing settings.xml\nPlease see the readme for more details.", NULL);
		exit(0);
	}

	//Check to make sure searchPaths have trailing slash
	for (std::string &path : settings.searchPaths) {
		if (path.back() != '/' && path.back() != '\\')
			path.push_back('/');
	}

	if (settings.patchDir.back() != '/' && settings.patchDir.back() != '\\')
		settings.patchDir.push_back('/');
}

void saveSettings() {
	bool ret = writeFile("settings.xml", serializeToXML(settings));
	SDL_assert_release(ret);
}

std::string readFile(const std::string & file) {
	std::ifstream t(file);
	if (!t.is_open())
		return std::string();
	return std::string(std::istreambuf_iterator<char>(t), std::istreambuf_iterator<char>());
}

bool writeFile(const std::string & file, const std::string &contents) {
	SDL_RWops *fp = SDL_RWFromFile(file.c_str(), "w");
	if (fp) {
		SDL_RWwrite(fp, contents.c_str(), 1, contents.size());
		SDL_RWclose(fp);
		return true;
	}
	return false;
}

std::unordered_map<uint32_t, xbgFile> xbgs;

xbgFile& loadXBG(const std::string &path) {
	uint32_t hash = Hash::getFilenameHash(path);
	if (xbgs.count(hash) == 0) {
		auto& model = xbgs[hash];
		SDL_Log("Loading %s\n", path);
		SDL_RWops* fp = FH::openFile(path.c_str());
		if (fp)
			model.open(CBinaryArchiveReader(fp));
	}
	return xbgs[hash];
}

xbgFile &loadXBG(uint32_t path) {
	if (xbgs.count(path) == 0) {
		auto &model = xbgs[path];
		SDL_Log("Loading %u.xbg\n", path);
		SDL_RWops *fp = FH::openFile(path);
		if (fp)
			model.open(CBinaryArchiveReader(fp));
	}
	return xbgs[path];
}

std::unordered_map<uint32_t, xbgMipFile> xbgmips;

xbgMipFile& loadXBGMIP(const std::string& path) {
	uint32_t hash = Hash::getFilenameHash(path);
	if (xbgs.count(hash) == 0) {
		auto& model = xbgmips[hash];
		SDL_Log("Loading %s\n", path);
		SDL_RWops* fp = FH::openFile(path.c_str());
		if (fp)
			model.open(CBinaryArchiveReader(fp));
	}
	return xbgmips[hash];
}

xbgMipFile& loadXBGMIP(uint32_t path) {
	if (xbgs.count(path) == 0) {
		auto& model = xbgmips[path];
		SDL_Log("Loading %u.xbgmip\n", path);
		SDL_RWops* fp = FH::openFile(path);
		if (fp)
			model.open(CBinaryArchiveReader(fp));
	}
	return xbgmips[path];
}

materialFile &loadMaterial(const std::string & path) {
	if (materials.count(path) == 0) {
		auto &model = materials[path];
		SDL_Log("Loading %s...\n", path.c_str());
		SDL_RWops *fp = FH::openFile(path.c_str());
		if (fp) {
			CBinaryArchiveReader reader(fp);
			model.open(reader);
			SDL_RWclose(fp);
		}
	}
	return materials[path];
}

static std::unordered_map<std::string, std::shared_ptr<xbtFile> > textures;

static void loadTextureAsync(std::shared_ptr<xbtFile> obj, char *path) {
	SDL_Log("Loading %s...", path);
	try {
		SDL_RWops* fp = FH::openFile(path);
		if (fp) {
			obj->open(fp);
			SDL_RWclose(fp);
		}
	} catch (...) {}
	free(path);
}

std::shared_ptr<xbtFile> loadTexture(const char *path) {
	std::shared_ptr<xbtFile> model = textures[path];
	if (!model) {
		model = textures[path] = std::make_shared<xbtFile>();
		/*std::thread thrd(loadTextureAsync, model, strdup(path));
		thrd.detach();*/
		loadTextureAsync(model, strdup(path));
	}
	return model;
}

std::unordered_map<std::string, GLuint> texturesRes;
GLuint loadResTexture(const std::string &path) {
	if (texturesRes.count(path) == 0) {
		int width, height, bpc;
		uint8_t *pixels = stbi_load(("res/" + path).c_str(), &width, &height, &bpc, 0);
		if (!pixels) return 0;
		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		switch (bpc) {
			case 4:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
				break;
			case 3:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
				break;
			case 2:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, pixels);
				break;
			case 1:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
				break;
		}
		glGenerateMipmap(GL_TEXTURE_2D);
		texturesRes[path] = id;
		free(pixels);
		return id;
	}
	return texturesRes[path];
}

void Settings::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(searchPaths);
	REGISTER_MEMBER(patchDir);

	REGISTER_MEMBER(near_plane);
	REGISTER_MEMBER(far_plane);
	REGISTER_MEMBER(fov);
	REGISTER_MEMBER(textDrawDistance);
	REGISTER_MEMBER(displayComponents);
	REGISTER_MEMBER(displayNear);
	REGISTER_MEMBER(drawTerrain);

	REGISTER_MEMBER(windowSize);
	REGISTER_MEMBER(maximized);
	REGISTER_MEMBER(openWindows);

	REGISTER_MEMBER(keyForward);
	REGISTER_MEMBER(keyBackward);
	REGISTER_MEMBER(keyLeft);
	REGISTER_MEMBER(keyRight);
	REGISTER_MEMBER(keyAscend);
	REGISTER_MEMBER(keyDescend);
	REGISTER_MEMBER(keyFast);
	REGISTER_MEMBER(keySlow);
}
