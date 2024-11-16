#pragma once

#include <d3dx9core.h>
#include <vector>
#include <array>
#include <mutex>
#include <shared_mutex>
#include "../valve_sdk/sdk.hpp"



/* class: render */
/* struct helpers */
struct vertice_t
{
	float x, y, z, rhw;
	int c;
};

/* class: render */
class odio {
public:
	odio(IDirect3DDevice9* device)
	{
		this->device = device;
		this->vertex_shader = vertex_shader;

		device->GetViewport(&screen_size);
	}

	IDirect3DVertexShader9* vertex_shader;
	IDirect3DDevice9* device;
	D3DVIEWPORT9 screen_size;
	D3DVIEWPORT9      m_viewport;
	void init_device();
	D3DVIEWPORT9 GetViewport();
	void invalidate_objects();
	void restore_objects(LPDIRECT3DDEVICE9 device);
	void SetViewport(D3DVIEWPORT9 vp);
	void SaveState(IDirect3DDevice9* pDevice);
	void RestoreState(IDirect3DDevice9* pDevice);
	int get_text_width(const char* text, LPD3DXFONT font);
	int get_text_height(const char* text, LPD3DXFONT font);

	void text(float x, float y, LPD3DXFONT font, std::string text, Color c, bool centered = false, int drop_shadow = 1, Color shadow_color = Color(0, 0, 0, 255));
	void gradient_vertical(Vector2D a, Vector2D b, Color c_a, Color c_b);
	void gradient_vertical_init(float x, float y, float w, float h, Color c_a, Color c_b);
	void set_viewport(D3DVIEWPORT9 vp)
	{
		device->SetViewport(&vp);
	}

	D3DVIEWPORT9 get_viewport()
	{
		D3DVIEWPORT9 vp;
		device->GetViewport(&vp);
		return vp;
	}
	struct
	{
		LPD3DXFONT small_font;
		LPD3DXFONT main_font;
		LPD3DXFONT tahoma;
		LPD3DXFONT calibri;
		LPD3DXFONT indicator_font;
	} fonts;
};
extern odio* draw_list1;