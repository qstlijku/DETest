#include "WorldRenderer.h"

#include <xbgFile.h>
#include <unordered_map>
#include <mutex>
#include <ResourceLoader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm\gtx\norm.hpp>
#include <Common.h>
#include <FileHandler.h>
#include <IBinaryArchive.h>

namespace WorldRenderer {
	static std::mutex mutex;
    struct XBGGeomEntry {
        std::shared_ptr<xbgFile> xbg;
        std::vector<glm::mat4> positions;
    };
    static std::unordered_map<CPathID, XBGGeomEntry> geomEntires;

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
            geomEntires[inst.xbgFile].positions.push_back(inst.mat);
        }
	}

	std::shared_ptr<Bucket> createBucket() {
		return std::make_shared<Bucket>();
	}

    void drawModel(ID3D11DeviceContext* pContext) {
        std::lock_guard<std::mutex> lck(mutex);

        pContext->VSSetShader(RenderInterface::instance().model.pVertexShader, NULL, NULL);
        pContext->PSSetShader(RenderInterface::instance().model.pPixelShader, NULL, NULL);

        auto aiden = loadXBG("graphics\\characters\\char\\char02\\char02.xbg");
        RenderInterface::instance().objectCB.Model = glm::mat4(1);
        aiden->draw(pContext);
    }

    void draw(ID3D11DeviceContext *pContext) {
		std::lock_guard<std::mutex> lck(mutex);

        pContext->VSSetShader(RenderInterface::instance().model.pVertexShader, NULL, NULL);
        pContext->PSSetShader(RenderInterface::instance().model.pPixelShader, NULL, NULL);

		for (auto& xbg : geomEntires) {
			if (!xbg.second.xbg)
				xbg.second.xbg = loadXBG(xbg.first);

            xbg.second.xbg->draw(pContext, xbg.second.positions);
		}

        for (auto& xbg : geomEntires)
            xbg.second.positions.clear();
	}
};
