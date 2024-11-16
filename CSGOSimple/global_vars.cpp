#include "../CSGOSimple/valve_sdk/interfaces/IClientEntity.hpp"
#include "../CSGOSimple/valve_sdk/misc/GlobalVars.hpp"
#include "../CSGOSimple/valve_sdk/misc/CUserCmd.hpp"
#include "../CSGOSimple/valve_sdk/csgostructs.hpp"

float CGlobalVarsBase::serverTime(CUserCmd* cmd) const noexcept
{
    static int tick;
    static CUserCmd* lastCmd;

    if (cmd) {
        if (g_LocalPlayer && (!lastCmd || lastCmd->hasbeenpredicted))
            tick = g_LocalPlayer->m_nTickBase();
        else
            tick++;
        lastCmd = cmd;
    }
    return tick * interval_per_tick;
}
