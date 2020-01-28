#pragma once

#include "CStringID.h"
class IBinaryArchive;
class Node;

class NomadDBRef {
public:
    NomadDBRef(const char* _type) : type(_type) {}
    const char* getTypeName() const { return type; }

    CStringID libID = -1;
    void read(IBinaryArchive& fp);
private:
    const char* type;
};

namespace NomadDB {
    Node* GetLibrary(const char* library);
    Node* GetLibraryObject(const char* library, CStringID objectID);
    Node* GetLibraryObject(const NomadDBRef &object);
}