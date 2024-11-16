#include "valve_sdk/csgostructs.hpp"
#include <algorithm>
#include "minimal_hook.h"
#include "hooks.hpp"
#include "features/bhop.hpp"
#include <d3dx9.h>
#include "helpers/utils.hpp"
#include "features/visuals.hpp"
#include <filesystem>
#include <io.h>
#include "../CSGOSimple/features/render2.h"
#include "features/lagcomp.h"
#include "features/NewOne.h"
float simpleSpline(float value) noexcept
{
    float valueSquared = value * value;

    return (3 * valueSquared - 2 * valueSquared * value);
}

float simpleSplineRemapValClamped(float val, float A, float B, float C, float D) noexcept
{
    if (A == B)
        return val >= B ? D : C;
    float cVal = (val - A) / (B - A);
    cVal = std::clamp(cVal, 0.0f, 1.0f);
    return C + (D - C) * simpleSpline(cVal);
}

float lerp(float percent, float a, float b) noexcept
{
    return a + (b - a) * percent;
}

#define FIRSTPERSON_TO_THIRDPERSON_VERTICAL_TOLERANCE_MIN 4.0f
#define FIRSTPERSON_TO_THIRDPERSON_VERTICAL_TOLERANCE_MAX 10.0f

static void __fastcall modifyEyePositionHook(void* thisPointer, void* edx, unsigned int* pos) noexcept
{
    // static auto original = hooks::modifyEyePosition.getOriginal<void>(pos);

    auto animState = reinterpret_cast<CCSGOPlayerAnimState*>(thisPointer);

    auto entity = reinterpret_cast<C_BasePlayer*>(animState->pBaseEntity);
    //  if (!entity || !entity->is_alive() || !entity->is_player() || !localPlayer || entity != localPlayer)
     //     return original(thisPointer, pos);

    const int bone = memesclass->lookUpBone(entity, "head_0");
    if (bone == -1)
        return;

    Vector eyePosition = reinterpret_cast<Vector&>(pos);

    if (animState->m_fLandingDuckAdditiveSomething || animState->m_fDuckAmount != 0.f || !entity->ground_entity())
    {
        Vector bonePos;
        entity->getBonePos(bone, bonePos);
        bonePos.z += 1.7f;

        if (bonePos.z < eyePosition.z)
        {
            float lerpFraction = simpleSplineRemapValClamped(fabsf(eyePosition.z - bonePos.z),
                FIRSTPERSON_TO_THIRDPERSON_VERTICAL_TOLERANCE_MIN,
                FIRSTPERSON_TO_THIRDPERSON_VERTICAL_TOLERANCE_MAX,
                0.0f, 1.0f);

            eyePosition.z = lerp(lerpFraction, eyePosition.z, bonePos.z);
        }
    }
    pos = reinterpret_cast<unsigned int*>(&eyePosition);
}

reset::fn reset_original = nullptr;
long __stdcall reset::hook(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp)
{
    const auto hr = reset_original(device, pp);
    return hr;
}
draw_print_text::fn draw_print_text_original = nullptr;
void __fastcall draw_print_text::hook(void* ecx, void* edx, const wchar_t* Text, int TextLength, int TextType) {
    if (wcsstr(Text, L"fps:")) {
        wchar_t buf[128] = L"[lobotomy] ";

        wcscat_s(buf, Text);

        return draw_print_text_original(ecx, edx, buf, wcslen(buf), TextType);
    }

    return draw_print_text_original(ecx, edx, Text, TextLength, TextType);
}
MinHook sound;
bool predicred;
static int __stdcall emitSound(void* filter, int entityIndex, int channel, const char* soundEntry, unsigned int soundEntryHash, const char* sample, float volume, int seed, int soundLevel, int flags, int pitch, const Vector& origin, const Vector& direction, void* utlVecOrigins, bool updatePositions, float soundtime, int speakerentity, void* soundParams) noexcept
{
    if (predicred)
        return 0;


    volume = std::clamp(volume, 0.0f, 1.0f);
    return sound.callOriginal<int, 5>(filter, entityIndex, channel, soundEntry, soundEntryHash, sample, volume, seed, soundLevel, flags, pitch, std::cref(origin), std::cref(direction), utlVecOrigins, updatePositions, soundtime, speakerentity, soundParams);
}

