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
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

static uint32_t loadedSPK = 0;

template <typename T>
void displayImGui(const char *name, CObjectReference<T> &obj) {
	ImGui::InputScalar(name, ImGuiDataType_U32, &obj.refAtomicId);
}

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
	} else if (std::holds_alternative<GranularResourceDescriptor>(obj.pResourceDesc.data)) {
		GranularResourceDescriptor &grd = std::get<GranularResourceDescriptor>(obj.pResourceDesc.data);

		if (ImGui::Button("Save"))
			grd.saveDecoded("record.wav");
		if (ImGui::Button("Play")) {
			Audio::instance().stopAll();
			grd.play();
		}
	}
	ImGui::PopID();
}

void displayImGui(SndData& obj) {
	ImGui::Text("This is sound data, to replace it, find the parent sbao object");
}

// Helper function to scan for SPK files in directory
std::vector<uint32_t> scanForSPKFiles(const std::string& directory) {
	std::vector<uint32_t> spkIds;
	
	try {
		for (const auto& entry : fs::directory_iterator(directory)) {
			if (entry.is_regular_file()) {
				std::string filename = entry.path().filename().string();
				// Check if file ends with .spk
				if (filename.length() >= 12 && filename.substr(filename.length() - 4) == ".spk") {
					// Extract the hex ID from filename (format: XXXXXXXX.spk)
					std::string hexId = filename.substr(0, 8);
					try {
						uint32_t id = std::stoul(hexId, nullptr, 16);
						spkIds.push_back(id);
					} catch (...) {
						// Skip invalid hex IDs
					}
				}
			}
		}
	} catch (...) {
		// Directory doesn't exist or can't be read
	}
	
	return spkIds;
}

// Batch export all audio from an SPK to WAV files
void batchExportSPKToWAV(uint32_t spkId, const std::string& outputDir) {
	try {
		// Create output directory if it doesn't exist
		fs::create_directories(outputDir);
		
		// Load the SPK
		DARE::instance().addSoundResource(spkId);
		
		auto& spk = DARE::instance().spks[spkId];
		int exportedCount = 0;
		
		for (sbaoFile& sbao : spk.objs) {
			std::string typeName = sbao.type.getReverseName();
			
			if (typeName == "ResourceDescriptor") {
				ResourceDescriptor& res = std::get<ResourceDescriptor>(sbao.data);
				
				// Use the SPK ID as the filename prefix
				char filename[256];
				
				if (std::holds_alternative<SampleResourceDescriptor>(res.pResourceDesc.data)) {
					SampleResourceDescriptor& srd = std::get<SampleResourceDescriptor>(res.pResourceDesc.data);
					snprintf(filename, sizeof(filename), "%s/%08x.wav", outputDir.c_str(), spkId);
					
					try {
						srd.saveDecoded(filename);
						exportedCount++;
					} catch (...) {
						// Skip failed exports
					}
					
				} else if (std::holds_alternative<MultiTrackResourceDescriptor>(res.pResourceDesc.data)) {
					MultiTrackResourceDescriptor& mtrd = std::get<MultiTrackResourceDescriptor>(res.pResourceDesc.data);
					
					for (uint32_t i = 0; i < mtrd.ulNbTrack; ++i) {
						snprintf(filename, sizeof(filename), "%s/%08x_track%d.wav", outputDir.c_str(), spkId, i);
						try {
							mtrd.saveDecoded(filename, i);
							exportedCount++;
						} catch (...) {
							// Skip failed exports
						}
					}
					
				} else if (std::holds_alternative<GranularResourceDescriptor>(res.pResourceDesc.data)) {
					GranularResourceDescriptor& grd = std::get<GranularResourceDescriptor>(res.pResourceDesc.data);
					snprintf(filename, sizeof(filename), "%s/%08x.wav", outputDir.c_str(), spkId);
					
					try {
						grd.saveDecoded(filename);
						exportedCount++;
					} catch (...) {
						// Skip failed exports
					}
				}
			}
		}
		
		printf("Exported %d audio files from SPK %08x\n", exportedCount, spkId);
		
	} catch (...) {
		printf("Failed to process SPK %08x\n", spkId);
	}
}

