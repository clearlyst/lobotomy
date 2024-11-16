#include "chams.hpp"
#include <fstream>

#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"
#include <deque>
#include "bt.h"
class KeyValues
{
public:
	void init(const char* key_name) {
		using initkv_t = void(__thiscall*)(void*, const char*, int, int);
		static auto initkv_fn = *reinterpret_cast<initkv_t>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 51 33 C0 C7 45"));
		initkv_fn(this, key_name, 0, 0);
	}

	void load_from_buffer(const char* resource, const char* buf, void* file = nullptr, const char* id = nullptr, void* eval_proc = nullptr, void* unk = nullptr) {
		using loadfrombuffer_t = void(__thiscall*)(void*, const char*, const char*, void*, const char*, void*, void*);
		static auto loadfrombuffer_fn = *reinterpret_cast<loadfrombuffer_t>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89 4C 24 04"));
		loadfrombuffer_fn(this, resource, buf, file, id, eval_proc, unk);
	}
};
IMaterial* material_textured;
IMaterial* material_flat;
IMaterial* material_metallic;
IMaterial* material_plastic;
IMaterial* material_elixir;
IMaterial* material_glow;
IMaterial* material_crystal;
IMaterial* material_space;
IMaterial* material_glass;

IMaterial* create_material(std::string name, std::string type, std::string material) {
	auto kv = static_cast<KeyValues*>(malloc(36));
	kv->init(type.c_str());
	kv->load_from_buffer(name.c_str(), material.c_str());
	return g_MatSystem->CreateMaterial(name.c_str(), kv);
}

void override_material(bool ignorez, int type, Color clr) {
	IMaterial* material = nullptr;

	switch (type) {
	case 0: material = material_textured; break;
	case 1: material = material_flat; break;
	case 2: material = material_metallic; break;
	case 3: material = material_plastic; break;
	case 4: material = material_elixir; break;
	case 5: material = material_glow; break;
	case 6: material = material_textured; break;
	case 7: material = material_crystal; break;
	case 8: material = material_space; break;
	case 9: material = material_glass; break;
	}

	material->set_material_var_flag(MATERIAL_VAR_IGNOREZ, ignorez);
	material->set_material_var_flag(MATERIAL_VAR_WIREFRAME, type == 6);

	material->ColorModulate(clr.r()/255.f, clr.g() / 255.f, clr.b() / 255.f);
	material->AlphaModulate(clr.a() / 255.f);
	g_MdlRender->ForcedMaterialOverride(material);
}

void override_overlay_material(bool ignorez, int type, Color clr) {
	IMaterial* material = nullptr;

	switch (type) {
	case 0: material = material_textured; break;
	case 1: material = material_flat; break;
	case 2: material = material_metallic; break;
	case 3: material = material_plastic; break;
	case 4: material = material_elixir; break;
	case 5: material = material_glow; break;
	case 6: material = material_textured; break;
	case 7: material = material_crystal; break;
	case 8: material = material_space; break;
	case 9: material = material_glass; break;
	}

	material->set_material_var_flag(MATERIAL_VAR_IGNOREZ, ignorez);
	material->set_material_var_flag(MATERIAL_VAR_WIREFRAME, type == 6);

	// overlay color
	auto overlay = material->find_var("$envmaptint");
	overlay->set_vector(Vector(clr.r(), clr.g(), clr.b()));

	// color modulate
	material->ColorModulate(clr.r(), clr.g(), clr.b());
	material->AlphaModulate(clr.a());
	g_MdlRender->ForcedMaterialOverride(material);
}