MinHook sendDatagram;
static int __fastcall sendDatagramHook(INetChannel* network, void* edx, void* datagram)
{
    static auto original = sendDatagram.getOriginal<int>(datagram);
    if (!g_Options.extend_bt || datagram || !g_EngineClient->IsInGame())
        return original(network, datagram);
    if (g_LocalPlayer)
    {

        if (g_LocalPlayer->IsAlive())
        {
            static ConVar* sv_maxunlag = g_CVar->FindVar(("sv_maxunlag"));
            INetChannelInfo* pNetChannelInfo = g_EngineClient->GetNetChannelInfo();

            int instate = network->m_nInReliableState;
            int insequencenr = network->m_nInSequenceNr;

            float delta = std::max(0.f, g_Options.exploit ? 1.f : 0.2f);


            CLagCompensation::Get().AddLatencyToNetChannel(network, delta);

            int result = original(network, datagram);

            network->m_nInReliableState = instate;
            network->m_nInSequenceNr = insequencenr;
            return result;
        }
    }
    return original(network, datagram);
}
#define Weight_Thin 100
#define Weight_ExtraLight 200
#define Weight_Light 300
#define Weight_Normal 400
#define Weight_Medium 500
#define Weight_SemiBold 600
#define Weight_Bold 700
#define Weight_ExtraBold 800
#define Weight_Black 900
#define exxxtra_weight 1000
#define extra_weight 1200
#define extra2_weight 1600




std::string rgb_to_hex(int r, int g, int b, bool with_hash) {
    std::stringstream ss;
    if (with_hash)
        ss << "#";

    ss << std::hex << (r << 16 | g << 8 | b);
    return ss.str();
}
#include "helpers/fnv.h"
using fn = void(__fastcall*)(int, int, const char*, int, const char*);
inline fn ofunc;
void __fastcall push_notice(int ecx, int edx, const char* text, int str_len, const char* null) {
    auto print_text = [](int m_ecx, int m_edx, const std::string& str) {
        std::ostringstream oss;
        oss << "<font color=\"#bebebe\">[</font><font color=\""
            << rgb_to_hex(g_Options.fuckingaccent[0] * 255, g_Options.fuckingaccent[1] * 255, g_Options.fuckingaccent[2] * 255, true)  // Replace with your actual color calculation
            << "\">lobotomy</font><font color=\"#bebebe\">]</font><font color=\""
            << rgb_to_hex(190, 190, 190, true)
            << "\"> " << str << "</font>";

        std::string this_str = oss.str();
        return ofunc(m_ecx, m_edx, this_str.c_str(), this_str.length(), this_str.c_str());
        };


    switch (FNV1A::Hash(text)) {
    case FNV1A::HashConst("#_print_created"):
        return print_text(ecx, edx, ("created config"));
        break;
    case FNV1A::HashConst("#_print_saved"):
        return print_text(ecx, edx, ("saved config"));
        break;
    case FNV1A::HashConst("#_print_canceled_save"):
        return print_text(ecx, edx, ("canceled save config"));
        break;
    case FNV1A::HashConst("#_print_loaded"):
        return print_text(ecx, edx, ("loaded config"));
        break;
    case FNV1A::HashConst("#_print_removed"):
        return print_text(ecx, edx, ("removed config"));
        break;
    case FNV1A::HashConst("#_print_canceled_remove"):
        return print_text(ecx, edx, ("canceled remove config"));
        break;
    case FNV1A::HashConst("#_print_refreshed"):
        return print_text(ecx, edx, ("refreshed config list"));
        break;
    case FNV1A::HashConst("#_print_reloaded"):
        return print_text(ecx, edx, ("reloaded fonts"));
        break;
    case FNV1A::HashConst("#_print_reseted"):
        return print_text(ecx, edx, ("reseted fonts"));
        break;
    case FNV1A::HashConst("#_print_injected"):
        return print_text(ecx, edx, ("successfully injected"));
        break;
    case FNV1A::HashConst("#_print_updated_hud"):
        return print_text(ecx, edx, ("force updated"));
        break;
    case FNV1A::HashConst("#_print_created_checkpoint"):
        return print_text(ecx, edx, ("succesfully created & saved checkpoint"));
        break;
    case FNV1A::HashConst("#_print_teleport_checkpoint"):
        return print_text(ecx, edx, ("teleported to checkpoint"));
        break;
    case FNV1A::HashConst("#_print_jumpbug"):
        return print_text(ecx, edx, ("jumpbugged"));
        break;
    case FNV1A::HashConst("#_print_edgebug"):
        return print_text(ecx, edx, ("edgebugged"));
        break;
    case FNV1A::HashConst("#_print_pixelsurf"):
        return print_text(ecx, edx, ("pixelsurfed"));
        break;
    case FNV1A::HashConst("#_print_unlock_convar"):
        return print_text(ecx, edx, ("unlocked convars"));
        break;
    case FNV1A::HashConst("#_print_psaisit"):
        return print_text(ecx, edx, ("pixelsurf assisted"));
        break;
    }

    ofunc(ecx, edx, text, str_len, null);
}

