#define IMGUI_DEFINE_MATH_OPERATORS
#include "hooks.hpp"
#include <intrin.h>  

#include "render.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "features/bhop.hpp"
#include "features/chams.hpp"
#include "features/visuals.hpp"
#include "features/glow.hpp"
#include "features/skins.h"
#include "renderer.h"
#include "features/bt.h"
#include "features/lagcomp.h"
#include <intrin.h>  
#include <TlHelp32.h>
#include "features/cmouse.h"
#include "features/NewOne.h"





template<class T>
static T* FindHudElements(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}
bool changed = false;
std::string backup_skybox = "";
void skybox_changer()
{
	static auto fnLoadNamedSkys = (void(__fastcall*)(const char*))Utils::PatternScan(GetModuleHandleA("engine.dll"), "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45");
	auto skybox_name = backup_skybox;

	switch (g_Options.skybox_num)
	{
	case 1:
		skybox_name = "cs_tibet";
		break;
	case 2:
		skybox_name = "cs_baggage_skybox_";
		break;
	case 3:
		skybox_name = "italy";
		break;
	case 4:
		skybox_name = "jungle";
		break;
	case 5:
		skybox_name = "office";
		break;
	case 6:
		skybox_name = "sky_cs15_daylight01_hdr";
		break;
	case 7:
		skybox_name = "sky_cs15_daylight02_hdr";
		break;
	case 8:
		skybox_name = "vertigoblue_hdr";
		break;
	case 9:
		skybox_name = "vertigo";
		break;
	case 10:
		skybox_name = "sky_day02_05_hdr";
		break;
	case 11:
		skybox_name = "nukeblank";
		break;
	case 12:
		skybox_name = "sky_venice";
		break;
	case 13:
		skybox_name = "sky_cs15_daylight03_hdr";
		break;
	case 14:
		skybox_name = "sky_cs15_daylight04_hdr";
		break;
	case 15:
		skybox_name = "sky_csgo_cloudy01";
		break;
	case 16:
		skybox_name = "sky_csgo_night02";
		break;
	case 17:
		skybox_name = "sky_csgo_night02b";
		break;
	case 18:
		skybox_name = "sky_csgo_night_flat";
		break;
	case 19:
		skybox_name = "sky_dust";
		break;
	case 20:
		skybox_name = "vietnam";
		break;
	}

	static auto skybox_number = 0;
	static auto old_skybox_name = skybox_name;
	static auto color_r = (unsigned char)255;
	static auto color_g = (unsigned char)255;
	static auto color_b = (unsigned char)255;


	if (skybox_number != g_Options.skybox_num)
	{
		changed = true;
		skybox_number = g_Options.skybox_num;
	}
	else if (old_skybox_name != skybox_name)
	{
		changed = true;
		old_skybox_name = skybox_name;
	}
	else if (color_r != g_Options.skybox_color.r())
	{
		changed = true;
		color_r = g_Options.skybox_color.r();
	}
	else if (color_g != g_Options.skybox_color.g())
	{
		changed = true;
		color_g = g_Options.skybox_color.g();
	}
	else if (color_b != g_Options.skybox_color.b())
	{
		changed = true;
		color_b = g_Options.skybox_color.b();
	}

	if (changed)
	{
		changed = false;
		fnLoadNamedSkys(skybox_name.c_str());
		auto materialsystem = g_MatSystem;

		for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
		{
			auto material = materialsystem->GetMaterial(i);

			if (!material)
				continue;

			if (strstr(material->GetTextureGroupName(), "SkyBox"))
				material->ColorModulate(g_Options.skybox_color.r() / 255.f, g_Options.skybox_color.g() / 255.f, g_Options.skybox_color.b() / 255.f);
		}
	}
}
namespace speclickis
{
	ImVec2 pos{};
	ImVec2 size{};
	void RenderText(ImDrawList* draw, const char* text, ImVec2 pos1, ImColor clr)
	{
		draw->AddText(win95, 14.f, ImVec2(pos) + ImVec2(pos1), ImColor(clr), text);
	}
	void RenderRect(ImDrawList* draw, ImVec2 pos1, ImVec2 size1, ImColor clr)
	{
		draw->AddRect(ImVec2(pos) + pos1, pos + pos1 + size1, ImColor(clr), 0.f);

	}
	void RenderFilledRect(ImDrawList* draw, ImVec2 pos1, ImVec2 size1, ImColor clr)
	{
		draw->AddRectFilled(ImVec2(pos) + pos1, pos + pos1 + size1, ImColor(clr), 0.f);
	}
	void RenderLine(ImDrawList* draw, ImVec2 pos1, ImVec2 pos2, ImColor clr)
	{
		draw->AddLine(ImVec2(pos) + pos1, pos + pos1 + pos2, ImColor(clr), 0.f);
	}
	void ButtonX(ImDrawList* draw, ImVec2 pos1)
	{
		RenderFilledRect(draw, pos1, { 13,11 }, ImColor(212, 208, 200));
		RenderLine(draw, pos1 - ImVec2(1, 1), { 15,0 }, ImColor(255, 255, 255, 255));
		RenderLine(draw, pos1 - ImVec2(1, 1), { 0,13 }, ImColor(255, 255, 255, 255));
		RenderLine(draw, pos1 - ImVec2(1, 1), { 0,13 }, ImColor(255, 255, 255, 255));
		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(1, 12), ImVec2(14, 0), ImColor(128, 128, 128));
		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(14, 12), ImVec2(0, -11), ImColor(128, 128, 128));

		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(1, 13), ImVec2(15, 0), ImColor(0, 0, 0));

		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(15, 12), ImVec2(0, -12), ImColor(0, 0, 0));
		RenderText(draw, "X", pos1 + ImVec2(3, -2), ImColor(0, 0, 0));
	}
	void ButtonNIZ(ImDrawList* draw, ImVec2 pos1)
	{
		RenderFilledRect(draw, pos1, { 13,11 }, ImColor(212, 208, 200));
		RenderLine(draw, pos1 - ImVec2(1, 1), { 15,0 }, ImColor(255, 255, 255, 255));
		RenderLine(draw, pos1 - ImVec2(1, 1), { 0,13 }, ImColor(255, 255, 255, 255));
		RenderLine(draw, pos1 - ImVec2(1, 1), { 0,13 }, ImColor(255, 255, 255, 255));
		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(1, 12), ImVec2(14, 0), ImColor(128, 128, 128));
		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(14, 12), ImVec2(0, -11), ImColor(128, 128, 128));

		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(1, 13), ImVec2(15, 0), ImColor(0, 0, 0));

		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(15, 12), ImVec2(0, -12), ImColor(0, 0, 0));
		RenderText(draw, "_", pos1 + ImVec2(3, -5), ImColor(0, 0, 0));
	}
	void Buttonrect(ImDrawList* draw, ImVec2 pos1)
	{
		RenderFilledRect(draw, pos1, { 13,11 }, ImColor(212, 208, 200));
		RenderLine(draw, pos1 - ImVec2(1, 1), { 15,0 }, ImColor(255, 255, 255, 255));
		RenderLine(draw, pos1 - ImVec2(1, 1), { 0,13 }, ImColor(255, 255, 255, 255));
		RenderLine(draw, pos1 - ImVec2(1, 1), { 0,13 }, ImColor(255, 255, 255, 255));
		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(1, 12), ImVec2(14, 0), ImColor(128, 128, 128));
		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(14, 12), ImVec2(0, -11), ImColor(128, 128, 128));

		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(1, 13), ImVec2(15, 0), ImColor(0, 0, 0));

		RenderLine(draw, pos1 - ImVec2(1, 1) + ImVec2(15, 12), ImVec2(0, -12), ImColor(0, 0, 0));
		RenderRect(draw, pos1 + ImVec2(2, 1), ImVec2(9, 9), ImColor(0, 0, 0));
	}
	void Spec()
	{
		{
			static float f = 0.0f;
			static int counter = 0;
			static float sizey = -200.f;
			static float sizex = -200.f;
			if (GetAsyncKeyState('C'))
				sizey += 1.f;
			if (GetAsyncKeyState('X'))
				sizex += 1.f;
			ImGui::SetNextWindowSize({ 400 + sizex, 300 + sizey });
			ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);                          // Create a window called "Hello, world!" and append into it.
			ImGui::PushFont(win95);
			pos = ImGui::GetWindowPos() + ImVec2(1, 1);
			size = ImGui::GetWindowSize();
			auto draw = ImGui::GetWindowDrawList();
			//BackGround


			RenderFilledRect(draw, { 1,1 }, { 397 + sizex,299 + sizey }, ImColor(255, 255, 255, 255));
			RenderRect(draw, { 0,0 }, { 399 + sizex,300 + sizey }, ImColor(223, 223, 223, 255));
			RenderFilledRect(draw, { 2,2 }, { 395 + sizex,296 + sizey }, ImColor(192, 192, 192, 255));
			// Objects
			RenderLine(draw, { 4,294 + sizey }, { 391 + sizex,0 }, ImColor(223, 223, 223));
			RenderLine(draw, { 394 + sizex,294 + sizey }, { 0,-14 }, ImColor(223, 223, 223));
			RenderLine(draw, { 4,280 + sizey }, { 0,15 }, ImColor(128, 128, 128));
			RenderLine(draw, { 4,280 + sizey }, { 391 + sizex,0 }, ImColor(128, 128, 128));
			
			//TiteBar
			RenderFilledRect(draw, { 4,4 }, { 391 + sizex,22 }, ImColor(0, 0, 128, 255));
			ImGui::SetCursorPos({ 4,2 });
			ImGui::Image(Logotype, ImVec2(33, 28));
			//BackGround Text
			RenderRect(draw, { 4,46 }, { 391 + sizex,250 - 18 + sizey }, ImColor(128, 128, 128, 255));
			RenderRect(draw, { 5,47 }, { 389 + sizex,248 - 18 + sizey }, ImColor(0, 0, 0, 255));
			RenderFilledRect(draw, { 6,48 }, { 387 + sizex,246 - 18 + sizey }, ImColor(255, 255, 255, 255));

			RenderText(draw, "Spectators", { 9 + 23,6 }, ImColor(255, 255, 255));
			RenderText(draw, "File  Edit  Format  Help", ImVec2(7, 28), ImColor(0, 0, 0));
			ButtonX(draw, { 378 + sizex,9 });
			ButtonNIZ(draw, { 344 + sizex,9 });
			Buttonrect(draw, { 360 + sizex,9 });
			// Render text and adjust sizey if text overflows
			float textHeight = 0.f;
			float textWidth = 0.f;
			int totality = 0;
			sizey = -200.f;
			sizex = -200.f;
			for (int i = 0; i < g_EngineClient->GetMaxClients(); i++)
			{
				C_BasePlayer* entity = C_BasePlayer::GetPlayerByIndex(i);

				if (!entity)
					continue;

				if (entity->IsAlive())
					continue;

				if (entity->IsDormant())
					continue;
			
				std::string spectators;
				player_info_t entityinfo1 = entity->GetPlayerInfo();
				bool pl = false;
				if (entity->m_hObserverTarget())
				{


					C_BasePlayer* target = entity->m_hObserverTarget();

					if (!target->IsPlayer())
						continue;

					player_info_t entityinfo = entity->GetPlayerInfo();
					player_info_t targetinfo = target->GetPlayerInfo();
					spectators = std::string(entityinfo.szName) + "->" + targetinfo.szName;
					bool checkplayer = false;
					if (target == g_LocalPlayer)
						pl = true;
				}
				else
				{
					
					spectators = std::string(entityinfo1.szName);
				}



				

				ImVec2 textPos = { 35, 55 + totality++ * 15.f };
				RenderText(draw, spectators.c_str(), textPos, ImColor(0, 0, 0));
				if (pl)
					draw->AddCircleFilled(pos + textPos - ImVec2(10, -8), 4.f, ImColor(0, 0, 0), 24);
				if (textPos.y > 48 + 246 - 18 + sizey - 25) // Check if text overflows the white background rectangle
				{
					textHeight = textPos.y - (48 + 246 - 18 + sizey - 25);
				}
				if (textPos.x + win95->CalcTextSizeA(14.f, FLT_MAX, NULL, spectators.c_str()).x > 387 + sizex) // Check if text overflows the white background rectangle horizontally (adjust 50 for text width)
				{
					textWidth = textPos.x + win95->CalcTextSizeA(14.f, FLT_MAX, NULL, spectators.c_str()).x - (387 + sizex);
				}





			

			}
			if (textHeight > 0.f)
			{
				sizey += textHeight;
			}
			if (textWidth > 0.f)
			{
				sizex += textWidth;
			}
			RenderText(draw, std::string(std::to_string(totality) + " object(s)").c_str(), { 6, 279 + sizey }, ImColor(0, 0, 0));

			ImGui::PopFont();
			ImGui::End();
		}
	}
}
struct Snowflake {
	float x, y, speed;
};

