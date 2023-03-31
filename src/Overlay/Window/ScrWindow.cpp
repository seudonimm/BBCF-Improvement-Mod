#include "ScrWindow.h"

#include "Core/interfaces.h"
#include "Core/Settings.h"
#include "Core/utils.h"
#include "Game/gamestates.h"
#include "Overlay/NotificationBar/NotificationBar.h"
#include "Overlay/WindowManager.h"
#include "Overlay/Window/HitboxOverlay.h"

#include "Psapi.h"
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>

void ScrWindow::Draw()
{
    if (m_showDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }

    DrawStatesSection();
    DrawPlaybackSection();
    DrawReplayTheaterSection();
    DrawVeryExperimentalSection();
}

void ScrWindow::DrawStatesSection()
{
    
    if (!ImGui::CollapsingHeader("States"))
        return;
    if (*g_gameVals.pGameMode != GameMode_Training) {
        ImGui::Text("Only works in training mode");
        return;
    }
    if (g_interfaces.player2.IsCharDataNullPtr()  || g_interfaces.player2.GetData()->charIndex == g_interfaces.player1.GetData()->charIndex) {
        ImGui::TextWrapped("Something invalid, you are in training mode char select, have 2 of the same characters or some other shit i haven't figured out yet that you should tell me so i can fix");
        return;
    }
    static int selected = 0;
    //Code for auto loading script upon character switch, prob move it to OnMatchInit() or smth
   if (p2_old_char_data == NULL || p2_old_char_data != (void*)g_interfaces.player2.GetData()){
        char* bbcf_base_adress = GetBbcfBaseAdress();
        std::vector<scrState*> states = parse_scr(bbcf_base_adress, 2);
        g_interfaces.player2.SetScrStates(states);
        g_interfaces.player2.states = states;
        p2_old_char_data = (void*)g_interfaces.player2.GetData();
        gap_register = {};
        wakeup_register = {};
        selected = 0;
    }


    if (ImGui::Button("Force Load P2 Script")) {
        char* bbcf_base_adress = GetBbcfBaseAdress();
        std::vector<scrState*> states = parse_scr(bbcf_base_adress, 2);
        g_interfaces.player2.SetScrStates(states);
        g_interfaces.player2.states = states;
        gap_register = {};
        wakeup_register = {};
        selected = 0;
    }
    auto states = g_interfaces.player2.states;
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
        static bool isActive_old;
        static bool isActive = false;
        if (ImGui::Checkbox("Naoto EN specials toggle", &isActive)) {
               memset(&g_interfaces.player2.GetData()->slot2_or_slot4, 0x00000018, 4);
        }
        if (isActive) {
            memset(&g_interfaces.player2.GetData()->slot2_or_slot4, 0x00000018, 4);
        }
        else {
            if (isActive != isActive_old) {
                memset(&g_interfaces.player2.GetData()->slot2_or_slot4, 0, 4);
            }
        }
        isActive_old = isActive;
        //ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 100)); // Leave room for 1 line below us
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 150)); // Leave room for 1 line below us
        if (states.size()>0){
            auto selected_state = states[selected];
            ImGui::Text("%s", selected_state->name.c_str());
            ImGui::Separator();
            ImGui::Text("Addr: 0x%x", selected_state->addr);
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
            int item_view_len;
            if (selected_state->hit_or_block_cancel.size() > 5){
               item_view_len = 100; }
            else {
               item_view_len = selected_state->hit_or_block_cancel.size() * 20;
            }
            ImGui::BeginChild("item view", ImVec2(0, item_view_len));
            for (std::string name : selected_state->hit_or_block_cancel) {
                ImGui::Text("    %s", name.c_str());
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();


        
        ImGui::Separator();
        if (ImGui::Button("Set as wakeup action")) {
            states = g_interfaces.player2.states;
            auto selected_state = states[selected];
            std::string substr = "CmnActUkemiLandNLanding";
            std::vector<std::string> matches = {};
            for (auto state : states) {
                std::string name = state->name;
                if (name.find(substr) != std::string::npos) {
                    if (!state->replaced_state_script[0]) {
                        memcpy(state->replaced_state_script, state->addr + 36, 36);
                    }
                    override_state(state->addr, &selected_state->name[0]);

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
                    if (!state->replaced_state_script[0]) {
                        memcpy(state->replaced_state_script, state->addr + 36, 36);
                    }
                    override_state(state->addr, &selected_state->name[0]);

                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Use")) {
            states = g_interfaces.player2.states;
            auto selected_state = states[selected];
            //auto tst = g_interfaces.player2.GetData();
            memcpy(&(g_interfaces.player2.GetData()->currentScriptActionLocationInMemory), &(selected_state->addr),4);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            states = g_interfaces.player2.states;
            for (auto state : states) {
                if (state->replaced_state_script[0]) {
                    memcpy(state->addr + 36, state->replaced_state_script, 36);
                    state->replaced_state_script[0] = 0;
                
                }
            }
            gap_register = {};
            wakeup_register = {};
        }
        
    
        if (ImGui::CollapsingHeader("Gap/wakeup random actions")) {
            ImGui::Columns(2);
            if (ImGui::Button("Add to wakeup action")) {
                states = g_interfaces.player2.states;
                wakeup_register.push_back(states[selected]);
            }
            ImGui::BeginChild("wakeup_register_display");
            for (auto e : wakeup_register) {
                ImGui::Text(e->name.c_str());
            }
            ImGui::EndChild();
            ImGui::NextColumn();
            if (ImGui::Button("Add to gap action")) {
                states = g_interfaces.player2.states;
                gap_register.push_back(states[selected]);
            }
        
            ImGui::BeginChild("gap_register_display");
            for (auto e : gap_register) {
                ImGui::Text(e->name.c_str());
            }
            ImGui::EndChild();
            if (!wakeup_register.empty()) {
                states = g_interfaces.player2.states;
                auto selected_state = states[selected];
                int random_pos = std::rand() % wakeup_register.size();
                std::string substr = "CmnActUkemiLandNLanding";
                for (auto state : states) {
                    std::string name = state->name;
                    if (name.find(substr) != std::string::npos) {
                        if (!state->replaced_state_script[0]) {
                            memcpy(state->replaced_state_script, state->addr + 36, 36);
                        }
                        override_state(state->addr, &wakeup_register[random_pos]->name[0]);

                    }
                }
            }


            if (!gap_register.empty()) {
                states = g_interfaces.player2.states;
                auto selected_state = states[selected];
                int random_pos = std::rand() % gap_register.size();
                std::string substr = "GuardEnd";
                for (auto state : states) {
                    std::string name = state->name;
                    if (name.find(substr) != std::string::npos) {
                        if (!state->replaced_state_script[0]) {
                            memcpy(state->replaced_state_script, state->addr + 36, 36);
                        }
                        override_state(state->addr, &gap_register[random_pos]->name[0]);
                    }
                }
            }
        }
        //ImGui::EndChild();
        ImGui::EndGroup(); 
    
    }

}
std::string interpret_move(char move) {
    //auto button_bits = move & ((4 << 1) - 1);
    auto button_bits = move & 0xf0;
    auto direction_bits = move & 0x0f;
    //auto direction_bits = move & ((8 << 1) - 1);
    std::string move_t{ "" };
    switch (direction_bits) {
    case 0x5:
        move_t += "Neutral";
        break;
    case 0x4:
        move_t += "Left";
        break;
    case 0x1:
        move_t += "DownLeft";
        break;
    case 0x2:
        move_t += "Down";
        break;
    case 0x3:
        move_t += "DownRight";
        break;
    case 0x6:
        move_t += "Right";
        break;
    case 0x9:
        move_t += "UpRight";
        break;
    case 0x8:
        move_t += "Up";
        break;
    case 0x7:
        move_t += "UpLeft";
        break;
    }
    switch (button_bits) {
    case 0x10:
        move_t += "+A";
        break;
    case 0x20:
        move_t += "+B";
        break;
    case 0x40:
        move_t += "+C";
        break;
    case 0x80:
        move_t += "+D";
        break;
    case 0x30:
        move_t += "+A+B";
        break;
    case 0x50:
        move_t += "+A+C";
        break;
    case 0x90:
        move_t += "+A+D";
        break;
    case 0x60:
        move_t += "+B+C";
        break;
    case 0xA0:
        move_t += "+B+D";
        break;
    case 0xC0:
        move_t += "+C+D";
        break;
    case 0xB0:
        move_t += "+A+B+D";
        break;
    case 0x70:
        move_t += "+A+B+D";
        break;
    case 0xD0:
        move_t += "+A+C+D";
        break;
    case 0xE0:
        move_t += "+B+C+D";
        break;
    case 0xF0:
        move_t += "+A+B+C+D";
        break;
    }

    return move_t;
}
void save_to_file(std::vector<char> slot_buffer, char facing_direction, char* fname) {
    CreateDirectory(L"slots", NULL);
    
    std::string fpath = "./slots/";
    fpath += fname;
    std::ofstream out(fpath);
    out << facing_direction;
    for (const auto& e : slot_buffer) out << e;
    out.close();
}
std::vector<char> load_from_file(char* fname) {
    std::string fpath = "./slots/";
    fpath +=fname;
    //char* filename = "./slots/" + fpath;
    std::streampos fileSize;
    std::ifstream file(fpath, std::ios::binary);
    if (file.good()) {
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);


        std::vector<char> fileData(fileSize);
        file.read((char*)&fileData[0], fileSize);
        return fileData;
    }

    else {
        std::vector<char> fd{};
        return fd;
    }
 

}
std::vector<char> trim_playback(std::vector<char> slot_buffer) {

    if (!slot_buffer.empty()) {
        //trim the start
        while (!slot_buffer.empty() && slot_buffer[0] == 5) {
            slot_buffer.erase(slot_buffer.begin());
 
        }
        //trim the end
        while (!slot_buffer.empty() && slot_buffer.back() == 5) {
            slot_buffer.pop_back();
        }
    }

    return slot_buffer;
}
void load_trimmed_playback(std::vector<char> trimmed_playback,char* frame_len_slot_p, char* start_of_slot_inputs) {
    //trim the start
    //auto trimmed_playback = trim_playback(slot_buffer);
    int frame_len_loaded_file = trimmed_playback.size();
    memcpy(frame_len_slot_p, &(frame_len_loaded_file), 4);
    int iter = 0;
    for (auto input : trimmed_playback) {
        memcpy(start_of_slot_inputs + (iter * 2), &input, 2);
        iter++;
    }
    }
void treat_random_slot_checkbox(std::vector<int> &slot_vec, bool slot_toggle, int slot_num) {
    if (slot_toggle) {
        slot_vec.push_back(slot_num);
    }
    else {
        auto iterator = std::find(slot_vec.begin(), slot_vec.end(), slot_num);
        if (iterator != slot_vec.end()) {
            slot_vec.erase(std::find(slot_vec.begin(), slot_vec.end(), slot_num));
        }
    }
};
void ScrWindow::DrawPlaybackSection() {
    char* bbcf_base_adress = GetBbcfBaseAdress();
    char* active_slot = bbcf_base_adress + 0x902C3C;
    if (ImGui::CollapsingHeader("Playback")) {

        if (ImGui::CollapsingHeader("SLOT_1")) {




            int time_count_slot_1_addr_offset = 0x9075E8;
            char* frame_len_slot_p = bbcf_base_adress + time_count_slot_1_addr_offset;
            int frame_len_slot;
            memcpy(&frame_len_slot, frame_len_slot_p, 4);


            int facing_direction_slot_1_addr_offset = 0x9075D8;
            char* facing_direction_p = bbcf_base_adress + facing_direction_slot_1_addr_offset;
            int facing_direction;
            memcpy(&facing_direction, facing_direction_p, 4);



            char* start_of_slot_inputs = bbcf_base_adress + time_count_slot_1_addr_offset + 0x10;
            std::vector<char> slot1_recording_frames{};
            for (int i = 0; i < frame_len_slot; i++) {
                slot1_recording_frames.push_back(*(start_of_slot_inputs + i * 2));
            }
            if (ImGui::Button("Save##slot1")) {
                save_to_file(slot1_recording_frames, facing_direction, fpath_s1);
            }
            ImGui::SameLine();
            if (ImGui::Button("Load##slot1")) {
                auto loaded_file = load_from_file(fpath_s1);
                if (!loaded_file.empty()) {
                    char facing_direction = loaded_file[0];
                    loaded_file.erase(loaded_file.begin());
                    memcpy(facing_direction_p, &(facing_direction), sizeof(char));
                }
                int frame_len_loaded_file = loaded_file.size();
                memcpy(frame_len_slot_p, &(frame_len_loaded_file), 4);
                int iter = 0;

                if (!loaded_file.empty()) {
                    for (auto input : loaded_file) {
                        memcpy(start_of_slot_inputs + (iter * 2), &input, 2);
                        iter++;
                    }
                }

            }
            ImGui::SameLine();
            if (ImGui::Button("Trim Playback##slot1")) {
                slot1_recording_frames = trim_playback(slot1_recording_frames);
                load_trimmed_playback(slot1_recording_frames, frame_len_slot_p, start_of_slot_inputs);
            }
            
            ImGui::InputText("File Path##slot1", fpath_s1, IM_ARRAYSIZE(fpath_s1));
            ImGui::TextWrapped("If the field isn't accepting keyboard input, try alt-tabbing out and back in, if that doesn't work copy and paste should still work(or restarting the game)");
            if (ImGui::Button("Set as gap action##slot1")) {
                slot_gap = 1;
            }
            ImGui::SameLine();
            if (ImGui::Button("Set as wakeup action##slot1")) {
                slot_wakeup = 1;
            }
            

            ImGui::SameLine();
            if (ImGui::Button("Reset##slot1")) {
                slot_gap = 0;
                slot_wakeup = 0;
            }
            ImGui::InputInt("Buffer frames##slot1", &slot_buffer[0]);
            ImGui::Separator();
            auto old_val = 0; auto frame_counter = 0;
            for (auto el : slot1_recording_frames) {
                frame_counter++;
                if (old_val != el) {
                    std::string move_string = interpret_move(el);
                    ImGui::Text("frame %d: %s (0x%x)", frame_counter, move_string.c_str(), el);
                    old_val = el;
                }
            }

        }
        
        if (ImGui::CollapsingHeader("SLOT_2")) {




            int time_count_slot_2_addr_offset = 0x9075EC;
            char* frame_len_slot_p = bbcf_base_adress + time_count_slot_2_addr_offset;
            int frame_len_slot;
            memcpy(&frame_len_slot, frame_len_slot_p, 4);


            int facing_direction_slot_2_addr_offset = 0x9075DC;
            char* facing_direction_p = bbcf_base_adress + facing_direction_slot_2_addr_offset;
            int facing_direction;
            memcpy(&facing_direction, facing_direction_p, 4);


            //0x960 is 2400 which is the size of the recording slot_1, recording slot_2 has this offset + 0xC relative to start of slot_1
            char* start_of_slot_inputs = bbcf_base_adress + time_count_slot_2_addr_offset + 0x960+ 0xC;
            std::vector<char> slot2_recording_frames{};
            for (int i = 0; i < frame_len_slot; i++) {
                slot2_recording_frames.push_back(*(start_of_slot_inputs + i * 2));
            }
            if (ImGui::Button("Save##slot2")) {
                save_to_file(slot2_recording_frames, facing_direction, fpath_s2);
            }
            ImGui::SameLine();
            if (ImGui::Button("Load##slot2")) {
                auto loaded_file = load_from_file(fpath_s2);
                if (!loaded_file.empty()) {
                    char facing_direction = loaded_file[0];
                    loaded_file.erase(loaded_file.begin());
                    memcpy(facing_direction_p, &(facing_direction), sizeof(char));
                }
                int frame_len_loaded_file = loaded_file.size();
                memcpy(frame_len_slot_p, &(frame_len_loaded_file), 4);
                int iter = 0;

                if (!loaded_file.empty()) {
                    for (auto input : loaded_file) {
                        memcpy(start_of_slot_inputs + (iter * 2), &input, 2);
                        iter++;
                    }
                }
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Trim Playback##slot2")) {
                slot2_recording_frames = trim_playback(slot2_recording_frames);
                load_trimmed_playback(slot2_recording_frames, frame_len_slot_p, start_of_slot_inputs);
            }
            ImGui::InputText("File Path##slot2", fpath_s2, IM_ARRAYSIZE(fpath_s2));
            ImGui::TextWrapped("If the field isn't accepting keyboard input, try alt-tabbing out and back in, if that doesn't work copy and paste should still work(or restarting the game)");
            if (ImGui::Button("Set as gap action##slot2")) {
                slot_gap = 2;
            }
            ImGui::SameLine();
            if (ImGui::Button("Set as wakeup action##slot2")) {
                slot_wakeup= 2;
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Reset##slot2")) {
                slot_gap = 0;
                slot_wakeup = 0;
            }
            ImGui::InputInt("Buffer frames ##slot2", &slot_buffer[1]);
            ImGui::Separator();
            auto old_val = 0; auto frame_counter = 0;
            for (auto el : slot2_recording_frames) {
                frame_counter++;
                if (old_val != el) {
                    std::string move_string = interpret_move(el);
                    ImGui::Text("frame %d: %s (0x%x)", frame_counter, move_string.c_str(), el);
                    old_val = el;
                }
            }

        
        }
        
        if (ImGui::CollapsingHeader("SLOT_3")) {




            int time_count_slot_3_addr_offset = 0x9075F0;
            char* frame_len_slot_p = bbcf_base_adress + time_count_slot_3_addr_offset;
            int frame_len_slot;
            memcpy(&frame_len_slot, frame_len_slot_p, 4);


            int facing_direction_slot_3_addr_offset = 0x9075E0;
            char* facing_direction_p = bbcf_base_adress + facing_direction_slot_3_addr_offset;
            int facing_direction;
            memcpy(&facing_direction, facing_direction_p, 4);


            //similar to slot_2 but with 0x8
            char* start_of_slot_inputs = bbcf_base_adress + time_count_slot_3_addr_offset + (0x960 * 2) + 0x8;
            std::vector<char> slot3_recording_frames{};
            for (int i = 0; i < frame_len_slot; i++) {
                slot3_recording_frames.push_back(*(start_of_slot_inputs + i * 2));
            }
            if (ImGui::Button("Save##slot3")) {
                save_to_file(slot3_recording_frames, facing_direction, fpath_s3);
            }
            ImGui::SameLine();
            if (ImGui::Button("Load##slot3")) {
                auto loaded_file = load_from_file(fpath_s3);
                if (!loaded_file.empty()) {
                    char facing_direction = loaded_file[0];
                    loaded_file.erase(loaded_file.begin());
                    memcpy(facing_direction_p, &(facing_direction), sizeof(char));
                }
                int frame_len_loaded_file = loaded_file.size();
                memcpy(frame_len_slot_p, &(frame_len_loaded_file), 4);
                int iter = 0;

                if (!loaded_file.empty()) {
                    for (auto input : loaded_file) {
                        memcpy(start_of_slot_inputs + (iter * 2), &input, 2);
                        iter++;
                    }
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Trim Playback##slot3")) {
                slot3_recording_frames = trim_playback(slot3_recording_frames);
                load_trimmed_playback(slot3_recording_frames, frame_len_slot_p, start_of_slot_inputs);
            }
            ImGui::InputText("File Path##slot3", fpath_s3, IM_ARRAYSIZE(fpath_s3));
            ImGui::TextWrapped("If the field isn't accepting keyboard input, try alt-tabbing out and back in, if that doesn't work copy and paste should still work(or restarting the game)");
            if (ImGui::Button("Set as gap action##slot3")) {
                slot_gap = 3;
            }
            ImGui::SameLine();
            if (ImGui::Button("Set as wakeup action##slot3")) {
                slot_wakeup = 3;
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Reset##slot3")) {
                slot_gap = 0;
                slot_wakeup = 0;
            }
            ImGui::InputInt("Buffer frames ##slot3", &slot_buffer[2]);
            ImGui::Separator();
            auto old_val = 0; auto frame_counter = 0;
            for (auto el : slot3_recording_frames) {
                frame_counter++;
                if (old_val != el) {
                    std::string move_string = interpret_move(el);
                    ImGui::Text("frame %d: %s (0x%x)", frame_counter, move_string.c_str(), el);
                    old_val = el;
                }
            }
          
        }

        if (ImGui::CollapsingHeader("SLOT_4")) {




            int time_count_slot_4_addr_offset = 0x9075F4;
            char* frame_len_slot_p = bbcf_base_adress + time_count_slot_4_addr_offset;
            int frame_len_slot;
            memcpy(&frame_len_slot, frame_len_slot_p, 4);


            int facing_direction_slot_4_addr_offset = 0x9075E4;
            char* facing_direction_p = bbcf_base_adress + facing_direction_slot_4_addr_offset;
            int facing_direction;
            memcpy(&facing_direction, facing_direction_p, 4);


            //similar to slot_2 and slot_3 but with 0x4
            char* start_of_slot_inputs = bbcf_base_adress + time_count_slot_4_addr_offset + (0x960 * 3) + 0x4;
            std::vector<char> slot4_recording_frames{};
            for (int i = 0; i < frame_len_slot; i++) {
                slot4_recording_frames.push_back(*(start_of_slot_inputs + i * 2));
            }
            if (ImGui::Button("Save##slot4")) {
                save_to_file(slot4_recording_frames, facing_direction, fpath_s4);
            }
            ImGui::SameLine();
            if (ImGui::Button("Load##slot4")) {
                auto loaded_file = load_from_file(fpath_s4);
                if (!loaded_file.empty()) {
                    char facing_direction = loaded_file[0];
                    loaded_file.erase(loaded_file.begin());
                    memcpy(facing_direction_p, &(facing_direction), sizeof(char));
                }
                int frame_len_loaded_file = loaded_file.size();
                memcpy(frame_len_slot_p, &(frame_len_loaded_file), 4);
                int iter = 0;

                if (!loaded_file.empty()) {
                    for (auto input : loaded_file) {
                        memcpy(start_of_slot_inputs + (iter * 2), &input, 2);
                        iter++;
                    }
                }

            }
           
            ImGui::SameLine();
            if (ImGui::Button("Trim Playback##slot4")) {
                slot4_recording_frames = trim_playback(slot4_recording_frames);
                load_trimmed_playback(slot4_recording_frames, frame_len_slot_p, start_of_slot_inputs);
            }
            ImGui::InputText("File Path##slot4", fpath_s4, IM_ARRAYSIZE(fpath_s4));
            ImGui::TextWrapped("If the field isn't accepting keyboard input, try alt-tabbing out and back in, if that doesn't work copy and paste should still work(or restarting the game)");
            if (ImGui::Button("Set as gap action##slot4")) {
                slot_gap = 4;
            }
            ImGui::SameLine();
            if (ImGui::Button("Set as wakeup action##slot4")) {
                slot_wakeup = 4;
            }
         
            ImGui::SameLine();
            if (ImGui::Button("Reset##slot4")) {
                slot_gap = 0;
                slot_wakeup = 0;
            }
            ImGui::InputInt("Buffer frames ##slot4", &slot_buffer[3]);
            ImGui::Separator();
            auto old_val = 0; auto frame_counter = 0;
            for (auto el : slot4_recording_frames) {
                frame_counter++;
                if (old_val != el) {
                    std::string move_string = interpret_move(el);
                    ImGui::Text("frame %d: %s (0x%x)", frame_counter, move_string.c_str(), el);
                    old_val = el;
                }
            }
          
        }
        

        //setup for randomized slots
        static bool random_wakeup_slot_toggle = false;
        static bool random_gap_slot_toggle = false;
        ImGui::Columns(2);
        ImGui::Checkbox("Gap random slots##gap_random_slots", &random_gap_slot_toggle);
        if (random_gap_slot_toggle){
            if (ImGui::Checkbox("Slot1##gap_random_slots", &random_gap_slot1)) {
                treat_random_slot_checkbox(random_gap, random_gap_slot1, 1);
            }
        
            if(ImGui::Checkbox("Slot2##gap_random_slots", &random_gap_slot2)) {
                treat_random_slot_checkbox(random_gap, random_gap_slot2, 2);

            }
           
            if(ImGui::Checkbox("Slot3##gap_random_slots", &random_gap_slot3)) {
                treat_random_slot_checkbox(random_gap, random_gap_slot3, 3);
            }
           
            if (ImGui::Checkbox("Slot4##gap_random_slots", &random_gap_slot4)) {
                treat_random_slot_checkbox(random_gap, random_gap_slot4, 4);
            }
          
        
        }
        ImGui::NextColumn();
        ImGui::Checkbox("Wakeup random slots##wakeup_random_slots", &random_wakeup_slot_toggle);
        if (random_wakeup_slot_toggle) {
            if (ImGui::Checkbox("Slot1##wakeup_random_slots", &random_wakeup_slot1)) {
                treat_random_slot_checkbox(random_wakeup, random_wakeup_slot1, 1);
            }
           
            if(ImGui::Checkbox("Slot2##wakeup_random_slots", &random_wakeup_slot2)) {
                treat_random_slot_checkbox(random_wakeup, random_wakeup_slot2, 2);
            }
             
            if(ImGui::Checkbox("Slot3##wakeup_random_slots", &random_wakeup_slot3)) {
                treat_random_slot_checkbox(random_wakeup, random_wakeup_slot3, 3);
            }
            
            if(ImGui::Checkbox("Slot4##wakeup_random_slots", &random_wakeup_slot4)) {
                treat_random_slot_checkbox(random_wakeup, random_wakeup_slot4, 4);
            }
            
        }

        if (!g_interfaces.player2.IsCharDataNullPtr()) {

            //does gap action for recorded slot
            //can optimize later by checking for same memory address
            std::string current_action = g_interfaces.player2.GetData()->currentAction;
            char* playback_control_ptr = bbcf_base_adress + 0x1392d10 + 0x1ac2c; //set to 3 to start playback without direction adjustment, 0 for dummy, 1 for recording standby, 2 for bugged recording, 3 for playback, 4 for controller, 5 for cpu, 6 for continuous playback
            int val_set = 3;
            int slot = 0;







            //checking for gap action
            ///std::string substr = "GuardEnd";
            auto gap_action_trigger_find = current_action.find("GuardEnd");
            if (random_gap_slot_toggle && !random_gap.empty() && gap_action_trigger_find != std::string::npos) {
                //does randomized
                int random_pos = std::rand() % random_gap.size();
                slot = random_gap[random_pos] - 1;
                memcpy(active_slot, &slot, 4);
                memcpy(playback_control_ptr, &val_set, 2);

            }
            else if (slot_gap != 0 && gap_action_trigger_find != std::string::npos) {
                //does pre-defined
                slot = slot_gap - 1;
                memcpy(active_slot, &slot, 4);
                memcpy(playback_control_ptr, &val_set, 2);
            }


            //change to const later?
            //states that happen immediately before CmnActUkemiLandNLanding that I know of, not an exhaustive list
            //static std::vector<std::tuple<std::string, int>> wakeup_buffer_actions{ {"ActFDownDown", 9} ,  //lasts 9 frames
            //                {"ActVDownDown", 18}, //lasts 18 frames
            //                {"ActBDownDown", 20}, //lasts 20 frames
            //                {"ActWallBoundDown", 15}
            //}; //lasts 15 frames
            static std::vector<std::tuple<std::string, int>> wakeup_buffer_actions{ {"ActUkemiLandN",30 } };
            
            //checks if 

 
            static int base_frame_count_triggered = 0; // hold the pFrameCount a certain action started
            static int frame_count_to_activate = 0; // will start the recording when pFrameCount reaches this number
            static std::string prev_action;
            //will run buffered for non random wakeup slot
            if (slot_wakeup != 0 && slot_buffer[slot_wakeup - 1] != 0) {
                if (prev_action != current_action) {
                    //sets the internal frame count to trigger shit
                    for (auto pair : wakeup_buffer_actions) {
                        if (base_frame_count_triggered == 0 && current_action.find(std::get<0>(pair)) != std::string::npos) {
                            //auto tst = current_action.find(std::get<0>(pair));
                            base_frame_count_triggered = *g_gameVals.pFrameCount;
                            frame_count_to_activate = base_frame_count_triggered + std::get<1>(pair) - slot_buffer[slot_wakeup - 1]; //sets the frame count to activate to base + (len of action- buffer)
                            break;
                        }
                        // if not in listed states reset the internal frame counts
                        base_frame_count_triggered = 0;
                        frame_count_to_activate = 0;
                    }

                }
                else if (frame_count_to_activate !=0 && *g_gameVals.pFrameCount == frame_count_to_activate) {
                    slot = slot_wakeup - 1;
                    memcpy(active_slot, &slot, 4);
                    memcpy(playback_control_ptr, &val_set, 2);
                }
            }




            //checking for wakeup action
            //std::string substr = "CmnActUkemiLandNLanding";
            auto wakeup_action_trigger_find = current_action.find("CmnActUkemiLandNLanding");
            if (random_wakeup_slot_toggle && !random_wakeup.empty() && wakeup_action_trigger_find != std::string::npos) {
                //does randomized
                int random_pos = std::rand() % random_gap.size();
                slot = random_wakeup[random_pos] - 1;
                memcpy(active_slot, &slot, 4);
                memcpy(playback_control_ptr, &val_set, 2);

            }
            else if (slot_wakeup != 0 && wakeup_action_trigger_find != std::string::npos) {
                //does pre-defined
                slot = slot_wakeup - 1;
                memcpy(active_slot, &slot, 4);
                memcpy(playback_control_ptr, &val_set, 2);
            }


            prev_action = current_action;
            
        }
}
}
//void get_files_in_directory(char** items) {
//    std::string path = "./Save/Replay/locals";
//    //char items[500] = {};
//    items = {}; int i = 0;
//    for (const auto& entry : std::filesystem::directory_iterator(path)) {
//        if (i >= 500) {
//            break;
//        }
//        //items.push_back(&(entry.path().string())[0]);
//        items[i] = entry.path().string().data();
//    }
//}

bool compareFiles(const std::string& p1, const std::string& p2) {
    std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

    if (f1.fail() || f2.fail()) {
        return false; //file problem
    }

    if (f1.tellg() != f2.tellg()) {
        return false; //size mismatch
    }

    //seek back to beginning and use std::equal to compare contents
    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
        std::istreambuf_iterator<char>(),
        std::istreambuf_iterator<char>(f2.rdbuf()));
}
std::string find_latest_backup_file() {
    std::string backup_path = "./Save/Replay/backup";
    auto latest_file_time = std::filesystem::last_write_time("BBCF.exe");
    std::string fname = "BBCF.exe";
    for (const auto& entry : std::filesystem::directory_iterator(backup_path)) {
        auto time = entry.last_write_time();
        if (entry.is_regular_file() && time > latest_file_time) {
            latest_file_time = time;
            fname = std::filesystem::path(entry.path()).filename().string();

        }
    }
    if (fname == "BBCF.exe") { return ""; }
    return fname;
}
std::string find_and_backup_latest_replay() {
    std::string replay_path = "./Save/Replay";
    auto latest_file_time = std::filesystem::last_write_time("BBCF.exe");
    std::string fname = "BBCF.exe";
    for (const auto& entry : std::filesystem::directory_iterator(replay_path)) {
        auto time = entry.last_write_time();
        if (entry.is_regular_file() && time > latest_file_time) {
            latest_file_time = time;
            fname = std::filesystem::path(entry.path()).filename().string();

        }
    }

    auto src_fpath = "./Save/Replay/" + fname;
    std::filesystem::path sourceFile = "./Save/Replay/" + fname;
    std::filesystem::path targetParent = "./Save/Replay/locals/backup";
    std::filesystem::path target = "./Save/Replay/locals/backup/" + fname;

    //must add exceptions later
    std::filesystem::create_directories(targetParent); // Recursively create target directory if not existing.
    //checks if the files are different
    if (std::filesystem::exists(target)) {
        if (!compareFiles(sourceFile.string(), target.string())) {
            std::filesystem::path target2 = target.string()+ "_1";
            std::filesystem::copy_file(sourceFile, target2, std::filesystem::copy_options::overwrite_existing);
        }

    }
    else {
        std::filesystem::copy_file(sourceFile, target, std::filesystem::copy_options::overwrite_existing);
    }

    return fname;
}
void replace_with_local_replay(std::string fsourcename, std::string ftargetname) {
    std::string replay_path = "./Save/Replay";
    std::filesystem::path sourceFile = "./Save/Replay/locals/" + fsourcename;
    std::filesystem::path target = "./Save/Replay/" + ftargetname;

    //must add exceptions later

    if (std::filesystem::exists(sourceFile)) {
        std::filesystem::copy_file(sourceFile, target, std::filesystem::copy_options::overwrite_existing);

    }

    return;
}
void restore_original_replay(std::string latest_backup_file) {
    //finish this later
    //auto latest_backup_file = find_latest_backup_file();
    
    if (latest_backup_file == "") {
        return;
    }
    std::filesystem::path sourceFile = "./Save/Replay/locals/backup/" + latest_backup_file;
    std::filesystem::path target = "./Save/Replay/" + latest_backup_file;
    std::filesystem::copy_file(sourceFile, target, std::filesystem::copy_options::overwrite_existing);
    }

void ScrWindow::DrawReplayTheaterSection() {
    std::filesystem::path targetParent = "./Save/Replay/locals";
    std::filesystem::create_directories(targetParent);

    if (ImGui::CollapsingHeader("Local Replays")) {
        static std::vector<char*> items{ "" };
        static char local_replay_name[128] = "fname";
        static std::string latest_backup = "";
        ImGui::InputText("File Path##replay_theater", local_replay_name, IM_ARRAYSIZE(local_replay_name));
        if (ImGui::Button("Load##replay_theater")) {
            auto latest_replay_name = find_and_backup_latest_replay();
            latest_backup = latest_replay_name;
            replace_with_local_replay(local_replay_name, latest_replay_name);
        }
        if (ImGui::Button("Restore original replay##replay_theater")) {
            restore_original_replay(latest_backup);
        
        }
    }
}

struct gameStates {
    CharData chardata;
    unsigned int pFrameCount;
};

void ScrWindow::DrawVeryExperimentalSection() {
    if (!ImGui::CollapsingHeader("Replay Rewind::experimental"))
        return;
    if (*g_gameVals.pGameMode != GameMode_ReplayTheater || *g_gameVals.pMatchState != MatchState_Fight) {
        ImGui::Text("Only works during a running replay");
        return;
    }
    const int FRAME_STEP = 60;
    static std::vector<CharData> p1_prev_states;
    static std::vector<CharData> p2_prev_states;
    char* bbcf_base_adress = GetBbcfBaseAdress();
    char* ptr_replay_theater_current_frame = bbcf_base_adress + 0x11C0348;
    static bool rec = false;
    static bool playing = false;
    static int curr_frame = *g_gameVals.pFrameCount;
    static int prev_frame;
    static int frames_recorded = 0;
    curr_frame = *g_gameVals.pFrameCount;

    //*ptr_replay_theater_current_frame = *g_gameVals.pFrameCount;
    memcpy(ptr_replay_theater_current_frame, g_gameVals.pFrameCount, sizeof(unsigned int));
    if (ImGui::Button("start_recording::experimental")) {
        rec = true;
        p1_prev_states.push_back(*g_interfaces.player1.GetData());
        p2_prev_states.push_back(*g_interfaces.player2.GetData());
        frames_recorded += 1;
        prev_frame = *g_gameVals.pFrameCount;
        
    }
    if (ImGui::Button("stop recording::experimental")) {
        rec = false;
    }
    if (ImGui::Button("clear vectors::experimental")) {
        p1_prev_states = {};
        p2_prev_states = {};
        frames_recorded = 0;
    }
    ImGui::Text("Frame stack: +%d", frames_recorded * FRAME_STEP);
    if (ImGui::Button("rewind frame::experimental")) {
        if (!p1_prev_states.empty()) {
            CharData* pP1_char_data = g_interfaces.player1.GetData();
            memcpy(pP1_char_data, &(p1_prev_states.back()), sizeof(CharData));
            p1_prev_states.pop_back();

            CharData* pP2_char_data = g_interfaces.player2.GetData();
            memcpy(pP2_char_data, &(p2_prev_states.back()), sizeof(CharData));
            p2_prev_states.pop_back();


            *g_gameVals.pFrameCount = *g_gameVals.pFrameCount - FRAME_STEP;
            //*ptr_replay_theater_current_frame = *ptr_replay_theater_current_frame - 60;
            frames_recorded -= 1;
        }
    }
    if (ImGui::Button("rewind 10 frames::experimental")) {
        if (p1_prev_states.size() > 10) {
            for (int i = 0; i < 10; i++) {
                if (!p1_prev_states.empty()) {
                    CharData* pP1_char_data = g_interfaces.player1.GetData();
                    memcpy(pP1_char_data, &(p1_prev_states.back()), sizeof(CharData));
                    p1_prev_states.pop_back();

                    CharData* pP2_char_data = g_interfaces.player2.GetData();
                    memcpy(pP2_char_data, &(p2_prev_states.back()), sizeof(CharData));
                    p2_prev_states.pop_back();


                    *g_gameVals.pFrameCount = *g_gameVals.pFrameCount - 1;
                    //*ptr_replay_theater_current_frame = *ptr_replay_theater_current_frame - 1;
                    frames_recorded -= 1;
                }
            }
        }
    }




    if (rec && p1_prev_states.size() < 1200) {
        if (curr_frame == prev_frame + 60) {
            p1_prev_states.push_back(*g_interfaces.player1.GetData());
            p2_prev_states.push_back(*g_interfaces.player2.GetData());
            frames_recorded += 1;
            prev_frame = curr_frame;
        }
    }
    //prev_frame = curr_frame;
    return;
}