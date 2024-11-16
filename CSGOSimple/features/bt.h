#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include <deque>
#include "../options.hpp"
#include <map>
#include <algorithm>


namespace Backtrack {
    void update(int framestage) noexcept;
    void run(CUserCmd* cmd) noexcept;


    struct Record {
        Vector head;
        Vector Pelvis;
        Vector   LChest;
        Vector    Chest;
        Vector  Neck;
        Vector origin;
        Vector max;
        Vector mins;
        float simulationTime;
        matrix3x4_t matrix[256];
        studiohdr_t* hdr;
    };

    extern std::deque<Record> records[65];

    struct Cvars {
        ConVar* updateRate;
        ConVar* maxUpdateRate;
        ConVar* interp;
        ConVar* interpRatio;
        ConVar* minInterpRatio;
        ConVar* maxInterpRatio;
        ConVar* maxUnlag;
    };

    extern Cvars cvars;

    struct IncomingSequence
    {
        int inreliablestate;
        int sequencenr;
        float servertime;
    };

    extern std::deque<IncomingSequence>sequences;

    inline float getExtraTicks() noexcept
    {
        auto network = g_EngineClient->GetNetChannelInfo();
        if (!network)
            return 0.f;

        return std::clamp(network->GetLatency(1) - network->GetLatency(0), 0.f, 0.2f);
    }

    constexpr auto getLerp() noexcept
    {
        auto ratio = std::clamp(cvars.interpRatio->GetFloat(), cvars.minInterpRatio->GetFloat(), cvars.maxInterpRatio->GetFloat());
#undef max
        return std::max(cvars.interp->GetFloat(), (ratio / ((cvars.maxUpdateRate) ? cvars.maxUpdateRate->GetFloat() : cvars.updateRate->GetFloat())));
    }

    inline   bool valid(float simtime) noexcept
    {
        auto network = g_EngineClient->GetNetChannelInfo();
        if (!network)
            return false;

        auto deadTime = static_cast<int>(g_GlobalVars->serverTime() - cvars.maxUnlag->GetFloat());
        if (simtime < deadTime)
            return false;

        auto delta = std::clamp(network->GetLatency(0) + network->GetLatency(1) + getLerp(), 0.f, cvars.maxUnlag->GetFloat()) - (g_GlobalVars->serverTime() - simtime);
        return std::fabsf(delta) <= 0.2f;
    }

    int timeToTicks(float time) noexcept;

    static void init() noexcept
    {
        records->clear();

        cvars.updateRate = g_CVar->FindVar("cl_updaterate");
        cvars.maxUpdateRate = g_CVar->FindVar("sv_maxupdaterate");
        cvars.interp = g_CVar->FindVar("cl_interp");
        cvars.interpRatio = g_CVar->FindVar("cl_interp_ratio");
        cvars.minInterpRatio = g_CVar->FindVar("sv_client_min_interp_ratio");
        cvars.maxInterpRatio = g_CVar->FindVar("sv_client_max_interp_ratio");
        cvars.maxUnlag = g_CVar->FindVar("sv_maxunlag");
    }
}