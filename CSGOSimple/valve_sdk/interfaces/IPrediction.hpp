#pragma once

#include "../Math/QAngle.hpp"
#include "../Misc/CUserCmd.hpp"
#include "IMoveHelper.hpp"

class CMoveData
{
public:
    bool    m_bFirstRunOfFunctions : 1;
    bool    m_bGameCodeMovedPlayer : 1;
    int     m_nPlayerHandle;        // edict index on server, client entity handle on client=
    int     m_nImpulseCommand;      // Impulse command issued.
    Vector  m_vecViewAngles;        // Command view angles (local space)
    Vector  m_vecAbsViewAngles;     // Command view angles (world space)
    int     m_nButtons;             // Attack buttons.
    int     m_nOldButtons;          // From host_client->oldbuttons;
    float   m_forwardmove;
    float   m_sidemove;
    float   m_flUpMove;
    float   m_flMaxSpeed;
    float   m_flClientMaxSpeed;
    Vector  m_vecVelocity;          // edict::velocity        // Current movement direction.
    Vector  m_vecAngles;            // edict::angles
    Vector  m_vecOldAngles;
    float   m_outStepHeight;        // how much you climbed this move
    Vector  m_outWishVel;           // This is where you tried 
    Vector  m_outJumpVel;           // This is your jump velocity
    Vector  m_vecConstraintCenter;
    float   m_flConstraintRadius;
    float   m_flConstraintWidth;
    float   m_flConstraintSpeedFactor;
    float   m_flUnknown[5];
    Vector  m_vecAbsOrigin;
};

class C_BasePlayer;

class IGameMovement
{
public:
    virtual			~IGameMovement(void) {}

    virtual void	          ProcessMovement(C_BasePlayer* pPlayer, CMoveData* pMove) = 0;
    virtual void	          Reset(void) = 0;
    virtual void	          StartTrackPredictionErrors(C_BasePlayer* pPlayer) = 0;
    virtual void	          FinishTrackPredictionErrors(C_BasePlayer* pPlayer) = 0;
    virtual void	          DiffPrint(char const* fmt, ...) = 0;
    virtual Vector const& GetPlayerMins(bool ducked) const = 0;
    virtual Vector const& GetPlayerMaxs(bool ducked) const = 0;
    virtual Vector const& GetPlayerViewOffset(bool ducked) const = 0;
    virtual bool		        IsMovingPlayerStuck(void) const = 0;
    virtual C_BasePlayer* GetMovingPlayer(void) const = 0;
    virtual void		        UnblockPusher(C_BasePlayer* pPlayer, C_BasePlayer* pPusher) = 0;
    virtual void            SetupMovementBounds(CMoveData* pMove) = 0;
};

class CGameMovement
    : public IGameMovement
{
public:
    virtual ~CGameMovement(void) {}
};

// 0x0018
class IPrediction
{
public:
    std::byte		pad0[0x4];						// 0x0000
    std::uintptr_t	hLastGround;					// 0x0004
    bool			bInPrediction;					// 0x0008
    bool			bIsFirstTimePredicted;			// 0x0009
    bool			bEnginePaused;					// 0x000A
    bool			bOldCLPredictValue;				// 0x000B
    int				iPreviousStartFrame;			// 0x000C
    int				nIncomingPacketNumber;			// 0x0010
    float			flLastServerWorldTimeStamp;		// 0x0014

    struct Split_t
    {
        bool		bIsFirstTimePredicted;			// 0x0018
        std::byte	pad0[0x3];						// 0x0019
        int			nCommandsPredicted;				// 0x001C
        int			nServerCommandsAcknowledged;	// 0x0020
        int			iPreviousAckHadErrors;			// 0x0024
        float		flIdealPitch;					// 0x0028
        int			iLastCommandAcknowledged;		// 0x002C
        bool		bPreviousAckErrorTriggersFullLatchReset; // 0x0030
        bool		bPerformedTickShift;			// 0x0045
    };
    Split_t			Split[1];						// 0x0018
    bool InPrediction()
    {
        typedef bool(__thiscall* oInPrediction)(void*);
        return CallVFunction<oInPrediction>(this, 14)(this);
    }

    void RunCommand(C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper)
    {
        typedef void(__thiscall* oRunCommand)(void*, C_BasePlayer*, CUserCmd*, IMoveHelper*);
        return CallVFunction<oRunCommand>(this, 19)(this, player, ucmd, moveHelper);
    }

    void SetupMove(C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper, void* pMoveData)
    {
        typedef void(__thiscall* oSetupMove)(void*, C_BasePlayer*, CUserCmd*, IMoveHelper*, void*);
        return CallVFunction<oSetupMove>(this, 20)(this, player, ucmd, moveHelper, pMoveData);
    }

    void FinishMove(C_BasePlayer* player, CUserCmd* ucmd, void* pMoveData)
    {
        typedef void(__thiscall* oFinishMove)(void*, C_BasePlayer*, CUserCmd*, void*);
        return CallVFunction<oFinishMove>(this, 21)(this, player, ucmd, pMoveData);
    }
};