void spectators()
{
    if (!g_Options.spectator_list)
        return;
    static int aboba{};

    static int iSpecsCount; // !!!!!!!
    static int iTextWidth = 0;
    static int r, g, b, a = 0;
    Color targetcol;
    int width, height;
    g_EngineClient->GetScreenSize(width, height);
    static Vector vecSpecSize{};
    Vector vecSpecPos = Vector(g_Options.specposx, g_Options.specposy, 0.f);
    vecSpecSize.y = 25.f + iSpecsCount * 15;
    if (iTextWidth + 10 > 95)
        vecSpecSize.x = iTextWidth + 10;
    else
        vecSpecSize.x = 95;

    draw_list1->text(vecSpecPos.x + 5, vecSpecPos.y + 5, draw_list1->fonts.tahoma, "spectators:", Color(255, 255, 255), false , 1);
    
    aboba = 0;
    iTextWidth = 0;
    if (iSpecsCount > 0)
        iSpecsCount = 0;

    if (!g_LocalPlayer)
        return;


    for (int i = 0; i < g_EngineClient->GetMaxClients(); i++)
    {


        std::string spectator;
        C_BasePlayer* eblanchik = C_BasePlayer::GetPlayerByIndex(i);
        if (!eblanchik)
            continue;
        if (eblanchik->IsAlive())
            continue;
        if (eblanchik->IsDormant())
            continue;

        auto local = g_LocalPlayer;
        player_info_t eblanchikinfo = eblanchik->GetPlayerInfo();
        C_BasePlayer* Nnchik = eblanchik->m_hObserverTarget();
        if (!Nnchik)
            continue;

        player_info_t nnchikinfo = Nnchik->GetPlayerInfo();
        if (strstr(eblanchikinfo.szName, "GOTV"))
            continue;

        spectator = std::string(eblanchikinfo.szName) + " -> " + nnchikinfo.szName;

        if (Nnchik == g_LocalPlayer || eblanchik == g_LocalPlayer)
        {

            r = g_Options.spec_color.r();
            g = g_Options.spec_color.g();
            b = g_Options.spec_color.b();
            a = 255;

        }
        else
        {

            r = 255;
            g = 255;
            b = 255;
            a = 255;
        }




        draw_list1->text(vecSpecPos.x + 5, vecSpecPos.y + 20 + iSpecsCount++ * 15, draw_list1->fonts.tahoma, spectator, Color(r, g, b, 255),false,1);

    }

}


using fn1 = void(__fastcall*)(void*, int, float*, float*);
fn1 override_mouse_input_original = nullptr;
float FlipValue(float input, float min_value, float max_value) {
    return (max_value + min_value) - input;
}
void __fastcall override_mouse_input(void* thisptr, int edx, float* x, float* y) {



    NewOne::edgebug_lock(*x, *y);



    if (HITGODA)
    {
        if (*x != 0.f)
        {
            *x *= g_Options.lockfactor;
        }
    }








    override_mouse_input_original(thisptr, edx, x, y);
}


namespace is_depth_of_field_enabled {
    using fn = void* (__thiscall*)(void*);
    inline fn ofunc;
    bool __stdcall is_depth_of_field_enabled();
}

