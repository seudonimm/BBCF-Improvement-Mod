#include "ScrWindow.h"

#include "Core/interfaces.h"
#include "Core/Settings.h"
#include "Core/utils.h"
#include "Game/gamestates.h"
#include "Overlay/NotificationBar/NotificationBar.h"
#include "Overlay/WindowManager.h"
#include "Overlay/Window/HitboxOverlay.h"
#include "Game/Scr/ScrStateReader.h"
#include "Psapi.h"

void ScrWindow::Draw()
{
    if (m_showDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }

    DrawStatesSection();
}

void ScrWindow::DrawStatesSection()
{
    if (!ImGui::CollapsingHeader("States"))
        return;
    if (ImGui::Button("Load P2 Script")) {
        char* bbcf_base_adress = GetBbcfBaseAdress();
        std::vector<scrState*> states = parse_scr(bbcf_base_adress, 2);
        g_interfaces.player2.SetScrStates(states);
        g_interfaces.player2.states = states;
    }
    auto states = g_interfaces.player2.states;
    static int selected = 0;
    {
        ImGui::BeginChild("left pane", ImVec2(200, 0), true);
        for (int i = 0; i < g_interfaces.player2.states.size(); i++)
        {
            std::string label= g_interfaces.player2.states[i]->name;
            if (ImGui::Selectable(label.c_str(), selected == i))
                selected = i;
        }
        ImGui::EndChild();
        ImGui::SameLine();
    }
    // Right
    {
        ImGui::BeginGroup();
        static bool isOpen = false;
        if (ImGui::Checkbox("Naoto enhanced specials toggle", &isOpen)) {
            memset(&g_interfaces.player2.GetData()->slot2_or_slot4, 24, 4);
        }
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
        if (states.size()>0){
            auto selected_state = states[selected];
            ImGui::Text("%s", selected_state->name.c_str());
            ImGui::Separator();
            ImGui::Text("Addr: %d", selected_state->addr);
            ImGui::Text("Frames: %d", selected_state->frames);
            ImGui::Text("Damage: %d",  selected_state->damage);
            ImGui::Text("Atk_level: %d", selected_state->atk_level);
            ImGui::Text("Hitstun: %d", selected_state->hitstun);
            ImGui::Text("Blockstun: %d", selected_state->blockstun);
            ImGui::Text("Hitstop: %d", selected_state->hitstop);
            ImGui::Text("Starter_rating: %d", selected_state->starter_rating);
            ImGui::Text("Atk_P1: %d", selected_state->attack_p1);
            ImGui::Text("Atk_P2: %d", selected_state->attack_p2);
            ImGui::Text("Hit_overhead: %d", selected_state->hit_overhead);
            ImGui::Text("Hit_low: %d", selected_state->hit_low);
            ImGui::Text("Hit_air_ublockable: %d", selected_state->hit_air_unblockable);
            ImGui::Text("fatal_counter: %d", selected_state->fatal_counter);
            ImGui::Text("Whiff_cancels:", selected_state->fatal_counter);
            for (std::string name: selected_state->whiff_cancel) {
                ImGui::Text("    %s", name.c_str());
            }
            ImGui::Text("Hit_or_block_cancels(gatlings):", selected_state->fatal_counter);
            for (std::string name : selected_state->hit_or_block_cancel) {
                ImGui::Text("    %s", name.c_str());
            }
 
        }
        ImGui::EndChild();


        

        if (ImGui::Button("Set as wakeup action")) {
            states = g_interfaces.player2.states;
            auto selected_state = states[selected];
            std::string substr = "CmnActUkemiLandNLanding";
            std::vector<std::string> matches = {};
            for (auto state : states) {
                std::string name = state->name;
                if (name.find(substr) != std::string::npos) {
                    override_state(state->addr, &selected_state->name[0]);
                    ImGui::Text("found:", name);
                    matches.push_back(name);
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Set as gap action")) {
            states = g_interfaces.player2.states;
            auto selected_state = states[selected];
            std::string substr = "GuardEnd";
            std::vector<std::string> matches = {};
            for (auto state : states) {
                std::string name = state->name;
                if (name.find(substr) != std::string::npos) {
                    override_state(state->addr, &selected_state->name[0]);
                    ImGui::Text("found:", name);
                    matches.push_back(name);
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Use")) {
            states = g_interfaces.player2.states;
            auto selected_state = states[selected];
            memcpy(&(g_interfaces.player2.GetData()->currentScriptActionLocationInMemory), &(selected_state->addr),4);
        }
        ImGui::EndGroup(); 
    
    }

}
