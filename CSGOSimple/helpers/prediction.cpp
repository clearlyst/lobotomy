#include "prediction.hpp"
#include "../valve_sdk/misc/GlobalVars.hpp"
#include "../minimal_hook.h"
static int localPlayerFlags;
PredictionSystem::PredictionSystem()
{
    m_pPredictionRandomSeed = *reinterpret_cast<int**>(Utils::PatternScan(GetModuleHandleA("client.dll"), "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 0x2);

    m_flOldCurtime = NULL;
    m_flOldFrametime = NULL;
}


void PredictionSystem::StartPrediction(CUserCmd* cmd)
{
    extern bool predicred;

    if (!g_LocalPlayer)
        return;
    predicred = true;
    localPlayerFlags = g_LocalPlayer->m_fFlags();
    *memesclass->predictionRandomSeed = 0;

    const auto oldCurrenttime = g_GlobalVars->curtime;
    const auto oldFrametime = g_GlobalVars->frametime;

    g_GlobalVars->curtime = g_GlobalVars->serverTime();
    g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

    g_MoveHelper->SetHost(g_LocalPlayer);
    g_Prediction->SetupMove(g_LocalPlayer, cmd, g_MoveHelper, &m_MoveData);
    g_GameMovement->ProcessMovement(g_LocalPlayer, &m_MoveData);
    g_Prediction->FinishMove(g_LocalPlayer, cmd, &m_MoveData);
    g_MoveHelper->SetHost(nullptr);

    *memesclass->predictionRandomSeed = -1;

    g_GlobalVars->curtime = oldCurrenttime;
    g_GlobalVars->frametime = oldFrametime;
    predicred = false;

}


void PredictionSystem::EndPrediction()
{

}
