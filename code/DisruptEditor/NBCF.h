#pragma once

#include "Vector.h"
#include <string>
#include "tinyxml2.h"
#include <SDL_rwops.h>
#include "CStringID.h"

class IBinaryArchive;

struct fcbHeader {
	char magic[4];
	uint16_t version;
	uint16_t headerFlags;
	uint32_t totalObjectCount;
	uint32_t totalValueCount;
	void swapEndian();
};

class Attribute {
public:
	Attribute() {}
	Attribute(SDL_RWops* fp, bool bigEndian);
	void deserializeA(SDL_RWops *fp, bool bigEndian);
	void deserializeB(SDL_RWops *fp, bool bigEndian);
	void serializeB(SDL_RWops* fp);
	void serializeA(SDL_RWops *fp);
	void deserializeXML(const tinyxml2::XMLAttribute *attr);
	void serializeXML(tinyxml2::XMLPrinter &printer);

	std::string getHashName();
	std::string getHumanReadable();
	CStringID name;
	Vector<uint8_t> buffer;
};

class Node {
public:
	Node() {};
	void deserializeB(SDL_RWops *fp, bool bigEndian);
	void deserializeA(SDL_RWops *fp, Vector<Node*> &list, bool bigEndian);
	void serializeB(SDL_RWops* fp);
	void serializeA(SDL_RWops *fp);
	void deserializeXML(const tinyxml2::XMLElement *node);
	void serializeXML(tinyxml2::XMLPrinter &printer);

	Node* findFirstChild(CStringID hash);
	Attribute* getAttribute(CStringID hash);

	template <typename T>
	T getAttrValue(CStringID hash);

	template <typename T>
	void setAttrValue(CStringID hash, const T& value);

	int countNodes();

	std::string getHashName();
	CStringID name;

	Vector<Node> children;
	Vector<Attribute> attributes;
};

Node readFCB(SDL_RWops *fp);
void readFCB(IBinaryArchive &fp, Node &root);

void writeFCBA(SDL_RWops* fp, Node& node);
void writeFCBB(SDL_RWops *fp, Node &node);

Node mergeNodes(Node& base, Node& patch);

template<typename T>
inline T Node::getAttrValue(CStringID name) {
	T ret;
	memset(&ret, 0, sizeof(ret));
	Attribute *attr = getAttribute(name);
	if (attr) {
		SDL_assert_release(attr->buffer.size() <= sizeof(T));
		memcpy(&ret, attr->buffer.data(), attr->buffer.size());
	}
	return ret;
}

template<typename T>
inline void Node::setAttrValue(CStringID name, const T& value) {
	Attribute* attr = getAttribute(name);
	if (!attr) {
		attr = &attributes.emplace_back();
		attr->name = name;
	}
	attr->buffer.resize(sizeof(T));
	memcpy(attr->buffer.data(), &value, attr->buffer.size());
}
