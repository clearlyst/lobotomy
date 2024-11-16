#pragma once

#include <string>
#include "singleton.hpp"
#include "imgui/imgui.h"

struct IDirect3DDevice9;
class ctab
{
public:
    const char* szName = nullptr;
    void (*render_function)() = nullptr;
};
class Menu
    : public Singleton<Menu>
{
public:
    void Initialize();
    void Shutdown();
    void switch_font_cfg(ImFontConfig& f, bool cfg[9]);
    void OnDeviceLost();
    void OnDeviceReset();
    bool fonts_initialized;
    void Render();
    void  SpectatorList();
    void Toggle();

    bool IsVisible() const { return _visible; }
    ImVec2 menu_pos = ImVec2();
    ImVec2 menu_size = ImVec2();
private:
    void CreateStyle();

    ImGuiStyle        _style;
    bool              _visible;
};