namespace supports_resolve_depth {
    using fn = bool(*)();
    inline fn original;
    bool supports_resolve_depth();
}

bool __stdcall is_depth_of_field_enabled::is_depth_of_field_enabled() {
    Visuals::Get().motion_blur(nullptr);
    return false;
}

constexpr auto VENDORID_NVIDIA = 0x10DEu;
constexpr auto VENDORID_ATI = 0x1002u;
constexpr auto VENDORID_INTEL = 0x8086u;

bool supports_resolve_depth::supports_resolve_depth()
{
    MaterialAdapterInfo_t info;
    auto adapter = g_MatSystem->GetCurrentAdapter();
    g_MatSystem->GetDisplayAdapterInfo(adapter, info);

    switch (g_Options.motionblur.video_adapter)
    {
    case 0:
        if (info.m_VendorID == VENDORID_ATI)
            return false;
        break;
    case 1:
        if (info.m_VendorID == VENDORID_NVIDIA)
            return false;
        break;
    case 2:
        if (info.m_VendorID == VENDORID_INTEL)
            return false;
        break;
    }

    return original();

}






unsigned int get_virtual(void* _class, unsigned int index) { return static_cast<unsigned int>((*static_cast<int**>(_class))[index]); }
bool shithooks::initialize()
{
    if (MH_Initialize() != MH_OK)
    {
        throw std::runtime_error(("[+] failed to initialize hooking system :("));
        return false;
    }
     sendDatagram.detour(memesclass->sendDatagram, sendDatagramHook);
    sound.init(g_SosiHuiPidor);
    sound.hookAt(5, emitSound);
    IDirect3DStateBlock9* d3d9_state_block = NULL;

    const auto present_target = reinterpret_cast<void*>(get_virtual(g_D3DDevice9, 17));
    auto draw_print_text_target = reinterpret_cast<void*>(get_virtual(g_VGuiSurface, 28));
    const auto reset_target = reinterpret_cast<void*>(get_virtual(g_D3DDevice9, 16));
    auto override_mouse_input_target = reinterpret_cast<void*>(get_virtual(g_ClientMode, 23));
    if (MH_CreateHook(draw_print_text_target, &draw_print_text::hook, reinterpret_cast<void**>(&draw_print_text_original)) != MH_OK)
        throw std::runtime_error("failed to initialize create_move. (outdated index?)");
    if (MH_CreateHook(reset_target, &reset::hook, reinterpret_cast<void**>(&reset_original)) != MH_OK)
        throw std::runtime_error("failed to initialize reset. (outdated index?)");

    if (MH_CreateHook(override_mouse_input_target, &override_mouse_input, reinterpret_cast<void**>(&override_mouse_input_original)) != MH_OK)
        throw std::runtime_error(("failed to initialize override_mouse_input hook"));
    
    if (MH_CreateHook(Utils::PatternScan2(("client.dll"), ("55 8B EC 83 E4 F8 B8 ? ? ? ? E8 ? ? ? ? 53 8B D9 8B 0D")), &push_notice, (void**)&ofunc))
        printf(("push notice hook failed.\n"));
    if (MH_CreateHook(Utils::PatternScan2(("client.dll"), ("8B 0D ? ? ? ? 56 8B 01 FF 50 ? 8B F0 85 F6 75 ?")), &is_depth_of_field_enabled::is_depth_of_field_enabled, (void**)&is_depth_of_field_enabled::ofunc))
        printf(("is depth of field enabled hook failed.\n"));

    if (MH_CreateHook(Utils::PatternScan2("shaderapidx9.dll", ("A1 ? ? ? ? A8 ? 75 ? 83 C8 ? B9 ? ? ? ? 68 ? ? ? ? A3")), &supports_resolve_depth::supports_resolve_depth, (void**)&supports_resolve_depth::original) != MH_OK)
        printf(("isupports resolve depth hook failed.\n"));

    modifyEyePosition.detour(memesclass->modifyEyePosition, modifyEyePositionHook);
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
    {
        throw std::runtime_error(("[+] failed to initialize hooks :("));
        return false;
    }


    return true;
}

