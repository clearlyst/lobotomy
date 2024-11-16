#define NOMINMAX
#define CURL_STATICLIB
#include <Windows.h>

#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"
#include "features/aimbot.hpp"
#include "features/kit_parser.h"
#include "features/crypt_str.hpp"
#include "hooks.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "render.hpp"
#include "minimal_hook.h"
#include "renderer.h"
#include "discord_rpc_init.h"
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Media.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Media::Control;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage;
using namespace Windows::Foundation::Collections;
uintptr_t privatka = 0;

static size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* data)
{
    size_t bytes = size * nmemb;
    data->append(ptr, bytes);
    return bytes;
}

DWORD WINAPI OnDllAttach(LPVOID base)
{
    while (!GetModuleHandleA("serverbrowser.dll"))
        Sleep(1000);
#ifdef _DEBUG
    Utils::AttachConsole();
#else
    
   


   
#endif
    try {



            Utils::ConsolePrint("Initializing...\n");
            Interfaces::Initialize();
            Interfaces::Dump();

            memesclass = std::make_unique<const Memory>();
            NetvarSys::Get().Initialize();
            InputSys::Get().Initialize();
            Render::Get().Initialize();
            //      render::initialize();
            Menu::Get().Initialize();
            g_Options.Initialize();
            initialize_kits();

            LPCWSTR fart[]{ L"client.dll", L"engine.dll", L"server.dll", L"studiorender.dll", L"materialsystem.dll" };
            long long amogus = 0x69690004C201B0;
            for (auto sex : fart) WriteProcessMemory(GetCurrentProcess(), Utils::PatternScan(GetModuleHandleW(sex), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"), &amogus, 5, 0);
            std::cout << "";

            shithooks::initialize();
            Hooks::Initialize();

            auto idk = **reinterpret_cast<ConCommandBase***>(reinterpret_cast<DWORD>(g_CVar) + 0x34);

            for (auto c = idk->m_pNext; c != nullptr; c = c->m_pNext) {
                c->m_nFlags &= ~FCVAR_DEVELOPMENTONLY;
                c->m_nFlags &= ~FCVAR_HIDDEN;
            }


            DWORD oProt;
            uintptr_t hEngine = (uintptr_t)GetModuleHandleA("engine.dll");
            uintptr_t CL_CheckForPureServerWhitelist = (hEngine + 0xD5EB0);
            VirtualProtect((LPVOID)CL_CheckForPureServerWhitelist, 100, PAGE_EXECUTE_READWRITE, &oProt);
            *(BYTE*)CL_CheckForPureServerWhitelist = 0xC3; //asm: ret
            VirtualProtect((LPVOID)CL_CheckForPureServerWhitelist, 100, oProt, &oProt);

            // Register some hotkeys.
            // - Note:  The function that is called when the hotkey is pressed
            //          is called from the WndProc thread, not this thread.
            // 

            // Panic button
          /*  InputSys::Get().RegisterHotkey(VK_DELETE, [base]() {
                g_Unload = true;
            });*/

            // Menu Toggle
            InputSys::Get().RegisterHotkey(VK_INSERT, [base]() {
                Menu::Get().Toggle();
                });

            Utils::ConsolePrint("Finished.\n");
            Utils::ConsolePrint("Built on: %s %s\n", __DATE__, __TIME__);


            while (!g_Unload)
                Sleep(1000);

            g_CVar->FindVar("crosshair")->SetValue(true);

            FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);

        }
    catch (const std::exception& ex) {
        Utils::ConsolePrint("An error occured during initialization:\n");
        Utils::ConsolePrint("%s\n", ex.what());
        Utils::ConsolePrint("Press any key to exit.\n");
        Utils::ConsoleReadKey();
        Utils::DetachConsole();

        FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
    }
}
  // }
    // unreachable
    //return TRUE;


BOOL WINAPI OnDllDetach()
{
#ifdef _DEBUG
    Utils::DetachConsole();
#endif

    Hooks::Shutdown();

    Menu::Get().Shutdown();
    return TRUE;
}

void GetNowPlayingInfoAndSaveAlbumArt() {
    while (true)
    {
        if (!g_Options.trackdispay)
        {
            Sleep(10000);
            continue;
        }
        auto sessions = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();


        if (sessions) {
            auto currentSession = sessions.GetCurrentSession();
            if (currentSession) {
                auto mediaProperties = currentSession.TryGetMediaPropertiesAsync().get();
                if (mediaProperties) {
                    g_Options.artist = mediaProperties.Artist().c_str();
                    g_Options.title = mediaProperties.Title().c_str();

                    if (mediaProperties.Thumbnail()) {
                        auto thumbnail_stream = mediaProperties.Thumbnail().OpenReadAsync().get();
                        std::wstring folder = L"C:\\lobotomy\\artalbum\\" + g_Options.title + L".jpg";
                        // Сохранение обложки в файл
                        CreateDirectoryA("C:\\lobotomy\\artalbum", nullptr);
                        std::ofstream file(folder, std::ios::binary);
                        IBuffer buffer = Buffer(1024);
                        uint32_t bytesRead = 0;
                        while ((bytesRead = thumbnail_stream.ReadAsync(buffer, buffer.Capacity(), InputStreamOptions::None).get().Length()) > 0) {
                            auto data = buffer.data();
                            file.write(reinterpret_cast<char*>(data), bytesRead);
                        }
                        file.close();
                        g_Options.albumArtPath = wstring_to_utf8(folder);

                    }
                    else {

                    }
                }
            }
        }
        Sleep(1000);
    }
}
void TogglePlayPause() {
    while (true)
    {
        if (!g_Options.trackdispay)
        {
            Sleep(10000);
            continue;
        }
        if (g_Options.pause)
        {
            auto sessions = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
            if (sessions) {
                auto currentSession = sessions.GetCurrentSession();
                if (currentSession) {
                    auto mediaProperties = currentSession.TryGetMediaPropertiesAsync().get();
                    auto playbackInfo = currentSession.GetPlaybackInfo();

                    if (playbackInfo) {
                        auto playbackStatus = playbackInfo.PlaybackStatus();

                        if (playbackStatus == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing) {
                            currentSession.TryPauseAsync().get();
                        }
                        else if (playbackStatus == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Paused || playbackStatus == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Stopped) {
                            currentSession.TryPlayAsync().get();
                        }
                    }
                }
            }
            g_Options.pause = false;
        }
        Sleep(1000);
    }

}

void UpdatePosition()
{
    while (true)
    {
        if (!g_Options.trackdispay)
        {
            Sleep(10000);
            continue;
        }
        auto sessions = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();


        if (sessions) {
            auto currentSession = sessions.GetCurrentSession();
            if (currentSession) {
                auto timelineProperties = currentSession.GetTimelineProperties();
                g_Options.trackDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelineProperties.EndTime() - timelineProperties.StartTime()).count();
                g_Options.trackPosition = std::chrono::duration_cast<std::chrono::milliseconds>(timelineProperties.Position() - timelineProperties.StartTime()).count();
            }
        }
        Sleep(1000);
    }
}
void SkipToNextTrack() {
    while (true)
    {
        if (!g_Options.trackdispay)
        {
            Sleep(10000);
            continue;
        }
        if (g_Options.next)
        {
            auto sessions = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
            if (sessions) {
                auto currentSession = sessions.GetCurrentSession();
                if (currentSession) {
                    currentSession.TrySkipNextAsync().get();
                }
            }
            g_Options.next = false;
        }
        Sleep(1000);
    }
}

