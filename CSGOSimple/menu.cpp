#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"
#include "features/item_definitions.h"
#include "features/kit_parser.h"
#include "features/skins.h"
#include "render.hpp"
#include "features/icons.h"
#include "features/chat_hud.h"
#include <map>
#include <algorithm>
#include "features/crypt_str.hpp"
#include "../CSGOSimple/hooks.hpp"
#include "imgui/impl/imgui_impl_dx9.h"
#include "features/visuals.hpp"
#include "imgui/freetype/imgui_freetype.h"
#include "features/Notify.h"
 bool open = true;
 int main_tab = 0;
 bool initialized = false;
 bool unhook = true;
 int indicator_tab = 0;
 int indicators_tab = 0;
 int chams_tab = 0;
 int esp_tab = 0;
 int visu_tab = 0;
 int aim_tab = 0;
 int subtab = 0;
 int weapon_selection = 0;
 int skin_selection = 0;
void ReadDirectory(const std::string& name, std::vector<std::string>& v)
{
	auto pattern(name);
	pattern.append("\\*.lobotomy");
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			v.emplace_back(data.cFileName);
		} while (FindNextFileA(hFind, &data) != 0);
		FindClose(hFind);
	}
}
template<class T>
static T* FindHuddElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(Utils::PatternScan(GetModuleHandleA("client.dll"), "B9 ? ? ? ? E8 ? ? ? ? 85 C0 74 0F FF") + 0x1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 53 8B 5D 08 56 57 8B F9 33"));
	return (T*)find_hud_element(pThis, name);
}

auto chatelemt = FindHuddElement<c_hudchat>("CHudChat");
CCStrike15ItemSystem* get_item_system()
{
	static auto item_system = Utils::PatternScan(GetModuleHandleA("client.dll"), "A1 ? ? ? ? 85 C0 75 53");
	static auto fn = reinterpret_cast<CCStrike15ItemSystem * (__stdcall*)()>(item_system);
	return fn();
}

void Menu::Initialize()
{
	//CreateStyle();

	_visible = true;
}

void Menu::Shutdown()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
	ImGui_ImplDX9_CreateDeviceObjects();
}

struct hud_weapons_t
{
	std::int32_t* get_weapon_count()
	{
		return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
	}
};

namespace ImGuiEx
{
	inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
	{
		float clr[4] = {
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};
		//clr[3]=255;
		if (ImGui::ColorEdit4(label, clr, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar)) {
			v->SetColor(clr[0], clr[1], clr[2], clr[3]);
			return true;
		}
		return false;
	}
	inline bool ColorEdit4a(const char* label, Color* v, bool show_alpha = true)
	{
		float clr[4] = {
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};
		//clr[3]=255;
		if (ImGui::ColorEdit4(label, clr, show_alpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar)) {
			v->SetColor(clr[0], clr[1], clr[2], clr[3]);
			return true;
		}
		return false;
	}

	inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}


struct tab_anim {
	float active_text_anim, active_outline_alpha, active_rect_alpha, active_line_alpha, active_text_anim2, active_gradient_alpha;
};
ImGuiColorEditFlags no_alpha = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha  | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar;
ImGuiColorEditFlags w_alpha = ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar  | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar;

int m_tabs{};
bool tab(const char* label, bool selected) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
	ImVec2 pos = window->DC.CursorPos;

	const ImRect rect(pos, ImVec2(pos.x + label_size.x + 5, pos.y + 20));
	ImGui::ItemSize(ImVec4(rect.Min.x, rect.Min.y, rect.Max.x + 2.f, rect.Max.y), style.FramePadding.y);
	if (!ImGui::ItemAdd(rect, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held, NULL);


	window->DrawList->AddLine(ImVec2(rect.Min.x, rect.Min.y + 18), ImVec2(rect.Max.x, rect.Max.y - 2),
		selected ? ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]) :
		ImColor(50 / 255.f, 50 / 255.f, 50 / 255.f, 255.f), 1.f);

	window->DrawList->AddText(ImVec2((rect.Min.x + rect.Max.x) / 2.f - (label_size.x / 2.f), (rect.Min.y + rect.Max.y) / 2.f - (label_size.y / 2.f)),
		selected ? ImColor(255, 255, 255, 255) :
		ImColor(50, 50, 50, 160), label);

	return pressed;
}
// Main code
void render_tab(const char* szTabBar, const ctab* arrTabs, const std::size_t nTabsCount, int* nCurrentTab, const ImVec4& colActive, ImGuiTabBarFlags flags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip) {
	// set active tab color
	ImGui::PushStyleColor(ImGuiCol_TabHovered, colActive);
	ImGui::PushStyleColor(ImGuiCol_TabActive, colActive);
	if (ImGui::BeginTabBar(szTabBar, flags)) {
		for (std::size_t i = 0U; i < nTabsCount; i++) {
			// add tab
			if (ImGui::BeginTabItem(arrTabs[i].szName)) {
				// set current tab index
				*nCurrentTab = i;
				ImGui::EndTabItem();
			}
		}

		// render inner tab
		if (arrTabs[*nCurrentTab].render_function != nullptr)
			arrTabs[*nCurrentTab].render_function();

		ImGui::EndTabBar();
	}
	ImGui::PopStyleColor(2);
}

void  Menu::switch_font_cfg(ImFontConfig& f, bool cfg[9]) {
	f.FontBuilderFlags = 0;
	if (cfg[0]) f.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_NoHinting;
	if (cfg[1]) f.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_NoAutoHint;
	if (cfg[2]) f.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LightHinting;
	if (cfg[3]) f.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_MonoHinting;
	if (cfg[4]) f.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bold;
	if (cfg[5]) f.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Oblique;
	if (cfg[6]) f.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Monochrome;
	if (cfg[7]) f.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
	if (cfg[8]) f.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bitmap;
}


