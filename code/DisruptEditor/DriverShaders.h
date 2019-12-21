#pragma once

#include <memory>
#include <vector>
#include <tinyxml2.h>

class DriverShaders {
public:
    DriverShaders();
    static DriverShaders& instance();
    std::vector<std::unique_ptr<tinyxml2::XMLDocument>> materialDescriptors;
    std::unique_ptr<tinyxml2::XMLDocument> samplerStates;
};