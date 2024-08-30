#pragma once

#include <Tabby.h>

namespace App {

struct SpawnpointComponent {
    std::string entityName;
    uint32_t spawnerID;

    SpawnpointComponent() = default;
    SpawnpointComponent(const SpawnpointComponent&) = default;
    SpawnpointComponent(const std::string& name)
        : entityName(name)
    {
    }
};

}
