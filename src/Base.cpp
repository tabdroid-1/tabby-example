#include "Base.h"
// #include <MapLoader.h>
#include <Resources.h>
#include <Components.h>
#include <Tabby/Renderer/ShaderLibrary.h>

#include <bx/math.h>
#include <bx/timer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

float fps = 0;

Tabby::GLTFLoader::GLTFData meshes;

struct PosColorVertex {
    float m_x;
    float m_y;
    float m_z;
    uint32_t m_abgr;

    static void init()
    {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    };

    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout PosColorVertex::ms_layout;

static PosColorVertex s_cubeVertices[] = {
    { -1.0f, 1.0f, 1.0f, 0xff000000 },
    { 1.0f, 1.0f, 1.0f, 0xff0000ff },
    { -1.0f, -1.0f, 1.0f, 0xff00ff00 },
    { 1.0f, -1.0f, 1.0f, 0xff00ffff },
    { -1.0f, 1.0f, -1.0f, 0xffff0000 },
    { 1.0f, 1.0f, -1.0f, 0xffff00ff },
    { -1.0f, -1.0f, -1.0f, 0xffffff00 },
    { 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t s_cubeTriStrip[] = {
    0,
    1,
    2,
    3,
    7,
    1,
    5,
    0,
    4,
    2,
    6,
    7,
    4,
    5,
};

static const uint16_t s_cubePoints[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};

namespace App {

Base::Base()
    : Layer("Base")
{
}

int64_t m_TimeOffset;

void Base::OnAttach()
{
    TB_PROFILE_SCOPE();

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

    /*auto image_asset_handle = Tabby::AssetManager::LoadAssetSource("textures/Tabby.png");*/
    // m_Image = Tabby::AssetManager::GetAsset<Tabby::Image>(image_asset_handle);

    /*Tabby::ShaderLibrary::LoadShader("shaders/vulkan/test.glsl");*/
    /*auto shader = Tabby::ShaderLibrary::GetShader("test.glsl");*/
    /*// meshes = Tabby::GLTFLoader::Parse("scenes/test_map.gltf");*/

    /*Tabby::GLTFParseSpecification gltf_spec;*/
    /*gltf_spec.filePath = "scenes/sponza-small/sponza.gltf";*/
    /*gltf_spec.create_entity_from_mesh = true;*/
    /*meshes = Tabby::GLTFLoader::Parse(gltf_spec);*/

    Tabby::Entity camera = Tabby::World::CreateEntity("Camera");
    auto& camera_camera_component = camera.AddComponent<Tabby::CameraComponent>();
    // camera_camera_component.

    Tabby::ImageSpecification image_spec = Tabby::ImageSpecification::Default();
    image_spec.usage = Tabby::ImageUsage::RENDER_TARGET;
    image_spec.format = bgfx::TextureFormat::Enum::BGRA8;
    image_spec.extent = { Tabby::Application::GetWindow().GetWidth(), Tabby::Application::GetWindow().GetHeight() };

    m_RenderTarget = Tabby::CreateShared<Tabby::Image>(image_spec);

    image_spec.usage = Tabby::ImageUsage::DEPTH_BUFFER;
    image_spec.format = bgfx::TextureFormat::Enum::D32F;
    m_DepthBuffer = Tabby::CreateShared<Tabby::Image>(image_spec);

    PosColorVertex::init();

    std::string vertex_path;
    std::string fragment_path;

    // More ellegant way of loading shaders after dealing with shaderc(bgfx)
    switch (bgfx::getRendererType()) {
    case bgfx::RendererType::OpenGL:
        TB_CORE_INFO("OPENGL");
        vertex_path = "shaders/sources/mesh_shader/glsl/vs.sc.bin";
        fragment_path = "shaders/sources/mesh_shader/glsl/fs.sc.bin";
        break;
    case bgfx::RendererType::OpenGLES:
        TB_CORE_INFO("OPENGLES");
        vertex_path = "shaders/sources/mesh_shader/essl/vs.sc.bin";
        fragment_path = "shaders/sources/mesh_shader/essl/fs.sc.bin";
        break;
    case bgfx::RendererType::Vulkan:
        TB_CORE_INFO("VULKAN");
        vertex_path = "shaders/sources/mesh_shader/spv/vs.sc.bin";
        fragment_path = "shaders/sources/mesh_shader/spv/fs.sc.bin";
        break;
    default:
        break;
    }

    Tabby::ShaderLibrary::LoadShader("simple_mesh_shader", vertex_path, fragment_path);
    m_ProgramHandle = Tabby::ShaderLibrary::GetShader("simple_mesh_shader");

    m_TimeOffset = bx::getHPCounter();

    m_GeometryViewSpecification = {
        0, { m_RenderTarget }, m_DepthBuffer, m_ViewportSize, { 0, 0 }, { 0.0f, 0.0f, 0.0f, 1.0f }
    };

    Tabby::Renderer::SetViewTarget(m_GeometryViewSpecification);

    Tabby::MeshSpecification mesh_spec;

    mesh_spec.name = "CUBESSSS!";
    mesh_spec.program_handle = m_ProgramHandle;
    mesh_spec.vertex_buffer_handle = bgfx::createVertexBuffer(
        bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), PosColorVertex::ms_layout);

    mesh_spec.index_buffer_handle = bgfx::createIndexBuffer(
        bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip)));

    mesh_spec.state = 0
        | BGFX_STATE_WRITE_R
        | BGFX_STATE_WRITE_G
        | BGFX_STATE_WRITE_B
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CW
        | BGFX_STATE_MSAA
        | BGFX_STATE_PT_TRISTRIP;

    m_Mesh = Tabby::Mesh::Create(mesh_spec);
}

void Base::OnDetach()
{
    TB_PROFILE_SCOPE();

    Tabby::World::OnStop();

    /*for (auto mesh : meshes.meshes) {*/
    /*    mesh.second->Destroy();*/
    /*}*/
}

void Base::OnUpdate()
{
    Tabby::World::OnViewportResize(Tabby::Application::GetWindow().GetWidth(), Tabby::Application::GetWindow().GetHeight());
    Tabby::World::Update();
    OnOverlayRender();

    m_GeometryViewSpecification = {
        0, { m_RenderTarget }, m_DepthBuffer, m_ViewportSize, { 0, 0 }, { 30, 30, 30, 255 }
    };

    Tabby::Renderer::SetViewTarget(m_GeometryViewSpecification);

    float time = (float)((bx::getHPCounter() - m_TimeOffset) / double(bx::getHPFrequency()));

    {
        Tabby::Matrix4 view = Tabby::Matrix4(1.0f);
        view = glm::translate(view, { 0.0f, -0.5f, 0.0f });
        view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        Tabby::Matrix4 proj = glm::perspective(glm::radians(60.0f), m_GeometryViewSpecification.render_area.x / (float)m_GeometryViewSpecification.render_area.y, 0.1f, 1000.0f);
        proj[1][1] *= -1;

        Tabby::Renderer::SetViewMatrix(m_GeometryViewSpecification.view_id, view, proj);
    }

    for (uint32_t yy = 0; yy < 11; ++yy) {
        for (uint32_t xx = 0; xx < 11; ++xx) {

            Tabby::Matrix4 transform = Tabby::Matrix4(1);
            transform = glm::translate(transform, { 0.0f, 5.0f, 0.0f });
            transform = glm::rotate(transform, time * glm::radians(10.0f), glm::vec3(0.0f, 0.5f, 1.0f));

            // Set model matrix for rendering.
            m_Mesh->SetTransform(transform);
            Tabby::Renderer::DrawMesh(0, m_Mesh);
        }
    }

    fps = 1.0f / Tabby::Time::GetDeltaTime();
    TB_INFO("FPS: {0} \n\t\tDeltaTime: {1}", fps, Tabby::Time::GetDeltaTime());
}

void Base::OnImGuiRender()
{
    TB_PROFILE_SCOPE();

    // Tabby::World::DrawImGui();

    // Note: Switch this to true to enable dockspace
    static bool dockspaceOpen = false;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    float minWinSizeX = style.WindowMinSize.x;
    style.WindowMinSize.x = 370.0f;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    style.WindowMinSize.x = minWinSizeX;

    // m_SceneHierarchyPanel.OnImGuiRender();
    // m_PropertiesPanel.SetEntity(m_SceneHierarchyPanel.GetSelectedNode(), m_SceneHierarchyPanel.IsNodeSelected());
    // m_PropertiesPanel.OnImGuiRender();

    ImGui::Begin("Stats");

    auto stats = bgfx::getStats();
    ImGui::Text("Renderer Stats:");
    ImGui::Text("Size: %d : %d", stats->width, stats->height);
    ImGui::Text("Draws: %d ", stats->numDraw);
    ImGui::Text("Views: %d ", stats->numViews);
    ImGui::Text("Shaders: %d ", stats->numShaders);
    ImGui::Text("Programs: %d ", stats->numPrograms);
    ImGui::Text("Textures: %d ", stats->numTextures);
    ImGui::Text("Uniforms: %d ", stats->numUniforms);
    ImGui::Text("Computes: %d ", stats->numCompute);
    ImGui::Text("Vertex buffers: %d ", stats->numVertexBuffers);
    ImGui::Text("Vertex layouts: %d ", stats->numVertexLayouts);

    ImGui::Text("CPU time begin: %ld ", stats->cpuTimeBegin);
    ImGui::Text("CPU time end: %ld ", stats->cpuTimeEnd);
    ImGui::Text("CPU time spent: %ld ", stats->cpuTimeEnd - stats->cpuTimeBegin);
    ImGui::Text("Memory used: %ld ", stats->rtMemoryUsed);

    ImGui::Text("GPU time begin: %ld ", stats->gpuTimeBegin);
    ImGui::Text("GPU time end: %ld ", stats->gpuTimeEnd);
    ImGui::Text("GPU time spent: %ld ", stats->gpuTimeEnd - stats->gpuTimeBegin);
    ImGui::Text("GPU memory used: %ld ", stats->gpuMemoryUsed);

    ImGui::NewLine();
    ImGui::Text("View Stats:");
    for (size_t i = 0; i < stats->numViews; i++) {
        ImGui::Text("   View name: %s ", stats->viewStats[i].name);
        ImGui::Text("   View ID: %i ", stats->viewStats[i].view);
        ImGui::Text("   View CPU time begin: %li ", stats->viewStats[i].cpuTimeBegin);
        ImGui::Text("   View CPU time end: %li ", stats->viewStats[i].cpuTimeEnd);
        ImGui::Text("   View CPU time spent: %li ", stats->viewStats[i].cpuTimeEnd - stats->viewStats[i].cpuTimeBegin);

        ImGui::Text("   View GPU time begin: %li ", stats->viewStats[i].gpuTimeBegin);
        ImGui::Text("   View GPU time end: %li ", stats->viewStats[i].gpuTimeEnd);
        ImGui::Text("   View GPU time spent: %li ", stats->viewStats[i].gpuTimeEnd - stats->viewStats[i].gpuTimeBegin);

        ImGui::Text("   View GPU frame num: %u ", stats->viewStats[i].gpuFrameNum);

        ImGui::Separator();
    }

    ImGui::End();

    ImGui::Begin("Settings");

    ImGui::Text("FPS: %.2f", fps);
    ImGui::Text("Viewport size: %d : %d", m_ViewportSize.x, m_ViewportSize.y);
    ImGui::Checkbox("Show physics colliders", &m_ShowPhysicsColliders);

    if (ImGui::Button("File Dialog Open(test)")) {
        TB_INFO("{0}", Tabby::FileDialogs::OpenFile(".png"));
    }
    if (ImGui::Button("File Dialog Save(test)")) {
        TB_INFO("{0}", Tabby::FileDialogs::SaveFile(".png"));
    }

    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 });
    ImGui::Begin("Viewport");
    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset = ImGui::GetWindowPos();
    m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
    m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

    ImGui::Image(m_RenderTarget->Raw().idx, ImVec2 { (float)m_ViewportSize.x, (float)m_ViewportSize.y }, ImVec2 { 0, 1 }, ImVec2 { 1, 0 });
    /*Tabby::UI::RenderImage(Tabby::Renderer::GetRenderPipelineFinalImage(), Tabby::Renderer::GetLinearSampler(), ImVec2 { m_ViewportSize.x, m_ViewportSize.y });*/

    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::End();
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
