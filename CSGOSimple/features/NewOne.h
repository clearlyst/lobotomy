#pragma once
#include "../valve_sdk/math/Vector.hpp"
#include "../valve_sdk/math/QAngle.hpp"
class C_BasePlayer;
class CUserCmd;
class QAngle;
class IGameEvent;
namespace NewOne {
   inline Vector BackupVelocity;
   inline Vector BackupOrigin;
   inline int BackupMoveType;
   inline int   BackupFlags;
   inline QAngle   BackupQAngle;
   inline bool EdgeBug_Founded;
   inline int EdgeBug_Ticks;
    void PrePredictionEdgeBug(CUserCmd* cmd);

    bool EdgeBugCheck(CUserCmd* cmd);

    void ReStorePrediction();

    void EdgeBugPostPredict(CUserCmd* cmd);

    void edgebug_detect(CUserCmd* cmd);

    void edgebug_lock(float& x, float& y);

}