void miscellaneous() {
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::Columns(2, nullptr, false); {
		ImGui::BeginChild(("misc.movement"), ImVec2(0, 265), true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted(("movement")); ImGui::PopFont();
				ImGui::EndMenuBar();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0));

			ImGui::Checkbox("bunny hop", &g_Options.misc_bhop);
			ImGui::Checkbox("auto strafe", &g_Options.autostrafe); ImGui::SameLine();     ImGui::Hotkey("##as", &g_Options.AutoStafe_key);
			ImGui::Checkbox("mini jump", &g_Options.misc_minijump); ImGui::SameLine();     ImGui::Hotkey("##mj", &g_Options.abobakey);
			if (g_Options.misc_minijump)
			{
				ImGui::Checkbox("show in binds##mj", &g_Options.showmj);

			}
			ImGui::Checkbox("auto jumpbug", &g_Options.jump_bug); ImGui::SameLine();          ImGui::Hotkey("##jb", &g_Options.jump_bug_key);
			if (g_Options.jump_bug)
			{
				ImGui::Checkbox("show in binds##jb", &g_Options.showjb); 	ImGui::SameLine();				ImGuiEx::ColorEdit4a("##jbdetect", &g_Options.jbdetect);
				ImGui::Checkbox("ladderbug fix", &g_Options.ladderbugfix);
			}
			ImGui::Checkbox("fireman", &g_Options.FireMan);
			ImGui::SameLine();
			ImGui::Hotkey("##FireManKey", &g_Options.FireManKey);
			if (g_Options.FireMan)
				ImGui::Combo("fireman type", &g_Options.FireManType, "delay (stable)\0 no delay (no stable)\0");
			ImGui::Checkbox("auto edgebug", &g_Options.edge_bug);
			ImGui::SameLine();
			ImGui::Hotkey("##edgebugkey", &g_Options.edge_bug_key);
			if (g_Options.edge_bug)
			{
				ImGui::Checkbox("auto strafe to edge", &g_Options.AutoStrafeEdgeBug);
				if (!g_Options.AutoStrafeEdgeBug)
				{
					ImGui::SliderFloat("strafe clamp", &g_Options.deltaScaler, 1.f, 179.f);
					ImGui::Combo("delta scale", &g_Options.DeltaType, "full\0two-fifths\0half\0quarter\0");
				}
				ImGui::SliderInt("circle (unlimited value)", &g_Options.EdgeBugCircle, 0, 360);
				ImGui::SliderInt("ticks (unlimited value)", &g_Options.EdgeBugTicks, 1, 128);
				ImGui::SliderFloat("mouse lock (unlimited value)", &g_Options.EdgeBugMouseLock, 0.0f, 10.0f, "%.2f");
				ImGui::Checkbox("silent", &g_Options.SiletEdgeBug);
				ImGui::Checkbox("extra advanced", &g_Options.MegaEdgeBug);
				ImGui::Checkbox("show in chat", &g_Options.EdgeBugDetectChat);
				ImGui::Checkbox("show in binds##eb", &g_Options.showeb); ImGui::SameLine(); ImGuiEx::ColorEdit4a("##ebdetect", &g_Options.ebdetect);
			}
			ImGui::Checkbox("auto airstuck", &g_Options.airstuck);
			ImGui::SameLine();
			ImGui::Hotkey("##AirStuckKey", &g_Options.airstuckkey);
			
		
			ImGui::Checkbox("autoduck (!)", &g_Options.AutoBounce);
			
			
			ImGui::Checkbox("edge jump", &g_Options.edgejump.enabled); ImGui::SameLine(); ImGui::Hotkey("##ej", &g_Options.edgejump.hotkey);
			if (g_Options.edgejump.enabled)
			{
				ImGui::Checkbox("lj on ej", &g_Options.edgejump.edge_jump_duck_in_air);
				ImGui::Checkbox("laj on ej", &g_Options.edgejump.lg_on_ej);
			}
			if (g_Options.edgejump.edge_jump_duck_in_air)
			{
				ImGui::Checkbox("show in binds##lj", &g_Options.showlj);
				ImGui::Checkbox("longjump end", &g_Options.ljend);
			}
			ImGui::Checkbox("delay hop", &g_Options.delayhop); ImGui::SameLine();          ImGui::Hotkey("##dh", &g_Options.dhkey);
			if (g_Options.delayhop)
			{
				ImGui::Spacing();
				ImGui::SliderInt("dh ticks", &g_Options.delayhopticks, 0, 15);
			}
			ImGui::Checkbox("auto pixelsurf", &g_Options.pixelsurf);
			if (g_Options.pixelsurf)
			{
				//ImGui::Checkbox("auto align", &g_Options.autoalign);
				//ImGui::Checkbox("free move on surf", &g_Options.freelook);
				ImGui::Checkbox("pixelsurf fix", &g_Options.ps_fix);
				ImGui::Checkbox("show in binds##ps", &g_Options.showps);			ImGui::SameLine();		ImGuiEx::ColorEdit4a("##psdetect", &g_Options.psdetect);


			}
			//ImGui::Checkbox("auto duck", &g_Options.autoduck);
			if (g_Options.autoduck)
			{
				ImGui::Spacing();
				ImGui::SliderFloat("  ticks##ad", &g_Options.autoducktick, 0.f, 50.f, "%.1f");
			}
			ImGui::Checkbox("adaptive key cancelling", &g_Options.nulls3);
			if (g_Options.nulls3)
			{
				ImGui::Spacing();
				if (ImGui::BeginCombo("##nulls_filter", "...##9w9w9ww", ImGuiComboFlags_NoArrowButton))
				{
					ImGui::Selectable("-w", &g_Options.forwardd, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("-s", &g_Options.back, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("-a", &g_Options.left, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("-d", &g_Options.right, ImGuiSelectableFlags_DontClosePopups);
					;

					ImGui::EndCombo();
				}
				if (ImGui::BeginCombo("##nulls_3filter", "...##shshhshshhw23u27u", ImGuiComboFlags_NoArrowButton))
				{
					ImGui::Selectable("ej", &g_Options.nulls3_ej, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("lj", &g_Options.nulls3_lj, ImGuiSelectableFlags_DontClosePopups);
					ImGui::Selectable("mj", &g_Options.nulls3_mj, ImGuiSelectableFlags_DontClosePopups);

					ImGui::EndCombo();
				}
			}
			//ImGui::Checkbox("yaw changer", &g_Options.yaw_changer);
			//if (g_Options.yaw_changer)
			//{
		//		ImGui::Spacing();
		//		ImGui::SliderFloat("m_yaw", &g_Options.yaw1, 0.0f, 5.0f);
		//		ImGui::Spacing();
		//		ImGui::SliderFloat("m_pitch", &g_Options.yaw2, 0.0f, 5.0f);
		//		ImGui::Spacing();
			//}
			ImGui::Checkbox("pixelsurf assistant", &g_Options.CHECKSURFLOLKEK); ImGui::SameLine();          ImGui::Hotkey("##ebgdfgdf", &g_Options.KEYCHECKSURFLOLKEK); ImGui::SameLine();          ImGui::Hotkey("##ebgd324fgdf", &g_Options.KEYCHECKSURFLOLKEKPoint);
			if (g_Options.CHECKSURFLOLKEK)
			{
				ImGui::Checkbox("show on screen", &g_Options.showonscreen);
				ImGui::Checkbox("show in binds##ast", &g_Options.showcslk);
				ImGui::Checkbox("show jump in chat", &g_Options.detectchat);
				ImGui::SliderFloat("unlock factor", &g_Options.lockfactor, 0.f, 1.f);
				ImGui::Text("pixelfinder"); ImGui::SameLine(); ImGui::Hotkey("##pololska", &g_Options.posloska);
				ImGui::Combo("hotkey##ast", &g_Options.type, "hold\0toggle\0");
			}
			ImGui::Checkbox("fast duck", &g_Options.fastduck);
			ImGui::Checkbox("checkpoints", &g_Options.checkpoint);
			ImGui::Text("checkpoint key"); ImGui::SameLine();
			ImGui::Hotkey("		##checkpoint_key", &g_Options.checkpoint_key);
			ImGui::Text("teleport key"); ImGui::SameLine();
			ImGui::Hotkey("		##teleport_key", &g_Options.teleport_key);
			ImGui::Text("next checkpoint Key"); ImGui::SameLine();
			ImGui::Hotkey("		##teleport_nextkey", &g_Options.teleport_nextkey);
			ImGui::Text("previous checkpoint key"); ImGui::SameLine();
			ImGui::Hotkey("		##teleport_prevkey", &g_Options.teleport_prevkey);
			ImGui::Text("undo checkpoint key"); ImGui::SameLine();
			ImGui::Hotkey("		##teleport_undokey", &g_Options.teleport_undokey);
			ImGui::PopStyleVar();
		}
		ImGui::EndChild();

		ImGui::BeginChild(("misc.indicators"), ImVec2{ }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted(("scene")); ImGui::PopFont();
			
				ImGui::EndMenuBar();
			}

			static const ctab misc_tabs[] =
			{
				ctab{ "indicators", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));

				ImGui::Checkbox("velocity", &g_Options.movement.indicators.velocity.enable);
				if (g_Options.movement.indicators.velocity.enable)
				{
					if (g_Options.movement.indicators.velocity.style == 0)
					{
						ImGui::Text("+velocity");
						ImGui::SameLine();
						ImGui::ColorEdit4("##velocity color 3", g_Options.movement.indicators.velocity.delta_color.positive, no_alpha);
						ImGui::Text("=velocity");
						ImGui::SameLine();
						ImGui::ColorEdit4("##velocity color 4", g_Options.movement.indicators.velocity.delta_color.neutral, no_alpha);
						ImGui::Text("-velocity");
						ImGui::SameLine();
						ImGui::ColorEdit4("##velocity color 5", g_Options.movement.indicators.velocity.delta_color.negative, no_alpha);
					}
					else if (g_Options.movement.indicators.velocity.style == 1)
					{
						ImGui::Text("-velocity");
						ImGui::SameLine();
						ImGui::ColorEdit4("##velocity color 1", g_Options.movement.indicators.velocity.interpolate_color.first, w_alpha);
						ImGui::Text("+velocity");
						ImGui::SameLine();
						ImGui::ColorEdit4("##velocity color 2", g_Options.movement.indicators.velocity.interpolate_color.second, w_alpha);
					}

					if (g_Options.movement.indicators.velocity.style == 1)
					{
						ImGui::SliderFloat("maximum value##1", &g_Options.movement.indicators.velocity.maximum_value, 255.0f, 400.0f, ("%.2f"));
					}
					else if (g_Options.movement.indicators.velocity.style == 2)
					{
						ImGui::SliderFloat("maximum value##2", &g_Options.movement.indicators.velocity.maximum_value, 255.0f, 1200.0f, ("%.2f"));
					}
					if (g_Options.movement.indicators.velocity.style == 2)
					{
						ImGui::SliderFloat("saturation##velocity", &g_Options.movement.indicators.velocity.hsb_color.saturation, 0.0f, 1.0f, ("%.2f"));
					}

					ImGui::Checkbox("take off##velocity", &g_Options.movement.indicators.velocity.takeoff);
					ImGui::Checkbox("disable takeoff on ground##velocity", &g_Options.movement.indicators.velocity.disable_takeoff_on_ground);
					ImGui::Text("change style color velocity");
					ImGui::Combo("##vsc", &g_Options.movement.indicators.velocity.style, "delta modulation\0interpolate modulation\0hsb modulation\0");
				}

				//ImGui::Checkbox("velocity graph", &g_Options.velocity_graph);
				if (g_Options.velocity_graph)
				{
					ImGui::SameLine();
					ImGuiEx::ColorEdit4("##velcol", &g_Options.vel_graph_clr);
					ImGui::Spacing();
					ImGui::SliderInt("graph y", &g_Options.graph_y, 1, 2000);
					ImGui::Spacing();
					ImGui::SliderInt("graph w", &g_Options.graph_w, 1, 2000);
				}

				ImGui::Checkbox("stamina", &g_Options.movement.indicators.stamina.enable);
				if (g_Options.movement.indicators.stamina.enable)
				{
					if (g_Options.movement.indicators.stamina.style == 0)
					{
						ImGui::Text("stamina");
						ImGui::SameLine();
						ImGui::ColorEdit4("##stamina color", g_Options.movement.indicators.stamina.color, no_alpha);
					}
					else if (g_Options.movement.indicators.stamina.style == 1)
					{
						ImGui::Text("-stamina");
						ImGui::SameLine();
						ImGui::ColorEdit4("##stamina color 1", g_Options.movement.indicators.stamina.interpolate_color.first, w_alpha);
						ImGui::Text("+stamina");
						ImGui::SameLine();
						ImGui::ColorEdit4("##stamina color 2", g_Options.movement.indicators.stamina.interpolate_color.second, w_alpha);
					}
					else if (g_Options.movement.indicators.stamina.style == 2)
					{
						ImGui::SliderFloat("maximum stamina color##stamina", &g_Options.movement.indicators.stamina.maximum_value, 30.0f, 80.0f, ("%.2f"));
						ImGui::SliderFloat("saturation##stamina", &g_Options.movement.indicators.stamina.hsb_color.saturation, 0.0f, 1.0f, ("%.2f"));
					}

					ImGui::Checkbox("take off##stamina", &g_Options.movement.indicators.stamina.takeoff);
					ImGui::Checkbox("disable takeoff on ground##stamina", &g_Options.movement.indicators.stamina.disable_takeoff_on_ground);
					ImGui::Text("change style color stamina");
					ImGui::Combo("##ssc", &g_Options.movement.indicators.stamina.style, "static modulation\0interpolate modulation\0hsb modulation\0");
				}
				//ImGui::Checkbox("stamina graph", &g_Options.stamina_graph);
				if (g_Options.stamina_graph)
				{
					ImGui::SameLine();
					ImGuiEx::ColorEdit4("##stamina graph col", &g_Options.stm_graph_clr);
					ImGui::Spacing();
					ImGui::SliderInt("graph y##33", &g_Options.stm_graph_y, 1, 2000);
					ImGui::Spacing();
					ImGui::SliderInt("graph w##123", &g_Options.stm_graph_w, 1, 2000);
				}
				ImGui::Checkbox("Indicators", &g_Options.misc_indicators);
				if (g_Options.misc_indicators)
				{
					ImGui::Spacing();
					ImGui::SliderInt("##indicators pos y", &g_Options.indicatorsposy, 1, 500);
				}
				ImGui::Checkbox("spectator list", &g_Options.spectator_list);
				if (g_Options.spectator_list)
				{
					ImGui::SameLine(); 			ImGuiEx::ColorEdit4a("##specclik", &g_Options.spec_color);
					ImGui::Spacing();
					ImGui::Combo("##21312wwwwsrre4", &g_Options.spec_type, "default\0experiment \0iwebz\0WINDOWS\0");
					if (g_Options.spec_type == 1)
					{
						ImGui::Spacing();
						ImGui::SliderInt("x##343434", &g_Options.specposx, 1, 1000);
						ImGui::Spacing();
						ImGui::SliderInt("y##123fd", &g_Options.specposy, 1, 1000);
						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SliderInt("Size##123fd", &g_Options.specsizex, 1, 1000);
						ImGui::Spacing();
					}
					if (g_Options.spec_type == 2)
					{
						ImGui::Spacing();
						ImGui::SliderInt("x##343434", &g_Options.specposx, 1, 1000);
						ImGui::Spacing();
						ImGui::SliderInt("y##123fd", &g_Options.specposy, 1, 1000);
						ImGui::Spacing();
						ImGui::Spacing();
					}
				}

				ImGui::PopStyleVar();
			}},
				ctab{ "binds", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));

				

				ImGui::PopStyleVar();

			}},
					ctab{ "graphs", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));

				ImGui::Checkbox("velocity", &g_Options.graphs.velocity.enable);
				if (g_Options.graphs.velocity.enable)
				{
					ImGui::SameLine();
					ImGui::ColorEdit4("##velocity graph", g_Options.graphs.velocity.color, no_alpha);
					ImGui::Checkbox("draw velocity", &g_Options.graphs.velocity.draw_velocity);
					ImGui::Checkbox("draw edgebug (wip)", &g_Options.graphs.velocity.draw_edgebug);
					ImGui::Checkbox("draw jumpbug", &g_Options.graphs.velocity.draw_jumpbug);
					ImGui::Checkbox("draw pixelsurf (wip)", &g_Options.graphs.velocity.draw_pixelsurf);
				}
				ImGui::Checkbox("stamina", &g_Options.graphs.stamina.enable);
				if (g_Options.graphs.stamina.enable)
				{
					ImGui::SameLine();
					ImGui::ColorEdit4("##stamina graph", g_Options.graphs.stamina.color, no_alpha);
				}
				if (g_Options.graphs.velocity.enable || g_Options.graphs.stamina.enable)
				{
					ImGui::Checkbox("fade on graphs", &g_Options.graphs.fade);
					ImGui::SliderFloat("size of graphs", &g_Options.graphs.size, 0.50f, 3.00f, ("%.2f"));
				}

				ImGui::PopStyleVar();

			}},
			};

			render_tab("misc_tab", misc_tabs, 3U, &indicators_tab, style.Colors[ImGuiCol_TabHovered], ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip);
		}
		ImGui::EndChild();
	}
	ImGui::NextColumn(); {
		ImGui::BeginChild(("misc.config"), ImVec2(0, 265), true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted(("configuration")); ImGui::PopFont();
				ImGui::EndMenuBar();
			}

			float group_w = ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().FramePadding.x * 2;

			static int config_index = -1;
			static char buffer[10];
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0));
			ImGui::PushItemWidth(-1);
			ImGui::Text("config");
			
			
			namespace fs = std::filesystem;
			static std::vector<std::string> configs;

			static auto load_configs = []() {
				std::vector<std::string> items = {};

				std::string path("C:\\lobotomy");
				if (!fs::is_directory(path))
					fs::create_directory(path);

				for (auto& p : fs::directory_iterator(path))
					items.push_back(p.path().string().substr(path.length() + 1));

				return items;
				};

			static auto is_configs_loaded = false;
			if (!is_configs_loaded) {
				is_configs_loaded = true;
				configs = load_configs();
			}

			static std::string current_config;

			static char config_name[32];
			{
				for (auto& config : configs) {
					if (ImGui::Selectable(config.c_str(), config == current_config)) {
						current_config = config;
					}
				}
			}
			ImGui::InputText(("config name"), buffer, sizeof(buffer));
			ImGui::PopItemWidth();
			if (ImGui::Button("create", ImVec2(-1, 17)))
			{
				current_config = std::string(buffer);
				notification::run("", "#_print_created", true, true, true);
				g_Options.SaveSettings(current_config + ".lobotomy");
				is_configs_loaded = false;
				memset(buffer, 0, 32);
			}
			if (ImGui::Button("save", ImVec2(-1, 17)))
			{
				ImGui::OpenPopup("config save popup");
			}
			if (ImGui::BeginPopup("config save popup"))
			{
				ImGui::Text("are you sure you want to save selected config? ");

				if (ImGui::Button("yes", ImVec2(-1, 17)))
				{
					g_Options.SaveSettings(current_config);

					// Сохранение конфигурации
					saveConfig("C:\\lobotomy\\LOOOOOOOL.dat");
					notification::run(" ", "#_print_saved", true, true, true);
				}

				if (ImGui::Button("no", ImVec2(-1, 17)))
				{
					notification::run(" ", "#_print_canceled_save", true, true, true);
				}

				ImGui::EndPopup();
			}
			if (ImGui::Button("load", ImVec2(-1, 17)))
			{
				PointsCheck.clear();
				loadConfig("C:\\lobotomy\\LOOOOOOOL.dat");
				g_Options.LoadSettings(current_config);
				notification::run(" ", "#_print_loaded", true, true, true);
			}
			if (ImGui::Button("delete", ImVec2(-1, 17)))
			{
				if (config_index >= 2)
				{
					ImGui::OpenPopup("config remove popup");
				}
			}
			if (ImGui::BeginPopup("config remove popup") && fs::remove("C:\\lobotomy\\" + current_config))
			{
				ImGui::Text("are you sure you want to remove selected config? ");

				if (ImGui::Button("yes", ImVec2(-1, 17)))
				{
					notification::run(" ", "#_print_removed", true, true, true);
					current_config.clear();

					is_configs_loaded = false;
				}

				if (ImGui::Button("no", ImVec2(-1, 17)))
				{
					notification::run(" ", "#_print_canceled_remove", true, true, true);
				}

				ImGui::EndPopup();
			}
			if (ImGui::Button("refresh", ImVec2(-1, 17)))
			{
				is_configs_loaded = false;
				notification::run(" ", "#_print_refreshed", true, true, true);
			}
			if (ImGui::Button("open directory", ImVec2(-1, 17)))
			{
				
			}
			if (ImGui::Button("connect to server", ImVec2(-1, 17))) {
				g_EngineClient->execute_cmd("connect 146.19.87.233:27015");
				
			}
			if (ImGui::Button("unlock convars", ImVec2(-1, 17)))
			{
				
			}

			ImGui::Separator();

			ImGui::Text("menu accent"); ImGui::SameLine(group_w - 10); ImGui::ColorEdit4("##main_theme", g_Options.fuckingaccent, no_alpha);
			//ImGui::Text("menu key"); ImGui::SameLine(group_w - 15); ImGui::Keybind("##menu key", &c::misc::menu_key);

			ImGui::PopStyleVar();
		}
		ImGui::EndChild();

		ImGui::BeginChild(("misc.main"), ImVec2{ }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted(("misc")); ImGui::PopFont();
				ImGui::EndMenuBar();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0));
			ImGui::Checkbox("kill streak sound", &g_Options.killss);
			ImGui::Checkbox("watermark", &g_Options.watermark);
			ImGui::Checkbox("track display", &g_Options.trackdispay);
			ImGui::PopStyleVar();
		}
		ImGui::EndChild();
	}
	ImGui::Columns(1);
}
void legitbot()
{
	ImGuiStyle& style = ImGui::GetStyle();
	static int definition_index = WEAPON_INVALID;

	auto localPlayer = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
	if (g_EngineClient->IsInGame() && localPlayer && localPlayer->IsAlive() && localPlayer->m_hActiveWeapon() && localPlayer->m_hActiveWeapon()->IsGun())
		definition_index = localPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex();
	else
		definition_index = WEAPON_INVALID;
	if (definition_index == WEAPON_INVALID)definition_index = WEAPON_DEAGLE;

	ImGui::Columns(2, nullptr, false); {
		ImGui::BeginChild(("legitbot.main"), ImVec2{ }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted("legit"); ImGui::PopFont();
				ImGui::EndMenuBar();
			}

			static const ctab legit_tabs[] =
			{
				ctab{ "trigger", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));

				

				ImGui::PopStyleVar();
			}},
				ctab{ "backtrack", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));

				ImGui::Checkbox("def bt", &g_Options.backtrack);
				ImGui::Checkbox("extended bt", &g_Options.extend_bt);
				ImGui::Checkbox("aim at backtrack", &g_Options.aim_at_bt);

				ImGui::PopStyleVar();
			}}
			};

			render_tab("legit_tabs", legit_tabs, 2U, &aim_tab, style.Colors[ImGuiCol_TabHovered]);

			ImGui::EndChild();
		}
	}
	ImGui::NextColumn(); {
		ImGui::BeginChild(("legitbot.settings"), ImVec2{ }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted("aimbot"); ImGui::PopFont();
				ImGui::EndMenuBar();
			}
			static int type = 0;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0));
			auto settings = &g_Options.weapons[type].legit;
			ImGui::Combo("Weapon", &type, "pistol\0heavy pistol\0shotgun\0heavy\0smg\0rifle\0sniper\0auto sniper\0");
			ImGui::Separator();

			ImGui::Checkbox("enable", &settings->enabled);
			ImGui::Checkbox("silent", &settings->smoke_check);
			;
			ImGui::Checkbox("zeus bug", &g_Options.zeusbug);
			if (ImGui::BeginCombo("hitbox", "select", ImGuiComboFlags_NoArrowButton))
			{
				ImGui::Selectable("head", &settings->hitboxes.head, ImGuiSelectableFlags_DontClosePopups);
				ImGui::Selectable("stomach", &settings->hitboxes.stomach, ImGuiSelectableFlags_DontClosePopups);
				ImGui::Selectable("chest", &settings->hitboxes.chest, ImGuiSelectableFlags_DontClosePopups);
				ImGui::Selectable("hands", &settings->hitboxes.hands, ImGuiSelectableFlags_DontClosePopups);
				ImGui::Selectable("legs", &settings->hitboxes.legs, ImGuiSelectableFlags_DontClosePopups);

				ImGui::EndCombo();
			}
			ImGui::Separator();

			
			ImGui::Spacing();
	
			ImGui::SliderFloat("fov", &settings->fov, 0, 360, "%.2f");
		
			ImGui::Separator();
			
			ImGui::Spacing();
		
			ImGui::SliderFloat("smooth", &settings->smooth, 1, 20, "%.2f");
			
			ImGui::Separator();
			//	ImGui::Separator("rcs");

			ImGui::Text("rcs");
			ImGui::Spacing();
			ImGui::Checkbox("enable##rcsss", &settings->rcs.enabled);
			
			ImGui::Spacing();
			ImGui::SliderInt("##rcsx", &settings->rcs.x, 0, 100, "%i");
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text("mindmg");
			ImGui::Spacing();
			ImGui::SliderInt("##minDamage", &settings->autowall.min_damage, 1, 100, "%i");

			ImGui::Spacing();
			ImGui::PopStyleVar();
			ImGui::EndChild();
		}
	}

	ImGui::Columns(1);
}
static const char* materials[10] = { "textured", "flat", "metalic", "plastic", "elixir", "glow", "wireframe", "crystal", "space", "glass" };

