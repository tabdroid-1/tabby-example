#pragma once

#include <Tabby.h>
#include <Tabby/Renderer/Mesh.h>

namespace App {

class Base : public Tabby::Layer {
public:
    Base();
    virtual ~Base() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    void OnUpdate() override;
    virtual void OnImGuiRender() override;
    void OnEvent(Tabby::Event& e) override;

    void OnOverlayRender();

private:
    // Tabby::SceneHierarchyPanel m_SceneHierarchyPanel;
    // Tabby::PropertiesPanel m_PropertiesPanel;
    bool m_ShowPhysicsColliders = true;
    int m_GizmoType = -1;

    Tabby::ViewSpecification m_GeometryViewSpecification;
    Tabby::Shared<Tabby::Image> m_RenderTarget;
    Tabby::Shared<Tabby::Image> m_DepthBuffer;

    Tabby::Shared<Tabby::Mesh> m_Mesh;
    bgfx::ProgramHandle m_ProgramHandle;

    // Tabby::Shared<Tabby::Framebuffer> m_Framebuffer;
    Tabby::UIntVector2 m_ViewportSize = { 1280.0f, 720.0f };
    Tabby::UIntVector2 m_ViewportBounds[2];
    bool m_ViewportFocused = false, m_ViewportHovered = false;
};

}