std::vector<Snowflake> snowflakes;

void InitializeSnowflakes(int count, int window_width, int window_height) {
	snowflakes.clear();
	std::srand(static_cast<unsigned>(std::time(nullptr)));
	for (int i = 0; i < count; ++i) {
		Snowflake flake;
		flake.x = static_cast<float>(std::rand() % window_width);
		flake.y = static_cast<float>(std::rand() % window_height);
		flake.speed = static_cast<float>((std::rand() % 50) / 100.0f + 0.1f); // speed between 0.5 and 1.0
		snowflakes.push_back(flake);
	}
}

void UpdateSnowflakes(int window_width, int window_height) {
	for (auto& flake : snowflakes) {
		flake.y += flake.speed;
		if (flake.y > window_height) {
			flake.x = static_cast<float>(std::rand() % window_width);
			flake.y = 0;
		}
	}
}

void RenderSnowflakes(ImDrawList* draw_list, int window_width, int window_height) {
	for (const auto& flake : snowflakes) {
		draw_list->AddCircleFilled(ImVec2(ImGui::GetWindowPos().x+10 + flake.x, ImGui::GetWindowPos().y + flake.y+3), 2.0f, IM_COL32(255, 255, 255, 140));
	}
}

void watermark() {
	static float current_time = 0.0f;
	static int last_fps = (int)(1.0f / g_GlobalVars->frametime);

	if (current_time > 0.5f) {
		last_fps = (int)(1.0f / g_GlobalVars->frametime);
		current_time = 0.f;
	}

	current_time += g_GlobalVars->frametime;

	std::string text = ("lobotomy | ") + std::to_string(last_fps) + (" fps | ") + ("");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(156.0f, 27.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 166.0f, 10.0f), ImGuiCond_Always);
	ImGui::Begin("##watermark", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse); 

		ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), IM_COL32(0, 0, 0, 225));
		ImGui::GetOverlayDrawList()->AddRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), IM_COL32(255, 255, 255, 225), 0.0f, 15, 1.0f);

	ImGui::End();
	ImGui::PopStyleVar(3);
}