void Chams::initialize() {
	material_textured = g_MatSystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL);
	material_textured->incrementreferencecount();

	material_flat = g_MatSystem->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL);
	material_flat->incrementreferencecount();

	material_plastic = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/gloss", TEXTURE_GROUP_MODEL);
	material_plastic->incrementreferencecount();

	material_glow = g_MatSystem->FindMaterial("dev/glow_armsrace", TEXTURE_GROUP_MODEL);
	material_glow->incrementreferencecount();

	material_glass = g_MatSystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_MODEL);
	material_glass->incrementreferencecount();

	material_metallic = create_material("metallic", "VertexLitGeneric", R"#("VertexLitGeneric" {
      "$basetexture" "vgui/white_additive"
      "$ignorez" "0"
      "$envmap" "env_cubemap"
      "$normalmapalphaenvmapmask" "1"
      "$envmapcontrast"  "1"
      "$nofog" "1"
      "$model" "1"
      "$nocull" "0"
      "$selfillum" "1"
      "$halflambert" "1"
      "$znearer" "0"
      "$flat" "1" 
}
)#");

	material_elixir = create_material("elixir", "VertexLitGeneric", R"#("VertexLitGeneric" {
        "$basetexture" "VGUI/white_additive"
		"$bumpmap" "de_nuke/hr_nuke/pool_water_normals_002"
		"$bumptransform" "center 0.5 0.5 scale 0.25 0.25 rotate 0.0 translate 0.0 0.0"
		"$ignorez" "0"
		"$nofog" "1"
		"$model" "1"
		"$color2" "[1.0, 1.0, 1.0]"
		"$halflambert" "1"
		"$envmap" "env_cubemap"
		"$envmaptint" "[0.1 0.1 0.1]"
		"$envmapfresnel" "1.0"
		"$envmapfresnelminmaxexp" "[1.0, 1.0, 1.0]"
		"$phong" "1"
		"$phongexponent" "1024"
		"$phongboost" "4.0"
		"$phongfresnelranges" "[1.0, 1.0, 1.0]"
		"$rimlight" "1"
		"$rimlightexponent" "4.0"
		"$rimlightboost" "2.0"
    
		"Proxies"
		{
			"TextureScroll"
			{
				"textureScrollVar" "$bumptransform"
				"textureScrollRate" "0.25"
				"textureScrollAngle" "0.0"
			}
		}
   }  )#");

	material_crystal = create_material("crystal", "VertexLitGeneric", R"#("VertexLitGeneric"
        {
            "$basetexture" "vgui/white"
            "$envmap" "models/effects/crystal_cube_vertigo_hdr"
            "$envmaptint" "[1 1 1]"
            "$envmapfresnel" "1"
            "$envmapfresnelminmaxexp" "[0 1 2]"
            "$phong" "1"
            "$reflectivity" "[2 2 2]"
            "$bumpmap" "models/weapons/customization/materials/origamil_camo"
            "$bumptransform" "center 0.5 0.5 scale 0.25 0.25 rotate 0.0 translate 0.0 0.0"
            "Proxies"
            {
                "TextureScroll"
                {
                    "textureScrollVar" "$bumptransform"
                    "textureScrollRate" "0.2"
                    "textureScrollAngle" "0.0"
                }
            }
        })#");

	material_space = create_material("space", "VertexLitGeneric", R"#("VertexLitGeneric"
        {
		   "$basetexture"                "dev/snowfield"
		   "$ignorez" "0"
		   "$nofog" "1"
		   "$model" "1"
		   "$nocull" "0"
		   "$selfillum" "1"
		   "$halflambert" "1"
		   "$znearer" "0"
		   "$flat" "1" 
		   "$additive"                    "1"
		   "$envmap"                    "editor/cube_vertigo"
		   "$envmaptint"                "[0 0.5 0.55]"
		   "$envmapfresnel"            "1"
		   "$envmapfresnelminmaxexp"   "[0.00005 0.6 6]"
		   "$alpha"                    "1"
 		   
		    Proxies
		    {
		        TextureScroll
		        {
		            "texturescrollvar"            "$baseTextureTransform"
		            "texturescrollrate"            "0.10"
		            "texturescrollangle"        "270"
		        }
		    }
        })#");
}

