#include "lagcomp.h"
#include "bt.h"

void CLagCompensation::Run(CUserCmd* pCmd)
{
    /*

we have much public info for that
now it is your own way gl*/
}

void CLagCompensation::UpdateIncomingSequences(INetChannel* pNetChannel)
{
    if (pNetChannel == nullptr)
        return;

    // set to real sequence to update, otherwise needs time to get it work again
    if (nLastIncomingSequence == 0)
        nLastIncomingSequence = pNetChannel->m_nInSequenceNr;

    // check how much sequences we can spike
    if (pNetChannel->m_nInSequenceNr > nLastIncomingSequence)
    {
        nLastIncomingSequence = pNetChannel->m_nInSequenceNr;
        vecSequences.emplace_front(SequenceObject_t(pNetChannel->m_nInReliableState, pNetChannel->m_nOutReliableState, pNetChannel->m_nInSequenceNr, g_GlobalVars->realtime));
    }

    // is cached too much sequences
    if (vecSequences.size() > 2048U)
        vecSequences.pop_back();
}

void CLagCompensation::ClearIncomingSequences()
{
    if (!vecSequences.empty())
    {
        nLastIncomingSequence = 0;
        vecSequences.clear();
    }
}

void CLagCompensation::AddLatencyToNetChannel(INetChannel* pNetChannel, float flLatency)
{
    for (const auto& sequence : vecSequences)
    {
        if (g_GlobalVars->realtime - sequence.flCurrentTime >= flLatency)
        {
            pNetChannel->m_nInReliableState = sequence.iInReliableState;
            pNetChannel->m_nInSequenceNr = sequence.iSequenceNr;
            break;
        }
    }
}