#include "DriverShaders.h"

#include "FileHandler.h"
#include "RML.h"
#include <SDL_rwops.h>
#include "DB.h"

DriverShaders::DriverShaders() {
	//Load Material Descriptors
	SDL_RWops* fp = FH::openFile("engine\\shaders\\materialdescriptors\\filelist.xml.txt");
	SDL_assert_release(fp);
	if (fp) {
		char buffer[255];
		while (RWgets(fp, buffer, sizeof(buffer))) {
			if (strlen(buffer) < 5) continue;
			std::string filename = "engine\\shaders\\materialdescriptors\\";
			filename.append(buffer);

			SDL_RWops* mfp = FH::openFile(filename.c_str());
			SDL_assert_release(mfp);
			if (mfp) {
				materialDescriptors.push_back(loadXml(mfp));
			}
		}
		SDL_RWclose(fp);
	}

	//Load Sampler States
	fp = FH::openFile("engine\\shaders\\samplerstates.xml");
	SDL_assert_release(fp);
	if (fp) {
		samplerStates = loadXml(fp);
	}

	//Add to DB CStringID
	for (auto& it : materialDescriptors) {
		DB::instance().crcList[CStringID(it->RootElement()->Attribute("name"))] = it->RootElement()->Attribute("name");
		for (tinyxml2::XMLElement* itt = it->RootElement()->FirstChildElement("parameter"); itt != NULL; itt = itt->NextSiblingElement("parameter"))
			DB::instance().crcList[CStringID(itt->Attribute("name"))] = itt->Attribute("name");
	}

	for(tinyxml2::XMLElement *it = samplerStates->RootElement()->FirstChildElement("samplerstate"); it != NULL; it = it->NextSiblingElement("samplerstate"))
		DB::instance().crcList[CStringID(it->Attribute("name"))] = it->Attribute("name");
}

DriverShaders& DriverShaders::instance() {
	static DriverShaders ds;
	return ds;
}
