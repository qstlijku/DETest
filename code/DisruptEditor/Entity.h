#pragma once

#include "NBCF.h"
#include "CPathID.h"
#include <map>
#include <unordered_map>

extern std::map<std::string, Node> entityLibrary;
extern std::unordered_map<CPathID, std::string> entityLibraryUID;

void loadEntityLibrary();
Node* findEntityByUID(CPathID UID);

void drawComponent(Node *entity, Node *node, bool drawImGui, bool draw3D);

uint32_t generateEntityIcon(Node *entity);
