#include "renderer.h"
#include <urlmon.h>
#include <filesystem>
#pragma comment(lib, "urlmon.lib")



unsigned long render::fonts::watermark_font;
unsigned long render::fonts::watermark_font_m;		// Medium size
unsigned long render::fonts::watermark_font_ns;		// No shadow
unsigned long render::fonts::playername_font;		// wchar support
unsigned long render::fonts::weapon_icon_font;		// No worky
unsigned long render::fonts::dina_font;				// Love it 2 much
extern char buf[64];
extern int l, ll, lll, llll, lllll, llllll, lllllll, llllllll, lllllllll;


void render::initialize() {


	render::fonts::watermark_font = g_VGuiSurface->font_create();
	render::fonts::watermark_font_m = g_VGuiSurface->font_create();
	render::fonts::watermark_font_ns = g_VGuiSurface->font_create();
	render::fonts::playername_font = g_VGuiSurface->font_create();
	render::fonts::weapon_icon_font = g_VGuiSurface->font_create();
	render::fonts::dina_font = g_VGuiSurface->font_create();

	g_VGuiSurface->set_font_glyph(render::fonts::watermark_font, "Verdana", 12, 500, 0, 0, font_flags::fontflag_dropshadow);
	g_VGuiSurface->set_font_glyph(render::fonts::watermark_font_m, "Tahoma", 14, 500, 0, 0, font_flags::fontflag_dropshadow);
	g_VGuiSurface->set_font_glyph(render::fonts::watermark_font_ns, "Tahoma", 12, 500, 0, 0, 0);
	g_VGuiSurface->set_font_glyph(render::fonts::playername_font, "Smallest Pixel-7", 11, 500, 0, 0, fontflag_dropshadow);

	// Other
	g_VGuiSurface->set_font_glyph(render::fonts::weapon_icon_font, "Tahoma", 13, FW_MEDIUM, 0, 0, fontflag_outline);			// No worky (it broky)
	g_VGuiSurface->set_font_glyph(render::fonts::playername_font, "Tahoma", 12, 5000, 0, 0, fontflag_dropshadow | fontflag_antialias);						// A personal favourite https://www.dcmembers.com/jibsen/download/61/
	g_Draw.m_MenuFont = g_VGuiSurface->font_create();
	//g_VGuiSurface->SetFontGlyphSet(m_MenuFont, /*Visitor TT2 BRK*/XorStr<0x83,16,0x5F987AA5>("\xD5\xED\xF6\xEF\xF3\xE7\xFB\xAA\xDF\xD8\xBF\xAE\xCD\xC2\xDA"+0x5F987AA5).s, 15, 500, 0, 0, 0x200); // 15
	g_VGuiSurface->set_font_glyph(g_Draw.m_MenuFont, "Tahoma", 11, 500, 0, 0, fontflag_outline); // 15
	g_Draw.strelochka = g_VGuiSurface->font_create();
	//g_VGuiSurface->SetFontGlyphSet(m_MenuFont, /*Visitor TT2 BRK*/XorStr<0x83,16,0x5F987AA5>("\xD5\xED\xF6\xEF\xF3\xE7\xFB\xAA\xDF\xD8\xBF\xAE\xCD\xC2\xDA"+0x5F987AA5).s, 15, 500, 0, 0, 0x200); // 15
	g_VGuiSurface->set_font_glyph(g_Draw.strelochka, "Smallest Pixel-7", 11, FW_MEDIUM, 0, 0, fontflag_outline); // 15

	g_Draw.m_ListItemFont = g_VGuiSurface->font_create();
	//g_VGuiSurface->SetFontGlyphSet(m_ListItemFont, /*Visitor TT2 BRK*/XorStr<0x83,16,0x5F987AA5>("\xD5\xED\xF6\xEF\xF3\xE7\xFB\xAA\xDF\xD8\xBF\xAE\xCD\xC2\xDA"+0x5F987AA5).s, 13, 500, 0, 0, 0x010); // 13
	g_VGuiSurface->set_font_glyph(g_Draw.m_ListItemFont, "Small Fonts", 11, FW_DONTCARE, 0, 0, fontflag_dropshadow);
	g_Draw.m_ESPFont = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.m_ESPFont, /*Calibri*/"Tahoma", 13, 5000, 0, 0, fontflag_outline);
	g_Draw.velofont = g_VGuiSurface->font_create();

	g_Draw.canseeyoufont = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.canseeyoufont, /*Calibri*/"Tahoma", 12, 5000, 0, 0, fontflag_dropshadow);

	g_VGuiSurface->set_font_glyph(g_Draw.velofont, /*Calibri*/"Tahoma", 11, 5000, 0, 0, fontflag_dropshadow);
	g_Draw.m_WatermarkFont = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.m_WatermarkFont, "Verdana Bold", 11, 2000, 0, 0, fontflag_dropshadow);
	g_Draw.spec = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.spec, "Tahoma", 11, 500, 0, 0, fontflag_dropshadow);
	g_Draw.specpl = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.specpl, "Tahoma", 11, 2000, 0, 0, 512 | 128);
	g_Draw.AntohaFont = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.AntohaFont, "Verdana Bold", 28, 2000, 0, 0, 0);





	g_Draw.m_MenuFont1 = g_VGuiSurface->font_create();
	//g_VGuiSurface->SetFontGlyphSet(m_MenuFont, /*Visitor TT2 BRK*/XorStr<0x83,16,0x5F987AA5>("\xD5\xED\xF6\xEF\xF3\xE7\xFB\xAA\xDF\xD8\xBF\xAE\xCD\xC2\xDA"+0x5F987AA5).s, 15, 500, 0, 0, 0x200); // 15
	g_VGuiSurface->set_font_glyph(g_Draw.m_MenuFont1, "Tahoma", 16, 600, 0, 0, 512); // 15
	g_Draw.strelochka1 = g_VGuiSurface->font_create();
	//g_VGuiSurface->SetFontGlyphSet(m_MenuFont, /*Visitor TT2 BRK*/XorStr<0x83,16,0x5F987AA5>("\xD5\xED\xF6\xEF\xF3\xE7\xFB\xAA\xDF\xD8\xBF\xAE\xCD\xC2\xDA"+0x5F987AA5).s, 15, 500, 0, 0, 0x200); // 15
	g_VGuiSurface->set_font_glyph(g_Draw.strelochka1, "Verdana", 14, 800, 0, 0, 0); // 15

	g_Draw.m_ListItemFont1 = g_VGuiSurface->font_create();
	//g_VGuiSurface->set_font_glyph(g_Draw.m_ListItemFont, /*Visitor TT2 BRK*/<0x83,16,0x5F987AA5>("\xD5\xED\xF6\xEF\xF3\xE7\xFB\xAA\xDF\xD8\xBF\xAE\xCD\xC2\xDA"+0x5F987AA5).s, 13, 500, 0, 0, 0x010); // 13
	//g_VGuiSurface->set_font_glyph(g_Draw.m_ListItemFont, "Tahoma", 12, 5000, 0, 0, 0x080);
	g_Draw.m_ESPFont1 = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.m_ESPFont1, /*Calibri*/"Tahoma", 13, FW_DONTCARE, 0, 0, 0x200);
	g_Draw.velofont1 = g_VGuiSurface->font_create();

	g_Draw.canseeyoufont1 = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.canseeyoufont1, /*Calibri*/"Tahoma", 25, 500, 0, 0, 0x200);

	g_VGuiSurface->set_font_glyph(g_Draw.velofont1, /*Calibri*/"Verdana Bold", 32, 900, 0, 0, 0x010 | 0x080);
	g_Draw.m_WatermarkFont1 = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.m_WatermarkFont1, "Tahoma", 20, FW_DONTCARE, 0, 0, 0x200);
	g_Draw.spec1 = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.spec1, "Tahoma", 13, 500, 0, 0, 512 | 128);
	g_Draw.specpl1 = g_VGuiSurface->font_create();
	g_VGuiSurface->set_font_glyph(g_Draw.specpl1, "Tahoma", 11, 2000, 0, 0, 512 | 128);

}
void CDraw::DrawOutlinedRect(int x0, int y0, int x1, int y1, int R, int G, int B, int A)
{
	int BoxWidth = x1 - x0;
	int BoxHeight = y1 - y0;

	if (BoxWidth < 10) BoxWidth = 10;
	if (BoxHeight < 15) BoxHeight = 15;


	g_VGuiSurface->set_drawing_color(R, G, B, A);

	g_VGuiSurface->draw_filled_rectangle1(x0, y0, x0 + (BoxWidth / 5), y0 + 1); //left top
	g_VGuiSurface->draw_filled_rectangle1(x0, y0, x0 + 1, y0 + (BoxHeight / 6)); //left top

	g_VGuiSurface->draw_filled_rectangle1(x1 - (BoxWidth / 5) + 1, y0, x1, y0 + 1); //right top
	g_VGuiSurface->draw_filled_rectangle1(x1, y0, x1 + 1, y0 + (BoxHeight / 6)); //right top

	g_VGuiSurface->draw_filled_rectangle1(x0, y1, x0 + (BoxWidth / 5), y1 + 1); //left bottom
	g_VGuiSurface->draw_filled_rectangle1(x0, y1 - (BoxHeight / 6) + 1, x0 + 1, y1 + 1); //left bottom

	g_VGuiSurface->draw_filled_rectangle1(x1 - (BoxWidth / 5) + 1, y1, x1, y1 + 1); //right bottom
	g_VGuiSurface->draw_filled_rectangle1(x1, y1 - (BoxHeight / 6) + 1, x1 + 1, y1 + 1); //right bottom

	int HealthWidth = ((x1 - x0) - 2);
	int HealthHeight = (y1 - y0) - 2;
	//DrawGUIBoxHorizontal( x0, y0 - (HealthHeight + 5), HealthWidth, HealthHeight, R, G, B, Health );
}
void render::draw_line(std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, Color colour) {
	g_VGuiSurface->set_drawing_color(colour.r(), colour.g(), colour.b(), colour.a());
	g_VGuiSurface->draw_line(x1, y1, x2, y2);
}