#pragma intrinsic(_ReturnAddress)  
int bIntFlag;
namespace Hooks {

	void Initialize()
	{
		hlclient_hook.setup(g_CHLClient);
		direct3d_hook.setup(g_D3DDevice9);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		sound_hook.setup(g_EngineSound);
		mdlrender_hook.setup(g_MdlRender);
		clientmode_hook.setup(g_ClientMode);
		stdrender_hook.setup(g_StudioRender);
		viewrender_hook.setup(g_ViewRender);

		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);

		gameevents_hook.setup(g_GameEvents);
		gameevents_hook.hook_index(index::FireEvent, hkFireEvent);
		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		clientmode_hook.hook_index(index::CreateMove, hkCreateMove_Proxy);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		sound_hook.hook_index(index::EmitSound1, hkEmitSound1);
		vguisurf_hook.hook_index(index::LockCursor, hkLockCursor);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);
		stdrender_hook.hook_index(index::DrawModelExecute2, hkDrawModelExecute2);
		viewrender_hook.hook_index(index::RenderSmokeOverlay, RenderSmokeOverlay);

		sequence_hook = new recv_prop_hook(C_BaseViewModel::m_nSequence(), RecvProxy);
	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sound_hook.unhook_all();
		sv_cheats.unhook_all();

		Glow::Get().Shutdown();
		sequence_hook->~recv_prop_hook();

	}
	bool __fastcall send_net_msg(void* ecx, void* edx, INetMessage* msg, bool reliable, bool voice)
	{
		static auto oFireEvent = hk_netchannel.get_original<sendnetmsg_fn>(40);

		if (!msg)
			return original_sendnetmsg(ecx, msg, reliable, voice);


		if (msg->GetType() == 14)
			return false;

		return oFireEvent(ecx, msg, reliable, voice);
	}

	bool __stdcall hkFireEvent(IGameEvent* pEvent) {
		static auto oFireEvent = gameevents_hook.get_original<FireEvent>(index::FireEvent);

		const char* szEventName = pEvent->GetName();
		Visuals::Get().FireGameEvent(pEvent);

		if (!strcmp(szEventName, "server_spawn"))
		{

			const auto net_channel = g_EngineClient->GetNetChannelInfo();

			if (net_channel != nullptr)
			{
				const auto index = 40;
				Hooks::hk_netchannel.setup(net_channel);
				Hooks::hk_netchannel.hook_index(index, send_net_msg);
			}
		}
	
		if (!strcmp(szEventName, "cs_game_disconnected"))
		{
			if (hk_netchannel.is_hooked())
			{
				hk_netchannel.unhook_all();
			}
		}

		if (!strcmp(szEventName, "player_hurt"))
		{
			int attacker = pEvent->GetInt("attacker");
			if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer())
			{
				Visuals::Get().hitmarker_time = 500;
				if (g_Options.hit_sound)
				{
					g_VGuiSurface->play_sound("ui\\beep07.wav");
				}
				BunnyHop::hitbox_invo(pEvent);
				if (g_Options.hitinfo)
				Visuals::Get().HIT_LOGGER(pEvent);
			}
		}
		if (!strcmp(szEventName, "player_death"))
		{
			if (g_Options.killss)
				BunnyHop::KillSound(pEvent);
		}
		if (!strcmp(szEventName, "round_start"))
		{
			killCounter = 1;
		}
		return oFireEvent(g_GameEvents, pEvent);
	}
	//--------------------------------------------------------------------------------

	std::string oldtitle;
	std::string oldartist; 
	IDirect3DTexture9* albumArtTexture = nullptr;

	
	std::wstring ConvertUtf8ToWide(const std::string& utf8Str) {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(utf8Str);
	}
	LPDIRECT3DTEXTURE9 LoadTextureFromFile(const std::string& path) {
		LPDIRECT3DTEXTURE9 texture = NULL;
		std::wstring widePath = ConvertUtf8ToWide(path);
		if (D3DXCreateTextureFromFileW(g_D3DDevice9, widePath.c_str(), &texture) != D3D_OK) {
			std::wcerr << L"Failed to load texture: " << widePath << std::endl;
			return NULL;
		}
		return texture;
	}

