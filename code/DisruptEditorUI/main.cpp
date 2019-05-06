#include "glad.h"
#include <SDL.h>
#include "Colors.h"
#include "Common.h"
#include "DDRenderInterface.h"
#include "cseqFile.h"
#include "wluFile.h"
#include "xbgFile.h"
#include "materialFile.h"
#include "Camera.h"
#include <map>
#include <unordered_map>
#include "Hash.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "LoadingScreen.h"
#include "Dialog.h"
#include "Entity.h"
#include "DominoBox.h"
#include <future>
#include <unordered_set>
#include <Ntsecapi.h>
#include "RML.h"
#include "glm/gtc/matrix_transform.hpp"
#include "CSector.h"
#include "batchFile.h"
#include "Audio.h"
#include "ImGuizmo.h"
#include "FileHandler.h"
#include "World.h"
#include "ImguiWindows.h"
#include "Serialization.h"
#include "Version.h"
#include "IBinaryArchive.h"
#include "HexBase64.h"
#include "DB.h"
#include "DARE.h"
#include "SplineLoft.h"
#include "CResourceDataBase.h"
#include "embedFile.h"
#include "CMoveResourceDataManager.h"
#include "batchFile.h"
#include "ResourceLoader.h"
#include <glm/gtx/norm.hpp>

#include <Windows.h>
#include <Shellapi.h>
#include <DbgHelp.h>
static LONG WINAPI HandleException(struct _EXCEPTION_POINTERS* apExceptionInfo) {
	HANDLE hFile = ::CreateFile(L"crash.mdmp", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile) {
		_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = apExceptionInfo;
		ExInfo.ClientPointers = FALSE;
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
		::CloseHandle(hFile);

		MessageBox(NULL, L"DisruptEditor has crashed! A crash.mdmp has been written",
			L"Your session has been Disrupted!",
			MB_ICONERROR | MB_OK
		);
	}
	exit(0);
}

static void LogOutputFunction(void* userdata, int category, SDL_LogPriority priority, const char* message) {
	FILE* fp = (FILE*)userdata;
	fprintf(fp, "%s\n", message);
	fflush(fp);
}

int main(int argc, char **argv) {
	SetUnhandledExceptionFilter(HandleException);
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_LogSetOutputFunction(LogOutputFunction, fopen("DisruptEditor.log", "wb"));

	LoadingScreen *loadingScreen = new LoadingScreen;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	reloadSettings();

	SDL_Window* window = SDL_CreateWindow("Disrupt Editor v" DE_VERSIONSTR, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, settings.windowSize.x, settings.windowSize.y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
	if (window == NULL) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create window", SDL_GetError(), NULL);
		return 1;
	}
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);
	SDL_GLContext glcontext2 = SDL_GL_CreateContext(window);
	if (glcontext == NULL || glcontext2 == NULL) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create gl context", SDL_GetError(), window);
		return 1;
	}
	//Share Lists
	BOOL error = wglShareLists((HGLRC)glcontext, (HGLRC)glcontext2);
	if (error == FALSE) {
		DWORD errorCode = GetLastError();
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& lpMsgBuf, 0, NULL);
		MessageBox(NULL, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION);
		LocalFree(lpMsgBuf);
		exit(0);
	}
	SDL_GL_MakeCurrent(window, glcontext);

	SDL_GL_SetSwapInterval(1);
	gladLoadGL();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGui::StyleColorsDark(NULL);
	RenderInterface::instance().window = window;
	RenderInterface::instance().context = glcontext;
	RenderInterface::instance().context2 = glcontext2;
	dd::initialize(&RenderInterface::instance());

	Camera &camera = RenderInterface::instance().camera;
	camera.type = Camera::FLYCAM;

