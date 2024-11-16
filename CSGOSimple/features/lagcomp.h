#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include <deque>
#include "../options.hpp"
#include <map>
#define LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR ( 64.0f * 64.0f )
#define LAG_COMPENSATION_EPS_SQR ( 0.1f * 0.1f )
#define LAG_COMPENSATION_ERROR_EPS_SQR ( 4.0f * 4.0f )
enum ELagCompensationState
{
    LC_NO = 0,
    LC_ALIVE = (1 << 0),
    LC_ORIGIN_CHANGED = (1 << 8),
    LC_ANGLES_CHANGED = (1 << 9),
    LC_SIZE_CHANGED = (1 << 10),
    LC_ANIMATION_CHANGED = (1 << 11)
};
struct SequenceObject_t
{
    SequenceObject_t(int iInReliableState, int iOutReliableState, int iSequenceNr, float flCurrentTime)
        : iInReliableState(iInReliableState), iOutReliableState(iOutReliableState), iSequenceNr(iSequenceNr), flCurrentTime(flCurrentTime) { }

    int iInReliableState;
    int iOutReliableState;
    int iSequenceNr;
    float flCurrentTime;
};
class CLagCompensation : public Singleton<CLagCompensation>
{
public:
    // Get
    void Run(CUserCmd* pCmd);

    // Main
    void UpdateIncomingSequences(INetChannel* pNetChannel);
    void ClearIncomingSequences();
    void AddLatencyToNetChannel(INetChannel* pNetChannel, float flLatency);
    std::deque<SequenceObject_t> vecSequences = { };
    int nLastIncomingSequence = 0;
private:
    // Values

    /* our real incoming sequences count /
    int nRealIncomingSequence = 0;
    / count of incoming sequences what we can spike */

};