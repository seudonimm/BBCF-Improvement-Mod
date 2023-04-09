#pragma once
#include "Core/interfaces.h"
#include "Core/Settings.h"
#include "Core/utils.h"
#include "Game/gamestates.h"
#include <ctime>
#include <cstdlib>
#include <array>
struct ChildEntity {
    uint8_t offset_ownerEntity;
    uint8_t offset_pEntityList;
    CharData entity;
};
class FrameState {
public:
    CharData p1;
    CharData p2;
    unsigned int frameCount;
    unsigned int matchTimer;


    std::array<uint8_t, 12> camPos;
    std::array<uint8_t, 12> camTarget;
    std::array<uint8_t, 12> camUpVector;
    D3DXMATRIX viewMatrix;
    std::array<uint8_t, 0xC> cam_mystery_vals0; //bbcf.exe+e3aa38 to bbcf.exe + e3aa44
    std::array<uint8_t, 0x18> cam_mystery_vals1; //bbcf.exe+e3aa84 to bbcf.exe + e3aa9C
    std::array<uint8_t, 0x34> cam_mystery_vals2;//bbcf.exe + e3aac0 to bbcf.exe + e3aaf4 



    FrameState();


    void load_frame_state();

    static std::array<std::array<uint8_t, 12>, 3> get_camera_vals();

    static std::array<uint8_t, 0xC> get_camera_mystery_vals0();

    static std::array<uint8_t, 0x18> get_camera_mystery_vals1();

    static std::array<uint8_t, 0x34> get_camera_mystery_vals2();
};