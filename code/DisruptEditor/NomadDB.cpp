#include "NomadDB.h"

#include "IBinaryArchive.h"
#include "FileHandler.h"
#include "NBCF.h"
#include <unordered_map>

void NomadDBRef::read(IBinaryArchive& fp) {
	fp.serialize(libID);
}

static std::unordered_map<std::string, std::unique_ptr<Node>> libraries;

Node* NomadDB::GetLibrary(const char* library) {
	auto &it = libraries[library];
	if (!it.get()) {
		char filename[255];
		snprintf(filename, sizeof(filename), "generated\\databases\\generic\\%s.lib", library);

		SDL_RWops* fp = FH::openFile(filename);
		if (fp) {
			it = std::make_unique<Node>();
			*it.get() = readFCB(fp);
			SDL_RWclose(fp);
		}
	}
	return it.get();
}

Node* NomadDB::GetLibraryObject(const char* library, CStringID objectID) {
	Node* root = GetLibrary(library);
	if (!root) return nullptr;

	for (Node& it : root->children) {
		if (it.getAttrValue<CStringID>("hidKey") == objectID)
			return &it;
	}

	return nullptr;
}

Node* NomadDB::GetLibraryObject(const NomadDBRef& object) {
	return GetLibraryObject(object.getTypeName(), object.libID);
}
