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
#include <portable-file-dialogs.h>
#include <filesystem>

Settings settings;

static void promptGameDir() {
#if WD2
	SDL_ShowSimpleMessageBox(0, "Disrupt Editor 2 Setup", "Please select the Watch Dogs 2 EAC.exe file in the next window", NULL);
	auto f = pfd::open_file("Please select the Watch Dogs 2 EAC.exe file", "", { "EAC exe", "EAC.exe" }, false);
	if (f.result().empty()) {
		exit(0);
	}

	settings.gameDir2 = f.result()[0];
	settings.gameDir2 = settings.gameDir2.substr(0, settings.gameDir2.size() - strlen("EAC.exe"));
#else
	SDL_ShowSimpleMessageBox(0, "Disrupt Editor Setup", "Please select the main Watch_Dogs.exe file in the next window", NULL);
	auto f = pfd::open_file("Please select the main Watch_Dogs.exe file", "", { "Watch Dogs exe", "Watch_Dogs.exe" }, false);
	if (f.result().empty()) {
		exit(0);
	}

	settings.gameDir = f.result()[0];
	settings.gameDir = settings.gameDir.substr(0, settings.gameDir.size() - strlen("bin/Watch_Dogs.exe"));
#endif
}

void reloadSettings() {
	std::string contents = readFile("settings.xml");
	unserializeFromXML(settings, contents.c_str());

#if WD2
	if (settings.gameDir2.empty() || !std::filesystem::exists(settings.gameDir2)) {
		promptGameDir();
		saveSettings();
	}

	if (settings.patchDir2.empty() || !std::filesystem::exists(settings.patchDir2)) {
		settings.patchDir2 = "patch2/";
		saveSettings();
	}
#else
	if (settings.gameDir.empty() || !std::filesystem::exists(settings.gameDir)) {
		promptGameDir();
		saveSettings();
	}

	if (settings.patchDir.empty() || !std::filesystem::exists(settings.patchDir)) {
		settings.patchDir = "patch/";
		saveSettings();
	}
#endif
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

void Settings::registerMembers(MemberStructure & ms) {
	REGISTER_MEMBER(gameDir);
	REGISTER_MEMBER(gameDir2);
	REGISTER_MEMBER(patchDir);
	REGISTER_MEMBER(patchDir2);
	REGISTER_MEMBER(soundLang);

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
