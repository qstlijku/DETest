#pragma once

#include <array>
#include <string>
#include <vector>

class IBinaryArchive;

class realTreeFile {
public:
    std::array<uint32_t, 0x120 / 4> unk1;

    std::vector<uint8_t> unk2;
    std::vector<uint8_t> unk3;
    std::vector<std::string> materials;

    void open(IBinaryArchive &fp);
};
