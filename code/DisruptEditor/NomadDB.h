#pragma once

#include <string>
class IBinaryArchive;

class NomadDBRef {
public:
    NomadDBRef(const char* _type) : type(_type) {}
    std::string_view getTypeName() const { return type; }

    uint32_t libID = -1;
    void read(IBinaryArchive& fp);
private:
    const char* type;
};

