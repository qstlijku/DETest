#pragma once

#include <vector>
#include <string>
#include <map>
#include "glm/glm.hpp"
#include <SDL_keycode.h>

class Node;
class MemberStructure;

typedef uint32_t SDL_Scancode_V;
typedef uint32_t SDL_Keymod_V;

struct Settings {
	//WD1 Settings
	std::string gameDir;
	std::string patchDir;
	bool wd1_dlc_exclusive = false;
	bool wd1_dlc_pill_people = false;
	bool wd1_dlc_solo = false;

	//WD2 Settings
	std::string patchDir2;
	std::string gameDir2;
	
	std::string platformFolder = "data_win64";
	bool bigEndian = false;
	std::string worldName = "windy_city";//san_francisco on WD2
	std::string wluExtension = "xml.data.fcb";//wlu on WD2
	std::string soundLang = "english";

	bool devTools = false;

	//Graphics Settings
	float near_plane = 0.25f;
	float far_plane = 6500.f;
	float fov = 3.14159f / 3.f;
	float drawDistance = 500.f;
	std::map<std::string, bool> displayComponents;
	bool displayNear = true;
	bool displayFar = true;
	bool displayWorld = true;
	bool drawTerrain = true;
	
	// Window Settings
	glm::ivec2 windowSize = glm::ivec2(1600, 900);
	bool maximized = false;
	std::map<std::string, bool> openWindows;

	//Camera Controls
	SDL_Scancode_V keyForward = SDL_SCANCODE_W;
	SDL_Scancode_V keyBackward = SDL_SCANCODE_S;
	SDL_Scancode_V keyLeft = SDL_SCANCODE_A;
	SDL_Scancode_V keyRight = SDL_SCANCODE_D;
	SDL_Scancode_V keyAscend = SDL_SCANCODE_R;
	SDL_Scancode_V keyDescend = SDL_SCANCODE_F;
	SDL_Keymod_V keyFast = KMOD_LSHIFT;
	SDL_Keymod_V keySlow = KMOD_LCTRL;
	float flyMultiplier = 1.f;

	void registerMembers(MemberStructure &ms);
};

extern Settings settings;

void reloadSettings();
void saveSettings();

std::string readFile(const std::string &file);
bool writeFile(const std::string &file, const std::string &contents);
