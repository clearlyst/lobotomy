#include "render.hpp"

#include <mutex>

#include "features/icons.h"
#include "../CSGOSimple/features/fonts.h"
#include "features/visuals.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "helpers/math.hpp"
#include "renderer.h"
#include "abobik.h"
#include "features/crypt_str.hpp"
#include "imgui/freetype/imgui_freetype.h"
//#include "imgui/freetype/imgui_freetype.h"
IDirect3DTexture9* legit_image = nullptr;
IDirect3DTexture9* rage_image = nullptr;
IDirect3DTexture9* visuals_image = nullptr;
IDirect3DTexture9* players_image = nullptr;
IDirect3DTexture9* misc_image = nullptr;
IDirect3DTexture9* settings_image = nullptr;
ImFont* win95;
IDirect3DTexture9* tImage = nullptr;
IDirect3DTexture9* Logotype = nullptr;
ImDrawListSharedData _data;

std::mutex render_mutex;



void Render::Initialize()
{
	ImGui::CreateContext();
	//if (tImage == nullptr) D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &kaban, sizeof(kaban), 1280, 720, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &tImage);

	ImGui_ImplWin32_Init(InputSys::Get().GetMainWindow());
	ImGui_ImplDX9_Init(g_D3DDevice9);

	draw_list = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_act = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_rendering = new ImDrawList(ImGui::GetDrawListSharedData());

	GetFonts();
}


void Render::GetFonts() {
	
	char windows_directory[MAX_PATH];
	GetSystemWindowsDirectoryA(windows_directory, MAX_PATH);

	constexpr ImWchar ranges[] = { 0xE000, 0xF8FF, 0 };

	std::string tahoma_bold_font_directory = (std::string)windows_directory + ("\\Fonts\\verdanab.ttf");
	std::string tahoma_font_directory = (std::string)windows_directory + ("\\Fonts\\verdana.ttf");

	ImFontConfig menufontconfig;
	menufontconfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting | ImGuiFreeTypeBuilderFlags_Bitmap;
	menufontconfig.PixelSnapH = true;
	menufontconfig.SizePixels = 9.0f;
	menufontconfig.RasterizerMultiply = 1.0f;

	ImFontConfig icon_config;
	icon_config.SizePixels = 12.0f;
	icon_config.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_NoHinting;

	ImFontConfig scene_font_flag;
	ImFontConfig sub_scene_flag;
	ImFontConfig indicators_font_flag;
	ImFontConfig sub_indicators_flag;
	ImFontConfig esp_font_flag;
	ImFontConfig sub_esp_flag;
	ImFontConfig debug_information_flag;

	Menu::Get().switch_font_cfg(scene_font_flag, g_Options.fonts.scene_flag);
	Menu::Get().switch_font_cfg(sub_scene_flag, g_Options.fonts.scene_sub_flag);
	Menu::Get().switch_font_cfg(indicators_font_flag, g_Options.fonts.indi_font_flag);
	Menu::Get().switch_font_cfg(sub_indicators_flag, g_Options.fonts.sub_indi_font_flag);
	Menu::Get().switch_font_cfg(esp_font_flag, g_Options.fonts.esp_flag);
	Menu::Get().switch_font_cfg(sub_esp_flag, g_Options.fonts.esp_sub_flag);
	Menu::Get().switch_font_cfg(debug_information_flag, g_Options.fonts.debug_information_flag);

	fonts::menu_font_thin = ImGui::GetIO().Fonts->AddFontFromFileTTF(tahoma_font_directory.c_str(), menufontconfig.SizePixels, &menufontconfig, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	fonts::menu_font_bold = ImGui::GetIO().Fonts->AddFontFromFileTTF(tahoma_bold_font_directory.c_str(), menufontconfig.SizePixels, &menufontconfig, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	fonts::scene_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fonts::font_directory_scene.c_str(), g_Options.fonts.scene_size, &scene_font_flag, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	fonts::sub_scene_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fonts::font_directory_sub_scene.c_str(), g_Options.fonts.scene_sub_size, &sub_scene_flag, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	fonts::indicator_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fonts::font_directory_indicator.c_str(), g_Options.fonts.indi_size, &indicators_font_flag, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	fonts::sub_indicator_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fonts::font_directory_sub_indicator.c_str(), g_Options.fonts.sub_indi_size, &sub_indicators_flag, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	fonts::esp_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fonts::font_directory_esp.c_str(), g_Options.fonts.esp_size, &esp_font_flag, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	fonts::sub_esp_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fonts::font_directory_sub_esp.c_str(), g_Options.fonts.esp_sub_size, &sub_esp_flag, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	fonts::icon_font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(icon_font, icon_font_size, icon_config.SizePixels, &icon_config, ranges);
	fonts::debug_information_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fonts::font_directory_debug_information.c_str(), g_Options.fonts.debug_information_size, &debug_information_flag, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	win95 = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontik, sizeof(fontik), 14.f, &menufontconfig, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	ImGuiFreeType::BuildFontAtlas(ImGui::GetIO().Fonts);
	if (!Logotype)
		D3DXCreateTextureFromFileInMemory(g_D3DDevice9, &smile, sizeof(smile), &Logotype);

	if (legit_image == nullptr)D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9
		, &legit, sizeof(legit),
		20, 20, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &legit_image);
	if (rage_image == nullptr)D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9
		, &rage, sizeof(rage),
		20, 20, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &rage_image);
	if (visuals_image == nullptr)D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9
		, &visuals, sizeof(visuals),
		20, 20, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &visuals_image);
	if (players_image == nullptr)D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9
		, &players, sizeof(players),
		20, 20, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &players_image);
	if (misc_image == nullptr)D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9
		, &misc, sizeof(misc),
		20, 20, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &misc_image);
	if (settings_image == nullptr)D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9
		, &settings, sizeof(settings),
		20, 20, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &settings_image);

}

