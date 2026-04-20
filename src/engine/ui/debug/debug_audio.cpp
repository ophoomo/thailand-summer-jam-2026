#include "ui/debug/debug_audio.h"
#include "imgui.h"
#include <algorithm>
#include <cstring>

DebugAudio::DebugAudio(AudioInterface *audio) : m_audio(audio) {}

// ============================================================
// Helpers
// ============================================================

static void fmt_duration(char *buf, int len, float sec)
{
    int m = (int)sec / 60;
    int s = (int)sec % 60;
    snprintf(buf, (size_t)len, "%d:%02d", m, s);
}

static void pool_bar(const char *label, int active, int total, ImVec4 col)
{
    char overlay[32];
    snprintf(overlay, sizeof(overlay), "%d / %d", active, total);
    ImGui::TextDisabled("%s", label);
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, col);
    ImGui::ProgressBar((float)active / (float)total, ImVec2(-1, 14), overlay);
    ImGui::PopStyleColor();
}

// ============================================================
// Construction / destruction
// ============================================================

void DebugAudio::onDraw()
{
    ImGui::SetNextWindowSize(ImVec2(480, 480), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(780, 30), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Assets & Audio##debug", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    if (!this->m_audio) {
        ImGui::TextDisabled("No audio backend");
        ImGui::End();
        return;
    }

    // ── Volume & source pools ─────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Audio Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
        float master = this->m_audio->get_master_volume() * 100.f;
        float sfx = this->m_audio->get_sfx_volume() * 100.f;
        float bgm = this->m_audio->get_bgm_volume() * 100.f;

        ImGui::PushItemWidth(-90);
        if (ImGui::SliderFloat("Master##vol", &master, 1.f, 100.f, "%.0f%%",
                               ImGuiSliderFlags_AlwaysClamp))
            this->m_audio->set_master_volume(master / 100.f);
        if (ImGui::SliderFloat("SFX##vol", &sfx, 1.f, 100.f, "%.0f%%",
                               ImGuiSliderFlags_AlwaysClamp))
            this->m_audio->set_sfx_volume(sfx / 100.f);
        if (ImGui::SliderFloat("BGM##vol", &bgm, 1.f, 100.f, "%.0f%%",
                               ImGuiSliderFlags_AlwaysClamp))
            this->m_audio->set_bgm_volume(bgm / 100.f);
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        int sfx2d = this->m_audio->get_active_sfx_count();
        int sfx3d = this->m_audio->get_active_sfx_3d_count();
        ImGui::TextDisabled("Source pools");
        pool_bar("2D ", sfx2d, 16, ImVec4(0.3f, 0.75f, 0.3f, 1.f));
        pool_bar("3D ", sfx3d, 8, ImVec4(0.3f, 0.55f, 0.95f, 1.f));
    }

    ImGui::Spacing();

    // ── BGM ───────────────────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("BGM", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto state = this->m_audio->get_bgm_state();
        auto bgm_name = this->m_audio->get_bgm_name();

        // State badge
        ImVec4 state_col = (state == "playing")  ? ImVec4(0.4f, 0.9f, 0.4f, 1.f)
                           : (state == "paused") ? ImVec4(1.f, 0.8f, 0.2f, 1.f)
                                                 : ImVec4(0.55f, 0.55f, 0.55f, 1.f);
        ImGui::TextColored(state_col, "%s",
                           state == "playing"  ? "● PLAYING"
                           : state == "paused" ? "❚❚ PAUSED"
                                               : "■ STOPPED");
        if (!bgm_name.empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled("  %s", bgm_name.c_str());
        }

        // Playback progress
        if (state == "playing" || state == "paused") {
            float pos = this->m_audio->get_bgm_position();
            float dur = this->m_audio->get_bgm_duration();

            char pos_buf[16], dur_buf[16], overlay[40];
            fmt_duration(pos_buf, sizeof(pos_buf), pos);
            fmt_duration(dur_buf, sizeof(dur_buf), dur);
            snprintf(overlay, sizeof(overlay), "%s / %s", pos_buf, dur_buf);

            float fraction = (dur > 0.f) ? (pos / dur) : 0.f;
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.75f, 0.3f, 1.f));
            ImGui::ProgressBar(fraction, ImVec2(-1, 14), overlay);
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();

        // Transport controls
        ImGui::BeginDisabled(state == "none");
        if (ImGui::Button("Stop"))
            this->m_audio->stop_bgm();
        ImGui::SameLine();
        ImGui::BeginDisabled(state != "playing");
        if (ImGui::Button("Pause"))
            this->m_audio->pause_bgm();
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(state != "paused");
        if (ImGui::Button("Resume"))
            this->m_audio->resume_bgm();
        ImGui::EndDisabled();
        ImGui::EndDisabled();
    }

    ImGui::Spacing();

    // ── Active SFX ───────────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Active SFX", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto sources = this->m_audio->get_sfx_sources_info();
        int sfx2d = this->m_audio->get_active_sfx_count();
        int sfx3d = this->m_audio->get_active_sfx_3d_count();

        // Pool bars + Stop All on same row
        ImGui::BeginGroup();
        pool_bar("2D", sfx2d, 16, ImVec4(0.3f, 0.75f, 0.3f, 1.f));
        pool_bar("3D", sfx3d, 8, ImVec4(0.3f, 0.55f, 0.95f, 1.f));
        ImGui::EndGroup();

        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 80.f);
        ImGui::BeginDisabled(sources.empty());
        if (ImGui::Button("Stop All##sfx", ImVec2(80, 0)))
            this->m_audio->stop_all_sfx();
        ImGui::EndDisabled();

        ImGui::Spacing();

        if (sources.empty()) {
            ImGui::TextDisabled("  No active SFX");
        } else {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 1.f));
            float sfx_list_h = std::min((float)sources.size() * 22.f + 8.f, 140.f);
            ImGui::BeginChild("##active_sfx", ImVec2(-1, sfx_list_h), true);

            for (const auto &src : sources) {
                // [2D] / [3D] badge
                if (src.is_3d) {
                    ImGui::TextColored(ImVec4(0.4f, 0.65f, 1.f, 1.f), "[3D]");
                } else {
                    ImGui::TextColored(ImVec4(0.4f, 0.85f, 0.4f, 1.f), "[2D]");
                }
                ImGui::SameLine();

                // Sound name (truncate if long)
                const char *nm = src.sound_name.empty() ? "?" : src.sound_name.c_str();
                ImGui::TextUnformatted(nm);

                // 3D position tooltip / inline display
                if (src.is_3d && ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("pos  x:%.1f  y:%.1f  z:%.1f", src.pos[0], src.pos[1],
                                      src.pos[2]);
                }

                // Gain bar — right-aligned, before stop button
                float bar_w = 70.f;
                float btn_w = 22.f;
                float right = ImGui::GetContentRegionAvail().x;
                ImGui::SameLine(right - bar_w - btn_w - 6.f);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                                      src.is_3d ? ImVec4(0.3f, 0.55f, 0.95f, 0.8f)
                                                : ImVec4(0.3f, 0.75f, 0.3f, 0.8f));
                char gain_lbl[16];
                snprintf(gain_lbl, sizeof(gain_lbl), "%.0f%%", src.gain * 100.f);
                ImGui::ProgressBar(src.gain, ImVec2(bar_w, 14.f), gain_lbl);
                ImGui::PopStyleColor();

                // Stop button
                ImGui::SameLine();
                char stop_id[32];
                snprintf(stop_id, sizeof(stop_id), "##stop_%d_%d", src.index, (int)src.is_3d);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.2f, 0.2f, 1.f));
                if (ImGui::SmallButton(stop_id))
                    this->m_audio->stop_sfx_source(src.index, src.is_3d);
                ImGui::PopStyleColor(2);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Stop this source");
            }

            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
    }

    ImGui::Spacing();

    // ── Loaded Sounds ─────────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Loaded Sounds", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto names = this->m_audio->get_loaded_names();
        std::sort(names.begin(), names.end());

        // Filter input
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##filter", "Filter sounds...", this->m_filter,
                                 sizeof(this->m_filter));

        ImGui::Spacing();

        // 3D test position
        if (ImGui::TreeNode("3D Test Position")) {
            ImGui::SetNextItemWidth(-1);
            ImGui::SliderFloat3("##3dpos", this->m_test_3d_pos, -50.f, 50.f, "%.1f");
            ImGui::TreePop();
        }

        ImGui::Spacing();

        if (names.empty()) {
            ImGui::TextDisabled("(none)");
        } else {
            // Column header
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.10f, 1.f));
            ImGui::BeginChild("##sounds_header", ImVec2(-1, 18), false,
                              ImGuiWindowFlags_NoScrollbar);
            ImGui::TextDisabled("  Name");
            ImGui::SameLine(130);
            ImGui::TextDisabled("Ch");
            ImGui::SameLine(155);
            ImGui::TextDisabled("Hz");
            ImGui::SameLine(205);
            ImGui::TextDisabled("Dur");
            ImGui::SameLine(245);
            ImGui::TextDisabled("SFX  BGM   3D");
            ImGui::EndChild();
            ImGui::PopStyleColor();

            // Sound rows
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 1.f));
            float list_h = ImGui::GetContentRegionAvail().y - 24.f;
            if (list_h < 60.f)
                list_h = 60.f;
            ImGui::BeginChild("##sounds", ImVec2(-1, list_h), true);

            for (const auto &n : names) {
                // Apply filter
                if (this->m_filter[0] != '\0') {
                    if (n.find(this->m_filter) == std::string::npos)
                        continue;
                }

                auto info = this->m_audio->get_sound_info(n);
                bool is_mono = (info.channels == 1);

                // Row highlight on hover
                ImVec2 row_min = ImGui::GetCursorScreenPos();

                // Name column
                ImVec4 name_col =
                    is_mono ? ImVec4(0.85f, 0.85f, 0.85f, 1.f) : ImVec4(0.65f, 0.75f, 1.f, 1.f);
                ImGui::TextColored(name_col, "  %s", n.c_str());

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s\n%s  %d Hz  %.2f s", n.c_str(),
                                      is_mono ? "Mono" : "Stereo", info.sample_rate,
                                      info.duration_sec);

                // Metadata columns
                if (info.valid) {
                    char dur_buf[12];
                    fmt_duration(dur_buf, sizeof(dur_buf), info.duration_sec);

                    ImGui::SameLine(130);
                    ImGui::TextDisabled("%s", is_mono ? "Mo" : "St");
                    ImGui::SameLine(155);
                    ImGui::TextDisabled("%dk", info.sample_rate / 1000);
                    ImGui::SameLine(205);
                    ImGui::TextDisabled("%s", dur_buf);
                }

                // Action buttons
                ImGui::SameLine(245);
                std::string sfx_id = "SFX##" + n;
                if (ImGui::SmallButton(sfx_id.c_str()))
                    this->m_audio->play_sfx(n);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Play as 2D SFX");

                ImGui::SameLine();
                std::string bgm_id = "BGM##" + n;
                if (ImGui::SmallButton(bgm_id.c_str()))
                    this->m_audio->play_bgm(n, false);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Play as BGM (no loop)");

                ImGui::SameLine();
                ImGui::BeginDisabled(!is_mono);
                std::string d3_id = "3D##" + n;
                if (ImGui::SmallButton(d3_id.c_str()))
                    this->m_audio->play_sfx_3d(n, this->m_test_3d_pos[0], this->m_test_3d_pos[1],
                                               this->m_test_3d_pos[2]);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::SetTooltip(is_mono ? "Play at test 3D position"
                                              : "3D requires mono buffer");
                }
                ImGui::EndDisabled();
            }

            ImGui::EndChild();
            ImGui::PopStyleColor();

            int shown = 0;
            for (const auto &n : names)
                if (this->m_filter[0] == '\0' || n.find(this->m_filter) != std::string::npos)
                    ++shown;
            ImGui::TextDisabled("%d / %zu sound(s)", shown, names.size());
        }
    }

    ImGui::End();
}
