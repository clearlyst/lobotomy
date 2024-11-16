#include "Utils.hpp"

#define NOMINMAX
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "../valve_sdk/csgostructs.hpp"
#include "Math.hpp"
#include <map>
#include "../features/virtualmethod.h"
#include <psapi.h>

HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;
std::map<const char*, HMODULE> modules;
namespace Utils {
    std::vector<char> HexToBytes(const std::string& hex) {
        std::vector<char> res;

        for (auto i = 0u; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            char byte = (char)strtol(byteString.c_str(), NULL, 16);
            res.push_back(byte);
        }

        return res;
    }
    int EpochTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
    std::string BytesToString(unsigned char* data, int len) {
        constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
        std::string res(len * 2, ' ');
        for (int i = 0; i < len; ++i) {
            res[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
            res[2 * i + 1] = hexmap[data[i] & 0x0F];
        }
        return res;
    }
    std::vector<std::string> Split(const std::string& str, const char* delim) {
        std::vector<std::string> res;
        char* pTempStr = _strdup(str.c_str());
        char* context = NULL;
        char* pWord = strtok_s(pTempStr, delim, &context);
        while (pWord != NULL) {
            res.push_back(pWord);
            pWord = strtok_s(NULL, delim, &context);
        }

        free(pTempStr);

        return res;
    }

    unsigned int FindInDataMap(datamap_t* pMap, const char* name) {
        while (pMap) {
            for (int i = 0; i < pMap->dataNumFields; i++) {
                if (pMap->dataDesc[i].fieldName == NULL)
                    continue;

                if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
                    return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

                if (pMap->dataDesc[i].fieldType == FIELD_EMBEDDED) {
                    if (pMap->dataDesc[i].td) {
                        unsigned int offset;

                        if ((offset = FindInDataMap(pMap->dataDesc[i].td, name)) != 0)
                            return offset;
                    }
                }
            }
            pMap = pMap->baseMap;
        }

        return 0;
    }
    bool IsDangerZone()
    {
        return g_GameTypes->GetCurrentGameType() == 6;
    }
    /*
     * @brief Create console
     *
     * Create and attach a console window to the current process
     */
    void AttachConsole()
    {
        _old_out = GetStdHandle(STD_OUTPUT_HANDLE);
        _old_err = GetStdHandle(STD_ERROR_HANDLE);
        _old_in = GetStdHandle(STD_INPUT_HANDLE);

        ::AllocConsole() && ::AttachConsole(GetCurrentProcessId());

        _out = GetStdHandle(STD_OUTPUT_HANDLE);
        _err = GetStdHandle(STD_ERROR_HANDLE);
        _in = GetStdHandle(STD_INPUT_HANDLE);

        SetConsoleMode(_out,
            ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

        SetConsoleMode(_in,
            ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
            ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
    }

    /*
     * @brief Detach console
     *
     * Detach and destroy the attached console
     */
    void DetachConsole()
    {
        if (_out && _err && _in) {
            FreeConsole();

            if (_old_out)
                SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
            if (_old_err)
                SetStdHandle(STD_ERROR_HANDLE, _old_err);
            if (_old_in)
                SetStdHandle(STD_INPUT_HANDLE, _old_in);
        }
    }

    /*
     * @brief Print to console
     *
     * Replacement to printf that works with the newly created console
     */
    bool ConsolePrint(const char* fmt, ...)
    {
        if (!_out)
            return false;

        char buf[1024];
        va_list va;

        va_start(va, fmt);
        _vsnprintf_s(buf, 1024, fmt, va);
        va_end(va);

        return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
    }

    /*
     * @brief Blocks execution until a key is pressed on the console window
     *
     */
    char ConsoleReadKey()
    {
        if (!_in)
            return false;

        auto key = char{ 0 };
        auto keysread = DWORD{ 0 };

        ReadConsoleA(_in, &key, 1, &keysread, nullptr);

        return key;
    }


    /*
     * @brief Wait for all the given modules to be loaded
     *
     * @param timeout How long to wait
     * @param modules List of modules to wait for
     *
     * @returns See WaitForSingleObject return values.
     */
    int WaitForModules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules)
    {
        bool signaled[32] = { 0 };
        bool success = false;

        std::uint32_t totalSlept = 0;

        if (timeout == 0) {
            for (auto& mod : modules) {
                if (GetModuleHandleW(std::data(mod)) == NULL)
                    return WAIT_TIMEOUT;
            }
            return WAIT_OBJECT_0;
        }

        if (timeout < 0)
            timeout = INT32_MAX;

        while (true) {
            for (auto i = 0u; i < modules.size(); ++i) {
                auto& module = *(modules.begin() + i);
                if (!signaled[i] && GetModuleHandleW(std::data(module)) != NULL) {
                    signaled[i] = true;

                    //
                    // Checks if all modules are signaled
                    //
                    bool done = true;
                    for (auto j = 0u; j < modules.size(); ++j) {
                        if (!signaled[j]) {
                            done = false;
                            break;
                        }
                    }
                    if (done) {
                        success = true;
                        goto exit;
                    }
                }
            }
            if (totalSlept > std::uint32_t(timeout)) {
                break;
            }
            Sleep(10);
            totalSlept += 10;
        }

    exit:
        return success ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
    }

    /*
     * @brief Scan for a given byte pattern on a module
     *
     * @param module    Base of the module to search
     * @param signature IDA-style byte array pattern
     *
     * @returns Address of the first occurence
     */
    HMODULE GetModule(const char* name)
    {
        if (!modules.count(name) || !modules[name])
            modules[name] = GetModuleHandleA(name);

        return modules[name];
    }

    std::uint8_t* PatternScan2(const char* module_name, const char* signature)
    {
        auto module = GetModule(module_name);

        static auto pattern_to_byte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for (auto current = start; current < end; ++current) {
                if (*current == '?') {
                    ++current;
                    if (*current == '?')
                        ++current;
                    bytes.push_back(-1);
                }
                else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)module;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = pattern_to_byte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i) {
            bool found = true;
            for (auto j = 0ul; j < s; ++j) {
                if (scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return &scanBytes[i];
            }
        }
        return nullptr;
    }
    std::uint8_t* PatternScan(void* module, const char* signature)
    {
        static auto pattern_to_byte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for (auto current = start; current < end; ++current) {
                if (*current == '?') {
                    ++current;
                    if (*current == '?')
                        ++current;
                    bytes.push_back(-1);
                }
                else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)module;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = pattern_to_byte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i) {
            bool found = true;
            for (auto j = 0ul; j < s; ++j) {
                if (scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return &scanBytes[i];
            }
        }
        return nullptr;
    }

    /*
         * @brief Set player clantag
         *
         * @param tag New clantag
         */
    void SetClantag(const char* tag)
    {
        static auto fnClantagChanged = (int(__fastcall*)(const char*, const char*))PatternScan(GetModuleHandleW(L"engine.dll"), "53 56 57 8B DA 8B F9 FF 15");

        fnClantagChanged(tag, tag);
    }
    void apply_clan_tag(const char* name) {
        using Fn = int(__fastcall*)(const char*, const char*);
        static auto apply_clan_tag_fn = reinterpret_cast<Fn>(PatternScan("engine.dll", "53 56 57 8B DA 8B F9 FF 15"));
        apply_clan_tag_fn(name, name);
    };
    /*
     * @brief Set player name
     *
     * @param name New name
     */
    void SetName(const char* name)
    {
        static auto nameConvar = g_CVar->FindVar("name");
        nameConvar->m_fnChangeCallbacks.m_Size = 0;

        // Fix so we can change names how many times we want
        // This code will only run once because of `static`
        static auto do_once = (nameConvar->SetValue("\n���"), true);

        nameConvar->SetValue(name);
    }
}
static std::pair<void*, std::size_t> getModuleInformation(const char* name) noexcept
{
    if (HMODULE handle = GetModuleHandleA(name)) {
        if (MODULEINFO moduleInfo; GetModuleInformation(GetCurrentProcess(), handle, &moduleInfo, sizeof(moduleInfo)))
            return std::make_pair(moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage);
    }
    return {};
}

[[nodiscard]] static auto generateBadCharTable(std::string_view pattern) noexcept
{
    assert(!pattern.empty());

    std::array<std::size_t, (std::numeric_limits<std::uint8_t>::max)() + 1> table;

    auto lastWildcard = pattern.rfind('?');
    if (lastWildcard == std::string_view::npos)
        lastWildcard = 0;

    const auto defaultShift = (std::max)(std::size_t(1), pattern.length() - 1 - lastWildcard);
    table.fill(defaultShift);

    for (auto i = lastWildcard; i < pattern.length() - 1; ++i)
        table[static_cast<std::uint8_t>(pattern[i])] = pattern.length() - 1 - i;

    return table;
}

static std::uintptr_t findPattern(const char* moduleName, std::string_view pattern, bool reportNotFound = true) noexcept
{
    static auto id = 0;
    ++id;

    const auto [moduleBase, moduleSize] = getModuleInformation(moduleName);

    if (moduleBase && moduleSize) {
        const auto lastIdx = pattern.length() - 1;
        const auto badCharTable = generateBadCharTable(pattern);

        auto start = static_cast<const char*>(moduleBase);
        const auto end = start + moduleSize - pattern.length();

        while (start <= end) {
            int i = lastIdx;
            while (i >= 0 && (pattern[i] == '?' || start[i] == pattern[i]))
                --i;

            if (i < 0)
                return reinterpret_cast<std::uintptr_t>(start);

            start += badCharTable[static_cast<std::uint8_t>(start[lastIdx])];
        }
    }

    if (reportNotFound)
        MessageBoxA(NULL, ("failed to find pattern #" + std::to_string(id) + '!').c_str(), "fuck(((", MB_OK | MB_ICONWARNING);
    return 0;
}
template <typename T>
static constexpr auto relativeToAbsolute(uintptr_t address) noexcept
{
    return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
}

Memory::Memory() noexcept
{
    setAbsOrigin = relativeToAbsolute<decltype(setAbsOrigin)>(findPattern("client.dll", "\xE8????\xEB\x19\x8B\x07") + 1);

    isDepthOfFieldEnabled = findPattern(CLIENT_DLL, "\x8B\x0D????\x56\x8B\x01\xFF\x50\x34\x8B\xF0\x85\xF6\x75\x04");
    loadSky = relativeToAbsolute<decltype(loadSky)>(findPattern(ENGINE_DLL, "\xE8????\x84\xC0\x74\x2D\xA1") + 1);
    getBonePos = relativeToAbsolute<decltype(getBonePos)>(findPattern("client.dll", "\xE8????\x8D\x14\x24") + 1);
    modifyEyePosition = relativeToAbsolute<decltype(modifyEyePosition)>(findPattern("client.dll", "\xE8????\x8B\x06\x8B\xCE\xFF\x90????\x85\xC0\x74\x50") + 1);
    lookUpBone = reinterpret_cast<decltype(lookUpBone)>(findPattern("client.dll", "\x55\x8B\xEC\x53\x56\x8B\xF1\x57\x83\xBE?????\x75\x14"));
    restoreEntityToPredictedFrame = reinterpret_cast<decltype(restoreEntityToPredictedFrame)>(findPattern("client", "\x55\x8B\xEC\x8B\x4D?\x56\xE8????\x8B\x75"));
    getPlayerViewmodelArmConfigForPlayerModel = relativeToAbsolute<decltype(getPlayerViewmodelArmConfigForPlayerModel)>(findPattern("client.dll", "\xE8????\x89\x87????\x6A") + 1);
    sendDatagram = findPattern(ENGINE_DLL, "\x55\x8B\xEC\x83\xE4\xF0\xB8????\xE8????\x56\x57\x8B\xF9\x89\x7C\x24\x14");
    disablePostProcessing = *reinterpret_cast<bool**>(findPattern(CLIENT_DLL, "\x83\xEC\x4C\x80\x3D") + 5);
    predictionRandomSeed = *reinterpret_cast<int**>(findPattern(CLIENT_DLL, "\x8B\x0D????\xBA????\xE8????\x83\xC4\x04") + 2);
}