#include "Core/interfaces.h"
#include "Core/Settings.h"
#include "Core/utils.h"
#include "Game/gamestates.h"
#include "Psapi.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <array>
#include "FrameState.h"

FrameState::FrameState() {
    if (!g_interfaces.player1.IsCharDataNullPtr() && !g_interfaces.player2.IsCharDataNullPtr()) {
        p1 = *g_interfaces.player1.GetData();
        p2 = *g_interfaces.player2.GetData();
        frameCount = *g_gameVals.pFrameCount;
        matchTimer = *g_gameVals.pMatchTimer;

        viewMatrix = *g_gameVals.viewMatrix;
        auto camArgs = FrameState::get_camera_vals();
        camPos = camArgs[0];
        camTarget = camArgs[1];
        camUpVector = camArgs[2];

        cam_mystery_vals0 = FrameState::get_camera_mystery_vals0();
        cam_mystery_vals1 = FrameState::get_camera_mystery_vals1();
        cam_mystery_vals2 = FrameState::get_camera_mystery_vals2();
    }
}

std::array<std::array<uint8_t, 12>, 3> FrameState::get_camera_vals() {
    auto bbcf_base_adress = GetBbcfBaseAdress();
    char*** ptr_D3CAM_args = (char***)(bbcf_base_adress + 0x6128A8);
    char* ptr_D3CAM_pos = **ptr_D3CAM_args + 0x4;
    char* ptr_D3CAM_target = **ptr_D3CAM_args + 0x1C;
    char* ptr_D3CAM_upVector = **ptr_D3CAM_args + 0x10;
    std::array<uint8_t, 12> D3CAM_pos{};
    std::array<uint8_t, 12>  D3CAM_target{};
    std::array<uint8_t, 12>  D3CAM_upVector{};
    for (int i = 0; i < 12; i++) {
        D3CAM_pos[i] = *(ptr_D3CAM_pos + i);
        D3CAM_target[i] = *(ptr_D3CAM_target + i);
        D3CAM_upVector[i] = *(ptr_D3CAM_upVector + i);
    }
    std::array<std::array<uint8_t, 12>, 3> ret{ D3CAM_pos ,D3CAM_target ,D3CAM_upVector };
    return ret;
}


std::array<uint8_t, 0xC> FrameState::get_camera_mystery_vals0() {
    auto bbcf_base_adress = GetBbcfBaseAdress();
    char* mystery_val_base = bbcf_base_adress + 0xE3AA38;
    std::array<uint8_t, 0xC> camera_mystery_vals0{};
    for (int i = 0; i < 0xC; i++) {
        camera_mystery_vals0[i] = *(mystery_val_base + i);
    }
    return camera_mystery_vals0;
}
std::array<uint8_t, 0x18> FrameState::get_camera_mystery_vals1() {
    auto bbcf_base_adress = GetBbcfBaseAdress();
    char* mystery_val_base = bbcf_base_adress + 0xE3AA84;
    std::array<uint8_t, 0x18> camera_mystery_vals1{};
    for (int i = 0; i < 0x18; i++) {
        camera_mystery_vals1[i] = *(mystery_val_base + i);
    }
    return camera_mystery_vals1;
}
std::array<uint8_t, 0x34> FrameState::get_camera_mystery_vals2() {
    auto bbcf_base_adress = GetBbcfBaseAdress();
    char* mystery_val_base = bbcf_base_adress + 0xE3AAC0;
    std::array<uint8_t, 0x34> camera_mystery_vals2{};
    for (int i = 0; i < 0x34; i++) {
        camera_mystery_vals2[i] = *(mystery_val_base + i);
    }
    return camera_mystery_vals2;
}


void FrameState::load_frame_state() {
    if (!g_interfaces.player1.IsCharDataNullPtr() && !g_interfaces.player2.IsCharDataNullPtr()){
        auto bbcf_base_adress = GetBbcfBaseAdress();
        char* ptr_replay_theater_current_frame = bbcf_base_adress + 0x11C0348;
        char*** ptr_D3CAM_args = (char***)(bbcf_base_adress + 0x6128A8);
        char* ptr_D3CAM_pos = **ptr_D3CAM_args + 0x4;
        char* ptr_D3CAM_target = **ptr_D3CAM_args + 0x1C;
        char* ptr_D3CAM_upVector = **ptr_D3CAM_args + 0x10;
        char* ptr_camera_mystery_val0 = bbcf_base_adress + 0xE3AA38;
        char* ptr_camera_mystery_val1 = bbcf_base_adress + 0xE3AA84;
        char* ptr_camera_mystery_val2 = bbcf_base_adress + 0xE3AAC0;


        CharData* pP1_char_data = g_interfaces.player1.GetData();
        memcpy(pP1_char_data, &(p1), sizeof(CharData));
        CharData* pP2_char_data = g_interfaces.player2.GetData();
        memcpy(pP2_char_data, &(p2), sizeof(CharData));


        memcpy(ptr_camera_mystery_val0, &(cam_mystery_vals0[0]), 0xC);
        memcpy(ptr_camera_mystery_val1, &(cam_mystery_vals1[0]), 0x18);
        memcpy(ptr_camera_mystery_val2, &(cam_mystery_vals2[0]), 0x34);
        memcpy(ptr_D3CAM_pos, &(camPos[0]), 12);
        memcpy(ptr_D3CAM_target, &(camTarget[0]), 12);
        memcpy(ptr_D3CAM_upVector, &(camUpVector[0]), 12);
        *g_gameVals.viewMatrix = viewMatrix;
    }
}