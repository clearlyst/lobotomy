#pragma once
#include "../CSGOSimple/valve_sdk/sdk.hpp"
#include "../CSGOSimple/valve_sdk/math/Vector2D.hpp"

enum font_flags {
	fontflag_none,
	fontflag_italic = 0x001,
	fontflag_underline = 0x002,
	fontflag_strikeout = 0x004,
	fontflag_symbol = 0x008,
	fontflag_antialias = 0x010,
	fontflag_gaussianblur = 0x020,
	fontflag_rotary = 0x040,
	fontflag_dropshadow = 0x080,
	fontflag_additive = 0x100,
	fontflag_outline = 0x200,
	fontflag_custom = 0x400,
	fontflag_bitmap = 0x800,
};

//inline int flPlayerAlpha[64];
class CDraw
{
public:
	void DrawOutlinedRect(int x0, int y0, int x1, int y1, int R, int G, int B, int A);
	unsigned long m_ESPFont, m_WatermarkFont, m_MenuFont, m_ListItemFont, velofont, spec, specpl, strelochka, canseeyoufont, AntohaFont;
	unsigned long m_ESPFont1, m_WatermarkFont1, m_MenuFont1, m_ListItemFont1, velofont1, spec1, specpl1, strelochka1, dina_font1, canseeyoufont1;
private:

};
inline CDraw g_Draw;
namespace render {
	void initialize();

	void draw_line(std::int32_t from_x, std::int32_t from_y, std::int32_t to_x, std::int32_t to_y, Color color);
	void draw_text_wchar(std::int32_t x, std::int32_t y, unsigned long font, const wchar_t* string, bool text_centered, Color colour);
	void draw_text_string(std::int32_t x, std::int32_t y, unsigned long font, std::string string, bool text_centered, Color colour);
	void DrawStringA(unsigned long font, bool center, int x, int y, int r, int g, int b, int a, const char* input, ...);
	void draw_text_string(unsigned long font, bool text_centered, std::int32_t x, std::int32_t y, int r, int g, int b, int a, std::string string);

	//void text(std::int32_t x, std::int32_t y, unsigned long font, const wchar_t *text, bool centered, Color color);
	//void text(std::int32_t x, std::int32_t y, unsigned long font, std::string text, bool centered, Color color);
	void draw_rect(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, Color color);
	void draw_filled_rect(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, Color color);
	void draw_outline(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, Color color);
	void draw_textured_polygon(std::int32_t vertices_count, Vertex_t* vertices, Color color);
	void draw_circle(std::int32_t x, std::int32_t y, std::int32_t radius, std::int32_t segments, Color color);
	Vector2D get_text_size(unsigned long font, std::string text);
	void draw_fade(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, Color color1, Color color2, bool horizontal);
	void gradient(int x, int y, int w, int h, Color c1, Color c2);
	namespace fonts {
		extern unsigned long watermark_font;
		extern unsigned long watermark_font_m;
		extern unsigned long watermark_font_ns;		// No shadow
		extern unsigned long playername_font;		// wchar support
		extern unsigned long weapon_icon_font;		// No worky
		extern unsigned long dina_font;				// A personal favourite https://www.dcmembers.com/jibsen/download/61/
	};
}