#pragma once

#include <Tabby/Asset/AssetManager.h>

#include <fastgltf/core.hpp>

namespace Tabby {
class Mesh;
class Texture;
}

namespace App {

class MapLoader {
public:
    static void Parse(const std::filesystem::path& filePath);

private:
    MapLoader() { }

    enum MaterialUniformFlags : std::uint32_t {
        None = 0 << 0,
        HasAlbedoMap = 1 << 0,
        HasNormalMap = 2 << 0,
        HasRoughnessMap = 3 << 0,
        HasOcclusionMap = 4 << 0,
    };

    struct MaterialUniforms {
        Tabby::Vector4 baseColorFactor;
        float alphaCutoff = 0.f;
        uint32_t flags = 0;

        Tabby::Vector2 padding;
    };

    static void LoadImages(fastgltf::Asset& asset, std::vector<Tabby::Shared<Tabby::Texture>>& images);
    static void LoadMaterials(fastgltf::Asset& asset, std::vector<MaterialUniforms>& materials);
    static void LoadMeshes(fastgltf::Asset& asset, std::vector<Tabby::Shared<Tabby::Texture>>& images, std::vector<MaterialUniforms>& materials, const std::string& mapName);
};

}