void visuals() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::Columns(2, nullptr, false); {
		ImGui::BeginChild(("visuals.esp"), ImVec2{ 0, 265 }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted("overlay");
				ImGui::PopFont();
				ImGui::EndMenuBar();
			}

			static const ctab overlay_tabs[] =
			{
				ctab{ "esp", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));
				
				ImGui::Checkbox("enable esp", &g_Options.players.enable);
				if (g_Options.players.enable)
				{
					
					ImGui::Checkbox("fade animations", &g_Options.players.fade.enable);
					if (g_Options.players.fade.enable)
					{
						ImGui::SliderFloat("fade time", &g_Options.players.fade.time, 0.0f, 1.0f, ("%.2f"));
					}
					ImGui::Checkbox("engine radar", &g_Options.players.engine_radar);
					ImGui::Checkbox("custom color##all", &g_Options.players.colors.custom);
					if (!g_Options.players.colors.custom)
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("##visible color", g_Options.players.colors.visible, w_alpha);
						ImGui::SameLine();
						ImGui::ColorEdit4("##invisible color", g_Options.players.colors.invisible, w_alpha);
					}
					ImGui::Checkbox("box", &g_Options.players.box.enable);
					if (g_Options.players.box.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##player box color", g_Options.players.colors.box, w_alpha);
							ImGui::SameLine();
							ImGui::ColorEdit4("##player outline box color", g_Options.players.colors.box_outline, w_alpha);
						}
						ImGui::Text("box type");
						ImGui::Combo("##boxtype", &g_Options.players.box.type, "filled\0corner");
						ImGui::Text("outline");
						
						if (ImGui::BeginCombo("##outlinetype", "...", ImGuiComboFlags_NoArrowButton))
						{
							ImGui::Selectable("outter", &g_Options.players.box.outline[0]);
							ImGui::Selectable("inner", &g_Options.players.box.outline[1]);
							ImGui::EndCombo();
						}
						if (g_Options.players.box.type == 1)
						{
							ImGui::SliderFloat("corner lenght", &g_Options.players.box.lenght, 1.55f, 20.0f, ("%.2f"));
						}
					}
					ImGui::Checkbox("name", &g_Options.players.name.enable);
					if (g_Options.players.name.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##name color", g_Options.players.colors.name, w_alpha);
						}
						ImGui::Checkbox("bot tag", &g_Options.players.name.bot_check);
					}
					ImGui::Checkbox("health bar", &g_Options.players.health_bar.enable);
					if (g_Options.players.health_bar.enable)
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("##healthbar outline color", g_Options.players.colors.health_bar_outline, w_alpha);
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##healthbar color", g_Options.players.colors.health_bar, w_alpha);
						}
						ImGui::Checkbox("gradient color", &g_Options.players.health_bar.gradient);
						if (g_Options.players.health_bar.gradient)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##healthbar upper color", g_Options.players.colors.health_bar_upper, w_alpha);
							ImGui::SameLine();
							ImGui::ColorEdit4("##healthbar lower color", g_Options.players.colors.health_bar_lower, w_alpha);
						}

						ImGui::Checkbox("color based on health##bar", &g_Options.players.health_bar.baseonhealth);
						ImGui::Checkbox("fill outline background##healthbar", &g_Options.players.health_bar.background);
						ImGui::Checkbox("text on bar", &g_Options.players.health_bar.text);
						ImGui::SliderInt("bar width##healthbar", &g_Options.players.health_bar.size, 1, 10, ("%.d"));
					}
					ImGui::Checkbox("skeleton", &g_Options.players.skeleton.enable);
					if (g_Options.players.skeleton.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##skeleton color", g_Options.players.colors.skeleton, w_alpha);
						}

						ImGui::SliderFloat("thickness##skeleton", &g_Options.players.skeleton.thickness, 0.50f, 2.0f, ("%.2f"));
					}
					ImGui::Checkbox("backtrack skeleton", &g_Options.players.backtrack_skeleton.enable);
					if (g_Options.players.backtrack_skeleton.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##backtrack skeleton color", g_Options.players.colors.backtrack_skeleton, w_alpha);
						}

						ImGui::SliderFloat("thickness##backtrack", &g_Options.players.backtrack_skeleton.thickness, 0.50f, 2.0f, ("%.2f"));
					}
					ImGui::Checkbox("backtrack dot", &g_Options.players.backtrack_dot.enable);
					if (g_Options.players.backtrack_dot.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##backtrack dot color", g_Options.players.colors.backtrack_dot, w_alpha);
						}
					}
					ImGui::Checkbox("view angle", &g_Options.players.view_angle.enable);
					if (g_Options.players.view_angle.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##view angle color", g_Options.players.colors.view_angle, w_alpha);
						}
						ImGui::SliderFloat("thickness##viewangle", &g_Options.players.view_angle.thickness, 0.50f, 2.0f, ("%.2f"));
						ImGui::SliderFloat("distance##viewangle", &g_Options.players.view_angle.distance, 0.0f, 150.0f, ("%.2f"));
					}
					ImGui::Checkbox("line to box", &g_Options.players.snapline.enable);
					if (g_Options.players.snapline.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##snap line color", g_Options.players.colors.snapline, w_alpha);
						}
						ImGui::Text("line type");
					
						if (ImGui::BeginCombo("##linetype", "...", ImGuiComboFlags_NoArrowButton))
						{
							ImGui::Selectable("top", &g_Options.players.snapline.type[0]);
							ImGui::Selectable("center", &g_Options.players.snapline.type[1]);
							ImGui::Selectable("bottom", &g_Options.players.snapline.type[2]);
							ImGui::EndCombo();
						}
						ImGui::SliderFloat("thickness##linetobox", &g_Options.players.snapline.thickness, 0.50f, 2.0f, ("%.2f"));
					}
					ImGui::Checkbox("flags", &g_Options.players.flags.enable);
					if (g_Options.players.flags.enable)
					{
						ImGui::Text("flags type");
					
						if (ImGui::BeginCombo("##flagstype", "...", ImGuiComboFlags_NoArrowButton))
						{
							ImGui::Selectable("armour", &g_Options.players.flags.flags[0]);
							ImGui::Selectable("money", &g_Options.players.flags.flags[1]);
							ImGui::Selectable("flashed", &g_Options.players.flags.flags[2]);
							ImGui::Selectable("scoped", &g_Options.players.flags.flags[3]);
							ImGui::Selectable("reloading", &g_Options.players.flags.flags[4]);
							ImGui::Selectable("defusing", &g_Options.players.flags.flags[5]);
							ImGui::Selectable("rescuing", &g_Options.players.flags.flags[6]);

							ImGui::EndCombo();
						}
					}
					ImGui::Checkbox("ammo bar", &g_Options.players.ammo_bar.enable);
					if (g_Options.players.ammo_bar.enable)
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("##ammobar outline color", g_Options.players.colors.ammo_bar_outline, w_alpha);
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##ammobar color", g_Options.players.colors.ammo_bar, w_alpha);
						}
						ImGui::Checkbox("fill outline background##ammobar", &g_Options.players.ammo_bar.background);
						ImGui::SliderInt("bar width##ammobar", &g_Options.players.ammo_bar.size, 1, 10, ("%.d"));
					}
					ImGui::Checkbox("health text", &g_Options.players.health_text.enable);
					if (g_Options.players.health_text.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##health text color", g_Options.players.colors.health_text, w_alpha);
						}
						ImGui::Checkbox("hide suffix", &g_Options.players.health_text.hide_suffix);
						ImGui::Checkbox("hide at max", &g_Options.players.health_text.hide_at_max);
						ImGui::Checkbox("color based on health##text", &g_Options.players.health_text.baseonhealth);
					}
					ImGui::Checkbox("weapon icon", &g_Options.players.weapon_icon.enable);
					if (g_Options.players.weapon_icon.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##weapon icon color", g_Options.players.colors.weapon_icon, w_alpha);
						}
					}
					ImGui::Checkbox("weapon text", &g_Options.players.weapon_text.enable);
					if (g_Options.players.weapon_text.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##weapon text color", g_Options.players.colors.weapon_text, w_alpha);
						}
					}
					ImGui::Checkbox("ammo text", &g_Options.players.ammo_text.enable);
					if (g_Options.players.ammo_text.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##ammo icon color", g_Options.players.colors.ammo_text, w_alpha);
						}
					}
					ImGui::Checkbox("distance", &g_Options.players.distance.enable);
					if (g_Options.players.distance.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##distancecolor", g_Options.players.colors.distance, w_alpha);
						}
					}
					ImGui::Checkbox("out of fov", &g_Options.players.out_of_view.enable);
					if (g_Options.players.out_of_view.enable)
					{
						if (g_Options.players.colors.custom)
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("##oofcolor", g_Options.players.colors.out_of_view, w_alpha);
						}
						ImGui::SliderFloat("size", &g_Options.players.out_of_view.size, 0.0f, 50.0f, ("%.2f"));
						ImGui::SliderFloat("distance", &g_Options.players.out_of_view.distance, 0.0f, 600.0f, ("%.2f"));
					}
					ImGui::Checkbox("emitted sound (wip)", &g_Options.players.emitted_sound.enable);
					if (g_Options.players.emitted_sound.enable)
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("##soundscolor", g_Options.players.colors.sounds, w_alpha);
					}
					ImGui::Checkbox("death information history", &g_Options.players.death_history.enable);
				}
			

				ImGui::PopStyleVar();
			}},
				ctab{ "world", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));
				
				ImGui::PopStyleVar();
			}}
			};

			render_tab(("overlay_tabs"), overlay_tabs, 2U, &esp_tab, style.Colors[ImGuiCol_TabHovered]);

			ImGui::EndChild();
		}

		ImGui::BeginChild(("visuals.glow"), ImVec2{ }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted("glow");
				ImGui::PopFont();
				ImGui::EndMenuBar();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0));
			ImGui::Checkbox("enable##glowq333", &g_Options.glow_enabled);
			ImGui::SameLine();
			ImGuiEx::ColorEdit4a("##Enemy   ", &g_Options.color_glow_enemy);
			ImGui::Checkbox("occluded   ", &g_Options.glow_enemiesOC);
			ImGui::SameLine();
			ImGuiEx::ColorEdit4a("##color_glow_enemiesOC   ", &g_Options.color_glow_enemyOC);
			const char* glow_enemies_type[] = {
				"glow outer",
				"glow inner"
			};
			if (g_Options.glow_enabled)
			{
				ImGui::Spacing();
				if (ImGui::BeginCombo("##glow_enemies_type", glow_enemies_type[g_Options.glow_enemies_type], ImGuiComboFlags_NoArrowButton))
				{
					for (int i = 0; i < IM_ARRAYSIZE(glow_enemies_type); i++)
					{
						if (ImGui::Selectable(glow_enemies_type[i], i == g_Options.glow_enemies_type))
							g_Options.glow_enemies_type = i;
					}

					ImGui::EndCombo();
				}
			}
			
			ImGui::PopStyleVar();
			ImGui::EndChild();
		}
	}
	ImGui::NextColumn(); {
		ImGui::BeginChild("visuals.chams", ImVec2{ 0, 265 }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted("chams");
				ImGui::PopFont();
				ImGui::EndMenuBar();
			}

			static const ctab cham_tabs[] =
			{
				ctab{ "enemy", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));
				ImGui::Checkbox("visible model", &g_Options.chams.visible.enable);
				ImGui::SameLine();
				ImGui::ColorEdit4("##visible model color", g_Options.chams.visible.color, w_alpha);
				if (g_Options.chams.visible.enable)
				{
					ImGui::Checkbox("hide original model##visiblemodel", &g_Options.chams.visible.hide_original_model);
					ImGui::Text("base layer");
					ImGui::Combo("##chamsstylevisiblemodel", &g_Options.chams.visible.type, materials, IM_ARRAYSIZE(materials));
				}
				ImGui::Checkbox("invisible model", &g_Options.chams.invisible.enable);
				ImGui::SameLine();
				ImGui::ColorEdit4("##invisible model color", g_Options.chams.invisible.color, w_alpha);
				if (g_Options.chams.invisible.enable)
				{
					ImGui::Checkbox("hide original model##invisiblemodel", &g_Options.chams.invisible.hide_original_model);
					ImGui::Text("base layer");
					ImGui::Combo("##chamsstyleinvisiblemodel", &g_Options.chams.invisible.type, materials, IM_ARRAYSIZE(materials));
				}
				ImGui::Checkbox("visible attachment model", &g_Options.chams.visible_attachment.enable);
				ImGui::SameLine();
				ImGui::ColorEdit4("##visible attachment model color", g_Options.chams.visible_attachment.color, w_alpha);
				if (g_Options.chams.visible_attachment.enable)
				{
					ImGui::Checkbox("hide original model##visibleattachmentmodel", &g_Options.chams.visible_attachment.hide_original_model);
					ImGui::Text("base layer");
					ImGui::Combo("##chamsstylevisibleattachmentmodel", &g_Options.chams.visible_attachment.type, materials, IM_ARRAYSIZE(materials));
				}
				ImGui::Checkbox("invisible attachment model", &g_Options.chams.invisible_attachment.enable);
				ImGui::SameLine();
				ImGui::ColorEdit4("##invisible attachment model color", g_Options.chams.invisible_attachment.color, w_alpha);
				if (g_Options.chams.invisible_attachment.enable)
				{
					ImGui::Checkbox("hide original model##invisibleattachmentmodel", &g_Options.chams.invisible_attachment.hide_original_model);
					ImGui::Text("base layer");
					ImGui::Combo("##chamsstyleinvisibleattachmentmodel", &g_Options.chams.invisible_attachment.type, materials, IM_ARRAYSIZE(materials));
				}
				ImGui::Checkbox("backtrack model", &g_Options.chams.backtrack.enable);
				ImGui::SameLine();
				ImGui::ColorEdit4("##backtrack model color 1", g_Options.chams.backtrack.color_1, w_alpha);
				if (g_Options.chams.backtrack.enable)
				{
					ImGui::Checkbox("draw all ticks", &g_Options.chams.backtrack.draw_all_ticks);
					if (g_Options.chams.backtrack.draw_all_ticks)
					{
						ImGui::Checkbox("gradient fade", &g_Options.chams.backtrack.gradient);
						ImGui::SameLine();
						ImGui::ColorEdit4("##backtrack model color 2", g_Options.chams.backtrack.color_2, w_alpha);
					}
					ImGui::Checkbox("draw when invisible", &g_Options.chams.backtrack.invisible);
					ImGui::Checkbox("hide original model##backtrack", &g_Options.chams.backtrack.hide_original_model);
					ImGui::Text("base layer");
					ImGui::Combo("##chamsstylebacktrackmodel", &g_Options.chams.backtrack.type, materials, IM_ARRAYSIZE(materials));

				}
				ImGui::PopStyleVar();
			}
				
			},

				ctab{ "local", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));
				ImGui::Checkbox("weapon", &g_Options.chams.weapon.enable);
				ImGui::SameLine();
				ImGui::ColorEdit4("##weapon model color##weapon", g_Options.chams.weapon.color, w_alpha);
				if (g_Options.chams.weapon.enable)
				{
					ImGui::Checkbox("hide original model", &g_Options.chams.weapon.hide_original_model);
					ImGui::Text("base layer");
					ImGui::Combo("##chamsstylesweaponmodel", &g_Options.chams.weapon.type, materials, IM_ARRAYSIZE(materials));
				}
				ImGui::Checkbox("hands", &g_Options.chams.hands.enable);
				ImGui::SameLine();
				ImGui::ColorEdit4("##hands model color", g_Options.chams.hands.color, w_alpha);
				if (g_Options.chams.hands.enable)
				{
					ImGui::Checkbox("hide original model##weapon", &g_Options.chams.hands.hide_original_model);
					ImGui::Text("base layer");
					ImGui::Combo("##chamsstylehandsmodel", &g_Options.chams.hands.type, materials, IM_ARRAYSIZE(materials));
				}
				ImGui::Checkbox("sleeves", &g_Options.chams.sleeves.enable);
				ImGui::SameLine();
				ImGui::ColorEdit4("##sleeves model color", g_Options.chams.sleeves.color, w_alpha);
				if (g_Options.chams.sleeves.enable)
				{
					ImGui::Checkbox("hide original model##sleeves", &g_Options.chams.sleeves.hide_original_model);
					ImGui::Text("base layer");
					ImGui::Combo("##chamsstylesleevesmodel", &g_Options.chams.sleeves.type, materials, IM_ARRAYSIZE(materials));
				}
				
				ImGui::PopStyleVar();
			}},

				ctab{ "team", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));

				ImGui::PopStyleVar();
			}}
			};

			render_tab("chams_tabs", cham_tabs, 3U, &chams_tab, style.Colors[ImGuiCol_TabHovered]);

			ImGui::EndChild();
		}

		ImGui::BeginChild(("visuals.other"), ImVec2{ }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted("other");
				ImGui::PopFont();
				ImGui::EndMenuBar();
			}

			static const ctab visu_tabs[] =
			{
				ctab{ "scene", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));
				ImGui::Checkbox("hit info", &g_Options.hitinfo);
				ImGui::Checkbox("hit sound", &g_Options.hit_sound);
				ImGui::Checkbox("hit marker", &g_Options.hitmarker);
				//ImGui::Checkbox("auto accept", &g_Options.autoaccept);
				ImGui::Checkbox("sniper crosshair", &g_Options.sniper_xhair);
				ImGui::Checkbox("hit bones info", &g_Options.hitbones); ImGui::SameLine(); ImGuiEx::ColorEdit4a("##hit bones col", &g_Options.hit_bones_col);
				ImGui::Checkbox("unlock hidden cvars", &g_Options.unlockcvars);
				ImGui::Checkbox("disable panorama blur", &g_Options.panoramablur);
				ImGui::Checkbox("disable post processing", &g_Options.postproccesing);
				ImGui::Checkbox("no flash", &g_Options.no_flash);
				ImGui::Checkbox("no smoke", &g_Options.no_smoke);
				ImGui::Checkbox("no view bob", &g_Options.noviewbob);
				ImGui::Checkbox("clan tag", &g_Options.clantag);
				ImGui::Checkbox("fog Modulation", &g_Options.esp_world_fog); ImGui::SameLine(125); ImGuiEx::ColorEdit4("##fogcolor", &g_Options.fog_color);
				if (g_Options.esp_world_fog) {
					ImGui::NewLine();
					ImGui::SliderFloat("fog start", &g_Options.esp_world_fog_start, 0, 3000, "%.0f");
					ImGui::SliderFloat("fog end", &g_Options.esp_world_fog_end, 0, 5000, "%.0f");
					ImGui::SliderFloat("fog destiny", &g_Options.esp_world_fog_destiny, 0, 1);
				}
				if (ImGui::Button("old shaders"))
				{
					Visuals::Get().TryingFunc();
				}
				

				ImGui::PopStyleVar();
			}},

				ctab{ "world", []()
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));

				ImGui::Checkbox("Sunset", &g_Options.sunset);
				if (g_Options.sunset) {
					ImGui::NewLine();
					ImGui::SliderInt("sunset x", &g_Options.sunset_x, 0, 200);
					ImGui::SliderInt("sunset y", &g_Options.sunset_y, 0, 200);
				}
				ImGui::Checkbox("motion blur", &g_Options.motionblur.enable);
				if (g_Options.motionblur.enable) {
					ImGui::Text("video adapter type");
					ImGui::Combo("##vat", &g_Options.motionblur.video_adapter, "AMD\0NVIDIA\0INTEL\0");
					ImGui::Checkbox("forward enabled", &g_Options.motionblur.forward_move_blur);
					ImGui::SliderFloat("strength", &g_Options.motionblur.strength, 0.0f, 10.0f, " %.2f");
					ImGui::SliderFloat("falling intensity", &g_Options.motionblur.falling_intensity, 0.0f, 20.0f, " %.2f");
					ImGui::SliderFloat("rotation intensity", &g_Options.motionblur.rotate_intensity, 0.0f, 20.0f, " %.2f");
					ImGui::SliderFloat("falling min", &g_Options.motionblur.falling_minimum, 0.0f, 50.0f, " %.2f");
					ImGui::SliderFloat("falling max", &g_Options.motionblur.falling_maximum, 0.0f, 50.0f, " %.2f");
				}


				ImGui::PopStyleVar();
			}}
			};

			render_tab(("visual_tabs"), visu_tabs, 2U, &visu_tab, style.Colors[ImGuiCol_TabHovered]);

			ImGui::EndChild();
		}
	}
	ImGui::Columns(1);
}
void skins1() {
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::Columns(2, nullptr, false); {
		ImGui::BeginChild(("skins.item"), ImVec2{ 0, 190 }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted(("item")); ImGui::PopFont();
				ImGui::EndMenuBar();
			}
			static auto definition_vector_index = 0;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0));
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Checkbox("SkinChanger", &g_Options.SkiNChanger);
			if (g_Options.SkiNChanger)
			{
				for (size_t w = 0; w < k_weapon_names.size(); w++)
				{
					switch (w)
					{
					case 0:
						ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "knife");
						break;
					case 2:
						ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "glove");
						break;
					case 4:
						ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "pistols");
						break;
					case 14:
						ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "semi-rifle");
						break;
					case 21:
						ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "rifle");
						break;
					case 28:
						ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "sniper-rifle");
						break;
					case 32:
						ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "machingun");
						break;
					case 34:
						ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "shotgun");
						break;
					}

					if (ImGui::Selectable(k_weapon_names[w].name, definition_vector_index == w))
					{
						definition_vector_index = w;
					}
				}
			}
			//ImGui::ListBoxFooter();

			ImGui::PopStyleVar();
		}
		ImGui::EndChild();

		ImGui::BeginChild(("skins.models"), ImVec2{ }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted(("other")); ImGui::PopFont();
				ImGui::EndMenuBar();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));

			ImGui::Text("   ct player model");
			ImGui::PushItemWidth(160.f);
			ImGui::Combo("##TPlayerModel", &g_Options.playerModelCT, "default\0Special Agent Ava\0Operator\0Markus Delrow\0Michael Syfers\0B Squadron Officer\0Seal Team 6 Soldier\0Buckshot\0Commander Ricksaw\0Third Commando\0'Two Times' McCoy\0Dragomir\0Rezan The Ready\0The Doctor Romanov\0Maximus\0Blackwolf\0The Elite Muhlik\0Ground Rebel\0Osiris\0hahmat\0Enforcer\0Slingshot\0Soldier\0Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman\0");
			ImGui::Text("   t player model");
			ImGui::PushItemWidth(160.f);
			ImGui::Combo("##CTPlayerModel", &g_Options.playerModelT, "default\0Special Agent Ava\0Operator\0Markus Delrow\0Michael Syfers\0B Squadron Officer\0Seal Team 6 Soldier\0Buckshot\0Commander Ricksaw\0Third Commando\0'Two Times' McCoy\0Dragomir\0Rezan The Ready\0The Doctor Romanov\0Maximus\0Blackwolf\0The Elite Muhlik\0Ground Rebel\0Osiris\0hahmat\0Enforcer\0Slingshot\0Soldier\0Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman\0");


			ImGui::PopStyleVar();
		}
		ImGui::EndChild();
	}
	ImGui::NextColumn(); {
		ImGui::BeginChild(("skins.skins"), ImVec2{ }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted(("skins")); ImGui::PopFont();
				ImGui::EndMenuBar();
			}
			static std::string selected_weapon_name = "";
			static std::string selected_skin_name = "";
			static auto definition_vector_index = 0;
			auto& entries = g_Options.changers.skin.m_items;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0));

			ImGui::Spacing();
			ImGui::Spacing();
			auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
			auto& satatt = g_Options.changers.skin.statrack_items[k_weapon_names[definition_vector_index].definition_index];
			selected_entry.definition_index = k_weapon_names[definition_vector_index].definition_index;
			selected_entry.definition_vector_index = definition_vector_index;
			if (g_Options.SkiNChanger)
			{
				if (selected_entry.definition_index == WEAPON_KNIFE || selected_entry.definition_index == WEAPON_KNIFE_T)
				{
					ImGui::PushItemWidth(160.f);

					ImGui::Combo("", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = k_knife_names.at(idx).name;
							return true;
						}, nullptr, k_knife_names.size(), 10);
					selected_entry.definition_override_index = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

					ImGui::PopItemWidth();
				}
				else if (selected_entry.definition_index == GLOVE_T_SIDE || selected_entry.definition_index == GLOVE_CT_SIDE)
				{
					ImGui::PushItemWidth(160.f);

					ImGui::Combo("", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = k_glove_names.at(idx).name;
							return true;
						}, nullptr, k_glove_names.size(), 10);
					selected_entry.definition_override_index = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;

					ImGui::PopItemWidth();
				}
				else {
					static auto unused_value = 0;
					selected_entry.definition_override_vector_index = 0;
				}

				if (skins_parsed)
				{
					static char filter_name[32];
					std::string filter = filter_name;

					bool is_glove = selected_entry.definition_index == GLOVE_T_SIDE ||
						selected_entry.definition_index == GLOVE_CT_SIDE;

					bool is_knife = selected_entry.definition_index == WEAPON_KNIFE ||
						selected_entry.definition_index == WEAPON_KNIFE_T;

					int cur_weapidx = 0;
					if (!is_glove && !is_knife)
					{
						cur_weapidx = k_weapon_names[definition_vector_index].definition_index;
						//selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
					}
					else
					{
						if (selected_entry.definition_index == GLOVE_T_SIDE ||
							selected_entry.definition_index == GLOVE_CT_SIDE)
						{
							cur_weapidx = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
						}
						if (selected_entry.definition_index == WEAPON_KNIFE ||
							selected_entry.definition_index == WEAPON_KNIFE_T)
						{
							cur_weapidx = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

						}
					}

					/*	ImGui::InputText("name filter [?]", filter_name, sizeof(filter_name));
						if (ImGui::ItemsToolTipBegin("##skinfilter"))
						{
							ImGui::Checkbox("show skins for selected weapon", &g_Options.changers.skin.show_cur);
							ImGui::ItemsToolTipEnd();
						}*/

					auto weaponName = weaponnames(cur_weapidx);
					/*ImGui::Spacing();

					ImGui::Spacing();
					ImGui::SameLine();*/
					//ImGui::ListBoxHeader("##sdsdadsdadas", ImVec2(155, 245));
					{
						if (selected_entry.definition_index != GLOVE_T_SIDE && selected_entry.definition_index != GLOVE_CT_SIDE)
						{
							if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
							{
								selected_entry.paint_kit_vector_index = -1;
								selected_entry.paint_kit_index = -1;
								selected_skin_name = "";
							}

							int lastID = ImGui::GetItemID();
							for (size_t w = 0; w < k_skins.size(); w++)
							{
								for (auto names : k_skins[w].weaponName)
								{
									std::string name = k_skins[w].name;

									if (g_Options.changers.skin.show_cur)
									{
										if (names != weaponName)
											continue;
									}

									if (name.find(filter) != name.npos)
									{
										ImGui::PushID(lastID++);

										ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(is_knife && g_Options.changers.skin.show_cur ? 6 : k_skins[w].rarity));
										{
											if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
											{
												selected_entry.paint_kit_vector_index = w;
												selected_entry.paint_kit_index = k_skins[selected_entry.paint_kit_vector_index].id;
												selected_skin_name = k_skins[w].name_short;
											}
										}
										ImGui::PopStyleColor();

										ImGui::PopID();
									}
								}
							}
						}
						else
						{
							int lastID = ImGui::GetItemID();

							if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
							{
								selected_entry.paint_kit_vector_index = -1;
								selected_entry.paint_kit_index = -1;
								selected_skin_name = "";
							}

							for (size_t w = 0; w < k_gloves.size(); w++)
							{
								for (auto names : k_gloves[w].weaponName)
								{
									std::string name = k_gloves[w].name;
									//name += " | ";
									//name += names;

									if (g_Options.changers.skin.show_cur)
									{
										if (names != weaponName)
											continue;
									}

									if (name.find(filter) != name.npos)
									{
										ImGui::PushID(lastID++);

										ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(6));
										{
											if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
											{
												selected_entry.paint_kit_vector_index = w;
												selected_entry.paint_kit_index = k_gloves[selected_entry.paint_kit_vector_index].id;
												selected_skin_name = k_gloves[selected_entry.paint_kit_vector_index].name_short;
											}
										}
										ImGui::PopStyleColor();

										ImGui::PopID();
									}
								}
							}
						}
					}
					//	ImGui::ListBoxFooter();
				}
				else
				{
					//	ImGui::Text("##wait...");
				}
				//ImGui::Checkbox("skin preview", &g_Options.changers.skin.skin_preview);
				//ImGui::Checkbox("stattrak##2", &selected_entry.stat_trak);


				ImGui::InputInt("##seed", &selected_entry.seed, 12);
				ImGui::Spacing();
				//ImGui::InputInt("stattrak", &satatt.statrack_new.counter);
				ImGui::SliderFloat("##wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);


				if (ImGui::Button("force update"))
				{
					g_ClientState->ForceFullUpdate();
					notification::run(" ", "#_print_updated_hud", true, true, true);
				}
			}
			ImGui::PopStyleVar();
		}
		ImGui::EndChild();
	}
	ImGui::Columns(1);
}
std::vector<std::string> get_installed_fonts() {
	std::vector<std::string> fonts;
	fonts.push_back("default");
	for (const auto& entry : std::filesystem::directory_iterator("C:\\Windows\\Fonts")) {
		std::string fontName = entry.path().filename().string();
		if (fontName.find(".ttf") != std::string::npos) {
			fonts.push_back(fontName);
		}
	}
	return fonts;
}
inline static std::vector<std::string> fontsList = get_installed_fonts();
void load_font_index() {
	fonts::selected_font_index_main_indi = g_Options.fonts.indi_font;
	if (fonts::selected_font_index_main_indi >= 0) {
		if (fontsList[fonts::selected_font_index_main_indi] == "default") {
			fonts::font_directory_indicator = "C:/windows/fonts/tahomabd.ttf";
		}
		else {
			fonts::font_directory_indicator = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_main_indi];
		}
		g_Options.fonts.indi_font = fonts::selected_font_index_main_indi;
	}

	fonts::selected_font_index_sub_indi = g_Options.fonts.sub_indi_font;
	if (fonts::selected_font_index_sub_indi >= 0) {
		if (fontsList[fonts::selected_font_index_sub_indi] == "default") {
			fonts::font_directory_sub_indicator = "C:/windows/fonts/tahomabd.ttf";
		}
		else {
			fonts::font_directory_sub_indicator = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_sub_indi];
		}
		g_Options.fonts.sub_indi_font = fonts::selected_font_index_sub_indi;
	}

	fonts::selected_font_index_main_scene = g_Options.fonts.scene_font;
	if (fonts::selected_font_index_main_scene >= 0) {
		if (fontsList[fonts::selected_font_index_main_scene] == "default") {
			fonts::font_directory_scene = "C:/windows/fonts/tahomabd.ttf";
		}
		else {
			fonts::font_directory_scene = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_main_scene];
		}
		g_Options.fonts.scene_font = fonts::selected_font_index_main_scene;
	}

	fonts::selected_font_index_sub_scene = g_Options.fonts.scene_sub_font;
	if (fonts::selected_font_index_sub_scene >= 0) {
		if (fontsList[fonts::selected_font_index_sub_scene] == "default") {
			fonts::font_directory_sub_scene = "C:/windows/fonts/tahomabd.ttf";
		}
		else {
			fonts::font_directory_sub_scene = "C:/windows/fonts/" +fontsList[fonts::selected_font_index_sub_scene];
		}
		g_Options.fonts.scene_sub_font = fonts::selected_font_index_sub_scene;
	}

	fonts::selected_font_index_main_esp = g_Options.fonts.esp_font;
	if (fonts::selected_font_index_main_esp >= 0) {
		if (fontsList[fonts::selected_font_index_main_esp] == "default") {
			fonts::font_directory_esp = "C:/windows/fonts/tahomabd.ttf";
		}
		else {
			fonts::font_directory_esp = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_main_esp];
		}
		g_Options.fonts.esp_font = fonts::selected_font_index_main_esp;
	}

	fonts::selected_font_index_sub_esp = g_Options.fonts.esp_sub_font;
	if (fonts::selected_font_index_sub_esp >= 0) {
		if (fontsList[fonts::selected_font_index_sub_esp] == "default") {
			fonts::font_directory_sub_esp = "C:/windows/fonts/tahoma.ttf";
		}
		else {
			fonts::font_directory_sub_esp = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_sub_esp];
		}
		g_Options.fonts.esp_sub_font = fonts::selected_font_index_sub_esp;
	}

	fonts::selected_font_index_debug_information = g_Options.fonts.debug_information_font;
	if (fonts::selected_font_index_debug_information >= 0) {
		if (fontsList[fonts::selected_font_index_debug_information] == "default") {
			fonts::font_directory_debug_information = "C:/windows/fonts/tahomabd.ttf";
		}
		else {
			fonts::font_directory_debug_information = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_debug_information];
		}
		g_Options.fonts.debug_information_font = fonts::selected_font_index_debug_information;
	}
}
inline static int font_tab = 0;
const char* fnt_tab[6] = { "main indicator font", "sub indicator font", "esp font", "sub esp font", "scene font", "sub scene font" };
bool MultiCombo(const char* name, const char** displayName, bool* data, int dataSize) {
	ImGui::PushID(name);

	char previewText[1024] = { 0 };
	char buf[1024] = { 0 };
	char buf2[1024] = { 0 };
	int currentPreviewTextLen = 0;
	float multicomboWidth = 115.f;

	for (int i = 0; i < dataSize; i++) {

		if (data[i] == true) {

			if (currentPreviewTextLen == 0)
				sprintf(buf, "%s", displayName[i]);
			else
				sprintf(buf, ", %s", displayName[i]);

			strcpy(buf2, previewText);
			sprintf(buf2 + currentPreviewTextLen, buf);
			ImVec2 textSize = ImGui::CalcTextSize(buf2);

			if (textSize.x > multicomboWidth) {

				sprintf(previewText + currentPreviewTextLen, "...");
				currentPreviewTextLen += (int)strlen("...");
				break;
			}

			sprintf(previewText + currentPreviewTextLen, buf);
			currentPreviewTextLen += (int)strlen(buf);
		}
	}

	if (currentPreviewTextLen > 0)
		previewText[currentPreviewTextLen] = NULL;
	else
		sprintf(previewText, "-");

	bool isDataChanged = false;

	if (ImGui::BeginCombo(name, previewText)) {

		for (int i = 0; i < dataSize; i++) {

			sprintf(buf, displayName[i]);

			if (ImGui::Selectable(buf, data[i], ImGuiSelectableFlags_DontClosePopups)) {

				data[i] = !data[i];
				isDataChanged = true;
			}
		}

		ImGui::EndCombo();
	}

	ImGui::PopID();
	return isDataChanged;
}
const char* font_flags[11] = { "no hinting","no autohint","light hinting","mono hinting","bold","italic","no antialiasing","load color","bitmap","drop shadow","outline" };

