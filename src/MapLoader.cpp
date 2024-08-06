#include <MapLoader.h>
#include <CustomComponent/Components.h>
#include <Tabby.h>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <stb_image.h>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace App {

void MapLoader::Parse(const std::filesystem::path& filePath)
{
    if (filePath.extension() == ".glb") {
        TB_ERROR("Only .gltf is supported for loader maps. {}", filePath.string());
        return;
    }

    fastgltf::Asset m_Asset;
    std::vector<Tabby::Shared<Tabby::Texture>> m_Images;
    std::vector<MaterialUniforms> m_Materials;

    std::string mapName = filePath.filename();
    size_t lastDot = mapName.find_last_of('.');

    if (lastDot != std::string::npos) {
        mapName = mapName.substr(0, lastDot);
    }

    std::string err;
    std::string warn;

    fastgltf::Parser parser;

    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::LoadExternalBuffers | fastgltf::Options::GenerateMeshIndices;

#ifdef TB_PLATFORM_ANDROID
    auto asset = parser.loadFileFromApk(filePath);
#else
    auto gltfFile = fastgltf::MappedGltfFile::FromPath(filePath);

    std::string message = "Failed to load glTF file: " + std::string(fastgltf::getErrorMessage(gltfFile.error()));
    TB_CORE_ASSERT_TAGGED(bool(gltfFile), message);
    auto asset = parser.loadGltf(gltfFile.get(), filePath.parent_path(), gltfOptions);
#endif

    message = "Failed to load glTF file: " + std::string(fastgltf::getErrorMessage(asset.error()));
    TB_CORE_ASSERT_TAGGED(asset.error() == fastgltf::Error::None, message);

    m_Asset = std::move(asset.get());
    auto& defaultMaterial = m_Materials.emplace_back();

    LoadImages(m_Asset, m_Images);
    LoadMaterials(m_Asset, m_Materials);
    LoadMeshes(m_Asset, m_Images, m_Materials, mapName);
}

void MapLoader::LoadImages(fastgltf::Asset& asset, std::vector<Tabby::Shared<Tabby::Texture>>& images)
{
    auto getLevelCount = [](int width, int height) -> int {
        return static_cast<int>(1 + floor(log2(width > height ? width : height)));
    };

    for (auto& image : asset.images) {
        Tabby::Shared<Tabby::Texture> imageptr;
        std::visit(fastgltf::visitor {
                       [](auto& arg) {},
                       [&](fastgltf::sources::URI& filePath) {
                           TB_CORE_ASSERT_TAGGED(filePath.fileByteOffset == 0, "File offsets not supported");
                           TB_CORE_ASSERT_TAGGED(filePath.uri.isLocalPath(), "Only local files allowed"); // We're only capable of loading local files.

                           const std::string path(filePath.uri.path());

                           Tabby::AssetHandle handle = Tabby::AssetManager::Get()->LoadAssetSource(path, handle);
                           imageptr = Tabby::AssetManager::Get()->GetAsset<Tabby::Texture>(handle);
                       },
                   },
            image.data);

        images.emplace_back(imageptr);
    }
}

void MapLoader::LoadMaterials(fastgltf::Asset& asset, std::vector<MaterialUniforms>& m_Materials)
{
    for (auto& material : asset.materials) {

        auto uniforms = m_Materials.emplace_back();
        uniforms.alphaCutoff = material.alphaCutoff;

        uniforms.baseColorFactor = Tabby::Vector4(material.pbrData.baseColorFactor.x(), material.pbrData.baseColorFactor.y(), material.pbrData.baseColorFactor.z(), material.pbrData.baseColorFactor.w());
        if (material.pbrData.baseColorTexture.has_value())
            uniforms.flags |= MaterialUniformFlags::HasAlbedoMap;

        if (material.normalTexture.has_value())
            uniforms.flags |= MaterialUniformFlags::HasNormalMap;

        if (material.pbrData.metallicRoughnessTexture.has_value())
            uniforms.flags |= MaterialUniformFlags::HasRoughnessMap;

        if (material.occlusionTexture.has_value())
            uniforms.flags |= MaterialUniformFlags::HasOcclusionMap;

        m_Materials.emplace_back(uniforms);
    }
}

