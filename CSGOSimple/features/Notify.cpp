#include "Notify.h"
#include "../options.hpp"
#include "../imgui/imgui.h"
#include "../render.hpp"
#include "chat_hud.h"
#include "../helpers/utils.hpp"
template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}
void notification::run(std::string text, std::string chat_text, bool console, bool chat, bool screen)
{
	Color _color = Color(g_Options.fuckingaccent[0] * 255, g_Options.fuckingaccent[1] * 255, g_Options.fuckingaccent[2] * 255);

	if (console)
	{
		log_in_console(text, _color);
	}

	if (chat && g_EngineClient->IsInGame() && g_EngineClient->IsConnected())
	{
		auto g_ChatElement = FindHudElement<c_hudchat>("CHudChat");
		if (g_ChatElement)
			g_ChatElement->chatprintf(0, 0, chat_text.c_str());
	}

	if (screen)
	{
		log_on_scene(text, _color);
	}
}

void notification::log_in_console(std::string text, Color _color)
{
	if (text.empty())
	{
		return;
	}

	
}

void notification::log_on_scene(std::string text, Color _color)
{
	if (text.empty())
	{
		return;
	}

	notification::notify_list.push_back(notify_t(text.c_str(), Color(_color.r(), _color.g(), _color.b(), _color.a())));
}

void notification::draw()
{
	const auto stay_time = 4.0f;
	const auto fade_time = 0.6f;

	for (offset_message = 0; offset_message < notification::notify_list.size(); offset_message++)
	{
		if (notification::notify_list.empty())
		{
			continue;
		}

		notify_t notify = notification::notify_list[offset_message];

		float elapsed_time = g_GlobalVars->curtime - (notify.time + stay_time);
		int fade_alpha = static_cast<int>(255.f - (elapsed_time / fade_time) * 255.f);
		fade_alpha = std::clamp(fade_alpha, 0, 255);

		if (fade_alpha <= 0)
		{
			notification::notify_list.erase(notification::notify_list.begin() + offset_message);
			offset_message--;
			continue;
		}

		ImVec2 size = Render::Get().GetTextSize("[bhop cheat] ", fonts::scene_font, g_Options.fonts.scene_size);

		lolrendertext(5, 5 + (offset_message * 12.0f), g_Options.fonts.scene_size, fonts::scene_font, ("[bhop cheat] "), false, Color(notify._color.r(), notify._color.g(), notify._color.b(), fade_alpha), g_Options.fonts.scene_flag[9], g_Options.fonts.scene_flag[10]);
		lolrendertext(5 + size.x, 5 + (offset_message * 12.0f), g_Options.fonts.scene_size, fonts::scene_font, notify.text, false, Color(255, 255, 255, fade_alpha), g_Options.fonts.scene_flag[9], g_Options.fonts.scene_flag[10]);
	}
}