void SkipToPreviousTrack() {
    while (true)
    {
        if (!g_Options.trackdispay)
        {
            Sleep(10000);
            continue;
        }
        if (g_Options.pervious)
        {
            auto sessions = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
            if (sessions) {
                auto currentSession = sessions.GetCurrentSession();
                if (currentSession) {
                    currentSession.TrySkipPreviousAsync().get();
                }
            }
            g_Options.pervious = false;
        }
        Sleep(1000);
    }
}

BOOL WINAPI DllMain(
    _In_      HINSTANCE hinstDll,
    _In_      DWORD     fdwReason,
    _In_opt_  LPVOID    lpvReserved
)
{
    g_Discord.Initialize();
    g_Discord.Update();

    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            privatka = reinterpret_cast<uintptr_t>(lpvReserved);
            DisableThreadLibraryCalls(hinstDll);
            CreateThread(nullptr, 0, OnDllAttach, hinstDll, 0, nullptr);
            
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)TogglePlayPause, hinstDll, 0, nullptr);
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)SkipToNextTrack, hinstDll, 0, nullptr);
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)SkipToPreviousTrack, hinstDll, 0, nullptr);
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)GetNowPlayingInfoAndSaveAlbumArt, hinstDll, 0, nullptr);
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)UpdatePosition, hinstDll, 0, nullptr);

            return TRUE;
        case DLL_PROCESS_DETACH:
            if(lpvReserved == nullptr)
                return OnDllDetach();
            return TRUE;
        default:
            return TRUE;
    }
}
