
#include "ui/debug/debug_scene.h"
#include "imgui.h"

// ============================================================
// Construction / destruction
// ============================================================

DebugScene::DebugScene(SceneManager *scenes)
{
    this->m_scenes = scenes;
}

DebugScene::~DebugScene() {}

// ============================================================
// Public Methods
// ============================================================

void DebugScene::onDraw()
{
    // ── Panel ────────────────────────────────────────────────────────────────
    ImGui::SetNextWindowSize(ImVec2(280, 340), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(8, 30), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Scene Inspector##debug", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    // ── Current scene ────────────────────────────────────────────────────────
    const auto &cur = m_scenes->CurrentName();
    const auto &prev = m_scenes->PreviousName();
    ImGui::Text("Current");
    ImGui::SameLine(80);
    ImGui::TextColored(ImVec4(1.f, 0.8f, 0.2f, 1.f), "%s", cur.empty() ? "(none)" : cur.c_str());

    ImGui::Text("Previous");
    ImGui::SameLine(80);
    ImGui::TextDisabled("%s", prev.empty() ? "(none)" : prev.c_str());

    ImGui::Separator();
    ImGui::TextDisabled("Switch scene:");
    ImGui::Spacing();

    // ── Scene list ───────────────────────────────────────────────────────────
    for (const auto &name : m_scenes->SceneNames()) {
        bool is_current = (name == cur);
        if (is_current) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.50f, 0.20f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.50f, 0.20f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.50f, 0.20f, 1.f));
        }

        if (ImGui::Button(name.c_str(), ImVec2(-1, 0)) && !is_current)
            m_scenes->change(name);

        if (is_current)
            ImGui::PopStyleColor(3);
    }

    ImGui::End();
}
