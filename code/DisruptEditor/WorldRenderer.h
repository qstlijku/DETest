#pragma once

#include "CPathID.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <d3d11.h>
#include "DDRenderInterface.h"

namespace WorldRenderer {

    struct Instance {
        bool enabled = true;
        CPathID xbgFile;
        glm::mat4 mat;
        uint64_t instance = 0;
    };
    struct Bucket {
        ~Bucket();
        void add(CPathID xbgFile, const glm::mat4 &mat, uint64_t instance = 0);
        void reset();
        void setEnabled(bool isEnabled);
        void draw();
        std::vector<Instance> instances;
    };
    std::shared_ptr<Bucket> createBucket();

    void draw(ID3D11DeviceContext* pContext);
};
