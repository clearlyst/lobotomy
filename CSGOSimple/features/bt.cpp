#include "bt.h"

inline void sinCos(float radians, PFLOAT sine, PFLOAT cosine)
{
    __asm
    {
        fld dword ptr[radians]
        fsincos
        mov edx, dword ptr[cosine]
        mov eax, dword ptr[sine]
        fstp dword ptr[edx]
        fstp dword ptr[eax]
    }
}
float AngleNormalize(float angle)
{
    while (angle < -180)    angle += 360;
    while (angle > 180)    angle -= 360;

    return angle;
}
void CalcAngle(Vector& vSource, Vector& vDestination, Vector& qAngle)
{
    Vector vDelta = vSource - vDestination;

    float fHyp = (vDelta.x * vDelta.x) + (vDelta.y * vDelta.y);

    float fRoot;

    __asm
    {
        sqrtss xmm0, fHyp
        movss fRoot, xmm0
    }

    qAngle.x = RAD2DEG(atan(vDelta.z / fRoot));
    qAngle.y = RAD2DEG(atan(vDelta.y / vDelta.x));

    if (vDelta.x >= 0.0f)
        qAngle.y += 180.0f;

    qAngle.x = AngleNormalize(qAngle.x);
    qAngle.y = AngleNormalize(qAngle.y);
}


inline void angleVectors(QAngle angles, Vector& f)
{
    float sp, sy, sr, cp, cy, cr;

    sinCos(DEG2RAD(angles[0]), &sp, &cp);
    sinCos(DEG2RAD(angles[1]), &sy, &cy);
    sinCos(DEG2RAD(angles[2]), &sr, &cr);

    f[0] = cp * cy;
    f[1] = cp * sy;
    f[2] = -sp;
}
#define CHECK_VALID( _v ) 0
inline void VectorSubtract(const Vector& a, const Vector& b, Vector& c)
{
    CHECK_VALID(a);
    CHECK_VALID(b);
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
}

inline float GetFov(Vector vLocalOrigin, Vector vPosition, Vector vForward)
{
    Vector vLocal;

    VectorSubtract(vPosition, vLocalOrigin, vLocal);

    vLocal.NormalizeInPlace();

    float fValue = vForward.Dot(vLocal);

    //np for kolo's math skills
    if (fValue < -1.0f)
        fValue = -1.0f;

    if (fValue > 1.0f)
        fValue = 1.0f;

    return RAD2DEG(acos(fValue));
}

std::deque<Backtrack::Record> Backtrack::records[65];
std::deque<Backtrack::IncomingSequence>Backtrack::sequences;
Backtrack::Cvars Backtrack::cvars;
int Backtrack::timeToTicks(float time) noexcept
{
    return static_cast<int>(0.5f + time / g_GlobalVars->interval_per_tick);
}
void Backtrack::update(int framestage) noexcept
{
    if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
        for (auto& record : records)
            record.clear();

        return;
    }

    if (framestage == FRAME_RENDER_START) {
        for (int i = 1; i <= g_EngineClient->GetMaxClients(); i++) {
            C_BasePlayer* entity = C_BasePlayer::GetPlayerByIndex(i);
            if (!entity || entity == g_LocalPlayer || entity->IsDormant() || !entity->IsAlive()) {
                records[i].clear();
                continue;
            }

            if (!records[i].empty() && (records[i].front().simulationTime == entity->m_flSimulationTime()))
                continue;

            Record record{ };
            if (const model_t* mod = entity->GetModel(); mod)
                record.hdr = g_MdlInfo->GetStudiomodel(mod);
            record.head = entity->GetHitboxPos(HITBOX_HEAD);
            record.Chest = entity->GetHitboxPos(HITBOX_CHEST);
            record.Pelvis = entity->GetHitboxPos(HITBOX_PELVIS);
            record.Neck = entity->GetHitboxPos(HITBOX_NECK);
            record.LChest = entity->GetHitboxPos(HITBOX_LOWER_CHEST);
            record.origin = entity->m_vecOrigin();
            record.simulationTime = entity->m_flSimulationTime();
            record.mins = entity->GetCollideable()->OBBMins();
            record.max = entity->GetCollideable()->OBBMaxs();
            entity->SetupBones(record.matrix, 256, 0x7FF00, g_GlobalVars->curtime);
            records[i].push_front(record);


            while (records[i].size() > 3 && records[i].size() > static_cast<size_t>(timeToTicks(static_cast<float>(200) / 1000.f + getExtraTicks())))
                records[i].pop_back();

            if (auto invalid = std::find_if(std::cbegin(records[i]), std::cend(records[i]), [](const Record& rec) { return !valid(rec.simulationTime); }); invalid != std::cend(records[i]))
                records[i].erase(invalid, std::cend(records[i]));
        }
    }
}

void Backtrack::run(CUserCmd* cmd) noexcept
{

    static Vector vAngle;

    if (!(cmd->buttons & IN_ATTACK))
        return;

    if (!g_LocalPlayer)
        return;


    float best_fov = 255.f;
    C_BaseEntity* best_target{ };
    int best_target_index, best_record{ };
    QAngle aim_punch = g_LocalPlayer->m_aimPunchAngle();

    for (int i = 1; i <= g_GlobalVars->maxClients; i++) {
        auto entity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));
        if (!entity || entity == g_LocalPlayer || entity->IsDormant() || !entity->IsAlive()) continue;

        auto fov = GetFov(g_LocalPlayer->GetEyePos(), entity->abs_origin(),vAngle);
        if (fov < best_fov) {
            best_fov = fov;
            best_target = entity;
            best_target_index = i;
        }
    }

    if (best_target) {
        if (records[best_target_index].size() <= 3) return;

        best_fov = 255.f;
        for (size_t i = 0; i < records[best_target_index].size(); i++) {
            auto& record = records[best_target_index][i];
            if (!valid(record.simulationTime)) continue;

            auto fov = GetFov(g_LocalPlayer->GetEyePos(), record.origin, vAngle);
            if (fov < best_fov) {
                best_fov = fov;
                best_record = i;
            }
        }
    }
    if (best_record) {
        auto record = records[best_target_index][best_record];
        cmd->tick_count = timeToTicks(record.simulationTime + getLerp());
    }
}