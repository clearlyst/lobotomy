#pragma once

#define NOMINMAX
#include <Windows.h>
#include <string>
#include <initializer_list>
#include "../valve_sdk/sdk.hpp"

namespace Utils {
    std::vector<char> HexToBytes(const std::string& hex);
    std::string BytesToString(unsigned char* data, int len);
    std::vector<std::string> Split(const std::string& str, const char* delim);
    unsigned int FindInDataMap(datamap_t* pMap, const char* name);
    /*
     * @brief Create console
     *
     * Create and attach a console window to the current process
     */
    void AttachConsole();

    /*
     * @brief Detach console
     *
     * Detach and destroy the attached console
     */
    void DetachConsole();
    int   EpochTime();
    /*
     * @brief Print to console
     *
     * Replacement to printf that works with the newly created console
     */
    bool ConsolePrint(const char* fmt, ...);

    /*
     * @brief Blocks execution until a key is pressed on the console window
     *
     */
    char ConsoleReadKey();

    /*
     * @brief Wait for all the given modules to be loaded
     *
     * @param timeout How long to wait
     * @param modules List of modules to wait for
     *
     * @returns See WaitForSingleObject return values.
     */
    int WaitForModules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules);

    /*
     * @brief Scan for a given byte pattern on a module
     *
     * @param module    Base of the module to search
     * @param signature IDA-style byte array pattern
     *
     * @returns Address of the first occurence
     */
    std::uint8_t* PatternScan(void* module, const char* signature);
    std::uint8_t* PatternScan2(const char* module_name, const char* signature);

    /*
     * @brief Set player clantag
     *
     * @param tag New clantag
     */
    void SetClantag(const char* tag);

    /*
     * @brief Set player name
     *
     * @param name New name
     */
    void SetName(const char* name);
 

    bool IsDangerZone();

}
class Memory {
public:
    Memory() noexcept;

    void(__stdcall* restoreEntityToPredictedFrame)(int, int);
    std::uintptr_t drawScreenEffectMaterial;
    std::uintptr_t isDepthOfFieldEnabled;
    std::uintptr_t sendDatagram;
    int(__thiscall* lookUpBone)(void*, const char*);
    void(__thiscall* getBonePos)(void*, int, Vector*);
    std::uintptr_t modifyEyePosition;
    std::add_pointer_t<const char** __fastcall(const char* playerModelName)> getPlayerViewmodelArmConfigForPlayerModel;
    bool* disablePostProcessing;
    std::add_pointer_t<void __fastcall(const char*)> loadSky;
    int* predictionRandomSeed;
    void(__thiscall* setAbsOrigin)(C_BaseEntity*, const Vector&);
};
inline std::unique_ptr<const Memory> memesclass;