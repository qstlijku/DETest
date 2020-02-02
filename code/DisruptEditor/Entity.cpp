#include "Entity.h"

#include <SDL.h>
#include <unordered_set>
#include "Common.h"
#include "ResourceLoader.h"
#include "Hash.h"
#include "DDRenderInterface.h"
#include "glm/gtc/matrix_transform.hpp"
#include "xbgFile.h"
#include "FileHandler.h"
#include "imgui.h"
#include "DB.h"

std::map<std::string, Node> entityLibrary;
std::unordered_map<CPathID, std::string> entityLibraryUID;

void addEntity(CPathID UID, Node &node) {
	Attribute *hidName = node.getAttribute("hidName");
	SDL_assert_release(hidName);

	std::string name = (char*)hidName->buffer.data();
	entityLibrary[name] = node;
	entityLibraryUID[UID] = name;
}

Node* findEntityByUID(CPathID UID) {
	if (entityLibraryUID.count(UID) > 0)
		return &entityLibrary[entityLibraryUID[UID]];
	return NULL;
}

void loadEntityLibrary() {
	SDL_RWops *fp = FH::openFile(("worlds\\" + settings.worldName + "\\generated\\entitylibrary_rt.fcb").c_str());
	if (!fp) {
		SDL_ShowSimpleMessageBox(0, "Disrupt Editor", "Failed to load entity Library", NULL);
		exit(0);
	}

	uint32_t infoOffset = SDL_ReadLE32(fp);
	uint32_t infoCount = SDL_ReadLE32(fp);

	SDL_RWseek(fp, infoOffset, RW_SEEK_SET);

	for (uint32_t i = 0; i < infoCount; ++i) {
		CPathID UID;
		SDL_RWread(fp, &UID.id, sizeof(UID.id), 1);
		uint32_t offset = SDL_ReadLE32(fp) + 8;

		size_t curOffset = SDL_RWtell(fp) + 4;

		SDL_RWseek(fp, offset, RW_SEEK_SET);
		Node entityParent;
		entityParent.deserializeB(fp, false);
		addEntity(UID, *entityParent.children.begin());

		SDL_RWseek(fp, curOffset, RW_SEEK_SET);
	}
	SDL_RWclose(fp);
}