// multi-byte -> wide
__forceinline std::wstring MultiByteToWide(const std::string& str) {
	std::wstring    ret;
	int		        str_len;

	// check if not empty str
	if (str.empty())
		return {};

	// count size
	str_len = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);

	// setup return value
	ret = std::wstring(str_len, 0);

	// final conversion
	MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &ret[0], str_len);

	return ret;
}
void render::draw_text_string(std::int32_t x, std::int32_t y, unsigned long font, std::string string, bool text_centered, Color colour) {
	if (wchar_t name[128]; MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, name, 128)) {

		int width, height;
		g_VGuiSurface->get_text_size(font, name, width, height);

		g_VGuiSurface->set_text_color(colour.r(), colour.g(), colour.b(), colour.a());
		g_VGuiSurface->draw_text_font(font);

		if (text_centered) g_VGuiSurface->draw_text_pos(x - (width / 2), y);
		else g_VGuiSurface->draw_text_pos(x, y);

		g_VGuiSurface->draw_render_text(name, wcslen(name));
	}
}
int getWidht(unsigned long font, const char* input)
{
	INT iWide = 0;
	INT iTall = 0;
	INT iBufSize = MultiByteToWideChar(CP_UTF8, 0x0, input, -1, NULL, 0);

	wchar_t* pszUnicode = new wchar_t[iBufSize];

	MultiByteToWideChar(CP_UTF8, 0x0, input, -1, pszUnicode, iBufSize);

	g_VGuiSurface->get_text_size(font, pszUnicode, iWide, iTall);

	delete[] pszUnicode;

	return iWide;
}