#define GAMEOVERLAYRENDERER_DLL ("gameoverlayrenderer.dll")
	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>(index::EndScene);
		static void* pUsedAddress = nullptr;

		static uintptr_t gameoverlay_return_address = 0;
		if (!gameoverlay_return_address) {
			MEMORY_BASIC_INFORMATION info;
			VirtualQuery(_ReturnAddress(), &info, sizeof(MEMORY_BASIC_INFORMATION));

			char mod[MAX_PATH];
			GetModuleFileNameA((HMODULE)info.AllocationBase, mod, MAX_PATH);

			if (strstr(mod, "gameoverlay"))
				gameoverlay_return_address = (uintptr_t)(_ReturnAddress());
		}

		if (gameoverlay_return_address != (uintptr_t)(_ReturnAddress()))
			return oEndScene(pDevice);


			//viewmodel_fov->SetValue(g_Options.viewmodel_fov);
			DWORD colorwrite, srgbwrite;
			IDirect3DVertexDeclaration9* vert_dec = nullptr;
			IDirect3DVertexShader9* vert_shader = nullptr;
			DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
			pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
			pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

			pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
			//removes the source engine color correction
			pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

			pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
			pDevice->GetVertexDeclaration(&vert_dec);
			pDevice->GetVertexShader(&vert_shader);
			pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
			pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
			pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
			pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);
			if (Menu::Get().fonts_initialized) {
				Render::Get().GetFonts();
				Menu::Get().fonts_initialized = false;
			}
			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			
			auto igrok = Visuals::Player();
			//igrok.oofarr(nullptr);
			if(g_Options.sunset)
			Visuals::Get().sunset();
			auto esp_drawlist = Render::Get().RenderScene();
			static ImVec2 sz{};
			int x, y;
			std::string strartist = wstring_to_utf8(g_Options.artist);
			std::string strtitle = wstring_to_utf8(g_Options.title);
			static bool oncei = false;
			if (!oncei)
			{
				InitializeSnowflakes(10,150,30);
				oncei = true;
			}
			if (g_Options.watermark) {
				watermark();
			}
			if (g_Options.trackdispay && strtitle !="")
			{
				g_EngineClient->GetScreenSize(x, y);
				if (!g_Options.watermark) {
					ImGui::SetNextWindowPos({ x - sz.x - 10,5 });
				}
				if (g_Options.watermark) {
					ImGui::SetNextWindowPos({ x - sz.x - 10,45 });
				}
				ImGui::Begin("Media Player", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
				if (oldtitle != strtitle || oldartist != strartist || !albumArtTexture)
				{
					albumArtTexture = NULL;
					albumArtTexture = LoadTextureFromFile("C:\\lobotomy\\artalbum\\" + strtitle + ".jpg");
					oldtitle = strtitle;
					oldartist = strartist;
				}
				if (albumArtTexture) {
					ImGui::SetCursorPos(ImVec2(5, 3));
					ImGui::Image(albumArtTexture, ImVec2(35, 35));
				}
				ImGui::PushFont(fonts::menu_font_thin);

				ImGui::SetCursorPos({ 45,3 });
				ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f), strartist.c_str());
				ImGui::SetCursorPos({ 44,2 });
				ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), strartist.c_str());

				ImGui::SetCursorPos({ 45,15 });
				ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f), strtitle.c_str());
				ImGui::SetCursorPos({ 44,14 });
				ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 1.f), strtitle.c_str());
				ImGui::PushItemWidth(-1);
				ImGui::SetCursorPos({ 5,40 });

				float progress = static_cast<float>(g_Options.trackPosition) / static_cast<float>(g_Options.trackDuration);
				ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
				ImGui::PopItemWidth();
				if (Menu::Get().IsVisible())
				{
					ImGui::SetCursorPos({ 5,49 });
					if (ImGui::Button("Play / Pause")) {
						g_Options.pause = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("Next")) {
						g_Options.next = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("Previous")) {
						g_Options.pervious = true;
					}
				}
				sz = ImGui::GetWindowSize();
				ImGui::PopFont();
				ImGui::End();
			}
			Menu::Get().Render();
			if (g_Options.spectator_list && g_Options.spec_type == 3)
				speclickis::Spec();
			
			BunnyHop::pxcalc(nullptr);
			ImGui::Render(esp_drawlist);
		
			
			
			if (g_Options.scaleform)
			{
				g_CVar->FindVar("joy_advanced")->SetValue(1);
			}
			else
			{
				g_CVar->FindVar("joy_advanced")->SetValue(0);
			}

			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

			pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
			pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
			pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
			pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
			pDevice->SetVertexDeclaration(vert_dec);
			pDevice->SetVertexShader(vert_shader);
	

		return oEndScene(pDevice);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto oReset = direct3d_hook.get_original<decltype(&hkReset)>(index::Reset);
		
		Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0)
			Menu::Get().OnDeviceReset();

		return hr;
	}
	//--------------------------------------------------------------------------------
	
	MegaPrivateAimbotOtAntohi aimbot;
	bool Hooks::hkCreateMove(float frameTime, CUserCmd* cmd, bool& sendPacket)
	{
		const auto oCreateMove = clientmode_hook.get_original<bool(__stdcall*)(float, CUserCmd*)>(index::CreateMove)(frameTime, cmd);
		auto flags2 = g_LocalPlayer->m_fFlags();
		float eb = floor(g_LocalPlayer->m_vecVelocity().z);
		if (!cmd || !cmd->command_number)
			return oCreateMove;
		auto net_channel = g_ClientState->m_NetChannel;
		if (Menu::Get().IsVisible())
			cmd->buttons &= ~IN_ATTACK;

		if (g_Options.misc_bhop)
			BunnyHop::OnCreateMove(cmd);

		Vector unpedvelocity = g_LocalPlayer->m_vecVelocity();
		BunnyHop::clantag();
		bIntFlag = g_LocalPlayer->m_fFlags();

		if (GetAsyncKeyState(g_Options.AutoStafe_key))
			BunnyHop::AutoStafe(cmd);
		Visuals::Get().graphs_data();
		Visuals::Get().gatherinfo();
		BunnyHop::PixelFinder(cmd);
		BunnyHop::PrePred(cmd);
		BunnyHop::CHECKSURFLOLKEK(cmd);
		g_GlobalVars->serverTime(cmd);
		BunnyHop::pxcalc(cmd);
		BunnyHop::delayhop(cmd);
		BunnyHop::checkPoint(cmd);
		float ayevelo = g_LocalPlayer->m_vecVelocity().z;
		NewOne::PrePredictionEdgeBug(cmd);
		static auto prediction = new PredictionSystem();
		auto flags = g_LocalPlayer->m_fFlags();
		QAngle angOldViewPoint = cmd->viewangles;
		int nCommandsPredicted = g_Prediction->Split->nCommandsPredicted;
		if (g_Options.fastduck)
		{
			cmd->buttons |= IN_BULLRUSH;
		}
		BunnyHop::autoalign(cmd);
		if (AlertJB)
			cmd->buttons |= IN_JUMP;
		
		float velocity = g_LocalPlayer->m_vecVelocity().z;
		prediction->StartPrediction(cmd);
		
		
	//	BunnyHop::clantag();
			aimbot.run(cmd);
			Backtrack::run(cmd);

			NewOne::edgebug_detect(cmd);
			
			
			auto newgeneration = g_LocalPlayer->m_hActiveWeapon();
			if (g_Options.zeusbug && !newgeneration->IsMiscWeapon())
			{
				if (cmd->buttons & IN_ATTACK)
				{
					g_EngineClient->execute_cmd("slot11");
				}
			}

			if (g_Options.panoramablur)
			{
				g_CVar->FindVar("@panorama_disable_blur")->SetValue("1");
			}
			else
			{
				g_CVar->FindVar("@panorama_disable_blur")->SetValue("0");
			}
			if (g_Options.noviewbob)
			{
				g_CVar->FindVar("cl_wpn_sway_scale")->SetValue("0");
			}
			else
			{
				g_CVar->FindVar("cl_wpn_sway_scale")->SetValue("1.6");
			}
			
			BunnyHop::Minijump(cmd);
		
			if (g_Options.edgejump.enabled && GetAsyncKeyState(g_Options.edgejump.hotkey))
			{

				if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && (flags & FL_ONGROUND))
				{
					cmd->buttons |= IN_JUMP;

					if (g_Options.nulls3_ej)
					{
						if (g_Options.forwardd)
							g_EngineClient->execute_cmd("-forward");
						if (g_Options.back)
							g_EngineClient->execute_cmd("-back");
						if (g_Options.moveright)
							g_EngineClient->execute_cmd("-moveright");
						if (g_Options.moveleft)
							g_EngineClient->execute_cmd("-moveleft");
					}
				}

			}
			BunnyHop::sw(cmd);
			BunnyHop::ps_fix(cmd);
		

			BunnyHop::Ladderjump(cmd);
			BunnyHop::pixel_surf(cmd);
			
			


		
			
			prediction->EndPrediction();
			BunnyHop::jump_bug(cmd, flags2, velocity);
			QAngle backup_viewangles = cmd->viewangles;
			

				NewOne::EdgeBugPostPredict(cmd);
				memesclass->restoreEntityToPredictedFrame(0, nCommandsPredicted);

			
			
			
		//	BunnyHop::auto_duck(cmd);
			BunnyHop::ljend(cmd);

			BunnyHop::AutoBounce(cmd);
			BunnyHop::FireMan(cmd);
	
			if (g_LocalPlayer && g_LocalPlayer->IsAlive())
			{
				if (g_Options.extend_bt)
					CLagCompensation::Get().UpdateIncomingSequences(net_channel);
				else
					CLagCompensation::Get().ClearIncomingSequences();
			}

			cmd->viewangles.normalize();
			cmd->viewangles.pitch = std::clamp(cmd->viewangles.pitch, -90.f, 90.f);
			cmd->viewangles.yaw = std::clamp(cmd->viewangles.yaw, -180.f, 180.f);
			cmd->viewangles.roll = 0.f; 
			cmd->forwardmove = std::clamp(cmd->forwardmove, -450.f, 450.f);
			cmd->sidemove = std::clamp(cmd->sidemove, -450.f, 450.f);
			// https://github.com/spirthack/CSGOSimple/issues/69
			if (g_Options.misc_showranks && cmd->buttons & IN_SCORE) // rank revealer will work even after unhooking, idk how to "hide" ranks  again
				g_CHLClient->DispatchUserMessage(CS_UM_ServerRankRevealAll, 0, 0, nullptr);

			if (g_Options.sniper_xhair && !g_LocalPlayer->m_bIsScoped())
				g_CVar->FindVar("weapon_debug_spread_show")->SetValue(3);
			else
				g_CVar->FindVar("weapon_debug_spread_show")->SetValue(0);
			
			static const auto fog_enable = g_CVar->FindVar("fog_enable");
			fog_enable->SetValue(g_Options.esp_world_fog);
			static const auto fog_override = g_CVar->FindVar("fog_override");
			fog_override->SetValue(g_Options.esp_world_fog);
			static const auto fog_color = g_CVar->FindVar("fog_color");
			fog_color->SetValue(std::string(std::to_string(g_Options.fog_color.r()) + " " + std::to_string(g_Options.fog_color.g()) + " " + std::to_string(g_Options.fog_color.b())).c_str()); //FOG rgb color
			static const auto fog_start = g_CVar->FindVar("fog_start");
			fog_start->SetValue(g_Options.esp_world_fog_start);
			static const auto fog_end = g_CVar->FindVar("fog_end");
			fog_end->SetValue(g_Options.esp_world_fog_end);
			static const auto fog_destiny = g_CVar->FindVar("fog_maxdensity");
			fog_destiny->SetValue(g_Options.esp_world_fog_destiny);

			return false;
		
	}

	//--------------------------------------------------------------------------------
	void __declspec(naked) hkCreateMove_Proxy()
	{
		__asm 
		{
			push ebp
			mov  ebp, esp
			push ebx
			push esp	
			push[ebp + 0xC]	
			push[ebp + 0x8]	
			call hkCreateMove
			add esp, 0xC
			pop ebx	
			mov esp, ebp
			pop ebp
			ret 0x8	
		}
	}

	//--------------------------------------------------------------------------------
	void vod_mark()
	{

		if (g_Options.misc_watermark) {

			int width, height;
			g_EngineClient->GetScreenSize(width, height);

			std::string qoq;

			const auto size = render::get_text_size(g_Draw.m_WatermarkFont, qoq);
			static auto frameRate = 1.0f;
			frameRate = 0.9f * frameRate + 0.1f * g_GlobalVars->absoluteframetime;
			std::string fps{ std::to_string(static_cast<int>(1 / frameRate)) + " frames " };

			qoq = "lobotomy / " + fps;
			//render::draw_filled_rect(width - 90 - 1, 7 - 1, 65 + 2 + size.x, 10 + 2 + size.y, Color(g_Options.fuckingaccent[0], g_Options.fuckingaccent[1], g_Options.fuckingaccent[2], 0.25f));
			// Render box
			//render::draw_filled_rect(width - 90, 7, 65 + size.x, 10 + size.y, Color(10, 10, 10, 255));
			// Render gradient line
			// Render text
			render::draw_text_string(width / 2, height - 15, g_Draw.m_MenuFont , qoq, true, Color(255, 255, 255, 255));
		//	render::draw_text_string(width - 55, 12, g_Draw.m_WatermarkFont, " | " + fps, false, Color(125, 125, 125, 250));
		}
	}

	namespace iwebz
	{
		inline     Vector m_vecPos = (Vector(250, 250, 0));
		Vector2D textsizespec;
		int m_iTotalSpectators{}; // u teby есть переменая которая за цвет спека отвечает?

		void SpectatorList()
		{
			if (g_Options.spectator_list)
			{

				std::string str[100] = { };
				size_t a = 0;
				size_t c = 0;
				static float rainbow;
				int gdsf = 0;
				rainbow += 0.005f;
				if (rainbow > 1.f)
					rainbow = 0.f;
				ImColor rgb = ImColor::HSV(rainbow, 1.f, 1.f);


				for (int i = 0; i < g_EngineClient->GetMaxClients(); i++)
				{
					C_BasePlayer* entity = C_BasePlayer::GetPlayerByIndex(i);

					if (!entity)
						continue;

					if (entity->IsAlive())
						continue;

					if (entity->IsDormant())
						continue;
					int specscol[3];
					std::string spectators;
					player_info_t entityinfo1 = entity->GetPlayerInfo();

					if (entity->m_hObserverTarget())
					{


						C_BasePlayer* target = entity->m_hObserverTarget();

						if (!target->IsPlayer())
							continue;

						player_info_t entityinfo = entity->GetPlayerInfo();
						player_info_t targetinfo = target->GetPlayerInfo();
						spectators = std::string(entityinfo.szName) + " -> " + targetinfo.szName;
						bool checkplayer = false;
						int index = spectators.find(u8"∞");

						if (index != std::string::npos) {
							checkplayer = true;
						}

						if (target == g_LocalPlayer)
						{
							specscol[0] = g_Options.spec_color.r();
							specscol[1] = g_Options.spec_color.g();
							specscol[2] = g_Options.spec_color.b();
						}
						else
						{
							specscol[0] = 240;
							specscol[1] = 240;
							specscol[2] = 240;

						}
						//if (checkplayer)
						//{
						//	specscol[0] = rgb.Value.x*255;
						//	specscol[1] = rgb.Value.y * 255;
						//	specscol[2] = rgb.Value.z * 255;
						//}
					}
					else
					{
						if (entityinfo1.fakeplayer)
						{
							specscol[0] = g_Options.spec_color.r();
							specscol[1] = g_Options.spec_color.g();
							specscol[2] = g_Options.spec_color.b();
						}
						else
						{
							specscol[0] = 240;
							specscol[1] = 240;
							specscol[2] = 240;
						}
						spectators = std::string(entityinfo1.szName);
					}



					str[i] = spectators;
					gdsf += 1;

					render::draw_text_string(g_Draw.specpl1, false, m_vecPos.x + 4 + 1, m_vecPos.y + 25 - 20 + (m_iTotalSpectators++ * 20), specscol[0], specscol[1], specscol[2], 255, spectators.c_str());







					for (int i = 0; i < 100; i++)
					{
						size_t b = strlen(str[i].c_str());
						if (a > b)
							continue;

						a = b;
						c = i;

					}
					textsizespec = render::get_text_size(g_Draw.specpl1, str[c]);

				}
			}
		}



		Vector m_vecSize{};
		void SpectatorFrame()
		{

			if (g_Options.spectator_list)
			{


				static int x = 157;
				m_vecSize = { Vector(x , (int)m_iTotalSpectators * 20 ,0) };
				if (x + 42 < textsizespec.x)
				{
					x += textsizespec.x - x;
				}
				static auto vecPos = m_vecPos;

				static bool bDrag = false;





				std::string szName = ("SPECTATOR LIST" /*() + std::to_string(m_iTotalSpectators) + _(")*/);

				static int iHeight = 20;

				static int iHeight2 = 21;  // + 1

				int iWidth = m_vecSize.x + 42;


				int iWidth2 = m_vecSize.x + 43; // + 1

				int kappi = 0;


				int oke = m_iTotalSpectators % 2;

				if (oke == 0)
				{
					kappi * 18;
				}






				render::draw_filled_rect(m_vecPos.x, m_vecPos.y + 24 - 24, m_vecSize.x + 43, m_vecSize.y, Color(87, 87, 87, 255));



				for (int i = 0; i <= iHeight; i++)
				{


					g_VGuiSurface->set_drawing_color(65, 65, 65, 255);

					g_VGuiSurface->draw_line(m_vecPos.x + i, m_vecPos.y, m_vecPos.x + iHeight + i, m_vecPos.y - iHeight);
					g_VGuiSurface->draw_line(m_vecPos.x + iWidth + iHeight - i, m_vecPos.y - iHeight, m_vecPos.x + iWidth - i, m_vecPos.y);



					if (i == iHeight)
					{

						render::draw_filled_rect(m_vecPos.x + iHeight, m_vecPos.y - iHeight, iWidth - iHeight, iHeight + 1, Color(65, 65, 65, 255));
					}

				}
				static int specscol[3];

				specscol[0] = g_Options.spec_color.r();
				specscol[1] = g_Options.spec_color.g();
				specscol[2] = g_Options.spec_color.b();
				if (cMouse::IsOver(m_vecPos.x, m_vecPos.y - 20, iWidth, 20))
				{
					g_VGuiSurface->set_drawing_color(specscol[0], specscol[1], specscol[3], 255);
				}
				else
				{
					g_VGuiSurface->set_drawing_color(100, 100, 100, 255);
				}

				g_VGuiSurface->draw_line(m_vecPos.x, m_vecPos.y, m_vecPos.x + iHeight, m_vecPos.y - iHeight);//  Erste Schräge nach Rechts
				g_VGuiSurface->draw_line(m_vecPos.x + iHeight, m_vecPos.y - iHeight, m_vecPos.x + iWidth + iHeight + 1, m_vecPos.y - iHeight);  //Gerade Linie von Oben
				g_VGuiSurface->draw_line(m_vecPos.x + iWidth, m_vecPos.y, m_vecPos.x + iWidth + iHeight, m_vecPos.y - iHeight);//  Zweite Schräge nach Rechts
				g_VGuiSurface->draw_line(m_vecPos.x, m_vecPos.y, m_vecPos.x + iWidth, m_vecPos.y); // Gerade Linie von Unten
				render::draw_text_string(g_Draw.spec1, true, m_vecPos.x + (iWidth + iHeight) * 0.5 - 10, m_vecPos.y + 1 - iHeight + 2.5 + 1, g_Options.spec_color.r(), g_Options.spec_color.g(), g_Options.spec_color.b(), 255, szName.c_str());



				for (int i = 0; i < m_iTotalSpectators; i += 2) {
					render::draw_filled_rect(m_vecPos.x, m_vecPos.y + 1 + (i * 20) + kappi, m_vecSize.x + 43, 20, Color(52, 52, 52, 255));
				}
				kappi++;

			}

		}
	}
	void speclik1()
	{
		if (!g_Options.spectator_list)
			return;

		int x, y, w;
		g_EngineClient->GetScreenSize(x, y);
		static int iSpecsCount; // !!!!!!!
		int ladno = 10;

		if (g_Options.watermark)
			ladno += 40;
		if (g_Options.trackdispay)
			ladno += 50;
		

		if (iSpecsCount > 0)
			iSpecsCount = 0;
		for (int i = 0; i < g_EngineClient->GetMaxClients(); i++)
		{
			auto entity = C_BasePlayer::GetPlayerByIndex(i);
			if (!entity || entity->IsDormant() || entity->IsAlive()) continue;
			auto obs_target = (C_BasePlayer*)entity->m_hObserverTarget();

			player_info_t info;
			g_EngineClient->GetPlayerInfo(entity->EntIndex(), &info);
			std::string player_name = info.szName;

			if (!info.szName || info.ishltv || player_name.empty() || (obs_target != g_LocalPlayer && entity != g_LocalPlayer)) continue;

			//	str[i] = spectator;

			render::draw_text_string(x - 10 - render::get_text_size(g_Draw.velofont, player_name.c_str()).x, ladno + iSpecsCount++ * 15, g_Draw.velofont, player_name, false, Color(255, 255, 255));
		}
	}
	void spectators()
	{
		if (!g_Options.spectator_list)
			return;
		static int aboba{};
		static Vector vecSpecSize{};
		Vector vecSpecPos = Vector(g_Options.specposx * 2, g_Options.specposy * 2, 0.f);
		static int iSpecsCount; // !!!!!!!
		static int iTextWidth = 0;
		static int r, g, b, a = 0;
		Color targetcol;
		int width, height;
		g_EngineClient->GetScreenSize(width, height);

		vecSpecSize.y = 15.f + iSpecsCount * 15;
		//if (iTextWidth + 15 > 80)
		//	vecSpecSize.x = iTextWidth + 15;
		//else
		//	vecSpecSize.x = 80;
		//
		vecSpecSize.x = g_Options.specsizex + 80;

		if (aboba || Menu::Get().IsVisible())
		{
		
			render::draw_filled_rect(vecSpecPos.x - 1, vecSpecPos.y - 1, vecSpecSize.x + 2, vecSpecSize.y + 2, Color(15 / 255.f, 15 / 255.f, 15 / 255.f, 0.25f));
			render::gradient(vecSpecPos.x, vecSpecPos.y, vecSpecSize.x, vecSpecSize.y, Color(15 / 255.f, 15 / 255.f, 15 / 255.f, 1.f), Color(0.1f, 0.1f, 0.1f,1.f));
			render::draw_text_string(vecSpecPos.x + (vecSpecSize.x / 2.f), vecSpecPos.y , g_Draw.m_WatermarkFont, "spectators", true, Color(255, 255, 255));
		}
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

				r = 36;
				g = 36;
				b = 36;
				a = 255;
			}



			std::string mode;

			switch (eblanchik->observer())
			{
			case OBS_MODE_IN_EYE:
				mode = "f-p";
				break;
			case OBS_MODE_CHASE:
				mode = "t-p";
				break;
			case OBS_MODE_ROAMING:
				mode = "obs";
				break;
			case OBS_MODE_DEATHCAM:
				mode = "d-c";
				break;
			case OBS_MODE_FREEZECAM:
				mode = "f-c";
				break;
			case OBS_MODE_FIXED:
				mode = "fix";
				break;
			default:
				mode = "";
				break;
			}

			spectator +=  " | " u8"\n";

			if (render::get_text_size(g_Draw.spec, spectator).x > iTextWidth)
			{
			//	iTextWidth = render::get_text_size(g_Draw.spec, spectator).x;
			}
			render::draw_text_string(vecSpecPos.x + 5, vecSpecPos.y + 10 + iSpecsCount++ * 15, g_Draw.spec, spectator, false, Color(r, g, b, 255));
			if (!strstr(eblanchikinfo.szName, "GOTV") || !strstr(nnchikinfo.szName, "GOTV"))
				aboba += 1;
		}

	}

	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);

		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);
		static bool init = false;
		if (!init)
		{
			BOOL inj = FALSE;
			//STARTOLDHUD(inj);
			render::initialize();
			render::initialize();
			init = true;
		}
		static int x, y;
		int bx, by;
		g_EngineClient->GetScreenSize(bx, by);
		if (x != bx || y != by)
		{
			render::initialize();
			x = bx;
			y = by;
		}



		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				panelId = panel;
			}
		}
		else if (panelId == panel)
		{
			//Ignore 50% cuz it called very often
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;
		
			if (g_Options.spec_type == 0)
			{
				speclik1();
			}
			else if (g_Options.spec_type == 1)
			{
				spectators();
			}
			else if (g_Options.spec_type == 2)
			{
				iwebz::SpectatorFrame();
				if (iwebz::m_iTotalSpectators > 0)
					iwebz::m_iTotalSpectators = 0;
				iwebz::SpectatorList();
			}
			iwebz::m_vecPos = Vector(g_Options.specposx * 2, g_Options.specposy * 2, 0.f);

			Render::Get().BeginScene();

			Visuals::Get().ConsoleColor(panel);


		}
	}

	//--------------------------------------------------------------------------------
	void __fastcall hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
		static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);


		if (g_Options.autoaccept && !strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			if (fnAccept) {

				fnAccept("");

				//This will flash the CSGO window on the taskbar
				//so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}

		ofunc(g_EngineSound, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);

	}
	//--------------------------------------------------------------------------------
	int __fastcall hkDoPostScreenEffects(void* _this, int edx, int a1)
	{
		static auto oDoPostScreenEffects = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>(index::DoPostScreenSpaceEffects);

		if (g_LocalPlayer && g_Options.glow_enabled)
			Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, edx, a1);
	}


	//--------------------------------------------------------------------------------
	void __fastcall hkFrameStageNotify(void* _this, int edx, ClientFrameStage_t stage)
	{
		static auto backtrackInit = (Backtrack::init(), false);
		static auto ofunc = hlclient_hook.get_original<decltype(&hkFrameStageNotify)>(index::FrameStageNotify);

		if (g_EngineClient->IsInGame()) {

			if (g_Options.SkiNChanger) {

				if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
					skins::on_frame_stage_notify(false);

				if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
					skins::on_frame_stage_notify(true);

			}

			Backtrack::update(stage);
			Visuals::Get().playerModel(stage);
			Visuals::Get().Postprocess(stage);

		}


		ofunc(g_CHLClient, edx, stage);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);
		Visuals::Get().motion_blur(vsView);
		ofunc(g_ClientMode, edx, vsView);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkLockCursor(void* _this)
	{
		static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

		if (Menu::Get().IsVisible()) {
			g_VGuiSurface->unlock_cursor();
			g_InputSystem->ResetInputState();
			return;
		}
		ofunc(g_VGuiSurface);

	}
	//--------------------------------------------------------------------------------
	void __fastcall hkDrawModelExecute(void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>(index::DrawModelExecute);

		if (g_MdlRender->IsForcedMaterialOverride() &&
			!strstr(pInfo.pModel->szName, "arms") &&
			!strstr(pInfo.pModel->szName, "weapons/v_")) {
			return ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
		}
	
			static auto chams_init = (Chams::Get().initialize(), false);

		Chams::Get().run(ctx, state, pInfo, pCustomBoneToWorld);
		Chams::Get().run_bt(ctx, state, pInfo, pCustomBoneToWorld);


		ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}

	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "85 C0 75 30 38 87");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall *)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}

	void RecvProxy(const CRecvProxyData* pData, void* entity, void* output)
	{
		static auto ofunc = sequence_hook->get_original_function();

		if (g_LocalPlayer && g_LocalPlayer->IsAlive()) {
			const auto proxy_data = const_cast<CRecvProxyData*>(pData);
			const auto view_model = static_cast<C_BaseViewModel*>(entity);

			if (view_model && view_model->m_hOwner() && view_model->m_hOwner().IsValid()) {
				const auto owner = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntityFromHandle(view_model->m_hOwner()));
				if (owner == g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer())) {
					const auto view_model_weapon_handle = view_model->m_hWeapon();
					if (view_model_weapon_handle.IsValid()) {
						const auto view_model_weapon = static_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
						if (view_model_weapon) {
							if (k_weapon_info.count(view_model_weapon->m_Item().m_iItemDefinitionIndex())) {
								auto original_sequence = proxy_data->m_Value.m_Int;
								const auto override_model = k_weapon_info.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
								proxy_data->m_Value.m_Int = skins::GetNewAnimation(override_model, proxy_data->m_Value.m_Int);
							}
						}
					}
				}
			}

		}

		ofunc(pData, entity, output);
	}

	void __fastcall hkDrawModelExecute2(void* _this, int, void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags)
	{
		static auto ofunc = stdrender_hook.get_original<decltype(&hkDrawModelExecute2)>(index::DrawModelExecute2);

		if (g_StudioRender->IsForcedMaterialOverride())
			return ofunc(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		static auto flash = g_MatSystem->FindMaterial("effects/flashbang", TEXTURE_GROUP_CLIENT_EFFECTS);
		static auto flash_white = g_MatSystem->FindMaterial("effects/flashbang_white", TEXTURE_GROUP_CLIENT_EFFECTS);
		flash->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Options.no_flash);
		flash_white->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Options.no_flash);
		std::vector<const char*> vistasmoke_mats =

		{
				"particle/vistasmokev1/vistasmokev1_fire",
				"particle/vistasmokev1/vistasmokev1_smokegrenade",
				"particle/vistasmokev1/vistasmokev1_emods",
				"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		};

		for (auto mat_s : vistasmoke_mats)
		{
			IMaterial* mat = g_MatSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, g_Options.no_smoke);
		}

		
		ofunc(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		g_StudioRender->ForcedMaterialOverride(nullptr);
	}

	void __fastcall Hooks::RenderSmokeOverlay(void* _this, int, const bool unk)
	{
		static auto ofunc = viewrender_hook.get_original<decltype(&RenderSmokeOverlay)>(index::RenderSmokeOverlay);

		if (!g_Options.no_smoke)
			ofunc(g_ViewRender, 0, unk);
	}
}
