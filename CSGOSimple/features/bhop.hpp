#pragma once
#include "../valve_sdk/math/Vector.hpp"
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
class C_BasePlayer;
class CUserCmd;
class QAngle;
class IGameEvent;
// detects
inline int iFlagsBackup;
inline bool should_edgebug;
inline bool should_jumpbug;
inline bool edgebugg;
inline bool should_longjump;
inline bool should_minijump;
inline bool should_wallclimb;
inline bool should_delayhop;
inline bool should_pixelsurf;
inline bool should_ag;
inline bool InCross;
inline bool bShouldEdgebug;
static int edgebugs = 0;
inline bool HITGODA{ false };
inline bool AlertJB = false;
inline int killCounter = 1;

namespace BunnyHop
{

    void PastedMovementCorrection(CUserCmd* cmd, QAngle old_angles, QAngle wish_angle);
    void OnCreateMove(CUserCmd* cmd);
    void egbag(CUserCmd* pCmd, QAngle& angView);
    void PrePred(CUserCmd* cmd);
    void autoalign(CUserCmd* cmd);
    void pxcalc(CUserCmd* cmd);
   
    void sw(CUserCmd* cmd);
    void edgebug_notify(float unpred_z, int unpred_flags);
   // void edgebug_notify(CUserCmd* cmd, float unpredicted_velocity);
    float lerpa(float a, float b, float t);
    void pixel_surf(CUserCmd* cmd);
    void ps_fix(CUserCmd* cmd);
   
    void pxcalc(CUserCmd* cmd);

    void pxcalcrender();
    void PixelFinder(CUserCmd* cmd);
    void KillSound(IGameEvent* pEvent);
    void delayhop(CUserCmd* cmd);
    void auto_duck(CUserCmd* cmd);
    void ljend(CUserCmd* cmd);
    void yaw_changer();
    void Ladderjump(CUserCmd* cmd);
    void clantag();
    void Minijump(CUserCmd* cmd);
    void AutoStafe(CUserCmd* cmd);
    void hitbox_invo(IGameEvent* hibox);
    void checkPoint(CUserCmd* cmd);
    void CHECKSURFLOLKEK(CUserCmd* cmd);

    void FireMan(CUserCmd* cmd);

    void renderCheckSurf();
    void AutoBounce(CUserCmd* cmd);
    void jump_bug(CUserCmd* cmd, int pre_flags, int pre_velocity);
}
struct PointsCheck1 {
    Vector Pos;
    std::string Map;

    PointsCheck1(const Vector& pos = {}, const std::string& map = "")
        : Pos(pos), Map(map) {}
};

inline std::vector<PointsCheck1> PointsCheck{};

inline bool WriteStringToFile(HANDLE hFile, const std::string& str) {
    DWORD written;
    DWORD length = str.size();
    if (!WriteFile(hFile, &length, sizeof(length), &written, nullptr)) {
        return false;
    }
    if (!WriteFile(hFile, str.c_str(), length, &written, nullptr)) {
        return false;
    }
    return true;
}

inline bool ReadStringFromFile(HANDLE hFile, std::string& str) {
    DWORD read;
    DWORD length;
    if (!ReadFile(hFile, &length, sizeof(length), &read, nullptr)) {
        return false;
    }
    str.resize(length);
    if (!ReadFile(hFile, &str[0], length, &read, nullptr)) {
        return false;
    }
    return true;
}

inline void saveConfig(const std::string& filename) {
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open file for writing" << std::endl;
        return;
    }

    DWORD written;
    DWORD size = PointsCheck.size();
    WriteFile(hFile, &size, sizeof(size), &written, nullptr);

    for (const auto& point : PointsCheck) {
        WriteFile(hFile, &point.Pos, sizeof(point.Pos), &written, nullptr);
        WriteStringToFile(hFile, point.Map);
    }

    CloseHandle(hFile);
}

inline void loadConfig(const std::string& filename) {
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open file for reading" << std::endl;
        return;
    }

    DWORD read;
    DWORD size;
    ReadFile(hFile, &size, sizeof(size), &read, nullptr);
    PointsCheck.resize(size);

    for (auto& point : PointsCheck) {
        ReadFile(hFile, &point.Pos, sizeof(point.Pos), &read, nullptr);
        ReadStringFromFile(hFile, point.Map);
    }

    CloseHandle(hFile);
}