void MapLoader::LoadMeshes(fastgltf::Asset& asset, std::vector<Tabby::Shared<Tabby::Texture>>& images, std::vector<MaterialUniforms>& materials, const std::string& mapName)
{

    std::vector<Tabby::Shared<Tabby::Mesh>> tabbyMeshes;

    std::vector<int> requiredMeshIndexes;
    for (auto& node : asset.nodes) {
        if (node.name.substr(0, 3) == "br_" || node.name.substr(0, 3) == "dt_")
            requiredMeshIndexes.push_back(node.meshIndex.value());
        else
            requiredMeshIndexes.push_back(-1);
    }

    int meshIndex = 0;
    for (auto& mesh : asset.meshes) {

        if (requiredMeshIndexes[meshIndex] == -1) {
            meshIndex++;
            continue;
        }
        meshIndex++;

        // TB_INFO("Mesh name: {}", mesh.name);
        for (auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it) {

            auto tabbyMesh = Tabby::CreateShared<Tabby::Mesh>();
            tabbyMesh->SetName(mesh.name.c_str());
            auto* positionIt = it->findAttribute("POSITION");
            TB_CORE_ASSERT(positionIt != it->attributes.end()); // A mesh primitive is required to hold the POSITION attribute.
            TB_CORE_ASSERT(it->indicesAccessor.has_value()); // We specify GenerateMeshIndices, so we should always have indices

            auto index = std::distance(mesh.primitives.begin(), it);
            // auto& primitive = .primitives[index];
            tabbyMesh->SetPrimitiveType((Tabby::Mesh::PrimitiveType)fastgltf::to_underlying(it->type));

            std::size_t materialUniformsIndex;
            std::size_t baseColorTexcoordIndex;

            Tabby::Shared<Tabby::Material> tabbyMaterial;
            if (Tabby::Renderer::GetAPI() == Tabby::RendererAPI::API::OpenGL46)
                tabbyMaterial = Tabby::CreateShared<Tabby::Material>("UnlitMaterial", "shaders/gl46/Renderer3D_MeshUnlit.glsl");
            else if (Tabby::Renderer::GetAPI() == Tabby::RendererAPI::API::OpenGL33)
                tabbyMaterial = Tabby::CreateShared<Tabby::Material>("UnlitMaterial", "shaders/gl33/Renderer3D_MeshUnlit.glsl");
            else if (Tabby::Renderer::GetAPI() == Tabby::RendererAPI::API::OpenGLES3)
                tabbyMaterial = Tabby::CreateShared<Tabby::Material>("UnlitMaterial", "shaders/gles3/Renderer3D_MeshUnlit.glsl");

            if (it->materialIndex.has_value()) {
                materialUniformsIndex = it->materialIndex.value() + 1; // Adjust for default material
                auto& material = asset.materials[it->materialIndex.value()];

                auto& baseColorTexture = material.pbrData.baseColorTexture;
                if (baseColorTexture.has_value()) {
                    auto& texture = asset.textures[baseColorTexture->textureIndex];
                    if (!texture.imageIndex.has_value())
                        return;

                    auto test = images[texture.imageIndex.value()];
                    tabbyMaterial->SetAlbedoMap(images[texture.imageIndex.value()]);

                    if (baseColorTexture->transform && baseColorTexture->transform->texCoordIndex.has_value()) {
                        baseColorTexcoordIndex = baseColorTexture->transform->texCoordIndex.value();
                    } else {
                        baseColorTexcoordIndex = material.pbrData.baseColorTexture->texCoordIndex;
                    }
                }
            } else {
                materialUniformsIndex = 0;
            }

            std::vector<Tabby::Mesh::Vertex> meshVertices;
            {
                // Position
                auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
                if (!positionAccessor.bufferViewIndex.has_value())
                    continue;

                meshVertices.resize(positionAccessor.count);

                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, positionAccessor, [&](fastgltf::math::fvec3 pos, std::size_t idx) {
                    meshVertices[idx].Position = Tabby::Vector4(pos.x(), pos.y(), pos.z(), 0.0f);
                });
            }

            auto texcoordAttribute = std::string("TEXCOORD_") + std::to_string(baseColorTexcoordIndex);
            if (const auto* texcoord = it->findAttribute(texcoordAttribute); texcoord != it->attributes.end()) {
                // Tex coord
                auto& texCoordAccessor = asset.accessors[texcoord->accessorIndex];
                if (!texCoordAccessor.bufferViewIndex.has_value())
                    continue;

                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset, texCoordAccessor, [&](fastgltf::math::fvec2 uv, std::size_t idx) {
                    meshVertices[idx].TexCoords = Tabby::Vector2(uv.x(), uv.y());
                });
            }

            auto& indexAccessor = asset.accessors[it->indicesAccessor.value()];
            if (!indexAccessor.bufferViewIndex.has_value())
                return;
            std::vector<uint32_t> meshIndices(indexAccessor.count);

            fastgltf::copyFromAccessor<std::uint32_t>(asset, indexAccessor, (std::uint32_t*)meshIndices.data());

            tabbyMesh->SetMaterial(tabbyMaterial);
            tabbyMesh->SetVertices(meshVertices);
            tabbyMesh->SetIndices(meshIndices);

            tabbyMesh->Create();
            tabbyMeshes.push_back(tabbyMesh);
        }
    }

    auto SceneEntity = Tabby::World::CreateEntity("Map");

    for (auto& node : asset.nodes) {

        // TB_INFO("Node name: {}", node.name);
        auto ent = Tabby::World::CreateEntity(node.name.c_str());
        auto& tc = ent.GetComponent<Tabby::TransformComponent>();

        std::visit(fastgltf::visitor { [&](const fastgltf::math::fmat4x4& matrix) {
                                          fastgltf::math::fvec3 scale;
                                          fastgltf::math::fquat rotation;
                                          fastgltf::math::fvec3 translation;
                                          fastgltf::math::decomposeTransformMatrix(matrix, scale, rotation, translation);

                                          Tabby::Vector3 Gscale = { scale.x(), scale.y(), scale.z() };
                                          Tabby::Quaternion Grotation = { rotation.w(), rotation.x(), rotation.y(), rotation.z() };
                                          Tabby::Vector3 Gtranslation = { translation.x(), translation.y(), translation.z() };
                                          Tabby::Matrix4 rotMat = glm::toMat4(Grotation);
                                          tc.ApplyTransformToLocal(glm::translate(Tabby::Matrix4(1.0f), (Tabby::Vector3&)Gtranslation) * rotMat * glm::scale(Tabby::Matrix4(1.0f), (Tabby::Vector3&)Gscale));
                                      },
                       [&](const fastgltf::TRS& trs) {
                           Tabby::Vector3 Gscale = { trs.scale.x(), trs.scale.y(), trs.scale.z() };
                           Tabby::Quaternion Grotation = { trs.rotation.w(), trs.rotation.x(), trs.rotation.y(), trs.rotation.z() };
                           Tabby::Vector3 Gtranslation = { trs.translation.x(), trs.translation.y(), trs.translation.z() };
                           Tabby::Matrix4 rotMat = glm::toMat4(Grotation);
                           tc.ApplyTransformToLocal(glm::translate(Tabby::Matrix4(1.0f), (Tabby::Vector3&)Gtranslation) * rotMat * glm::scale(Tabby::Matrix4(1.0f), (Tabby::Vector3&)Gscale));
                       } },
            node.transform);

        if (node.name.substr(0, 3) == "br_") {
            auto& mC = ent.AddComponent<Tabby::MeshComponent>();
            mC.m_Mesh = tabbyMeshes[node.meshIndex.value()];
        } else if (node.name.substr(0, 3) == "sp_") {
            auto& sc = ent.AddComponent<App::SpawnpointComponent>();
        } else if (node.name.substr(0, 3) == "dt_") {

            auto& sc = ent.AddComponent<Tabby::SpriteRendererComponent>();
            if (tabbyMeshes[node.meshIndex.value()]->GetMaterial()->GetAlbedoMap()) {
                sc.Texture = tabbyMeshes[node.meshIndex.value()]->GetMaterial()->GetAlbedoMap()->Handle; // :3
            }
            tc.LocalRotation.x = 0;
            tc.LocalScale.x *= 2;
            tc.LocalScale.y *= 2;
        }

        SceneEntity.AddChild(ent);
    }
}
}
