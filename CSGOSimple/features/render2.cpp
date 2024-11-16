/* defines */
#define _CRT_SECURE_NO_WARNINGS

/* includes */
#include "render2.h"

/* prefine */
odio* draw_list1;

/* init device */
void odio::init_device() {

	/* setup shaders */
	device->SetVertexShader(nullptr);
	device->SetPixelShader(nullptr);
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->SetRenderState(D3DRS_LIGHTING, false);
	device->SetRenderState(D3DRS_FOGENABLE, false);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	/* setup states */
	device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	device->SetRenderState(D3DRS_ZWRITEENABLE, false);
	device->SetRenderState(D3DRS_STENCILENABLE, false);

	/* setup states for fonts */
	device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, true);
	device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, true);

	/* setup states for render system */
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);

	/* setup states for color */
	device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

	/* setup states for textures */
	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	/* init samples */
	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
}
IDirect3DStateBlock9* pixel_state = NULL; IDirect3DVertexDeclaration9* vertDec; IDirect3DVertexShader9* vertShader;
DWORD dwOld_D3DRS_COLORWRITEENABLE;

void odio::SaveState(IDirect3DDevice9* pDevice)
{
	pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
	//	pDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &pixel_state); // This seam not to be needed anymore because valve fixed their shit
	pDevice->GetVertexDeclaration(&vertDec);
	pDevice->GetVertexShader(&vertShader);
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);
}

void odio::RestoreState(IDirect3DDevice9* pDevice) // not restoring everything. Because its not needed.
{
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
	//pixel_state->Apply(); 
	//pixel_state->Release();
	pDevice->SetVertexDeclaration(vertDec);
	pDevice->SetVertexShader(vertShader);
}
/* render->init_text setup */
int odio::get_text_width(const char* text, LPD3DXFONT font) {
	RECT rcRect = { 0, 0, 0, 0 };
	if (font)
	{
		font->DrawTextA(NULL, text, strlen(text), &rcRect, DT_CALCRECT,
			D3DCOLOR_XRGB(0, 0, 0));
	}

	return rcRect.right - rcRect.left;
}

int odio::get_text_height(const char* text, LPD3DXFONT font) {
	RECT rcRect = { 0, 0, 0, 0 };
	if (font)
	{
		font->DrawTextA(NULL, text, strlen(text), &rcRect, DT_CALCRECT,
			D3DCOLOR_XRGB(0, 0, 0));
	}

	return rcRect.bottom - rcRect.top;
}



void odio::text(float x, float y, LPD3DXFONT font, std::string text, Color c, bool center, int drop_shadow, Color shadow_color) {
	if (center)
		x -= (get_text_width(text.c_str(), font) / 2);

	int screenx, screeny;
	g_EngineClient->GetScreenSize(screenx, screeny);

	RECT DrawArea = { x, y, screenx, screeny };

	// draw drop shadows
	if (drop_shadow == 1)
	{
		// draws a shadow 1px down and 1px right of the text
		RECT DrawArea2 = { x + 1, y + 1, screenx, DrawArea.top + 200 };
		font->DrawTextA(NULL, text.c_str(), -1, &DrawArea2, 0, D3DCOLOR_RGBA(0, 0, 0, c.a()));
	}
	else if (drop_shadow == 2)
	{
		// draws a shadow all around the text
		RECT DrawArea2 = { x + 1.f, y, screenx, DrawArea.top + 200 };
		RECT DrawArea3 = { x - 1.f, y, screenx, DrawArea.top + 200 };
		RECT DrawArea4 = { x, y + 1.f, screenx, DrawArea.top + 200 };
		RECT DrawArea5 = { x, y - 1.f, screenx, DrawArea.top + 200 };
		font->DrawTextA(NULL, text.c_str(), -1, &DrawArea2, 0, D3DCOLOR_RGBA(shadow_color.r(), shadow_color.g(), shadow_color.b(), shadow_color.a()));
		font->DrawTextA(NULL, text.c_str(), -1, &DrawArea3, 0, D3DCOLOR_RGBA(shadow_color.r(), shadow_color.g(), shadow_color.b(), shadow_color.a()));
		font->DrawTextA(NULL, text.c_str(), -1, &DrawArea4, 0, D3DCOLOR_RGBA(shadow_color.r(), shadow_color.g(), shadow_color.b(), shadow_color.a()));
		font->DrawTextA(NULL, text.c_str(), -1, &DrawArea5, 0, D3DCOLOR_RGBA(shadow_color.r(), shadow_color.g(), shadow_color.b(), shadow_color.a()));
	}

	// draw the text
	font->DrawTextA(NULL, text.c_str(), -1, &DrawArea, 0, D3DCOLOR_RGBA(c.r(), c.g(), c.b(), c.a()));
}

void odio::gradient_vertical(Vector2D a, Vector2D b, Color c_a, Color c_b) {
	b += a;

	vertice_t verts[4] = {
		{ a.x, a.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_a.r(), c_a.g(), c_a.b(), c_a.a())},
	{ b.x, a.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_a.r(), c_a.g(), c_a.b(), c_a.a())},
	{ a.x, b.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_b.r(), c_b.g(), c_b.b(), c_b.a())},
	{ b.x, b.y, 0.01f, 0.01f, D3DCOLOR_RGBA(c_b.r(), c_b.g(), c_b.b(), c_b.a())}
	};

	device->SetTexture(0, nullptr);
	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &verts, 20);
}

void odio::gradient_vertical_init(float x, float y, float w, float h, Color c_a, Color c_b) {
	gradient_vertical({ x, y }, { w, h }, c_a, c_b);
}
