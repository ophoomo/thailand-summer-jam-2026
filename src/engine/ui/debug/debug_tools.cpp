
#include "audio/audio_interface.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "ui/debug/debug_audio.h"
#include "ui/debug/debug_tool.h"

// ============================================================
// Construction / destruction
// ============================================================

DebugTools::DebugTools(OxRenderer *renderer, SceneManager *scenes, AudioInterface *audio)
{
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGuiStyle &s = ImGui::GetStyle();

    apply_style();
    renderer->initImGui();

    this->m_panel_scene = std::make_unique<DebugScene>(scenes);
    this->m_panel_audio = std::make_unique<DebugAudio>(audio);
}

DebugTools::~DebugTools() = default;

// ============================================================
// Public Methods
// ============================================================

void DebugTools::onDraw(double dt)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar()) {
        // Brand
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.4f, 1.f));
        ImGui::TextUnformatted("DEBUG");
        ImGui::PopStyleColor();
        ImGui::Separator();

        // Panel toggles
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.55f, 0.25f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.30f, 0.65f, 0.30f, 1.f));

        ImGui::MenuItem("Scene", nullptr, &this->m_show_scene);
        ImGui::MenuItem("Audio", nullptr, &this->m_show_audio);

        ImGui::PopStyleColor(2);

        // FPS pushed to the right
        float fps = (dt > 0.0) ? (float)(1.0 / dt) : 0.f;
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f FPS  %.2f ms  ", fps, (float)(dt * 1000.0));
        float text_width = ImGui::CalcTextSize(buf).x;
        float avail = ImGui::GetContentRegionAvail().x;
        if (avail > text_width)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - text_width);

        ImVec4 fps_col = fps >= 55.f   ? ImVec4(0.4f, 0.9f, 0.4f, 1.f)
                         : fps >= 30.f ? ImVec4(1.f, 0.8f, 0.2f, 1.f)
                                       : ImVec4(1.f, 0.35f, 0.35f, 1.f);
        ImGui::PushStyleColor(ImGuiCol_Text, fps_col);
        ImGui::TextUnformatted(buf);
        ImGui::PopStyleColor();

        ImGui::EndMainMenuBar();
    }

    if (this->m_show_scene) {
        this->m_panel_scene->onDraw();
    }
    if (this->m_show_audio) {
        this->m_panel_audio->onDraw();
    }

    ImGui::Render();
}

void DebugTools::apply_style()
{
    ImGui::StyleColorsDark();
    ImGuiStyle &s = ImGui::GetStyle();

    s.WindowRounding = 4.f;
    s.FrameRounding = 3.f;
    s.ScrollbarRounding = 3.f;
    s.GrabRounding = 3.f;
    s.TabRounding = 3.f;
    s.WindowBorderSize = 1.f;
    s.FrameBorderSize = 0.f;
    s.WindowPadding = ImVec2(10, 8);
    s.FramePadding = ImVec2(6, 3);
    s.ItemSpacing = ImVec2(8, 5);

    // Slightly warmer dark palette
    auto *c = s.Colors;
    c[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.96f);
    c[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.06f, 0.08f, 1.00f);
    c[ImGuiCol_Header] = ImVec4(0.20f, 0.45f, 0.20f, 0.80f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.55f, 0.25f, 0.90f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.65f, 0.30f, 1.00f);
    c[ImGuiCol_Button] = ImVec4(0.18f, 0.38f, 0.18f, 0.80f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.50f, 0.25f, 0.90f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.60f, 0.30f, 1.00f);
    c[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.24f, 1.00f);
    c[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.08f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.22f, 0.10f, 1.00f);
    c[ImGuiCol_Tab] = ImVec4(0.12f, 0.22f, 0.12f, 0.80f);
    c[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.42f, 0.20f, 1.00f);
    c[ImGuiCol_TabSelected] = ImVec4(0.18f, 0.38f, 0.18f, 1.00f);
    c[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.65f, 0.30f, 1.00f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.80f, 0.40f, 1.00f);
    c[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.85f, 0.40f, 1.00f);
    c[ImGuiCol_SeparatorHovered] = ImVec4(0.30f, 0.65f, 0.30f, 1.00f);
    c[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.80f, 0.40f, 1.00f);
    c[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.80f, 0.40f, 1.00f);
    c[ImGuiCol_PlotHistogram] = ImVec4(0.35f, 0.72f, 0.35f, 1.00f);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.22f, 0.28f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.28f, 0.36f, 1.00f);
}
