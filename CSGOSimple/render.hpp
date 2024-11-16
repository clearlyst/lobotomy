#pragma once
#include <string>
#include <sstream>
#include <stdint.h>
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

#include "singleton.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"

#include "valve_sdk/misc/Color.hpp"
extern IDirect3DTexture9* legit_image;
extern IDirect3DTexture9* rage_image;
extern IDirect3DTexture9* visuals_image;
extern IDirect3DTexture9* players_image;
extern IDirect3DTexture9* misc_image;
extern IDirect3DTexture9* settings_image;

extern IDirect3DTexture9* tImage;
extern IDirect3DTexture9* Logotype;
extern ImFont* win95;

class Vector;
namespace fonts {
	void reset_fonts();

	inline ImFont* menu_font_thin;
	inline ImFont* menu_font_bold;
	inline ImFont* scene_font;
	inline ImFont* sub_scene_font;
	inline ImFont* indicator_font;
	inline ImFont* sub_indicator_font;
	inline ImFont* esp_font;
	inline ImFont* sub_esp_font;
	inline ImFont* icon_font;
	inline ImFont* debug_information_font;

	inline std::string font_directory_scene = "C:/windows/fonts/tahomabd.ttf";
	inline std::string font_directory_sub_scene = "C:/windows/fonts/tahomabd.ttf";
	inline std::string font_directory_indicator = "C:/windows/fonts/tahomabd.ttf";
	inline std::string font_directory_sub_indicator = "C:/windows/fonts/tahoma.ttf";
	inline std::string font_directory_esp = "C:/windows/fonts/tahoma.ttf";
	inline std::string font_directory_sub_esp = "C:/windows/fonts/tahoma.ttf";
	inline std::string font_directory_debug_information = "C:/windows/fonts/tahomabd.ttf";

