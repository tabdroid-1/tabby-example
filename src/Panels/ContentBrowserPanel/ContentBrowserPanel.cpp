#include <Panels/ContentBrowserPanel/ContentBrowserPanel.h>

#include <Tabby/Asset/AssetManager.h>
#include <imgui.h>

namespace Tabby {

ContentBrowserPanel::ContentBrowserPanel()
    : m_BaseDirectory(std::filesystem::current_path())
    , m_CurrentDirectory(m_BaseDirectory)
{
    m_DirectoryIcon = AssetManager::Get()->GetAsset<Texture>(AssetManager::Get()->LoadAssetSource("icons/ContentBrowser/DirectoryIcon.png", UUID()));
    m_FileIcon = AssetManager::Get()->GetAsset<Texture>(AssetManager::Get()->LoadAssetSource("icons/ContentBrowser/FileIcon.png", UUID()));
}

void ContentBrowserPanel::OnImGuiRender()
{
    if (m_IsOpen) {

        ImGui::Begin("Content Browser");

        if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory)) {
            if (ImGui::Button("<-")) {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        static float padding = 16.0f;
        static float thumbnailSize = 128.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
            const auto& path = directoryEntry.path();
            std::string filenameString = path.filename().string();

            ImGui::PushID(filenameString.c_str());
            Shared<Texture> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

            if (ImGui::BeginDragDropSource()) {
                std::filesystem::path relativePath(path);
                const char* itemPath = (char*)relativePath.c_str();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (strlen(itemPath) + 1) * sizeof(wchar_t));
                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (directoryEntry.is_directory())
                    m_CurrentDirectory /= path.filename();
            }
            ImGui::TextWrapped(filenameString.c_str());

            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::Columns(1);

        ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
        ImGui::SliderFloat("Padding", &padding, 0, 32);

        // TODO: status bar
        ImGui::End();
    }
}

}