void font() {
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::Columns(1, nullptr, false); {
		ImGui::BeginChild(("fonts.main"), ImVec2{ }, true, ImGuiWindowFlags_MenuBar); {
			if (ImGui::BeginMenuBar()) {
				ImGui::PushFont(fonts::menu_font_bold);
				ImGui::TextUnformatted(("fonts"));
				ImGui::PopFont();
				ImGui::EndMenuBar();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, 0));

			static ImGuiTextFilter filter;
			load_font_index();
			ImGui::Text("selected font");
			ImGui::Combo("##font_part_selector", &font_tab, fnt_tab, IM_ARRAYSIZE(fnt_tab));

			ImGui::Text("font list");

			if (font_tab == 0) {
				ImGui::ListBoxHeader("##font_list_indicators", ImVec2(-1, 190)); {
					for (int i = 0; i < fontsList.size(); i++) {

						std::string fonts = fontsList[i];

						if (filter.PassFilter(fonts.c_str())) {
							if (ImGui::Selectable(fontsList[i].c_str(), i == fonts::selected_font_index_main_indi)) {
								fonts::selected_font_index_main_indi = i;
							}
						}
					}
					ImGui::ListBoxFooter();
				}

				if (fonts::selected_font_index_main_indi >= 0) {
					if (fontsList[fonts::selected_font_index_main_indi] == "default") {
						fonts::font_directory_indicator = "C:/windows/fonts/tahomabd.ttf";
					}
					else {
						fonts::font_directory_indicator = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_main_indi];
					}
					g_Options.fonts.indi_font = fonts::selected_font_index_main_indi;
				}

				ImGui::Text("search font");
				filter.Draw("##filter_skin");
				ImGui::SliderInt("font size##indicators", &g_Options.fonts.indi_size, 1, 50);
				ImGui::Text("font flags");
				MultiCombo("##indicators_font_flags", font_flags, g_Options.fonts.indi_font_flag, 11);
			}
			else if (font_tab == 1) {
				ImGui::ListBoxHeader("##font_list_sub_indicators", ImVec2(-1, 190)); {
					for (int i = 0; i < fontsList.size(); i++) {

						std::string fonts = fontsList[i];

						if (filter.PassFilter(fonts.c_str())) {
							if (ImGui::Selectable(fontsList[i].c_str(), i == fonts::selected_font_index_sub_indi)) {
								fonts::selected_font_index_sub_indi = i;
							}
						}
					}
					ImGui::ListBoxFooter();
				}

				if (fonts::selected_font_index_sub_indi >= 0) {
					if (fontsList[fonts::selected_font_index_sub_indi] == "default") {
						fonts::font_directory_sub_indicator = "C:/windows/fonts/tahoma.ttf";
					}
					else {
						fonts::font_directory_sub_indicator = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_sub_indi];
					}
					g_Options.fonts.sub_indi_font = fonts::selected_font_index_sub_indi;
				}

				ImGui::Text("search font");
				filter.Draw("##filter_skin");
				ImGui::SliderInt("font size##sub_indicators", &g_Options.fonts.sub_indi_size, 1, 50);
				ImGui::Text("font flags");
				MultiCombo("##sub_indicators_font_flags", font_flags, g_Options.fonts.sub_indi_font_flag, 11);
			}
			else if (font_tab == 2) {
				ImGui::ListBoxHeader("##font_list_esp", ImVec2(-1, 190)); {
					for (int i = 0; i < fontsList.size(); i++) {

						std::string fonts = fontsList[i];

						if (filter.PassFilter(fonts.c_str())) {
							if (ImGui::Selectable(fontsList[i].c_str(), i == fonts::selected_font_index_main_esp)) {
								fonts::selected_font_index_main_esp = i;
							}
						}
					}
					ImGui::ListBoxFooter();
				}

				if (fonts::selected_font_index_main_esp >= 0) {
					if (fontsList[fonts::selected_font_index_main_esp] == "default") {
						fonts::font_directory_esp = "C:/windows/fonts/tahoma.ttf";
					}
					else {
						fonts::font_directory_esp = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_main_esp];
					}
					g_Options.fonts.esp_font = fonts::selected_font_index_main_esp;
				}

				ImGui::Text("search font");
				filter.Draw("##filter_skin");
				ImGui::SliderInt("font size##esp", &g_Options.fonts.esp_size, 1, 50);
				ImGui::Text("font flags");
				MultiCombo("##esp_font_flags", font_flags, g_Options.fonts.esp_flag, 11);
			}
			else if (font_tab == 3) {
				ImGui::ListBoxHeader("##font_list_sub_esp", ImVec2(-1, 190)); {
					for (int i = 0; i < fontsList.size(); i++) {

						std::string fonts = fontsList[i];

						if (filter.PassFilter(fonts.c_str())) {
							if (ImGui::Selectable(fontsList[i].c_str(), i == fonts::selected_font_index_sub_esp)) {
								fonts::selected_font_index_sub_esp = i;
							}
						}
					}
					ImGui::ListBoxFooter();
				}

				if (fonts::selected_font_index_sub_esp >= 0) {
					if (fontsList[fonts::selected_font_index_sub_esp] == "default") {
						fonts::font_directory_sub_esp = "C:/windows/fonts/tahoma.ttf";
					}
					else {
						fonts::font_directory_sub_esp = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_sub_esp];
					}
					g_Options.fonts.esp_sub_font = fonts::selected_font_index_sub_esp;
				}

				ImGui::Text("search font");
				filter.Draw("##filter_skin");
				ImGui::SliderInt("font size##sub_esp", &g_Options.fonts.esp_sub_size, 1, 50);
				ImGui::Text("font flags");
				MultiCombo("##sub_esp_font_flags", font_flags, g_Options.fonts.esp_sub_flag, 11);
			}
			else if (font_tab == 4) {
				ImGui::ListBoxHeader("##font_list_scene", ImVec2(-1, 190)); {
					for (int i = 0; i < fontsList.size(); i++) {

						std::string fonts = fontsList[i];

						if (filter.PassFilter(fonts.c_str())) {
							if (ImGui::Selectable(fontsList[i].c_str(), i == fonts::selected_font_index_main_scene)) {
								fonts::selected_font_index_main_scene = i;
							}
						}
					}
					ImGui::ListBoxFooter();
				}

				if (fonts::selected_font_index_main_scene >= 0) {
					if (fontsList[fonts::selected_font_index_main_scene] == "default") {
						fonts::font_directory_scene = "C:/windows/fonts/tahomabd.ttf";
					}
					else {
						fonts::font_directory_scene = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_main_scene];
					}
					g_Options.fonts.scene_font = fonts::selected_font_index_main_scene;
				}

				ImGui::Text("search font");
				filter.Draw("##filter_skin");
				ImGui::SliderInt("font size##scene", &g_Options.fonts.scene_size, 1, 50);
				ImGui::Text("font flags");
				MultiCombo("##scene_font_flags", font_flags, g_Options.fonts.scene_flag, 11);
			}
			else if (font_tab == 5) {
				ImGui::ListBoxHeader("##font_list_sub_scene", ImVec2(-1, 190)); {
					for (int i = 0; i < fontsList.size(); i++) {

						std::string fonts = fontsList[i];

						if (filter.PassFilter(fonts.c_str())) {
							if (ImGui::Selectable(fontsList[i].c_str(), i == fonts::selected_font_index_sub_scene)) {
								fonts::selected_font_index_sub_scene = i;
							}
						}
					}
					ImGui::ListBoxFooter();
				}

				if (fonts::selected_font_index_sub_scene >= 0) {
					if (fontsList[fonts::selected_font_index_sub_scene] == "default") {
						fonts::font_directory_sub_scene = "C:/windows/fonts/tahomabd.ttf";
					}
					else {
						fonts::font_directory_sub_scene = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_sub_scene];
					}
					g_Options.fonts.scene_sub_font = fonts::selected_font_index_sub_scene;
				}

				ImGui::Text("search font");
				filter.Draw("##filter_skin");
				ImGui::SliderInt("font size##sub_scene", &g_Options.fonts.scene_sub_size, 1, 50);
				ImGui::Text("font flags");
				MultiCombo("##sub_scene_font_flags", font_flags, g_Options.fonts.scene_sub_flag, 11);
			}
			else if (font_tab == 6) {
				ImGui::ListBoxHeader("##font_list_debug_information", ImVec2(-1, 190)); {
					for (int i = 0; i < fontsList.size(); i++) {

						std::string fonts = fontsList[i];

						if (filter.PassFilter(fonts.c_str())) {
							if (ImGui::Selectable(fontsList[i].c_str(), i == fonts::selected_font_index_debug_information)) {
								fonts::selected_font_index_debug_information = i;
							}
						}
					}
					ImGui::ListBoxFooter();
				}

				if (fonts::selected_font_index_debug_information >= 0) {
					if (fontsList[fonts::selected_font_index_debug_information] == "default") {
						fonts::font_directory_debug_information = "C:/windows/fonts/tahomabd.ttf";
					}
					else {
						fonts::font_directory_debug_information = "C:/windows/fonts/" + fontsList[fonts::selected_font_index_debug_information];
					}
					g_Options.fonts.scene_sub_font = fonts::selected_font_index_debug_information;
				}

				ImGui::Text("search font");
				filter.Draw("##filter_skin");
				ImGui::SliderInt("font size##sub_scene", &g_Options.fonts.debug_information_size, 1, 50);
				ImGui::Text("font flags");
				MultiCombo("##sub_scene_font_flags", font_flags, g_Options.fonts.debug_information_flag, 11);
			}
			if (ImGui::Button("reset fonts"))
			{
				fonts::reset_fonts();
				notification::run(" ", "#_print_reseted", true, true, true);
				
			}
			ImGui::SameLine();
			if (ImGui::Button("reload fonts"))
			{
				Menu::Get().fonts_initialized = true;
				load_font_index();
				notification::run(" ", "#_print_reloaded", true, true, true);
				
			}

			ImGui::PopStyleVar();
		}
		ImGui::EndChild();
	}
}
void Menu::Render()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.WindowPadding = ImVec2(8, 8);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32, 32);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 0.0f;
	style.FramePadding = ImVec2(4, 4);
	style.FrameRounding = 0.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8, 4);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.IndentSpacing = 6.0f;
	style.ColumnsMinSpacing = 10.0f;
	style.ScrollbarSize = 2.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabMinSize = 0.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.TabBorderSize = 0.0f;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.5f);
	style.MouseCursorScale = 0.75f;

	
	style.Colors[ImGuiCol_Text] = ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(200 / 255.f, 200 / 255.f, 200 / 255.f, 255 / 255.f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 0.10f, 0.85f);       // Dark blue
	style.Colors[ImGuiCol_WindowBg] = ImVec4(15 / 255.f, 15 / 255.f, 15 / 255.f, 255 / 255.f);             // Darker background
	style.Colors[ImGuiCol_ChildBg] = ImVec4(15 / 255.f, 15 / 255.f, 15 / 255.f, 255 / 255.f);            // Dark background
	style.Colors[ImGuiCol_PopupBg] = ImVec4(15 / 255.f, 15 / 255.f, 15 / 255.f, 255 / 255.f);         // Dark background
	style.Colors[ImGuiCol_Border] = ImVec4(30 / 255.f, 30 / 255.f, 30 / 255.f, 255 / 255.f);            // Dark gray border
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 255 / 255.f);          // Transparent
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);              // Darker background
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);       // Dark blue
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);              // Darker background for title
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.f);        // Dark blue background for active title
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 1.f);     // Dark blue background for collapsed title
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.70f);             // Dark background for menu bar
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.30f);           // Darker background for scrollbar
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 0.90f); // Dark gray for scrollbar grab when hovered
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.60f, 0.60f, 0.10f); // Light gray for scrollbar grab when active
	style.Colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);                // Darker background for buttons
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);         // Dark gray for buttons when hovered
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);         // Dark gray for header when hovered
	style.Colors[ImGuiCol_Separator] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);             // Dark gray separator
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);      // Dark gray separator when hovered
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.70f);      // Dark gray resize grip when hovered
	style.Colors[ImGuiCol_Tab] = ImVec4(0.05f, 0.05f, 0.05f, 0.80f);                   // Darker background for tabs
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.80f);            // Dark gray for tabs when hovered
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.10f, 0.70f);          // Dark gray for unfocused tabs
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.50f);       // Darker background for modal windows
	style.Colors[ImGuiCol_SliderGrab] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);             // color for slider grab
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);       // color for slider grab when active
	style.Colors[ImGuiCol_ButtonActive] = ImColor(0.10f, 0.10f, 0.10f, 1.00f);           // color for buttons when active
	style.Colors[ImGuiCol_Header] = ImColor(0.10f, 0.10f, 0.10f, 1.00f);                // color for header
	style.Colors[ImGuiCol_HeaderActive] = ImColor(0.10f, 0.10f, 0.10f, 1.00f);          // color for header when active
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);       // separator when active
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);             // resize grip color
	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);       // resize grip color when active
	style.Colors[ImGuiCol_TabActive] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);            // for active tab
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);   // for unfocused active tab
	style.Colors[ImGuiCol_PlotLines] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);              // for plot lines
	style.Colors[ImGuiCol_PlotLinesHovered] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);       // for plot lines when hovered
	style.Colors[ImGuiCol_PlotHistogram] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);          // for plot histogram
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);   // for plot histogram when hovered
	style.Colors[ImGuiCol_DragDropTarget] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);         // for drag and drop target
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2]);
	ImGui::GetIO().MouseDrawCursor = _visible;

	if (!_visible)
		return;

	ImGui::SetNextWindowSize(ImVec2(550, 560), ImGuiCond_Always);
	ImGui::Begin("##menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar); 
		{
		menu_pos = ImGui::GetCursorScreenPos();
		menu_size = ImGui::GetWindowSize();
		static int selection_count{};
		// add tabs
		ImGui::PushFont(fonts::menu_font_thin);
		static const ctab tabs[] =
		{
			ctab{ "legit", &legitbot },
			ctab{ "visuals", &visuals },
			ctab{ "misc", &miscellaneous },
			ctab{ "fonts", &font },
			ctab{ "skins", &skins1 },
		};
		render_tab("main_tabs", tabs, 5U, &main_tab, style.Colors[ImGuiCol_TabHovered], ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip);
		ImGui::PopFont();
		
		ImGui::End();
		
	}
	
	
}
void Menu::Toggle()
{
    _visible = !_visible;
}

