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
#include "stb_image.h"
#include "FileHandler.h"
#include "Serialization.h"
#include <SDL_messagebox.h>
#include "IBinaryArchive.h"
#include <future>
#include "DDRenderInterface.h"
#include <noc_file_dialog.h>
#include <filesystem>

Settings settings;

static void promptGameDir() {
	SDL_ShowSimpleMessageBox(0, "Disrupt Editor Setup", "Please select the main Watch_Dogs.exe file in the next window", NULL);
	const char* dir = noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, "Watch Dogs Exe\0Watch_Dogs.exe\0", NULL, NULL);
	if (!dir) {
		exit(0);
	}

	settings.gameDir = dir;
	settings.gameDir = settings.gameDir.substr(0, settings.gameDir.size() - strlen("bin/Watch_Dogs.exe"));
}

static void promptPatchDir() {
	SDL_ShowSimpleMessageBox(0, "Disrupt Editor Setup", "Please select the (patch) folder to save data to", NULL);
	const char* dir = noc_file_dialog_open(NOC_FILE_DIALOG_OPEN | NOC_FILE_DIALOG_DIR, NULL, NULL, NULL);
	if (!dir) {
		exit(0);
	}

	settings.patchDir = dir;
}

void reloadSettings() {
	std::string contents = readFile("settings.xml");
	unserializeFromXML(settings, contents.c_str());

	if (settings.gameDir.empty() || !std::filesystem::exists(settings.gameDir)) {
		promptGameDir();
		saveSettings();
	}

	if (settings.patchDir.empty() || !std::filesystem::exists(settings.patchDir)) {
		promptPatchDir();
		saveSettings();
	}
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

static std::unordered_map<std::string, uint32_t> texturesRes;
uint32_t loadResTexture(const std::string &path) {
	/*if (texturesRes.count(path) == 0) {
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
	}*/
	return texturesRes[path];
}

void Settings::registerMembers(MemberStructure & ms) {
	//REGISTER_MEMBER(searchPaths);
	REGISTER_MEMBER(gameDir);
	REGISTER_MEMBER(soundLang);
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