	inline int selected_font_index_main_scene = -1;
	inline int selected_font_index_sub_scene = -1;
	inline int selected_font_index_main_indi = -1;
	inline int selected_font_index_sub_indi = -1;
	inline int selected_font_index_main_esp = -1;
	inline int selected_font_index_sub_esp = -1;
	inline int selected_font_index_debug_information = -1;
}
class Render
	: public Singleton<Render>
{
	friend class Singleton<Render>;

private:
	ImDrawList * draw_list_act;
	ImDrawList * draw_list_rendering;
	ImDrawList* draw_list;
	ImDrawData draw_data;

	ImU32 GetU32(Color _color)
	{
		return ((_color[3] & 0xff) << 24) + ((_color[2] & 0xff) << 16) + ((_color[1] & 0xff) << 8)
			+ (_color[0] & 0xff);
	}
public:
	void Initialize();
	void GetFonts();
	void ClearDrawList();
	void BeginScene();
	ImDrawList* RenderScene();

	//float RenderText(const std::string& text, ImVec2 position, float size, Color color, bool center = false, bool outline2 = true, bool outline = false, ImFont* pFont = g_pDefaultFont);

	float RenderText(const std::string& text, ImVec2 position, float size, Color color, bool center = false, bool outline = true, ImFont* pFont = fonts::indicator_font,bool dropshadow = false);

	void arc(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness);

	void filled_triangle(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness);

	void triangle(float x, float y, float size, float rotation, Color col, float thickness);

	void RenderCircle3D(Vector position, float points, float radius, Color color);

	void RenderImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a = ImVec2(0, 0), const ImVec2& uv_b = ImVec2(1, 1), ImU32 col = 0xFFFFFFFF) {
		draw_list->AddImage(user_texture_id, a, b, uv_a, uv_b, col);
	}
	inline void Triangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const Color& color)
	{
		draw_list->AddTriangleFilled(p1, p2, p3, GetU32(color));
	}
	template <class T>
	inline void RenderBoxByType(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, int type = 0) {
		if (type == 0)
			RenderBox(x1, y1, x2, y2, color, thickness);
		else if (type == 1)
			RenderCoalBox(x1, y1, x2, y2, color);
		else if (type == 2)
			RenderBox(x1, y1, x2, y2, color, thickness, 8.f);
	}

	template <class T>
	inline void RenderBoxFilledByType(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, int type = 0) {
		if (type == 0 || type == 1)
			RenderBoxFilled(x1, y1, x2, y2, color, thickness);
		else if (type == 2)
			RenderBoxFilled(x1, y1, x2, y2, color, thickness, 8.f);
	}

	template <class T>
	inline void RenderCoalBox(T x1, T y1, T x2, T y2, Color color, float th = 1.f) {
		int w = x2 - x1;
		int h = y2 - y1;

		int iw = w / 4;
		int ih = h / 4;
		// top
		RenderLine(x1, y1, x1 + iw, y1, color, th);					// left
		RenderLine(x1 + w - iw, y1, x1 + w, y1, color, th);			// right
		RenderLine(x1, y1, x1, y1 + ih, color, th);					// top left
		RenderLine(x1 + w - 1, y1, x1 + w - 1, y1 + ih, color, th);	// top right
																	// bottom
		RenderLine(x1, y1 + h, x1 + iw, y1 + h, color, th);			// left
		RenderLine(x1 + w - iw, y1 + h, x1 + w, y1 + h, color, th);	// right
		RenderLine(x1, y1 + h - ih, x1, y1 + h, color, th);			// bottom left
		RenderLine(x1 + w - 1, y1 + h - ih, x1 + w - 1, y1 + h, color, th);	// bottom right
	}

	template <class T>
	inline void RenderBox(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, float rounding = 0.f) {
		draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), GetU32(color), rounding, 15, thickness);
	}
	inline void RenderBox(RECT r, Color color,float thickness = 1.f, float rounding = 0.f) {
		RenderBox(r.left, r.top, r.right, r.bottom, color, thickness, rounding);
	}
	template <class T>
	inline void RenderBoxFilled(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, float rounding = 0.f) {
		draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), GetU32(color), rounding, 15);
	}
	template <class T>
	inline void RenderLine(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f) {
		draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), GetU32(color), thickness);
	}

	inline void RenderGradientBox(const float x, const float y, const float w, const float h, const Color& colUprLeft, const Color& colUprRight, const Color& colBotRight, const Color& colBotLeft) {
		draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + w, y + h), GetU32(colUprLeft), GetU32(colUprRight), GetU32(colBotRight), GetU32(colBotLeft));
	}
	template <class T>
	inline float RenderText(const std::string& text, T x, T y, float size, Color clr, bool center = false, bool outline = true, ImFont* pFont = fonts::indicator_font) {
		return RenderText(text, ImVec2(x, y), size, clr, center, outline, pFont);
	}
	template <class T>
	inline void RenderCircle(T x, T y, float radius, int points, Color color, float thickness = 1.f) {
		draw_list->AddCircle(ImVec2(x, y), radius, GetU32(color), points, thickness);
	}
	template <class T>
	inline void RenderCircleFilled(T x, T y, float radius, int points, Color color) {
		draw_list->AddCircleFilled(ImVec2(x, y), radius, GetU32(color), points);
	}
	inline ImVec2 GetTextSize(std::string text, ImFont* font, float size)
	{
		return font->CalcTextSizeA(size, FLT_MAX, NULL, text.c_str());
	}
	inline void RenderPolygonFilled(const ImVec2* points, int num_points, Color color) {
		draw_list->AddConvexPolyFilled(points, num_points, GetU32(color));
	}
};
inline void lolrendertext(const float x, const float y, const float fontSize, ImFont* font, const std::string& text, const bool centered, const Color& color, const bool dropShadow, const bool outline)
{

	Render::Get().RenderText(text, ImVec2(x, y), fontSize, color, centered, outline, font, dropShadow);
}
#include <codecvt>
#include <string>

inline std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}