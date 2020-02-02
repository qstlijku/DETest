#include "WorldRenderer.h"

#include <xbgFile.h>
#include <unordered_map>
#include <mutex>
#include <ResourceLoader.h>
#include <glm\gtx\norm.hpp>
#include <Common.h>

namespace WorldRenderer {
	static std::mutex mutex;
    struct XBGGeomEntry {
        std::shared_ptr<xbgFile> xbg;
        std::vector<glm::mat4> positions;
    };
    static std::unordered_map<CPathID, XBGGeomEntry> geomEntires;
    static std::map<std::pair<int, int>, std::vector<Instance>> spatialHash;
    static const int spatialHashScale = 64;

	Bucket::~Bucket() {
		reset();
	}

	void Bucket::add(CPathID xbg, const glm::mat4& mat, uint64_t instance) {
        //Preload the xbg
        loadXBG(xbg);

        glm::vec2 pos(mat[3]);
        pos = glm::round(pos / (float)spatialHashScale);

		std::lock_guard<std::mutex> lck(mutex);

        Instance &i = instances.emplace_back();
        i.mat = mat;
        i.xbgFile = xbg;
        i.instance = instance;

        spatialHash[std::pair<int, int>(pos.x, pos.y)].push_back(i);
	}

	void Bucket::reset() {
        instances.clear();
	}

    void Bucket::setEnabled(bool isEnabled) {
        for (auto& it : instances)
            it.enabled = isEnabled;
    }

	std::shared_ptr<Bucket> createBucket() {
		return std::make_shared<Bucket>();
	}

    void draw(ID3D11DeviceContext *pContext) {
		std::lock_guard<std::mutex> lck(mutex);

        pContext->VSSetShader(RenderInterface::instance().model.pVertexShader, NULL, NULL);
        pContext->PSSetShader(RenderInterface::instance().model.pPixelShader, NULL, NULL);

        glm::vec2 camPos2D(RenderInterface::instance().camera.location);
        camPos2D = glm::round(camPos2D / (float)spatialHashScale);
        int range = settings.drawDistance / spatialHashScale;
        for (int x = -range; x < range; ++x) {
            for (int y = -range; y < range; ++y) {
                glm::ivec2 pos(glm::ivec2(camPos2D) + glm::ivec2(x, y));
                auto& it = spatialHash.find(std::pair<int, int>(pos.x, pos.y));
                if (it == spatialHash.end())
                    continue;
                for (auto& inst : it->second) {
                    geomEntires[inst.xbgFile].positions.push_back(inst.mat);
                }
            }
        }

		for (auto& xbg : geomEntires) {
			if (!xbg.second.xbg)
				xbg.second.xbg = loadXBG(xbg.first);

            xbg.second.xbg->draw(pContext, xbg.second.positions);
		}

        for (auto& xbg : geomEntires)
            xbg.second.positions.clear();
	}
};