void Chams::run(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix)
{
	if (!g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
	{
		return;
	}
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

	bool is_player = strstr(info.pModel->szName, "models/player") != nullptr;
	bool is_hand = strstr(info.pModel->szName, "arms") != nullptr;
	bool is_sleeve = strstr(info.pModel->szName, "sleeve") != nullptr;
	bool is_weapon = strstr(info.pModel->szName, "weapons/v_") != nullptr;
	bool is_weapon_on_back = strstr(info.pModel->szName, "_dropped.mdl") && strstr(info.pModel->szName, "models/weapons/w") && !strstr(info.pModel->szName, "arms") && !strstr(info.pModel->szName, "ied_dropped");
	bool is_weapon_enemy_hands = strstr(info.pModel->szName, "models/weapons/w") && !strstr(info.pModel->szName, "arms") && !strstr(info.pModel->szName, "ied_dropped");
	bool is_defuse_kit = strstr(info.pModel->szName, "defuser") && !strstr(info.pModel->szName, "arms") && !strstr(info.pModel->szName, "ied_dropped");

	if (is_player)
	{
		C_BasePlayer* player = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(info.entity_index));

		if (!player || !player->IsAlive() || player->IsDormant() || player == g_LocalPlayer || player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
		{
			return;
		}

		auto material = g_MatSystem->FindMaterial(info.pModel->szName, TEXTURE_GROUP_MODEL);

		if (!material)
		{
			return;
		}


		if (g_Options.chams.invisible.enable)
		{
			if (g_Options.chams.invisible.hide_original_model)
			{
				material->set_material_var_flag(MATERIAL_VAR_NO_DRAW, true);
				g_MdlRender->ForcedMaterialOverride(material);
			}

			override_material(true, g_Options.chams.invisible.type, Color(g_Options.chams.invisible.color[0], g_Options.chams.invisible.color[1], g_Options.chams.invisible.color[2], g_Options.chams.invisible.color[3]));
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
		}

		if (g_Options.chams.invisible.overlay.enable)
		{
			override_material(true, g_Options.chams.invisible.overlay.type, Color(g_Options.chams.invisible.overlay.color[0], g_Options.chams.invisible.overlay.color[1], g_Options.chams.invisible.overlay.color[2], g_Options.chams.invisible.overlay.color[3]));
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
		}

		g_MdlRender->ForcedMaterialOverride(nullptr);

		if (g_Options.chams.visible.enable)
		{
			if (g_Options.chams.visible.hide_original_model)
			{
				material->set_material_var_flag(MATERIAL_VAR_NO_DRAW, true);
				g_MdlRender->ForcedMaterialOverride(material);
			}

			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			override_material(false, g_Options.chams.visible.type, Color(g_Options.chams.visible.color[0], g_Options.chams.visible.color[1], g_Options.chams.visible.color[2], g_Options.chams.visible.color[3]));
		}

		if (g_Options.chams.visible.overlay.enable)
		{
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			override_overlay_material(false, g_Options.chams.visible.overlay.type, Color(g_Options.chams.visible.overlay.color[0], g_Options.chams.visible.overlay.color[1], g_Options.chams.visible.overlay.color[2], g_Options.chams.visible.overlay.color[3]));
		}
	}

	if (is_sleeve)
	{
		auto material = g_MatSystem->FindMaterial(info.pModel->szName, TEXTURE_GROUP_MODEL);

		if (!material)
		{
			return;
		}

		g_MdlRender->ForcedMaterialOverride(nullptr);

		if (g_Options.chams.sleeves.enable)
		{
			if (g_Options.chams.sleeves.hide_original_model)
			{
				material->set_material_var_flag(MATERIAL_VAR_NO_DRAW, true);
				g_MdlRender->ForcedMaterialOverride(material);
			}

			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			override_material(false, g_Options.chams.sleeves.type, Color(g_Options.chams.sleeves.color[0], g_Options.chams.sleeves.color[1], g_Options.chams.sleeves.color[2], g_Options.chams.sleeves.color[3]));
		}

		if (g_Options.chams.sleeves.overlay.enable)
		{
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			override_overlay_material(false, g_Options.chams.sleeves.overlay.type, Color(g_Options.chams.sleeves.overlay.color[0], g_Options.chams.sleeves.overlay.color[1], g_Options.chams.sleeves.overlay.color[2], g_Options.chams.sleeves.overlay.color[3]));
		}
	}

	if (is_hand && !is_sleeve)
	{
		auto material = g_MatSystem->FindMaterial(info.pModel->szName, TEXTURE_GROUP_MODEL);

		if (!material)
		{
			return;
		}

		g_MdlRender->ForcedMaterialOverride(nullptr);

		if (g_Options.chams.hands.enable)
		{
			if (g_Options.chams.hands.hide_original_model)
			{
				material->set_material_var_flag(MATERIAL_VAR_NO_DRAW, true);
				g_MdlRender->ForcedMaterialOverride(material);
			}

			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			override_material(false, g_Options.chams.hands.type, Color(g_Options.chams.hands.color[0], g_Options.chams.hands.color[1], g_Options.chams.hands.color[2], g_Options.chams.hands.color[3]));
		}

		if (g_Options.chams.hands.overlay.enable)
		{
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			override_overlay_material(false, g_Options.chams.hands.overlay.type, Color(g_Options.chams.hands.overlay.color[0], g_Options.chams.hands.overlay.color[1], g_Options.chams.hands.overlay.color[2], g_Options.chams.hands.overlay.color[3]));
		}
	}

	if (is_weapon && !is_hand && !is_sleeve)
	{
		auto material = g_MatSystem->FindMaterial(info.pModel->szName, TEXTURE_GROUP_MODEL);

		if (!material)
		{
			return;
		}

		g_MdlRender->ForcedMaterialOverride(nullptr);

		if (g_Options.chams.weapon.enable)
		{
			if (g_Options.chams.weapon.hide_original_model)
			{
				material->set_material_var_flag(MATERIAL_VAR_NO_DRAW, true);
				g_MdlRender->ForcedMaterialOverride(material);
			}

			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			override_material(false, g_Options.chams.weapon.type, Color(g_Options.chams.weapon.color[0], g_Options.chams.weapon.color[1], g_Options.chams.weapon.color[2], g_Options.chams.weapon.color[3]));
		}

		if (g_Options.chams.weapon.overlay.enable)
		{
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			override_overlay_material(false, g_Options.chams.weapon.overlay.type, Color(g_Options.chams.weapon.overlay.color[0], g_Options.chams.weapon.overlay.color[1], g_Options.chams.weapon.overlay.color[2], g_Options.chams.hands.overlay.color[3]));
		}
	}

	if (is_weapon_on_back || is_weapon_enemy_hands || is_defuse_kit)
	{
		auto material = g_MatSystem->FindMaterial(info.pModel->szName, TEXTURE_GROUP_MODEL);

		if (!material)
		{
			return;
		}

		if (g_Options.chams.invisible_attachment.enable)
		{
			if (g_Options.chams.invisible_attachment.hide_original_model)
			{
				material->set_material_var_flag(MATERIAL_VAR_NO_DRAW, true);
				g_MdlRender->ForcedMaterialOverride(material);
			}

			override_material(true, g_Options.chams.invisible_attachment.type, Color(g_Options.chams.invisible_attachment.color[0], g_Options.chams.invisible_attachment.color[1], g_Options.chams.invisible_attachment.color[2], g_Options.chams.invisible_attachment.color[3]));
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
		}

		if (g_Options.chams.invisible_attachment.overlay.enable)
		{
			override_material(true, g_Options.chams.invisible_attachment.overlay.type, Color(g_Options.chams.invisible_attachment.overlay.color[0], g_Options.chams.invisible_attachment.overlay.color[1], g_Options.chams.invisible_attachment.overlay.color[2], g_Options.chams.invisible_attachment.overlay.color[3]));
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
		}

		g_MdlRender->ForcedMaterialOverride(nullptr);

		if (g_Options.chams.visible_attachment.enable)
		{
			if (g_Options.chams.visible_attachment.hide_original_model)
			{
				material->set_material_var_flag(MATERIAL_VAR_NO_DRAW, true);
				g_MdlRender->ForcedMaterialOverride(material);
			}

			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			override_material(false, g_Options.chams.visible_attachment.type, Color(g_Options.chams.visible_attachment.color[0], g_Options.chams.visible_attachment.color[1], g_Options.chams.visible_attachment.color[2], g_Options.chams.visible_attachment.color[3]));
		}

		if (g_Options.chams.visible_attachment.overlay.enable)
		{
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			override_overlay_material(false, g_Options.chams.visible_attachment.overlay.type, Color(g_Options.chams.visible_attachment.overlay.color[0], g_Options.chams.visible_attachment.overlay.color[1], g_Options.chams.visible.overlay.color[2], g_Options.chams.visible.overlay.color[3]));
		}
	}
}

void override_material_bt(bool ignorez, int type, Color clr) {
	IMaterial* material = nullptr;

	switch (type) {
	case 0: material = material_textured; break;
	case 1: material = material_flat; break;
	case 2: material = material_metallic; break;
	case 3: material = material_plastic; break;
	case 4: material = material_elixir; break;
	case 5: material = material_glow; break;
	case 6: material = material_textured; break;
	case 7: material = material_crystal; break;
	case 8: material = material_space; break;
	case 9: material = material_glass; break;
	}

	material->set_material_var_flag(MATERIAL_VAR_IGNOREZ, ignorez);
	material->set_material_var_flag(MATERIAL_VAR_WIREFRAME, type == 6);

	material->ColorModulate(clr.r()/255.f, clr.g() / 255.f, clr.b() / 255.f);
	material->AlphaModulate(clr.a() / 255.f);
	g_MdlRender->ForcedMaterialOverride(material);
}
inline float fromhsvtor(float h, float s, float v) {
	float out_r; float out_g; float out_b;
	if (s == 0.0f)
	{
		out_r = out_g = out_b = v;
	}

	h = fmodf(h, 1.0f) / (60.0f / 360.0f);
	int   i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (i)
	{
	case 0: out_r = v; out_g = t; out_b = p; break;
	case 1: out_r = q; out_g = v; out_b = p; break;
	case 2: out_r = p; out_g = v; out_b = t; break;
	case 3: out_r = p; out_g = q; out_b = v; break;
	case 4: out_r = t; out_g = p; out_b = v; break;
	case 5: default: out_r = v; out_g = p; out_b = q; break;
	}
	return out_r;
}

inline float fromhsvtog(float h, float s, float v) {
	float out_r; float out_g; float out_b;
	if (s == 0.0f)
	{
		out_r = out_g = out_b = v;
	}

	h = fmodf(h, 1.0f) / (60.0f / 360.0f);
	int   i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (i)
	{
	case 0: out_r = v; out_g = t; out_b = p; break;
	case 1: out_r = q; out_g = v; out_b = p; break;
	case 2: out_r = p; out_g = v; out_b = t; break;
	case 3: out_r = p; out_g = q; out_b = v; break;
	case 4: out_r = t; out_g = p; out_b = v; break;
	case 5: default: out_r = v; out_g = p; out_b = q; break;
	}
	return out_g;
}

inline float fromhsvtob(float h, float s, float v) {
	float out_r; float out_g; float out_b;
	if (s == 0.0f)
	{
		out_r = out_g = out_b = v;
	}

	h = fmodf(h, 1.0f) / (60.0f / 360.0f);
	int   i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (i)
	{
	case 0: out_r = v; out_g = t; out_b = p; break;
	case 1: out_r = q; out_g = v; out_b = p; break;
	case 2: out_r = p; out_g = v; out_b = t; break;
	case 3: out_r = p; out_g = q; out_b = v; break;
	case 4: out_r = t; out_g = p; out_b = v; break;
	case 5: default: out_r = v; out_g = p; out_b = q; break;
	}
	return out_b;
}

inline float rgbtoh(float rr, float gg, float bb) {
	const auto r = rr;
	const auto g = gg;
	const auto b = bb;

	auto h = 0.0f, s = 0.0f, v = 0.0f;

	const auto max = std::max < float >(r, std::max < float >(g, b));
	const auto min = std::min < float >(r, std::min < float >(g, b));

	v = max;

	if (max == 0.0f) {
		s = 0.0f;
		h = 0.0f;
	}
	else if (max - min == 0.0f) {
		s = 0.0f;
		h = 0.0f;
	}
	else {
		s = (max - min) / max;

		if (max == r) {
			h = 60.0f * ((g - b) / (max - min)) + 0.0f;
		}
		else if (max == g) {
			h = 60.0f * ((b - r) / (max - min)) + 120.0f;
		}
		else {
			h = 60.0f * ((r - g) / (max - min)) + 240.0f;
		}
	}

	if (h < 0.0f)
		h += 360.0f;

	return h / 2.0f / 180.f;
}

inline float rgbtos(float rr, float gg, float bb) {
	const auto r = rr;
	const auto g = gg;
	const auto b = bb;

	auto h = 0.0f, s = 0.0f, v = 0.0f;

	const auto max = std::max < float >(r, std::max < float >(g, b));
	const auto min = std::min < float >(r, std::min < float >(g, b));

	v = max;

	if (max == 0.0f) {
		s = 0.0f;
		h = 0.0f;
	}
	else if (max - min == 0.0f) {
		s = 0.0f;
		h = 0.0f;
	}
	else {
		s = (max - min) / max;

		if (max == r) {
			h = 60.0f * ((g - b) / (max - min)) + 0.0f;
		}
		else if (max == g) {
			h = 60.0f * ((b - r) / (max - min)) + 120.0f;
		}
		else {
			h = 60.0f * ((r - g) / (max - min)) + 240.0f;
		}
	}

	if (h < 0.0f)
		h += 360.0f;

	return s;
}

inline float rgbtov(float rr, float gg, float bb) {
	const auto r = rr;
	const auto g = gg;
	const auto b = bb;

	auto h = 0.0f, s = 0.0f, v = 0.0f;

	const auto max = std::max < float >(r, std::max < float >(g, b));
	const auto min = std::min < float >(r, std::min < float >(g, b));

	v = max;

	if (max == 0.0f) {
		s = 0.0f;
		h = 0.0f;
	}
	else if (max - min == 0.0f) {
		s = 0.0f;
		h = 0.0f;
	}
	else {
		s = (max - min) / max;

		if (max == r) {
			h = 60.0f * ((g - b) / (max - min)) + 0.0f;
		}
		else if (max == g) {
			h = 60.0f * ((b - r) / (max - min)) + 120.0f;
		}
		else {
			h = 60.0f * ((r - g) / (max - min)) + 240.0f;
		}
	}

	if (h < 0.0f)
		h += 360.0f;

	return v;
}
void Chams::run_bt(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
	{
		return;
	}

	bool is_player = strstr(info.pModel->szName, "models/player") != nullptr;
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

	if (is_player)
	{
		C_BasePlayer* player = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(info.entity_index));

		if (!player || !player->IsAlive() || player->IsDormant() || player == g_LocalPlayer || player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum() )
		{
			return;
		}

		if (g_Options.chams.backtrack.enable)
		{
			auto record = &Backtrack::records[player->EntIndex()];

			if (record->empty())
			{
				return;
			}

			if (g_Options.chams.backtrack.draw_all_ticks)
			{
				for (int i = 0; i < record->size(); i++)
				{
					if (record->at(i).simulationTime)
					{
						float h, s, v, a;
						h = (rgbtoh(g_Options.chams.backtrack.color_2[0], g_Options.chams.backtrack.color_2[1], g_Options.chams.backtrack.color_2[2]) - rgbtoh(g_Options.chams.backtrack.color_1[0], g_Options.chams.backtrack.color_1[1], g_Options.chams.backtrack.color_1[2])) / Backtrack::records[player->EntIndex()].size();
						s = (rgbtos(g_Options.chams.backtrack.color_2[0], g_Options.chams.backtrack.color_2[1], g_Options.chams.backtrack.color_2[2]) - rgbtos(g_Options.chams.backtrack.color_1[0], g_Options.chams.backtrack.color_1[1], g_Options.chams.backtrack.color_1[2])) / Backtrack::records[player->EntIndex()].size();
						v = (rgbtov(g_Options.chams.backtrack.color_2[0], g_Options.chams.backtrack.color_2[1], g_Options.chams.backtrack.color_2[2]) - rgbtov(g_Options.chams.backtrack.color_1[0], g_Options.chams.backtrack.color_1[1], g_Options.chams.backtrack.color_1[2])) / Backtrack::records[player->EntIndex()].size();
						a = (g_Options.chams.backtrack.color_2[3] - g_Options.chams.backtrack.color_1[3]) / Backtrack::records[player->EntIndex()].size();

						float hh, ss, vv;
						hh = rgbtoh(g_Options.chams.backtrack.color_1[0], g_Options.chams.backtrack.color_1[1], g_Options.chams.backtrack.color_1[2]) + (h * i);
						ss = rgbtos(g_Options.chams.backtrack.color_1[0], g_Options.chams.backtrack.color_1[1], g_Options.chams.backtrack.color_1[2]) + (s * i);
						vv = rgbtov(g_Options.chams.backtrack.color_1[0], g_Options.chams.backtrack.color_1[1], g_Options.chams.backtrack.color_1[2]) + (v * i);
						float fade_color[3] = { fromhsvtor(hh, ss, vv), fromhsvtog(hh, ss, vv), fromhsvtob(hh, ss, vv) };

						Color gradient = Color(fade_color[0], fade_color[1], fade_color[2], g_Options.chams.backtrack.color_1[3] + (a * i));

						override_material_bt(g_Options.chams.backtrack.invisible, g_Options.chams.backtrack.type, g_Options.chams.backtrack.gradient ? gradient : Color(g_Options.chams.backtrack.color_2[0], g_Options.chams.backtrack.color_2[1], g_Options.chams.backtrack.color_2[2], g_Options.chams.backtrack.color_2[3]));

						fnDME(g_MdlRender, 0, ctx, state, info, Backtrack::records[player->EntIndex()][i].matrix);

						g_MdlRender->ForcedMaterialOverride(nullptr);
					}
				}
			}
			else
			{
				if (record->front().simulationTime)
				{
					override_material_bt(g_Options.chams.backtrack.invisible, g_Options.chams.backtrack.type, Color(g_Options.chams.backtrack.color_2[0], g_Options.chams.backtrack.color_2[1], g_Options.chams.backtrack.color_2[2], g_Options.chams.backtrack.color_2[3]));
					fnDME(g_MdlRender, 0, ctx, state, info, record->back().matrix);

					g_MdlRender->ForcedMaterialOverride(nullptr);
				}
			}
		}
	}
}
