#pragma once

#include <cstddef>


#define LINUX_ARGS(...)
#define RETURN_ADDRESS() std::uintptr_t(_ReturnAddress())
#define FRAME_ADDRESS() (std::uintptr_t(_AddressOfReturnAddress()) - sizeof(std::uintptr_t))
#define IS_WIN32() true
#define WIN32_LINUX(win32, linux) win32

constexpr auto CLIENT_DLL = "client";
constexpr auto SERVER_DLL = "server";
constexpr auto ENGINE_DLL = "engine";
constexpr auto FILESYSTEM_DLL = "filesystem_stdio";
constexpr auto INPUTSYSTEM_DLL = "inputsystem";
constexpr auto LOCALIZE_DLL = "localize";
constexpr auto MATERIALSYSTEM_DLL = "materialsystem";
constexpr auto PANORAMA_DLL = "panorama";
constexpr auto SOUNDEMITTERSYSTEM_DLL = "soundemittersystem";
constexpr auto STEAMNETWORKINGSOCKETS_DLL = "steamnetworkingsockets";
constexpr auto STUDIORENDER_DLL = "studiorender";
constexpr auto TIER0_DLL = "tier0";
constexpr auto DATACACHE_DLL = "datacache";
constexpr auto VGUI2_DLL = "vgui2";
constexpr auto VGUIMATSURFACE_DLL = "vguimatsurface";
constexpr auto VPHYSICS_DLL = "vphysics";
constexpr auto VSTDLIB_DLL = "vstdlib";

namespace VirtualMethod
{
    template <typename T, std::size_t Idx, typename ...Args>
    constexpr T call(void* classBase, Args... args) noexcept
    {
        return (*reinterpret_cast<T(__thiscall***)(void*, Args...)>(classBase))[Idx](classBase, args...);
    }
}

#define VIRTUAL_METHOD(returnType, name, idx, args, argsRaw) \
returnType name args noexcept \
{ \
    return VirtualMethod::call<returnType, idx>argsRaw; \
}

#define VIRTUAL_METHOD_V(returnType, name, idx, args, argsRaw) VIRTUAL_METHOD(returnType, name, idx, args, argsRaw)