// Batch export SPK to XML
void batchExportSPKToXML(uint32_t spkId, const std::string& outputDir) {
	try {
		fs::create_directories(outputDir);
		
		DARE::instance().addSoundResource(spkId);
		auto& spk = DARE::instance().spks[spkId];
		
		char filename[256];
		snprintf(filename, sizeof(filename), "%s/%08x.spk.xml", outputDir.c_str(), spkId);
		
		writeFile(filename, serializeToXML(spk));
		printf("Exported XML for SPK %08x\n", spkId);
		
	} catch (...) {
		printf("Failed to export XML for SPK %08x\n", spkId);
	}
}

void UI::displayDARE() {
	if (!settings.openWindows["DARE"])
		return;
	if (!ImGui::Begin("DARE Converter", &settings.openWindows["DARE"], 0)) {
		ImGui::End();
		return;
	}

	// === BATCH CONVERTER SECTION ===
	ImGui::SeparatorText("BATCH CONVERTER");
	
	static char soundPatchFolder[256] = "soundbinary";
	static char wavOutputFolder[256] = "exported_wav";
	static char xmlOutputFolder[256] = "exported_xml";
	static bool isProcessing = false;
	static std::vector<uint32_t> foundSPKs;
	
	ImGui::InputText("Sound Patch Folder", soundPatchFolder, sizeof(soundPatchFolder));
	ImGui::InputText("WAV Output Folder", wavOutputFolder, sizeof(wavOutputFolder));
	ImGui::InputText("XML Output Folder", xmlOutputFolder, sizeof(xmlOutputFolder));
	
	if (ImGui::Button("Scan for SPK Files")) {
		foundSPKs = scanForSPKFiles(soundPatchFolder);
		ImGui::OpenPopup("Scan Results");
	}
	
	ImGui::SameLine();
	ImGui::Text("Found: %zu SPK files", foundSPKs.size());
	
	// Scan results popup
	if (ImGui::BeginPopup("Scan Results")) {
		ImGui::Text("Found %zu SPK files:", foundSPKs.size());
		ImGui::Separator();
		for (uint32_t id : foundSPKs) {
			ImGui::Text("%08x.spk", id);
		}
		ImGui::EndPopup();
	}
	
	ImGui::BeginDisabled(isProcessing || foundSPKs.empty());
	
	if (ImGui::Button("Batch Export All to WAV")) {
		isProcessing = true;
		
		for (uint32_t spkId : foundSPKs) {
			DARE::instance().reset();
			batchExportSPKToWAV(spkId, wavOutputFolder);
		}
		
		isProcessing = false;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Export Complete", 
			"All SPK files have been exported to WAV!", nullptr);
	}
	
	ImGui::SameLine();
	
	if (ImGui::Button("Batch Export All to XML")) {
		isProcessing = true;
		
		for (uint32_t spkId : foundSPKs) {
			DARE::instance().reset();
			batchExportSPKToXML(spkId, xmlOutputFolder);
		}
		
		isProcessing = false;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Export Complete", 
			"All SPK files have been exported to XML!", nullptr);
	}
	
	ImGui::SameLine();
	
	if (ImGui::Button("Batch Export Both (WAV + XML)")) {
		isProcessing = true;
		
		for (uint32_t spkId : foundSPKs) {
			DARE::instance().reset();
			batchExportSPKToWAV(spkId, wavOutputFolder);
			DARE::instance().reset();
			batchExportSPKToXML(spkId, xmlOutputFolder);
		}
		
		isProcessing = false;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Export Complete", 
			"All SPK files have been exported to WAV and XML!", nullptr);
	}
	
	ImGui::EndDisabled();
	
	if (isProcessing) {
		ImGui::Text("Processing... Please wait...");
	}
	
	ImGui::Separator();

	// === MANUAL CONVERTER SECTION ===
	ImGui::SeparatorText("MANUAL CONVERTER");
	
	static uint32_t inputSpk;
	ImGui::InputScalar("SPK", ImGuiDataType_U32, &inputSpk, nullptr, nullptr, "%08x", ImGuiInputTextFlags_CharsHexadecimal);
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
			SDL_ShowSimpleMessageBox(0, "Failed to open file for writing", buffer, nullptr);
		}
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

	// === RESOURCE INSPECTOR ===
	ImGui::SeparatorText("RESOURCE INSPECTOR");
	
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
