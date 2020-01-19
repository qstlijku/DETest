#include "ImguiWindows.h"

#include "imgui.h"
#include "Common.h"
#include "FileHandler.h"
#include "sbaoFile.h"
#include "spkFile.h"
#include "Audio.h"
#include "dr_wav.h"
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"
#include <SDL.h>
#include "DARE.h"
#include <portable-file-dialogs.h>

static uint32_t loadedSPK = 0;

template <typename T>
void displayImGui(const char *name, CObjectReference<T> &obj) {
	ImGui::InputScalar(name, ImGuiDataType_U32, &obj.refAtomicId);
}

/*void displayImGui(SwitchEventDescriptor &obj) {
	displayImGui(obj.pBase);
	ImGui::InputScalar("switchValueId", ImGuiDataType_U32, &obj.switchTypeId);
	displayImGui("defaultEvent", obj.defaultEvent);
	//displayImGui(obj.m_elements);
}*/

void displayImGui(ResourceDescriptor& obj) {
	ImGui::PushID(&obj);
	ImGui::Text("resourceDescriptor Type: %s", obj.pResourceDesc.type.getReverseName().c_str());
	ImGui::InputFloat("ResVolume", &obj.resVolume.vol.m_volume_dB);

	if (std::holds_alternative<SampleResourceDescriptor>(obj.pResourceDesc.data)) {
		SampleResourceDescriptor &srd = std::get<SampleResourceDescriptor>(obj.pResourceDesc.data);
		
		char saveText[80], saveFilename[80];
		snprintf(saveFilename, sizeof(saveFilename), "%08x.wav", obj.Id);
		snprintf(saveText, sizeof(saveText), "Save %s", saveFilename);

		ImGui::Checkbox("Looping", &srd.bLooping);
		ImGui::Checkbox("Tool", &srd.bTool);
		ImGui::Checkbox("Notifying", &srd.bIsNotifying);
		ImGui::InputScalar("Loop Byte", ImGuiDataType_U32, &srd.ulLoopByte);
		ImGui::InputScalar("Loop Sample", ImGuiDataType_U32, &srd.ulLoopSample);
		ImGui::InputScalar("Bit Rate", ImGuiDataType_U32, &srd.ulBitRate);
		ImGui::InputScalar("Resource Notification User Data", ImGuiDataType_U32, &srd.ulResNotificationUserData);
		switch (srd.CompressionFormat) {
		case 1:
			ImGui::Text("PCM");
			break;
		case 2:
			ImGui::Text("ADPCM");
			break;
		case 3:
			ImGui::Text("SeekableADPCM");
			break;
		case 4:
			ImGui::Text("OGG");
			break;
		default:
			ImGui::Text("Unknown audio format");
		}
		ImGui::Text("Channels: %u", srd.ulNbChannels);
		ImGui::Text("Sample Rate: %u", srd.ulFreq);

		if (ImGui::Button(saveText)) {
			try {
				srd.saveDecoded(saveFilename);
			} catch (...) {

			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Play (Use with caution)")) {
			try {
				Audio::instance().stopAll();
				srd.play();
			} catch (...) { }
		}
		ImGui::SameLine();
		if (ImGui::Button("Replace")) {
			auto newFile = pfd::open_file("Choose a wav file", "", { "Wav File (.wav)", "*.wav" }, false);
			if (!newFile.result().empty()) {
				unsigned int channels, sampleRate;
				drwav_uint64 totalSampleCount;
				short* pSampleData = drwav_open_and_read_file_s16(newFile.result()[0].data(), &channels, &sampleRate, &totalSampleCount);
				if (pSampleData) {
					srd.ulFreq = sampleRate;
					srd.ulNbChannels = channels;
					srd.CompressionFormat = 1;//WAV
					if (srd.stToolSourceFormat.bStream) {
						srd.stToolSourceFormat.bStream = false;
						srd.stToolSourceFormat.bZeroLatency = false;
						srd.stToolSourceFormat.ulOffsetData = 0;
						//Let's create a dummy sbao
						srd.stToolSourceFormat.dataRef = srd.stToolSourceFormat.streamRef;
						uint32_t newID = srd.stToolSourceFormat.dataRef.refAtomicId;
						spkFile& spk = DARE::instance().spks[loadedSPK];
						sbaoFile &newSbao = spk.getSbao(newID);
						newSbao.type = CDobbsID("SndData");
						newSbao.data.emplace<SndData>();
					}
					sbaoFile& sbao = DARE::instance().loadAtomicObject(srd.stToolSourceFormat.dataRef.refAtomicId);
					SndData& snd = std::get<SndData>(sbao.data);
					snd.rawData.resize(totalSampleCount * sizeof(short));
					memcpy(snd.rawData.data(), pSampleData, snd.rawData.size());
					srd.stToolSourceFormat.ulNbBytes = snd.rawData.size();

					drwav_free(pSampleData);
				}
			}
		}
	} else if (std::holds_alternative<MultiTrackResourceDescriptor>(obj.pResourceDesc.data)) {
		MultiTrackResourceDescriptor &mtrd = std::get<MultiTrackResourceDescriptor>(obj.pResourceDesc.data);

		ImGui::Text("Layers: %u", mtrd.m_tracks.size());
		for (uint32_t i = 0; i < mtrd.ulNbTrack; ++i) {
			ImGui::PushID(i);
			if (ImGui::Button("Save"))
				mtrd.saveDecoded("r.wav", i);
			if (ImGui::Button("Play")) {
				Audio::instance().stopAll();
				mtrd.play(i);
			}
			ImGui::PopID();

		}

		/*ImGui::PushID(&mtrd);
		if (ImGui::Button("Save recording.wav"))
			mtrd.saveDecoded("recording.wav", 0);
		ImGui::PopID();*/
	} else if (std::holds_alternative<GranularResourceDescriptor>(obj.pResourceDesc.data)) {
		GranularResourceDescriptor &grd = std::get<GranularResourceDescriptor>(obj.pResourceDesc.data);


		if (ImGui::Button("Save"))
			grd.saveDecoded("record.wav");
		if (ImGui::Button("Play")) {
			Audio::instance().stopAll();
			grd.play();
		}

		/*ImGui::PushID(&mtrd);
		if (ImGui::Button("Save recording.wav"))
			mtrd.saveDecoded("recording.wav", 0);
		ImGui::PopID();*/
	}
	ImGui::PopID();
}

void displayImGui(SndData& obj) {
	ImGui::Text("This is sound data, to replace it, find the parent sbao object");
}

void UI::displayDARE() {
	if (!settings.openWindows["DARE"])
		return;
	if (!ImGui::Begin("DARE Converter", &settings.openWindows["DARE"], 0)) {
		ImGui::End();
		return;
	}

	static uint32_t inputSpk;
	ImGui::InputScalar("SPK", ImGuiDataType_U32, &inputSpk, NULL, NULL, "%08x", ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::SameLine();
	if (ImGui::Button("Load")) {
		DARE::instance().reset();
		loadedSPK = 0;
		try {
			DARE::instance().addSoundResource(inputSpk);
			loadedSPK = inputSpk;
		} catch(...) {}
	}
	ImGui::SameLine();
	if (ImGui::Button("Save SPK")) {
		char buffer[255];
		snprintf(buffer, sizeof(buffer), "soundbinary/%08x.spk", DARE::instance().spks.begin()->first);

		SDL_RWops* fp = FH::openFileWrite(buffer);
		if (fp) {
			CBinaryArchiveWriter writer(fp);
			DARE::instance().spks.begin()->second.open(writer);
			SDL_RWclose(fp);
		} else {
			SDL_ShowSimpleMessageBox(0, "Failed to open file for writing", buffer, NULL);
		}

		//TODO: Save external sbao files
	}
	ImGui::SameLine();
	if (ImGui::Button("XML")) {
		char buffer[24];
		snprintf(buffer, sizeof(buffer), "%08x.spk.xml", loadedSPK);
		writeFile(buffer, serializeToXML(DARE::instance().spks[loadedSPK]));
	}
	ImGui::SameLine();
	if (ImGui::Button("Import XML")) {
		char buffer[24];
		snprintf(buffer, sizeof(buffer), "%08x.spk.xml", loadedSPK);
		unserializeFromXML(DARE::instance().spks[loadedSPK], readFile(buffer).c_str());
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop All Sounds")) {
		Audio::instance().stopAll();
	}
	ImGui::Separator();

	for (auto &spk : DARE::instance().spks) {
		for (sbaoFile &sbao : spk.second.objs) {
			ImGui::PushID(&sbao);

			char buffer[160];
			snprintf(buffer, sizeof(buffer), "Atomic Object: %08x", sbao.Id);

			if (ImGui::Selectable(buffer)) {
				char buffer[16];
				snprintf(buffer, sizeof(buffer), "%08x", sbao.Id);
				SDL_SetClipboardText(buffer);
			}


			std::string typeName = sbao.type.getReverseName();
			ImGui::Text("Type: %s", typeName.c_str());

			if (typeName == "ResourceDescriptor")
				displayImGui(std::get<ResourceDescriptor>(sbao.data));
			else
				displayImGui(sbao);

			ImGui::PopID();
			ImGui::Separator();
		}
	}
	

	ImGui::End();
}