std::wstring stringToWide(const std::string& text)
{
	std::wstring wide(text.length(), L' ');
	std::copy(text.begin(), text.end(), wide.begin());

	return wide;
}
void render::DrawStringA(unsigned long font, bool center, int x, int y, int r, int g, int b, int a, const char* input, ...)
{
	CHAR szBuffer[MAX_PATH];

	if (!input)
		return;

	vsprintf(szBuffer, input, (char*)&input + _INTSIZEOF(input));

	if (center)
		x -= getWidht(font, szBuffer) / 2;

	g_VGuiSurface->set_text_color(r, g, b, a);
	g_VGuiSurface->draw_text_font(font);
	g_VGuiSurface->draw_text_pos(x, y);
	std::wstring wide = stringToWide(std::string(szBuffer));
	g_VGuiSurface->draw_render_text(wide.c_str(), wide.length());
}

void wstring(int x, int y, Color color, const std::wstring& text, bool text_centred, unsigned long font) {
	int w, h;

	g_VGuiSurface->get_text_size(font, text.c_str(), w, h);
	g_VGuiSurface->draw_text_font(font);
	g_VGuiSurface->set_text_color(color.r(), color.g(), color.b(), color.a());

	if (text_centred) g_VGuiSurface->draw_text_pos(x - (w / 2), y);
	else g_VGuiSurface->draw_text_pos(x, y);


	g_VGuiSurface->draw_render_text(text.c_str(), (int)text.size());
}
void render::draw_text_string(unsigned long font, bool text_centered, std::int32_t x, std::int32_t y, int r, int g, int b, int a, std::string string)
{

	wstring(x, y, Color(r, g, b, a), MultiByteToWide(string), text_centered, font);

}