#if _DEBUG
	{
		{
			CMoveResourceDataManager move;
			SDL_RWops* fp = FH::openFile("worlds/windy_city/generated/combinedmovefile.bin");
			CBinaryArchiveReader reader(fp);
			move.open(reader);
		}

		FH::Init();

		Vector<FileInfo> files;// = FH::getFileList("soundbinary", "spk");
		//FILE* fpa = fopen("res/dare.txt", "ab");
		for (auto it : files) {
			it.name[8] = '\0';
			//SDL_Log("Loading %s", file.name);

			uint32_t objID;
			sscanf(it.name.c_str(), "%08x", &objID);

			DARE::instance().reset();
			DARE::instance().addSoundResource(objID);

			/*for (auto ita : DARE::instance().atomicObjects) {
				fprintf(fpa, "%u,%u\n", objID, ita.first);
				fflush(fpa);
			}*/

			/*for (auto it : DARE::instance().atomicObjects) {
				sbaoFile &sbao = it.second.ao;
				std::string typeName = sbao.type.getReverseName();
				if (typeName != "ResourceDescriptor") continue;

				BaseResourceDescriptor &brd = sbao.resourceDescriptor->pResourceDesc;

				typeName = brd.type.getReverseName();
				if (typeName == "SampleResourceDescriptor") {
					if (brd.sampleResourceDescriptor->CompressionFormat == 2 || brd.sampleResourceDescriptor->CompressionFormat == 1 || (brd.sampleResourceDescriptor->stToolSourceFormat.bStream && brd.sampleResourceDescriptor->stToolSourceFormat.uSndDataZeroLatencyMemPart.refAtomicId != 0xFFFFFFFF)) {
						uint32_t spkID = it.second.spkFile;
						uint32_t sbaoID = brd.sampleResourceDescriptor->getHelpfulId();

						char buffer[160];
						snprintf(buffer, sizeof(buffer), "C:\\Users\\Jonathan\\Desktop\\english_sbao\\%08x_%08x_%u.wav", spkID, sbaoID, brd.sampleResourceDescriptor->CompressionFormat);
						try {
							brd.sampleResourceDescriptor->saveDecoded(buffer);
						}
						catch (...) {
							int a = 0;
						}

					}
				}
				else if(typeName == "MultiTrackResourceDescriptor") {
					int a = 1;
				}
				else if (typeName == "GranularResourceDescriptor") {
					/*uint32_t spkID = it.second.spkFile;
					uint32_t sbaoID = brd.granularResourceDescriptor->getHelpfulId();

					char buffer[160];
					snprintf(buffer, sizeof(buffer), "C:\\Users\\Jonathan\\Desktop\\gran_sbao\\%08x_%08x_%u.wav", spkID, sbaoID, brd.granularResourceDescriptor->m_compression);
					try {
						brd.granularResourceDescriptor->saveDecoded(buffer);
					}
					catch (...) {
						int a = 0;
					}*/
				/*}
			}*/

			/*for (auto it : DARE::instance().atomicObjects) {
				sbaoFile &sbao = *it.second.ao;
				uint32_t spkID = it.second.spkFile;
				uint32_t sbaoID = it.first;

				char buffer[160];
				snprintf(buffer, sizeof(buffer), "C:\\Users\\Jonathan\\Desktop\\spk_windy_city\\%08x_%08x.spk.xml", spkID, sbaoID);

				std::string xml = serializeToXML(sbao);
				FILE *fp = fopen(buffer, "wb");
				fwrite(xml.c_str(), 1, xml.size(), fp);
				fclose(fp);
			}*/
		}
	}
