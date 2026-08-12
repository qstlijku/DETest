#include "ImguiWindows.h"

#include "imgui.h"
#include "Common.h"
#include <DB.h>
#include <FileHandler.h>
#include <SDL.h>
#include <DatFat.h>

const char* guessFileExt(int read, const char * guess) {
    if (read == 0) {
        return ".";
    }

    if (read >= 5 &&
        guess[0] == 'M' &&
        guess[1] == 'A' &&
        guess[2] == 'G' &&
        guess[3] == 'M' &&
        guess[4] == 'A') {
        return ".mgb";
    }

    if (read >= 3 &&
        guess[0] == 'B' &&
        guess[1] == 'I' &&
        guess[2] == 'K') {
        return ".bik";
    }

    if (read >= 3 &&
        guess[0] == 'U' &&
        guess[1] == 'E' &&
        guess[2] == 'F') {
        return ".feu";
    }

    if (read >= 3 &&
        guess[0] == 0 &&
        guess[1] == 0 &&
        guess[2] == 0xFF) {
        return ".rml";
    }

    if (read >= 8 &&
        guess[4] == 'h' &&
        guess[5] == 'M' &&
        guess[6] == 'v' &&
        guess[7] == 'N') {
        return ".hMvN";
    }

    if (read >= 8 &&
        guess[4] == 'Q' &&
        guess[5] == 'E' &&
        guess[6] == 'S' &&
        guess[7] == 0) {
        return ".cseq";
    }

    if (read >= 20 &&
        guess[16] == 'W' &&
        guess[17] == 0xE0 &&
        guess[18] == 0xE0 &&
        guess[19] == 'W') {
        return ".hkx";
    }

    if (read >= 4) {
        uint32_t magic = *(uint32_t*)guess;

        if (magic == 0x00584254 || magic == 0x54425800) // '\0XBT'
        {
            return ".xbt";
        }

        if (magic == 0x4D455348) // 'MESH'
        {
            return ".xbg";
        }

        if (magic == 0x54414D00 || magic == 0x004D4154) // '\0MAT'
        {
            return ".material.bin";
        }

        if (magic == 0x53504B02) // 'SPK\2'
        {
            return ".spk";
        }

        if (magic == 0x00032A02) {
            return ".sbao";
        }

        if (magic == 0x4643626E) // 'FCbn'
        {
            return ".fcb";
        }

        if (magic == 0x534E644E) // 'SNdN'
        {
            return ".rnv";
        }

        if (magic == 0x474E5089) // 'PNG\x89'
        {
            return ".png";
        }

        if (magic == 0x4D564D00) {
            return ".MvN";
        }

        if (magic == 0x61754C1B) {
            return ".lua";
        }

        if (magic == 0x47454F4D) {
            return ".xbg";
        }

        if (magic == 0x00014C53) {
            return ".loc";
        }
    }

    /*string text = Encoding.ASCII.GetString(guess, 0, read);

    if (read >= 3 && text.StartsWith("-- ") == true) {
        return new Tuple<string, string>("scripts", "lua");
    }

    if (read >= 6 && text.StartsWith("<root>") == true) {
        return new Tuple<string, string>("misc", "root.xml");
    }

    if (read >= 9 && text.StartsWith("<package>") == true) {
        return new Tuple<string, string>("ui", "mbg.desc");
    }

    if (read >= 12 && text.StartsWith("<NewPartLib>") == true) {
        return new Tuple<string, string>("misc", "NewPartLib.xml");
    }

    if (read >= 14 && text.StartsWith("<BarkDataBase>") == true) {
        return new Tuple<string, string>("misc", "BarkDataBase.xml");
    }

    if (read >= 13 && text.StartsWith("<BarkManager>") == true) {
        return new Tuple<string, string>("misc", "BarkManager.xml");
    }

    if (read >= 17 && text.StartsWith("<ObjectInventory>") == true) {
        return new Tuple<string, string>("misc", "ObjectInventory.xml");
    }

    if (read >= 21 && text.StartsWith("<CollectionInventory>") == true) {
        return new Tuple<string, string>("misc", "CollectionInventory.xml");
    }

    if (read >= 14 && text.StartsWith("<SoundRegions>") == true) {
        return new Tuple<string, string>("misc", "SoundRegions.xml");
    }

    if (read >= 11 && text.StartsWith("<MovieData>") == true) {
        return new Tuple<string, string>("misc", "MovieData.xml");
    }

    if (read >= 8 && text.StartsWith("<Profile") == true) {
        return new Tuple<string, string>("misc", "Profile.xml");
    }

    if (read >= 12 && text.StartsWith("<stringtable") == true) {
        return new Tuple<string, string>("text", "xml");
    }

    if (read >= 5 && text.StartsWith("<?xml") == true) {
        return new Tuple<string, string>("misc", "xml");
    }

    if (read >= 1 && text.StartsWith("<Sequence>") == true) {
        return new Tuple<string, string>("game", "seq");
    }*/
    return ".";
}

void UI::displayDevTools() {
	if (!settings.openWindows["DevTools"])
		return;
	if (!ImGui::Begin("DevTools", &settings.openWindows["DevTools"], 0)) {
		ImGui::End();
		return;
	}

	if (ImGui::Button("Scan FileType")) {
		auto &fileTypes = DB::instance().fileTypes;
		fileTypes.clear();

		for (auto& dat : FH::dats) {
			SDL_Log("Scanning %s", dat.name.c_str());

			for (auto& it : dat.files) {
				SDL_RWops* fp = dat.openRead(it.first);
				if (fp) {
                    char buf[20];
                    int read = SDL_RWread(fp, buf, 1, sizeof(buf));
                    const char* type = guessFileExt(read, buf);

                    fileTypes[it.first] = FH::getTypeFromExtension(type);

					SDL_RWclose(fp);
				}
			}
		}

	}

	ImGui::End();
}