void Render::ClearDrawList() {
	render_mutex.lock();
	draw_list_act->Clear();
	render_mutex.unlock();
}

#include "../../features/Notify.h"
void Render::BeginScene() {
	draw_list->Clear();
	draw_list->PushClipRectFullScreen();
	//RenderText("oeaofdsof", ImVec2(10, 10), 22.f,Color(255,255,255));
	
	Visuals::Get().Velocity();
	Visuals::Get().stamina();
	Visuals::Get().RenderIndicators();
	//Visuals::Get().velo_graph();
	if (g_Options.hitinfo)
	Visuals::Get().paint_traverse();
	BunnyHop::PixelFinder(nullptr);
	BunnyHop::pxcalcrender();
	BunnyHop::renderCheckSurf();
//	Visuals::Get().RenderIndicators();
	if ( g_LocalPlayer)
		Visuals::Get().AddToDrawList();
	Visuals::Get().DrawHitmarker();
	render_mutex.lock();
	*draw_list_act = *draw_list;
	render_mutex.unlock();

}

ImDrawList* Render::RenderScene() {

	if (render_mutex.try_lock()) {
		*draw_list_rendering = *draw_list_act;
		render_mutex.unlock();
	}

	return draw_list_rendering;
}


float Render::RenderText(const std::string& text, ImVec2 pos, float size, Color color, bool center, bool outline, ImFont* pFont,bool dropshadow)
{
	ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
	if (!pFont->ContainerAtlas) return 0.f;
	draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

	if (center)
		pos.x -= textSize.x / 2.0f;

	if (dropshadow)
	{
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());

	}
	if (outline) {
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
	}

	draw_list->AddText(pFont, size, pos, GetU32(color), text.c_str());

	draw_list->PopTextureID();

	return pos.y + textSize.y;
}

void Render::arc(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness) {
	draw_list->PathArcTo(ImVec2(x, y), radius, DEG2RAD(min_angle), DEG2RAD(max_angle), 32);
	draw_list->PathStroke(GetU32(col), false, thickness);
}
void Render::filled_triangle(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness) {
	ImVec2 center(x, y);
	int num_segments = 32; // You can adjust the number of segments as needed.

	// Calculate the angle step between segments.
	float angle_step = (max_angle - min_angle) / num_segments;

	// Create an array to hold the vertices of the triangle fan.
	std::vector<ImVec2> vertices;

	// Add the center point as the first vertex.
	vertices.push_back(center);

	// Calculate the vertices around the arc.
	for (int i = 0; i <= num_segments; ++i) {
		float angle = min_angle + angle_step * i;
		ImVec2 vertex(center.x + radius * cosf(angle), center.y + radius * sinf(angle));
		vertices.push_back(vertex);
	}

	// Draw the filled triangle fan.
	for (int i = 2; i < vertices.size(); ++i) {
		draw_list->AddTriangleFilled(vertices[0], vertices[i - 1], vertices[i], GetU32(col));
	}
}
void Render::triangle(float x, float y, float size, float rotation, Color col, float thickness) {
	float halfSize = size / 2.f;
	float radRotation = DEG2RAD(rotation);

	// Вычисляем вершины треугольника
	ImVec2 p1(x + halfSize * cos(radRotation), y + halfSize * sin(radRotation));
	ImVec2 p2(x + halfSize * cos(radRotation + 2.f * M_PI / 3.f), y + halfSize * sin(radRotation + 2.f * M_PI / 3.f));
	ImVec2 p3(x + halfSize * cos(radRotation - 2.f * M_PI / 3.f), y + halfSize * sin(radRotation - 2.f * M_PI / 3.f));

	// Рисуем треугольник
	draw_list->PathClear();
	draw_list->PathLineTo(p1);
	draw_list->PathLineTo(p2);
	draw_list->PathLineTo(p3);
	draw_list->PathLineTo(p1);

	draw_list->PathStroke(GetU32(col), true, thickness);
}
void Render::RenderCircle3D(Vector position, float points, float radius, Color color)
{
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
	{
		Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
		Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

		Vector start2d, end2d;
		if (g_DebugOverlay->ScreenPosition(start, start2d) || g_DebugOverlay->ScreenPosition(end, end2d))
			return;

		RenderLine(start2d.x, start2d.y, end2d.x, end2d.y, color);
	}
}

void fonts::reset_fonts()
{
	font_directory_scene = "C:/windows/fonts/tahomabd.ttf";
	font_directory_sub_scene = "C:/windows/fonts/tahomabd.ttf";
	font_directory_indicator = "C:/windows/fonts/tahomabd.ttf";
	font_directory_sub_indicator = "C:/windows/fonts/tahomabd.ttf";
	font_directory_esp = "C:/windows/fonts/tahoma.ttf";
	font_directory_sub_esp = "C:/windows/fonts/tahoma.ttf";
	font_directory_debug_information = "C:/windows/fonts/tahomabd.ttf";

	g_Options.fonts.indi_font = 0;
	g_Options.fonts.sub_indi_font = 0;
	g_Options.fonts.esp_font = 0;
	g_Options.fonts.esp_sub_font = 0;
	g_Options.fonts.scene_font = 0;
	g_Options.fonts.scene_sub_font = 0;
	g_Options.fonts.debug_information_font = 0;
	g_Options.fonts.indi_size = 28;
	g_Options.fonts.sub_indi_size = 9;
	g_Options.fonts.esp_size = 9;
	g_Options.fonts.esp_sub_size = 9;
	g_Options.fonts.scene_size = 12;
	g_Options.fonts.scene_size = 12;
	g_Options.fonts.debug_information_size = 32;
}