#endif

	{
		loadingScreen->setTitle("Setting Up Database");
		std::future<void> f = std::async(FH::Init);
		loadingScreen->waitForFuture(f);

		f = std::async([]() { DB::instance(); });
		loadingScreen->waitForFuture(f);

		loadingScreen->setTitle("Loading Entity Library");
		f = std::async(loadEntityLibrary);
		loadingScreen->waitForFuture(f);

		loadingScreen->setTitle("Loading Language Files");
		//Dialog::instance();

		/*SDL_PumpEvents();
		loadingScreen->setTitle("Loading Particle Library");
		world.particles = loadRml(FH::openFile("worlds/windy_city/generated/windy_city_deploadnewparticles.rml"));*/

		world.spawnPointList = loadXml(FH::openFile("worlds/windy_city/generated/spawnpointlist.xml"));

		std::thread resourceLoader(resourceLoaderThread);
		resourceLoader.detach();

		std::thread sectorThread(world.loadSectors);
		sectorThread.detach();

		std::thread wluThread(world.loadWLUAsync);
		wluThread.detach();

		std::thread batchThread(world.loadBatchAsync);
		batchThread.detach();
	}

	Uint32 ticks = SDL_GetTicks();
	uint64_t frameCount = 0;

	if (settings.maximized)
		SDL_MaximizeWindow(window);
	SDL_ShowWindow(window);
	delete loadingScreen;
	loadingScreen = NULL;

	bool windowOpen = true;
	while (windowOpen) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindVertexArray(RenderInterface::instance().VertexArrayID);
		glEnable(GL_DEPTH_TEST);

		float delta = (SDL_GetTicks() - ticks) / 1000.f;
		ticks = SDL_GetTicks();
		if (delta > 0.5f)
			delta = 0.5f;

		RenderInterface &renderInterface = RenderInterface::instance();

		SDL_GetWindowSize(window, &renderInterface.windowSize.x, &renderInterface.windowSize.y);
		glViewport(0, 0, renderInterface.windowSize.x, renderInterface.windowSize.y);
		renderInterface.View = glm::lookAtLH(camera.location, camera.lookingAt, camera.up);
		renderInterface.Projection = glm::perspective(settings.fov, (float)renderInterface.windowSize.x / renderInterface.windowSize.y, settings.near_plane, settings.far_plane);
		renderInterface.VP = renderInterface.Projection * renderInterface.View;

		world.mutex.lock();

		UI::displayTopMenu();
		UI::displayTempWindows();
		UI::displayWindows();

		int resources;
		std::string file;
		getResourceLoaderProgress(resources, file);
		if (world.loadingProgress != 1.f || resources > 0 || true) {
			ImGui::SetNextWindowPos(ImVec2(15, 25), ImGuiCond_Always);
			ImGui::Begin("Loading", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text("%i%% %s", (int)(world.loadingProgress * 100), world.loadingStatus.c_str());
			ImGui::Text("Resource Loader: %i %s", resources, file.c_str());
			ImGui::End();
		}

		glBindVertexArray(RenderInterface::instance().VertexArrayID);
		CHECK_GL_ERROR();
		
		if (settings.drawTerrain) {
			RenderInterface::instance().terrain.use();
			size_t maxSectors = world.sectors.size();
			for (size_t i = 0; i < maxSectors; ++i)
				world.sectors[i].draw();
		}

		//Draw XBG
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		RenderInterface::instance().model.use();
		glm::mat4 MVP = RenderInterface::instance().VP;
		glUniformMatrix4fv(RenderInterface::instance().model.uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);
		CHECK_GL_ERROR();
		static std::shared_ptr<xbgFile> xbg;
		static int selLod = 0;
		static char buffer[255];
		ImGui::InputText("Filename", buffer, sizeof(buffer));
		if (ImGui::Button("Load")) {
			xbg = loadXBG(buffer);
			selLod = 0;
		}
		static uint32_t hashID = 0;
		ImGui::InputScalar("HashID", ImGuiDataType_U32, &hashID);
		if (ImGui::Button("Load ID")) {
			xbg = loadXBG(hashID);
			selLod = 0;
		}
		if (ImGui::Button("XML")) {
			std::string str = serializeToXML(*xbg);
			SDL_SetClipboardText(str.c_str());
		}
		if (xbg) {
			ImGui::SliderInt("Lod", &selLod, 0, xbg->lods.size() - 1);
			xbg->draw(selLod);
		}

		//Draw Building Batches
		for (auto& it : world.batches) {
			auto& component = it.second->componentMBP;
			for (auto& it : component.batchProcessors) {
				for (auto& it : it.processors) {
					auto batch = std::get_if<batchFile::CGraphicBatchProcessor>(&it.data);
					if (!batch) continue;

					std::shared_ptr<xbgFile> xbg = loadXBG(batch->xbg.file.id);
					for (auto& it : batch->ranges) {
						if (glm::distance2(RenderInterface::instance().camera.location, it.unk1) > 100 * 100)
							continue;
						glm::mat4 MVP = glm::translate(RenderInterface::instance().VP, it.unk1);
						glUniformMatrix4fv(RenderInterface::instance().model.uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);
						xbg->draw(0);
					}
				}
			}

			batchFile::CBuildingMultiBatchProcessor& building = it.second->buildingMBP;
			if (building.lowGeom.id != -1) {
				std::shared_ptr<xbgFile> xbg = loadXBG(building.lowGeom.id);
				glm::mat4 MVP = glm::translate(RenderInterface::instance().VP, building.unk6);
				glUniformMatrix4fv(RenderInterface::instance().model.uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);
				xbg->draw(0);
			}
			if (building.roofGeom.id != -1) {
				std::shared_ptr<xbgFile> xbg = loadXBG(building.roofGeom.id);
				glm::mat4 MVP = glm::translate(RenderInterface::instance().VP, building.unk6);
				glUniformMatrix4fv(RenderInterface::instance().model.uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);
				xbg->draw(0);
			}
			
			/*if (!building.buildingResources.empty())
				std::string a = serializeToXML(*it.second);*/
		}

		ImGui::InputFloat3("CameraPos", &RenderInterface::instance().camera.location.x);

		world.mutex.unlock();

		if (!ImGui::IsAnyWindowHovered())
			camera.update(delta);

		dd::flush(0);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
		frameCount++;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			switch (event.type) {
			case SDL_WINDOWEVENT: {
				switch(event.window.event) {
				case SDL_WINDOWEVENT_CLOSE:
					windowOpen = false;
					saveSettings();
					exit(0);
				case SDL_WINDOWEVENT_RESIZED:
					settings.windowSize = glm::ivec2(event.window.data1, event.window.data2);
					saveSettings();
					break;
				case SDL_WINDOWEVENT_MAXIMIZED:
					settings.maximized = true;
					saveSettings();
					break;
				case SDL_WINDOWEVENT_RESTORED:
					settings.maximized = false;
					saveSettings();
					break;
				}
				break;
			}
			case SDL_DROPFILE: {
				SDL_RWops* fp = SDL_RWFromFile(event.drop.file, "rb");
				Node root = readFCB(fp);
				tinyxml2::XMLPrinter printer;
				root.serializeXML(printer);
				std::string output = event.drop.file + std::string(".xml");
				SDL_RWclose(fp);
				fp = SDL_RWFromFile(output.c_str(), "wb");
				SDL_RWwrite(fp, printer.CStr(), printer.CStrSize(), 1);
				SDL_RWclose(fp);
				break;
			}
			}
		}
	}

	return 0;
}