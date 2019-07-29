#include <stdio.h>
#include <string>
#include "SDL_rwops.h"

#include "locFile.h"
#include "IBinaryArchive.h"
#include "Serialization.h"

void convertLocToXML(const std::string &filename, const std::string &outFilename) {
	SDL_RWops* fp = SDL_RWFromFile(filename.c_str(), "rb");
	if (!fp) {
		printf("Failed to open file for reading\n");
		return;
	}
	CBinaryArchiveReader reader(fp);
	locFile loc;
	loc.open(reader);
	SDL_RWclose(fp);

	std::string str = serializeToXML(loc);
	fp = SDL_RWFromFile(outFilename.c_str(), "wb");
	if (!fp) {
		printf("Failed to open file for writing\n");
		return;
	}
	SDL_RWwrite(fp, str.data(), str.size(), 1);
	SDL_RWclose(fp);
}

void convertXMLToLoc(const std::string &filename, const std::string &outFilename) {
	SDL_RWops* fp = SDL_RWFromFile(filename.c_str(), "rb");
	if (!fp) {
		printf("Failed to open file for reading\n");
		return;
	}
	Vector<char> str(SDL_RWsize(fp) + 1);
	SDL_RWread(fp, str.data(), 1, str.size());
	SDL_RWclose(fp);

	locFile loc;
	unserializeFromXML(loc, str.data());

	fp = SDL_RWFromFile(outFilename.c_str(), "wb");
	if (!fp) {
		printf("Failed to open file for writing\n");
		return;
	}
	CBinaryArchiveWriter writer(fp);
	loc.open(writer);
	SDL_RWclose(fp);
}

bool hasEnding(const std::string  &fullString, const std::string &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	return false;
}

int main(int argc, char **argv) {
	printf("Disrupt Editor - Localization Converter Standalone\nCreated by Jon : https://github.com/j301scott/DisruptEditor\n");
	printf("Original Implementation by celikeins : https://github.com/ahmet-celik\n");

	if (argc < 2) {
		printf("Usage: ConvertLoc.exe patch1_english.loc - Converts to XML\n");
		printf("Usage: ConvertLoc.exe patch1_english.loc.xml - Converts to LOC\n");
		return 0;
	}

	for (int i = 1; i < argc; ++i) {
		printf("Converting file %s\n", argv[i]);
		std::string filename(argv[i]);

		if (hasEnding(filename, ".loc.xml")) {
			std::string outFilename = filename.substr(0, filename.size() - 4);
			printf("Converting to LOC: %s\n", outFilename.c_str());
			convertXMLToLoc(filename, outFilename);
		} else if(hasEnding(filename, ".loc")) {
			std::string outFilename = filename + ".xml";
			printf("Converting to XML: %s\n", outFilename.c_str());
			convertLocToXML(filename, outFilename);
		} else {
			wprintf(L"I'm not sure what to do with this file based on it's file extension ¯\_(ツ)_/¯");
		}

		printf("\n");
	}

	return 0;
}