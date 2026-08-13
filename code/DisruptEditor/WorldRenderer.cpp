#include "WorldRenderer.h"

#include <xbgFile.h>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <ResourceLoader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm\gtx\norm.hpp>
#include <Common.h>
#include <FileHandler.h>
#include <IBinaryArchive.h>
#include <World.h>
#include <../EngineTest/systemclass.h>
#include <xbg3File.h>

namespace WorldRenderer {
	static std::mutex mutex;
    struct XBGGeomEntry {
        std::shared_ptr<xbgFile> xbg;
        std::vector<glm::mat4> positions;
    };
    static std::unordered_map<CPathID, XBGGeomEntry> geomEntries;

	Bucket::~Bucket() {
		reset();
	}

	void Bucket::add(CPathID xbg, const glm::mat4& mat, uint64_t instance) {
        //Preload the xbg
        loadXBG(xbg);

		std::lock_guard<std::mutex> lck(mutex);

        Instance &i = instances.emplace_back();
        i.mat = mat;
        i.xbgFile = xbg;
        i.instance = instance;
	}

	void Bucket::reset() {
        instances.clear();
	}

    void Bucket::setEnabled(bool isEnabled) {
        for (auto& it : instances)
            it.enabled = isEnabled;
    }

	void Bucket::draw() {
        for (auto& inst : instances) {
            geomEntries[inst.xbgFile].positions.push_back(inst.mat);
        }
	}

	std::shared_ptr<Bucket> createBucket() {
		return std::make_shared<Bucket>();
	}

    void drawModel(ID3D11DeviceContext* pContext) {
        std::lock_guard<std::mutex> lck(mutex);
        /*
        //SDL_RWops* fp = FH::openFileHash("graphics\\buildings\\facade\\facade_back_gdoor_base_8x12_01a_open.xbg");
        SDL_RWops* fp = FH::openFileHash("worlds\\windy_city\\generated\\batchmeshentity\\batchmeshentity_c2_i0_xn0255_yp2049_xn0129_yp2175_building_low.xbg");
        CBinaryArchiveReader reader(fp);

        std::ofstream output("C:\\DisruptEditor-03_2020\\bdump.xbg", std::ios::binary);

        while (reader.tell() < reader.size())
        {
            uint32_t data = 0;
            reader.serialize(data);
            output.write(reinterpret_cast<const char*>(&data), sizeof(data));
        }
        for (auto xbgPath : world.xbgs)
        {
            //break;
            // TODO: resolve hash conflict (skipping for now)
            long r = xbgPath.rfind("batchmeshentity_c2_i0_xn0255_yp2049_xn0129_yp2175_building_low.xbg");
            if (r > 0)
                continue;
            auto model = loadXBG(xbgPath);
            auto texList = model->getDiffuseTexture();
            for (auto tex : texList)
            {
                long x = tex.rfind("_m.xbt");
                long y = tex.rfind("_d.xbt");
                long w = tex.rfind("_s.xbt");
                long z = tex.rfind("_a.xbt");
                if (x < 0 && y < 0 && w < 0)
                {
                    //OutputDebugStringA(tex.c_str());
                    //OutputDebugStringA("\n");
                }
                
                if (z > 0)
                {
                    std::string toOutput = xbgPath + ": " + tex;
                    OutputDebugStringA(toOutput.c_str());
                    OutputDebugStringA("\n");
                }
            }
        }
        //return;*/
        //auto model = loadXBG("graphics\\buildings\\landmark\\lm_pavillion_pritzker.xbg");
        //auto model = loadXBG("graphics\\buildings\\specific\\07_generalstore.xbg");
        //auto model = loadXBG("graphics\\buildings\\facade\\facade_mod_base_8x12_01a.xbg");

        RenderInterface::instance().objectCB.Model = glm::mat4(1);
        pContext->VSSetShader(RenderInterface::instance().model.pVertexShader, NULL, NULL);
        pContext->PSSetShader(RenderInterface::instance().model.pPixelShader, NULL, NULL);
        
        //SDL_RWops* fp = FH::openFile("char01_head.xbg");
        //SDL_RWops* fp = FH::openFile("robot_dedsec_wrench_01.xbg");
        //SDL_RWops* fp = FH::openFile("fanceiling_01.xbg");
        //SDL_RWops* fp = FH::openFile("river_fish02.xbg");
        /*
        auto model = std::make_shared<xbg3File>();
        CBinaryArchiveReader reader(fp);
        model->open(reader);

        //Create Buffers
        for (auto& it : model->buffers)
            it.createBuffers();
        */
        auto model = loadXBG(world.selectedModel);
        model->draw(pContext);
    }

    void draw(ID3D11DeviceContext *pContext) {
		std::lock_guard<std::mutex> lck(mutex);

        pContext->VSSetShader(RenderInterface::instance().model.pVertexShader, NULL, NULL);
        pContext->PSSetShader(RenderInterface::instance().model.pPixelShader, NULL, NULL);

		for (auto& xbg : geomEntries) {
			if (!xbg.second.xbg)
				xbg.second.xbg = loadXBG(xbg.first);

            xbg.second.xbg->draw(pContext, xbg.second.positions);
		}

        for (auto& xbg : geomEntries)
            xbg.second.positions.clear();
	}
};