void render::draw_text_wchar(std::int32_t x, std::int32_t y, unsigned long font, const wchar_t* string, bool text_centered, Color colour) {
	int width, height;
	g_VGuiSurface->get_text_size(font, string, width, height);

	g_VGuiSurface->set_text_color(colour.r(), colour.g(), colour.b(), colour.a());
	g_VGuiSurface->draw_text_font(font);

	if (text_centered) g_VGuiSurface->draw_text_pos(x - (width / 2), y);
	else g_VGuiSurface->draw_text_pos(x, y);

	g_VGuiSurface->draw_render_text(string, wcslen(string));
}

void render::draw_rect(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, Color color) {
	g_VGuiSurface->set_drawing_color(color.r(), color.g(), color.b(), color.a());
	g_VGuiSurface->draw_outlined_rect(x, y, width, height);
}

void render::draw_filled_rect(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, Color color) {
	g_VGuiSurface->set_drawing_color(color.r(), color.g(), color.b(), color.a());
	g_VGuiSurface->draw_filled_rectangle(x, y, width, height);
}

void render::draw_outline(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, Color color) {
	g_VGuiSurface->set_drawing_color(color.r(), color.g(), color.b(), color.a());
	g_VGuiSurface->draw_outlined_rect(x, y, width, height);
}

void render::draw_textured_polygon(std::int32_t vertices_count, Vertex_t* vertices, Color color) {
	static unsigned char buf[4] = { 255, 255, 255, 255 };
	unsigned int texture_id{};
	if (!texture_id) {
		texture_id = g_VGuiSurface->create_new_texture_id(true);
		g_VGuiSurface->set_texture_rgba(texture_id, buf, 1, 1);
	}
	g_VGuiSurface->set_drawing_color(color.r(), color.g(), color.b(), color.a());
	g_VGuiSurface->set_texture(texture_id);
	g_VGuiSurface->draw_polygon(vertices_count, vertices);
}

void render::draw_circle(std::int32_t x, std::int32_t y, std::int32_t radius, std::int32_t segments, Color color) {
	float step = 3.14 * 2.0 / segments;
	for (float a = 0; a < (3.14 * 2.0); a += step) {
		float x1 = radius * cos(a) + x;
		float y1 = radius * sin(a) + y;
		float x2 = radius * cos(a + step) + x;
		float y2 = radius * sin(a + step) + y;
		g_VGuiSurface->set_drawing_color(color.r(), color.g(), color.b(), color.a());
		g_VGuiSurface->draw_line(x1, y1, x2, y2);
	}
}

Vector2D render::get_text_size(unsigned long font, std::string text) {
	std::wstring a = MultiByteToWide(text);
	const wchar_t* wstr = a.c_str();
	static int w, h;

	g_VGuiSurface->get_text_size(font, wstr, w, h);
	return { static_cast<float>(w), static_cast<float>(h) };
}

void render::draw_fade(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, Color color1, Color color2, bool horizontal) {
	g_VGuiSurface->set_drawing_color(color1.r(), color1.g(), color1.b(), color1.a());
	g_VGuiSurface->draw_filled_rect_fade(x, y, x + width, y + height, 255, 0, horizontal);

	g_VGuiSurface->set_drawing_color(color2.r(), color2.g(), color2.b(), color2.a());
	g_VGuiSurface->draw_filled_rect_fade(x, y, x + width, y + height, 0, 255, horizontal);
}

void render::gradient(int x, int y, int w, int h, Color c1, Color c2)
{
	render::draw_filled_rect(x, y, w, h, c1);
	for (int i = 0; i < h; i++)
	{
		float fi = i, fh = h;
		float a = fi / fh;
		DWORD ia = a * 255;
		render::draw_filled_rect(x, y + i, w, 1, Color(c2.r(), c2.g(), c2.b(), ia));
	}
}
