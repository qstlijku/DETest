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
#include "tinyfiles.h"
#include "stb_image.h"
#include "FileHandler.h"
#include "Serialization.h"
#include <SDL_messagebox.h>
#include "IBinaryArchive.h"
#include <future>
#include "DDRenderInterface.h"

Settings settings;

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

static std::unordered_map<std::string, GLuint> texturesRes;
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