void Menu::CreateStyle()
{
/*	ImGui::StyleColorsDark();
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
	_style.FrameRounding = 0.f;
	_style.WindowRounding = 0.f;
	_style.ChildRounding = 0.f;
	_style.Colors[ImGuiCol_Button] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_Header] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.260f, 0.590f, 0.980f, 1.000f);
	//_style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.000f, 0.545f, 1.000f, 1.000f);
	//_style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.060f, 0.416f, 0.980f, 1.000f);
	_style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.30f, 1.0f);
	_style.Colors[ImGuiCol_WindowBg] = ImVec4(0.000f, 0.009f, 0.120f, 0.940f);
	_style.Colors[ImGuiCol_PopupBg] = ImVec4(0.076f, 0.143f, 0.209f, 1.000f);
	ImGui::GetStyle() = _style;*/
	ImGui::StyleColorsLight();
	ImGuiStyle* Style = &ImGui::GetStyle();

	Style->WindowBorderSize = 0;
	Style->FrameBorderSize = 1;
	Style->ChildBorderSize = 0.5;
	Style->WindowRounding = 2;
	Style->ChildRounding = 2;
	Style->FrameRounding = 2;
	Style->ScrollbarSize = 1;
	Style->ScrollbarRounding = 2;
	Style->PopupRounding = 2;
	Style->GrabRounding = 2;



	Style->Colors[ImGuiCol_Text] = ImColor(255, 255, 255); // 255 255 255
	Style->Colors[ImGuiCol_Border] = ImColor(20, 20, 20, 150);  //49 49 49
	Style->Colors[ImGuiCol_WindowBg] = ImColor(3, 3, 3); // 27 27 27
	Style->Colors[ImGuiCol_ChildBg] = ImColor(27,27,27); // 27 all
	Style->Colors[ImGuiCol_FrameBg] = ImColor(3, 3, 3); // 33 all
	Style->Colors[ImGuiCol_Button] = ImColor(66, 150, 250);
	//Style->Colors[ImGuiCol_BorderShadow] = ImColor(255,255,0);
	Style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.34f,0.34f,0.34f, 1.f);
	Style->Colors[ImGuiCol_ButtonActive] = ImColor(0.34f,0.34f,0.34f, 1.f);
	Style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(0.34f,0.34f,0.34f, 1.f);
	Style->Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(0.34f,0.34f,0.34f, 1.f);
	Style->Colors[ImGuiCol_CheckMark] = ImVec4(0.34f,0.34f,0.34f, 1.f);
	Style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.34f,0.34f,0.34f, 1.f);
	Style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.34f,0.34f,0.34f, 1.f);
	Style->Colors[ImGuiCol_Button] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
	Style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	Style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.34f,0.34f,0.34f, 0.1f);
	Style->Colors[ImGuiCol_Header] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	Style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.5);
	Style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 0.5f);
	Style->Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	Style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	Style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	Style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	Style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	Style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	Style->Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
	Style->Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
	Style->Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
	Style->Colors[ImGuiCol_TabUnfocused] = ImVec4(0.34f,0.34f,0.34f, 1.0f);
	Style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.34f,0.34f,0.34f, 0.f);
	Style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	Style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	Style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	Style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	Style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	Style->Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	Style->Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	Style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	Style->Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	Style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

