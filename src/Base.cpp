#include "Base.h"
// #include <MapLoader.h>
#include <Resources.h>
#include <Components.h>

#include <Tabby/World/WorldRenderer.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

float fps = 0;

Tabby::GLTFLoader::GLTFData meshes;

struct Uniform {
    Tabby::Matrix4 model;
    Tabby::Matrix4 view;
    Tabby::Matrix4 proj;
};

namespace App {

Base::Base()
    : Layer("Base")
{
}

void Base::OnAttach()
{
    TB_PROFILE_SCOPE();

    // Tabby::FramebufferSpecification fbSpec;
    // fbSpec.Attachments = { Tabby::FramebufferTextureFormat::RGBA8, Tabby::FramebufferTextureFormat::RED_INTEGER, Tabby::FramebufferTextureFormat::DEPTH24STENCIL8 };
    // fbSpec.Width = 2560;
    // fbSpec.Height = 1600;
    // fbSpec.Samples = 1;
    // m_Framebuffer = Tabby::Framebuffer::Create(fbSpec);

    Tabby::World::Init();

    {
        auto cameraEntity = Tabby::World::CreateEntity("cameraEntity");
        auto& cc = cameraEntity.AddComponent<Tabby::CameraComponent>();
        // cc.Camera.SetOrthographicFarClip(10000);
        // cc.Camera.SetOrthographicSize(100);
        cameraEntity.AddComponent<Tabby::AudioListenerComponent>();
    }

    Tabby::Application::GetWindow().SetVSync(false);

    // MapLoader::Parse("scenes/test_map.gltf");

    auto& data = Tabby::World::AddResource<PlayerInputData>();

    Tabby::World::OnStart();

    auto image_asset_handle = Tabby::AssetManager::LoadAssetSource("textures/Tabby.png");
    // m_Image = Tabby::AssetManager::GetAsset<Tabby::Image>(image_asset_handle);

    Tabby::ImageSamplerSpecification sampler_spec = {};
    sampler_spec.min_filtering_mode = Tabby::SamplerFilteringMode::LINEAR;
    sampler_spec.mag_filtering_mode = Tabby::SamplerFilteringMode::NEAREST;
    sampler_spec.mipmap_filtering_mode = Tabby::SamplerFilteringMode::LINEAR;
    sampler_spec.address_mode = Tabby::SamplerAddressMode::REPEAT;
    sampler_spec.min_lod = 0.0f;
    sampler_spec.max_lod = 1000.0f;
    sampler_spec.lod_bias = 0.0f;
    sampler_spec.anisotropic_filtering_level = 16;

    Tabby::ShaderSpecification shader_spec = Tabby::ShaderSpecification::Default();
    shader_spec.culling_mode = Tabby::PipelineCullingMode::NONE;
    shader_spec.output_attachments_formats = { Tabby::ImageFormat::RGBA32_UNORM };

    Tabby::ShaderLibrary::LoadShader(shader_spec, "shaders/vulkan/test.glsl");
    auto shader = Tabby::ShaderLibrary::GetShader("test.glsl");
    meshes = Tabby::GLTFLoader::Parse("scenes/test_map.gltf");
    // meshes = Tabby::GLTFLoader::Parse("scenes/sponza-small/sponza.gltf");

    for (auto& data : meshes.mesh_data) {
        if (!data.primitives.size())
            continue;

        Tabby::MaterialSpecification mat_spec;
        mat_spec.name = "test_mat";
        mat_spec.shader = shader;

        for (auto primitive : data.primitives) {
            Tabby::Shared<Tabby::Material> material = Tabby::Material::Create(mat_spec);

            primitive.primitive->SetMaterial(material);
            // for (auto& image : primitive.images) {
            //     primitive.primitive->GetMaterial()->UploadData("texSampler", 0, image.second, Tabby::Renderer::GetNearestSampler());
            // }

            if (primitive.images.find("albedo") != primitive.images.end())
                primitive.primitive->GetMaterial()->UploadData("texSampler", 0, primitive.images.find("albedo")->second, Tabby::Renderer::GetNearestSampler());
            else
                primitive.primitive->GetMaterial()->UploadData("texSampler", 0, Tabby::AssetManager::GetMissingTexture(), Tabby::Renderer::GetNearestSampler());
        }
    }

    m_WorldRenderer = Tabby::WorldRenderer::Create();
}

void Base::OnDetach()
{
    TB_PROFILE_SCOPE();

    Tabby::World::OnStop();

    for (auto mesh : meshes.meshes) {
        mesh.second->Destroy();
    }
}

void Base::OnUpdate()
{
    // Tabby::Renderer2D::ResetStats();

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    static Uniform* ubo;

    if (!ubo)
        ubo = new Uniform();

    ubo->model = glm::rotate(glm::mat4(1.0f), time * glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo->model = glm::rotate(ubo->model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo->model = glm::scale(ubo->model, { 3.0f, 3.0f, 3.0f });
    ubo->model = glm::translate(ubo->model, { 0.0f, 0.0f, -10.0f });
    ubo->view = Tabby::Matrix4(1.0f);
    ubo->view = glm::translate(ubo->view, { 0.0f, -0.5f, 0.0f });
    // ubo.view = glm::lookAt(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo->view = glm::rotate(ubo->view, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo->proj = glm::perspective(glm::radians(45.0f), Tabby::Application::GetWindow().GetWidth() / (float)Tabby::Application::GetWindow().GetHeight(), 0.1f, 1000.0f);
    ubo->proj[1][1] *= -1;

    for (auto& mesh : meshes.meshes) {
        if (!mesh.second)
            continue;

        m_WorldRenderer->BeginScene();
        Tabby::Renderer::RenderTasks(mesh.second, { Tabby::MaterialData("ubo", 0, ubo, sizeof(Uniform)) });
        m_WorldRenderer->EndScene();
    }

    Tabby::World::Update();
#if !TB_HEADLESS
    OnOverlayRender();
#endif // TB_HEADLESS

    if (Tabby::Input::GetKeyDown(Tabby::Key::Q))
        m_GizmoType = -1;
    if (Tabby::Input::GetKeyDown(Tabby::Key::T))
        m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
    if (Tabby::Input::GetKeyDown(Tabby::Key::S))
        m_GizmoType = ImGuizmo::OPERATION::SCALE;
    if (Tabby::Input::GetKeyDown(Tabby::Key::R))
        m_GizmoType = ImGuizmo::OPERATION::ROTATE;

    fps = 1.0f / Tabby::Time::GetDeltaTime();
    TB_INFO("FPS: {0} \n\t\tDeltaTime: {1}", fps, Tabby::Time::GetDeltaTime());
}

void Base::OnImGuiRender()
{
    // TB_PROFILE_SCOPE();
    //
    // // Tabby::World::DrawImGui();
    //
    // // Note: Switch this to true to enable dockspace
    // static bool dockspaceOpen = false;
    // // static bool dockspaceOpen = true;
    // static bool opt_fullscreen_persistant = true;
    // bool opt_fullscreen = opt_fullscreen_persistant;
    // static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    //
    // // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // // because it would be confusing to have two docking targets within each others.
    // ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    // if (opt_fullscreen) {
    //     ImGuiViewport* viewport = ImGui::GetMainViewport();
    //     ImGui::SetNextWindowPos(viewport->Pos);
    //     ImGui::SetNextWindowSize(viewport->Size);
    //     ImGui::SetNextWindowViewport(viewport->ID);
    //     ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    //     ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    //     window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    //     window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    // }
    //
    // // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    // if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    //     window_flags |= ImGuiWindowFlags_NoBackground;
    //
    // // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // // all active windows docked into it will lose their parent and become undocked.
    // // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    // ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    // ImGui::PopStyleVar();
    //
    // if (opt_fullscreen)
    //     ImGui::PopStyleVar(2);
    //
    // // DockSpace
    // ImGuiIO& io = ImGui::GetIO();
    // ImGuiStyle& style = ImGui::GetStyle();
    // float minWinSizeX = style.WindowMinSize.x;
    // style.WindowMinSize.x = 370.0f;
    // if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    //     ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    //     ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    // }
    //
    // style.WindowMinSize.x = minWinSizeX;
    //
    // m_SceneHierarchyPanel.OnImGuiRender();
    // m_PropertiesPanel.SetEntity(m_SceneHierarchyPanel.GetSelectedNode(), m_SceneHierarchyPanel.IsNodeSelected());
    // m_PropertiesPanel.OnImGuiRender();
    //
    // ImGui::Begin("Stats");
    //
    // auto stats = Tabby::Renderer2D::GetStats();
    // ImGui::Text("Renderer2D Stats:");
    // ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    // ImGui::Text("Quads: %d", stats.QuadCount);
    // ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    // ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
    //
    // ImGui::End();
    //
    // ImGui::Begin("Settings");
    //
    // ImGui::Text("FPS: %.2f", fps);
    // ImGui::DragFloat2("Size", glm::value_ptr(m_ViewportSize));
    // ImGui::Checkbox("Show physics colliders", &m_ShowPhysicsColliders);
    //
    // if (ImGui::Button("File Dialog Open(test)")) {
    //     TB_INFO("{0}", Tabby::FileDialogs::OpenFile(".png"));
    // }
    // if (ImGui::Button("File Dialog Save(test)")) {
    //     TB_INFO("{0}", Tabby::FileDialogs::SaveFile(".png"));
    // }
    //
    // ImGui::End();
    //
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 });
    // ImGui::Begin("Viewport");
    // auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    // auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    // auto viewportOffset = ImGui::GetWindowPos();
    // m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
    // m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
    //
    // m_ViewportFocused = ImGui::IsWindowFocused();
    // m_ViewportHovered = ImGui::IsWindowHovered();
    //
    // Tabby::Application::GetImGuiLayer().BlockEvents(!m_ViewportHovered);
    //
    // ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    // m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
    //
    // uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID(0);
    // ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2 { m_ViewportSize.x, m_ViewportSize.y }, ImVec2 { 0, 1 }, ImVec2 { 1, 0 });
    //
    // // if (ImGui::BeginDragDropTarget()) {
    // //     if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
    // //         const wchar_t* path = (const wchar_t*)payload->Data;
    // //         OpenScene(path);
    // //     }
    // //     ImGui::EndDragDropTarget();
    // // }
    //
    // // Gizmos
    // Tabby::Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedNode();
    // if (selectedEntity && m_GizmoType != -1) {
    //
    //     // Camera
    //
    //     // Runtime camera from entity
    //     auto cameraEntity = Tabby::World::GetPrimaryCameraEntity();
    //     const auto& camera = cameraEntity.GetComponent<Tabby::CameraComponent>().Camera;
    //     const Tabby::Matrix4& cameraProjection = camera.GetProjection();
    //     if (camera.GetProjectionType() == Tabby::Camera::ProjectionType::Perspective)
    //         ImGuizmo::SetOrthographic(false);
    //     else
    //         ImGuizmo::SetOrthographic(true);
    //
    //     ImGuizmo::SetDrawlist();
    //
    //     ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);
    //
    //     Tabby::Matrix4 cameraView = glm::inverse(cameraEntity.GetComponent<Tabby::TransformComponent>().GetTransform());
    //
    //     // Entity transform
    //     auto& tc = selectedEntity.GetComponent<Tabby::TransformComponent>();
    //     Tabby::Matrix4 transform = tc.GetWorldTransform();
    //
    //     // Snapping
    //     bool snap = Tabby::Input::GetKey(Tabby::Key::LControl);
    //     float snapValue = 0.5f; // Snap to 0.5m for translation/scale
    //     // Snap to 45 degrees for rotation
    //     if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
    //         snapValue = 45.0f;
    //
    //     float snapValues[3] = { snapValue, snapValue, snapValue };
    //
    //     ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
    //         (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
    //         nullptr, snap ? snapValues : nullptr);
    //
    //     if (ImGuizmo::IsUsing()) {
    //         Tabby::Vector3 translation, rotation, scale;
    //         Tabby::Math::DecomposeTransform(transform, (Tabby::Vector3&)translation, (Tabby::Vector3&)rotation, (Tabby::Vector3&)scale);
    //
    //         Tabby::Vector3 deltaRotation = Tabby::Math::RAD2DEG * rotation - tc.GetWorldRotation();
    //         tc.position = translation;
    //         tc.rotation += deltaRotation;
    //         tc.scale = scale;
    //     }
    // }
    //
    // ImGui::End();
    // ImGui::PopStyleVar();
    //
    // ImGui::End();
}
void Base::OnOverlayRender()
{
    // Tabby::Entity camera = Tabby::World::GetPrimaryCameraEntity();
    // if (!camera)
    //     return;
    //
    // Tabby::Renderer2D::BeginScene(camera.GetComponent<Tabby::CameraComponent>().Camera, camera.GetComponent<Tabby::TransformComponent>().GetTransform());
    //
    // if (m_ShowPhysicsColliders) {
    //     // Box Colliders
    //     {
    //         auto view = Tabby::World::GetAllEntitiesWith<Tabby::TransformComponent, Tabby::BoxCollider2DComponent>();
    //         for (auto e : view) {
    //             // auto [tc, bc2d] = view.get<Tabby::TransformComponent, Tabby::BoxCollider2DComponent>(entity);
    //             Tabby::Entity entity(e);
    //             auto& tc = entity.GetComponent<Tabby::TransformComponent>();
    //             auto& bc2d = entity.GetComponent<Tabby::BoxCollider2DComponent>();
    //
    //             Tabby::Vector3 translation = tc.GetWorldPosition() + Tabby::Vector3(bc2d.Offset, 0.001f);
    //             Tabby::Vector3 scale = Tabby::Vector3(bc2d.Size * 2.0f, 1.0f);
    //
    //             Tabby::Matrix4 transform = glm::translate(Tabby::Matrix4(1.0f), translation)
    //                 * glm::rotate(Tabby::Matrix4(1.0f), Tabby::Math::DEG2RAD * tc.GetWorldRotation().z, Tabby::Vector3(0.0f, 0.0f, 1.0f))
    //                 * glm::translate(Tabby::Matrix4(1.0f), Tabby::Vector3(bc2d.Offset, 0.001f))
    //                 * glm::scale(Tabby::Matrix4(1.0f), scale);
    //
    //             Tabby::Renderer2D::DrawRect(transform, Tabby::Vector4(0, 1, 0, 1));
    //         }
    //     }
    //
    //     // Circle Colliders
    //     {
    //         auto view = Tabby::World::GetAllEntitiesWith<Tabby::TransformComponent, Tabby::CircleCollider2DComponent>();
    //         for (auto e : view) {
    //             Tabby::Entity entity(e);
    //             auto& tc = entity.GetComponent<Tabby::TransformComponent>();
    //             auto& cc2d = entity.GetComponent<Tabby::CircleCollider2DComponent>();
    //
    //             Tabby::Vector3 translation = tc.GetWorldPosition() + Tabby::Vector3(cc2d.Offset, 0.001f);
    //             Tabby::Vector3 scale = (Tabby::Vector3&)tc.GetWorldScale() * Tabby::Vector3(cc2d.Radius * 2.0f);
    //
    //             Tabby::Matrix4 transform = glm::translate(Tabby::Matrix4(1.0f), (Tabby::Vector3&)tc.GetWorldPosition())
    //                 * glm::rotate(Tabby::Matrix4(1.0f), Tabby::Math::DEG2RAD * tc.GetWorldRotation().z, Tabby::Vector3(0.0f, 0.0f, 1.0f))
    //                 * glm::translate(Tabby::Matrix4(1.0f), Tabby::Vector3(cc2d.Offset, 0.001f))
    //                 * glm::scale(Tabby::Matrix4(1.0f), scale);
    //
    //             Tabby::Renderer2D::DrawCircle(transform, Tabby::Vector4(0, 1, 0, 1), 0.07f);
    //         }
    //     }
    //
    //     // Capsule Colliders
    //     {
    //         auto view = Tabby::World::GetAllEntitiesWith<Tabby::TransformComponent, Tabby::CapsuleCollider2DComponent>();
    //         for (auto e : view) {
    //             Tabby::Entity entity(e);
    //             auto& tc = entity.GetComponent<Tabby::TransformComponent>();
    //             auto& cc2d = entity.GetComponent<Tabby::CapsuleCollider2DComponent>();
    //
    //             Tabby::Vector3 translation1 = tc.GetWorldPosition() + Tabby::Vector3(cc2d.center1, 0.001f);
    //             Tabby::Vector3 scale1 = (Tabby::Vector3&)tc.GetWorldScale() * Tabby::Vector3(cc2d.Radius * 2.0f);
    //             Tabby::Matrix4 transform1 = glm::translate(Tabby::Matrix4(1.0f), (Tabby::Vector3&)tc.GetWorldPosition())
    //                 * glm::rotate(Tabby::Matrix4(1.0f), Tabby::Math::DEG2RAD * tc.GetWorldRotation().z, Tabby::Vector3(0.0f, 0.0f, 1.0f))
    //                 * glm::translate(Tabby::Matrix4(1.0f), Tabby::Vector3(cc2d.center1, 0.001f))
    //                 * glm::scale(Tabby::Matrix4(1.0f), scale1);
    //             Tabby::Renderer2D::DrawCircle(transform1, Tabby::Vector4(0, 1, 0, 1), 0.1f);
    //
    //             Tabby::Vector3 translation2 = tc.GetWorldPosition() + Tabby::Vector3(cc2d.center2, 0.001f);
    //             Tabby::Vector3 scale2 = (Tabby::Vector3&)tc.GetWorldScale() * Tabby::Vector3(cc2d.Radius * 2.0f);
    //             Tabby::Matrix4 transform2 = glm::translate(Tabby::Matrix4(1.0f), (Tabby::Vector3&)tc.GetWorldPosition())
    //                 * glm::rotate(Tabby::Matrix4(1.0f), Tabby::Math::DEG2RAD * tc.GetWorldRotation().z, Tabby::Vector3(0.0f, 0.0f, 1.0f))
    //                 * glm::translate(Tabby::Matrix4(1.0f), Tabby::Vector3(cc2d.center2, 0.001f))
    //                 * glm::scale(Tabby::Matrix4(1.0f), scale2);
    //             Tabby::Renderer2D::DrawCircle(transform2, Tabby::Vector4(0, 1, 0, 1), 0.1f);
    //
    //             Tabby::Vector3 translation3 = tc.GetWorldPosition() + Tabby::Vector3(cc2d.center2, 0.001f);
    //             Tabby::Vector3 scale3 = (Tabby::Vector3&)tc.GetWorldScale() * Tabby::Vector3(cc2d.Radius * 2.0f) * Tabby::Vector3(0.95f, 1.5f, 0.95f);
    //             Tabby::Matrix4 transform3 = glm::translate(Tabby::Matrix4(1.0f), (Tabby::Vector3&)tc.GetWorldPosition())
    //                 * glm::rotate(Tabby::Matrix4(1.0f), Tabby::Math::DEG2RAD * tc.GetWorldRotation().z, Tabby::Vector3(0.0f, 0.0f, 1.0f))
    //                 * glm::scale(Tabby::Matrix4(1.0f), scale3);
    //             Tabby::Renderer2D::DrawRect(transform3, Tabby::Vector4(0, 1, 0, 1));
    //         }
    //     }
    //
    //     // Mesh Colliders
    //     {
    //         auto view = Tabby::World::GetAllEntitiesWith<Tabby::TransformComponent, Tabby::PolygonCollider2DComponent>();
    //         for (auto e : view) {
    //             Tabby::Entity entity(e);
    //             auto& tc = entity.GetComponent<Tabby::TransformComponent>();
    //             auto& pc2d = entity.GetComponent<Tabby::PolygonCollider2DComponent>();
    //
    //             Tabby::Matrix4 rotation = glm::toMat4(glm::quat(glm::radians((glm::vec3)tc.GetWorldRotation())));
    //             Tabby::Matrix4 transform = glm::translate(Tabby::Matrix4(1.0f), (glm::vec3)tc.GetWorldPosition()) * rotation * glm::scale(Tabby::Matrix4(1.0f), (glm::vec3)tc.GetWorldScale());
    //
    //             for (int i = 0; i < pc2d.Points.size(); i++) {
    //
    //                 Tabby::Vector4 p1;
    //                 Tabby::Vector4 p2;
    //
    //                 if (i == pc2d.Points.size() - 1) {
    //                     p1 = { pc2d.Points[i], 1.0f, 1.0f };
    //                     p2 = { pc2d.Points[0], 1.0f, 1.0f };
    //                 } else {
    //                     p1 = { pc2d.Points[i], 1.0f, 1.0f };
    //                     p2 = { pc2d.Points[i + 1], 1.0f, 1.0f };
    //                 }
    //
    //                 p1 = transform * p1;
    //                 p2 = transform * p2;
    //                 Tabby::Renderer2D::DrawLine(p1, p2, Tabby::Vector4(0, 1, 0, 1));
    //             }
    //         }
    //     }
    //
    //     // Segment Colliders
    //     {
    //         auto view = Tabby::World::GetAllEntitiesWith<Tabby::TransformComponent, Tabby::SegmentCollider2DComponent>();
    //         for (auto e : view) {
    //             Tabby::Entity entity(e);
    //             auto& tc = entity.GetComponent<Tabby::TransformComponent>();
    //             auto& sc2d = entity.GetComponent<Tabby::SegmentCollider2DComponent>();
    //
    //             Tabby::Vector3 point1 = tc.GetWorldPosition() + Tabby::Vector3(sc2d.point1, 0.001f);
    //             Tabby::Vector3 point2 = tc.GetWorldPosition() + Tabby::Vector3(sc2d.point2, 0.001f);
    //
    //             // Apply rotation to both endpoints
    //             Tabby::Matrix4 rotationMatrix = glm::rotate(Tabby::Matrix4(1.0f), Tabby::Math::DEG2RAD * tc.GetWorldRotation().z, Tabby::Vector3(0.0f, 0.0f, 1.0f));
    //
    //             point1 = tc.GetWorldPosition() + Tabby::Vector3(rotationMatrix * Tabby::Vector4(sc2d.point1, 0.0f, 0.0f));
    //             point2 = tc.GetWorldPosition() + Tabby::Vector3(rotationMatrix * Tabby::Vector4(sc2d.point2, 0.0f, 0.0f));
    //
    //             // Draw the line
    //             Tabby::Renderer2D::DrawLine(point1, point2, Tabby::Vector4(0, 1, 0, 1));
    //         }
    //     }
    // }
    //
    // // Draw selected entity outline
    // if (Tabby::Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedNode()) {
    //
    //     auto& tc = selectedEntity.GetComponent<Tabby::TransformComponent>();
    //     Tabby::Renderer2D::DrawRect(tc.GetTransform(), Tabby::Vector4(1.0f, 0.5f, 0.0f, 1.0f));
    // }
    //
    // Tabby::Renderer2D::EndScene();
}

void Base::OnEvent(Tabby::Event& e)
{
    Tabby::EventDispatcher dispatcher(e);
    // dispatcher.Dispatch<Tabby::KeyPressedEvent>(TB_BIND_EVENT_FN(Base::OnKeyPressed));
    // dispatcher.Dispatch<Tabby::MouseButtonPressedEvent>(TB_BIND_EVENT_FN(Base::OnMouseButtonPressed));
}
}
