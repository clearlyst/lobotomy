#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <algorithm>

#include "visuals.hpp"

#include "../options.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "render2.h"
#include "../renderer.h"
#include <deque>
#include "bt.h"
#include "NewOne.h"
#include "PlayerVisual.h"
static int alpha = 0;
RECT GetBBox(C_BaseEntity* ent)
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) {
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) {
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}

Visuals::Visuals()
{
	InitializeCriticalSection(&cs);
}

Visuals::~Visuals() {
	DeleteCriticalSection(&cs);
}

//--------------------------------------------------------------------------------
void Visuals::Render() {
}
//--------------------------------------------------------------------------------
int flPlayerAlpha[64];
int flAlphaFade = 5.f;
void DrawHeadBox(C_BaseEntity* pEnt, float& boxz)
{
	matrix3x4_t Matrix[128];

	if (!pEnt->SetupBones(Matrix, 128, 0x00000100, 0))
		return;

	studiohdr_t* pStudioHeader = g_MdlInfo->GetStudiomodel(pEnt->GetModel());

	if (!pStudioHeader)
		return;

	mstudiobbox_t* pBox = pStudioHeader->GetHitboxSet(0)->GetHitbox(0);

	if (!pBox)
		return;

	Vector vTraced = (pBox->bbmin + pBox->bbmax) * 0.5f;

	static Vector vTrans;
	Math::VectorTransform(vTraced, Matrix[pBox->bone], vTrans);

	boxz = vTrans.z + 7.0f;

}


void Visuals::TryingFunc()
{
	if (!g_LocalPlayer)
		return;
	if (!g_EngineClient->IsInGame())
		return;
	for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
	{
		IMaterial* pMaterial = g_MatSystem->GetMaterial(i);

		if (!pMaterial)
			continue;

		const char* group = pMaterial->GetTextureGroupName();
		const char* name = pMaterial->GetName();





		if (strstr(group, TEXTURE_GROUP_MODEL))
		{
			auto $rimlight = pMaterial->find_var("$rimlight");
			if ($rimlight)
				$rimlight->set_int(0);
			auto $phong = pMaterial->find_var("$phong");
			if ($phong)
				$phong->set_int(0);
		
		
			pMaterial->set_material_var_flag(MATERIAL_VAR_BASEALPHAENVMAPMASK, 0);
			pMaterial->set_material_var_flag(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK, 0);
			pMaterial->set_material_var_flag(MATERIAL_VAR_ENVMAPMODE, 0);
			pMaterial->set_material_var_flag(MATERIAL_VAR_HALFLAMBERT, 1);

		}
	}
}




bool Visuals::Player::Begin(C_BasePlayer* pl) {
	if (!pl->IsAlive())
		return false;

	if (pl->IsDormant() && flPlayerAlpha[pl->EntIndex()] > 0)
	{
		flPlayerAlpha[pl->EntIndex()] -= flAlphaFade;
	}
	else if (flPlayerAlpha[pl->EntIndex()] < 255 && !(pl->IsDormant()))
	{
		flPlayerAlpha[pl->EntIndex()] += flAlphaFade;
	}
	if (flPlayerAlpha <= 0 && pl->IsDormant())
		return false;
	ctx.pl = pl;
	ctx.is_enemy = g_LocalPlayer->m_iTeamNum() != pl->m_iTeamNum();
	ctx.is_visible = g_LocalPlayer->CanSeePlayer(pl, HITBOX_CHEST);

	if (!ctx.is_enemy)
		return false;
	//    if (!ctx.is_visible && !g_Options.esp_player_boxesOccluded)
		//    return false;
		//ctx.clr = ctx.is_enemy ? (ctx.is_visible ? g_Options.color_esp_enemy_visible : g_Options.color_esp_enemy_occluded) : (ctx.is_visible ? g_Options.color_esp_ally_visible : g_Options.color_esp_ally_occluded);

	auto head = pl->GetHitboxPos(HITBOX_HEAD);
	auto origin = pl->m_vecOrigin();

	if (!Math::WorldToScreen(head, ctx.head_pos) ||
		!Math::WorldToScreen(origin, ctx.feet_pos))
		return false;

	ctx.bbox = GetBBox(pl);

	std::swap(ctx.bbox.top, ctx.bbox.bottom);

	return !(!ctx.bbox.left || !ctx.bbox.top || !ctx.bbox.right || !ctx.bbox.bottom);
}

//--------------------------------------------------------------------------------

void Visuals::Player::RenderBox(C_BaseEntity* pl) {
	if (ctx.pl->IsDormant())
		return;
	if (g_Options.esp_player_boxes)
	{

		if (g_Options.box_type == 1)
		{

			//	Render::Get().RenderBoxByType(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, Color(g_Options.Colorespbox), 1);
			g_Draw.DrawOutlinedRect(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, 0, 0, 0, 255);
			g_Draw.DrawOutlinedRect(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, g_Options.Colorespbox.r(), g_Options.Colorespbox.g(), g_Options.Colorespbox.b(), 255);
		}
		else
		{
			if (g_Options.box_type == 0)
			{
				g_VGuiSurface->set_drawing_color(0, 0, 0, 255);

				g_VGuiSurface->draw_outlined_rect(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1 - ctx.bbox.left + 1, ctx.bbox.bottom - 1 - ctx.bbox.top + 1);
				g_VGuiSurface->set_drawing_color(g_Options.Colorespbox.r(), g_Options.Colorespbox.g(), g_Options.Colorespbox.b(), 255);
				g_VGuiSurface->draw_outlined_rect(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right - ctx.bbox.left, ctx.bbox.bottom - ctx.bbox.top);
			}
		}
	}

	//AntohaRender::draw_rect(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, Color(g_Options.Colorespbox));
}

//--------------------------------------------------------------------------------
void Visuals::Player::RenderName(C_BaseEntity* pl)
{

	std::string name_esp;
	player_info_t info = ctx.pl->GetPlayerInfo();

	if (info.szSteamID[0] == 'B')
	{
		name_esp = "[bot] " + std::string(info.szName);
	}
	else
	{
		name_esp = info.szName;
	}
//	Render::Get().RenderText(name_esp,ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) / 2, ctx.bbox.top - 15, 15.f, Color(g_Options.esp_col.r(), g_Options.esp_col.g(), g_Options.esp_col.b(), flPlayerAlpha[pl->EntIndex()]), true,false,true);

	render::draw_text_string(ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) / 2, ctx.bbox.top - 20, g_Draw.m_MenuFont,name_esp, true, Color(g_Options.esp_col.r(), g_Options.esp_col.g(), g_Options.esp_col.b(), flPlayerAlpha[pl->EntIndex()]));
	//if (ctx.is_visible && g_Options.esp_player_boxesOccluded)
//	{
	//	render::draw_text_string(ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) / 2, ctx.bbox.top - 28, g_Draw.velofont, "can see you", true, Color(255, 0, 0, 250));
	//}

}
struct loginfo_t {
	loginfo_t(const float log_time, std::string message, const Color color) {
		this->log_time = log_time;
		this->message = message;
		this->color = color;

		this->x = 6.f;
		this->y = 0.f;
	}

	float log_time;
	std::string message;
	Color color;
	float x, y;
};

std::deque< loginfo_t > logs;
void Add(std::string text, Color color)
{
	logs.push_front(loginfo_t(Utils::EpochTime(), text, color));
}
void Visuals::paint_traverse()
{
	if (logs.empty())
		return;

	while (logs.size() > 20)
		logs.pop_back();

	auto last_y = 146;

	for (size_t i = 0; i < logs.size(); i++)
	{
		auto& log = logs.at(i);

		if (Utils::EpochTime() - log.log_time > 1200)
		{
			auto factor = log.log_time + 2000.0f - (float)Utils::EpochTime();
			factor *= 0.001f;

			auto opacity = (int)(255.0f * factor);

			if (opacity < 2)
			{
				logs.erase(logs.begin() + i);
				continue;
			}

			log.color.SetAlpha(opacity);
			log.y -= factor * 1.25f;
		}

		last_y -= 14;

		auto logs_size_inverted = 10 - logs.size();
		render::draw_text_string(log.x, last_y + log.y - logs_size_inverted * 14, g_Draw.m_MenuFont, log.message.c_str(),false,Color::White);
	}
}
void Visuals::HIT_LOGGER(IGameEvent* BANUS)
{
	// Получаем указатели на атакующего и цель
	int attackerUserID = BANUS->GetInt("attacker");
	int targetUserID = BANUS->GetInt("userid");
	C_BasePlayer* attacker = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(attackerUserID)));
	C_BasePlayer* target = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(targetUserID)));

	// Проверяем, что атакующий, цель и локальный игрок существуют, и что цель не является тиммейтом
	if (attacker && target && g_LocalPlayer)
	{
		// Массив с названиями частей тела для сообщения
		const char* hitgroups[10] =
		{
			"generic", "head", "chest", "stomach", "right arm", "left arm", "left leg", "right leg", "gear"
		};

		// Получаем данные о нанесенном уроне
		int dmg_health = BANUS->GetInt("dmg_health");
		int health = BANUS->GetInt("health");

		// Создаем сообщение о попадании
		std::stringstream gg;
		gg << "hit " << target->GetPlayerInfo().szName << " in the " << hitgroups[BANUS->GetInt("hitgroup")] << " for " << std::to_string(dmg_health);

		// Ваш код для добавления сообщения куда-то, например, на экран или в чат
		// Например, если у вас есть функция Add для добавления сообщений:
		Add(gg.str(), Color::White);
	}
}

void Visuals::Player::RenderHealthBar(C_BaseEntity* pl) {

	if (ctx.pl->IsDormant())
		return;
	// Health clamp and color
	auto  hp = ctx.pl->m_iHealth();
	float box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 7;

	int height = (box_h * hp) / 100;

	int green = int(hp * 2.55f);
	int red = 255 - green;

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;

	render::draw_filled_rect(ctx.bbox.left - 6, y - 1, 3, h + 2, { 10, 10, 10,  flPlayerAlpha[pl->EntIndex()] });


	// render actual bar.
	render::draw_rect(ctx.bbox.left - 5, y + h - height, 1, height, Color(red,green,0, flPlayerAlpha[pl->EntIndex()]));

	// if hp is below max, draw a string.
	if (hp < 100)
		render::draw_text_string(ctx.bbox.left - 17, y + (h - height) - 5, g_Draw.m_MenuFont, std::to_string(hp), true, Color(255,255,255, flPlayerAlpha[pl->EntIndex()]));
}
void Visuals::Player::RenderBacktrack(C_BaseEntity* ent) {
	if (!g_Options.esp_backtrack)
		return;

	auto data = &Backtrack::records[ent->EntIndex()];
	int counter = data->size();
	for (int i = 0; i < counter; i++) {
		Vector backtrackHead;
		if (!Math::WorldToScreen(data->at(i).head, backtrackHead))
			continue;
		render::draw_filled_rect(backtrackHead.x, backtrackHead.y, 2, 2, Color(g_Options.btdot));
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderHealth(C_BaseEntity* pl)
{
	auto  hp = ctx.pl->m_iHealth();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	//float off = (box_h / 6.f) + 5;
	float off = 8;

	int height = (box_h * hp) / 100;

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;
	int green = int(hp * 2.55f);
	int red = 255 - green;

//	Render::Get().RenderBox(x, y, x + w, y + h, Color::Black, 1.f, true);
//	Render::Get().RenderBox(x + 1, y + 1, x + w - 1, y + height - 2, Color(0, 255, 0, 255), 1.f, true);

	std::string text = std::to_string(hp) + " hp";



	render::draw_text_string(ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) / 2, ctx.bbox.bottom + 5, g_Draw.m_MenuFont, text.c_str(), true, Color(g_Options.esp_col.r(), g_Options.esp_col.g(), g_Options.esp_col.b(), flPlayerAlpha[pl->EntIndex()]));

}

void Visuals::playerModel(ClientFrameStage_t stage)
{
	static int originalIdx = 0;

	if (!g_LocalPlayer) {
		originalIdx = 0;
		return;
	}

	constexpr auto getModel = [](int team) constexpr noexcept -> const char* {
		constexpr std::array models{
			"models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
				"models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
				"models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
				"models/player/custom_player/legacy/ctm_fbi_varianth.mdl",
				"models/player/custom_player/legacy/ctm_sas_variantf.mdl",
				"models/player/custom_player/legacy/ctm_st6_variante.mdl",
				"models/player/custom_player/legacy/ctm_st6_variantg.mdl",
				"models/player/custom_player/legacy/ctm_st6_varianti.mdl",
				"models/player/custom_player/legacy/ctm_st6_variantk.mdl",
				"models/player/custom_player/legacy/ctm_st6_variantm.mdl",
				"models/player/custom_player/legacy/tm_balkan_variantf.mdl",
				"models/player/custom_player/legacy/tm_balkan_variantg.mdl",
				"models/player/custom_player/legacy/tm_balkan_varianth.mdl",
				"models/player/custom_player/legacy/tm_balkan_varianti.mdl",
				"models/player/custom_player/legacy/tm_balkan_variantj.mdl",
				"models/player/custom_player/legacy/tm_leet_variantf.mdl",
				"models/player/custom_player/legacy/tm_leet_variantg.mdl",
				"models/player/custom_player/legacy/tm_leet_varianth.mdl",
				"models/player/custom_player/legacy/tm_leet_varianti.mdl",
				"models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
				"models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
				"models/player/custom_player/legacy/tm_phoenix_varianth.mdl",
				"models/player/custom_player/legacy/ctm_diver_variantb.mdl"
		};

		switch (team) {
		case 2: return static_cast<std::size_t>(g_Options.playerModelT - 1) < models.size() ? models[g_Options.playerModelT - 1] : nullptr;
		case 3: return static_cast<std::size_t>(g_Options.playerModelCT - 1) < models.size() ? models[g_Options.playerModelCT - 1] : nullptr;
		default: return nullptr;
		}
	};

	if (const auto model = getModel(g_LocalPlayer->m_iTeamNum())) {
		if (stage == FRAME_RENDER_START)
			originalIdx = g_LocalPlayer->m_nModelIndex();

		const auto idx = stage == FRAME_RENDER_END && originalIdx ? originalIdx : g_MdlInfo->GetModelIndex(model);

		g_LocalPlayer->setModelIndex(idx);

		if (const auto ragdoll = g_LocalPlayer->get_entity_from_handle(g_LocalPlayer->m_hRagdoll()))
			ragdoll->setModelIndex(idx);
	}
}
void Visuals::Player::flags(C_BaseEntity* pl)
{
	if (!g_Options.flags)
		return;
	float off = 8;
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;
	std::vector<std::pair<std::string, Color>> flags;
	if (g_Options.money_flags)
		flags.push_back({ std::string("$").append(std::to_string(pl->money())), Color(0,255,0,flPlayerAlpha[pl->EntIndex()]) });
	if (g_Options.hk_flags && pl->armor() > 0)
		flags.push_back({ pl->has_helmet() ? "hk" : "k", Color(255,255,255,flPlayerAlpha[pl->EntIndex()]) });
	if (g_Options.kits_flags && pl->IsDefuseKit())
		flags.push_back({ "kit", Color(255,255,255,flPlayerAlpha[pl->EntIndex()]) });

	auto position = 0;
	for (auto text : flags) {
		render::draw_text_string(ctx.bbox.right + 10, ctx.bbox.top + position - 2, g_Draw.m_MenuFont, text.first,false, text.second);
		position += 10;
	}
}
float checksurflolkek_alpha;
float jumpbug_alpha;
float longjump_alpha;
float lg_alpha;
float minijump_alpha;
float pixelsurf_alpha;
float wcalpha;
float freelook_alpha;
float fireman_alpha;
float brokehop_alpha;

float edgebug_alpha = 0.0f;
float fr_alpha = 0.0f;
clock_t lastTime = 0;
const double duration = 0.3;  // Продолжительность изменения в секундах
const float alphaChangePerSecond = 1.0f / duration;  // Изменение альфа-канала в секунду





void Visuals::FireGameEvent(IGameEvent* event) {
	if (!strcmp(event->GetName(), "player_hurt")) {
		int attacker = event->GetInt("attacker");
		if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer()) {
			flHurtTime = 500.f;
			timer = g_GlobalVars->curtime + .9f;
			alpha = 255;
		}
	}
}
void Visuals::Player::RenderSkeleton(C_BaseEntity* ent) {
	auto model = ent->GetModel();
	if (ent && !model) return;

	if (ctx.pl->IsDormant())
		return;
	studiohdr_t* pStudioModel = g_MdlInfo->GetStudiomodel(model);

	if (pStudioModel) {
		static matrix3x4_t pBoneToWorldOut[128];

		if (ent->SetupBones(pBoneToWorldOut, 128, 256, g_GlobalVars->curtime)) {
			for (int i = 0; i < pStudioModel->numbones; i++) {
				mstudiobone_t* pBone = pStudioModel->GetBone(i);
				if (!pBone || !(pBone->flags & 256) || pBone->parent == -1) continue;

				Vector vBonePos1;
				if (!Math::WorldToScreen(Vector(pBoneToWorldOut[i][0][3], pBoneToWorldOut[i][1][3], pBoneToWorldOut[i][2][3]), vBonePos1)) continue;

				Vector vBonePos2;
				if (!Math::WorldToScreen(Vector(pBoneToWorldOut[pBone->parent][0][3], pBoneToWorldOut[pBone->parent][1][3], pBoneToWorldOut[pBone->parent][2][3]), vBonePos2)) continue;

				Render::Get().RenderLine((int)vBonePos1.x, (int)vBonePos1.y, (int)vBonePos2.x, (int)vBonePos2.y, Color(g_Options.skeleton_col));
			}
		}
	}
}


void Visuals::Postprocess(ClientFrameStage_t stage) {

	if (stage != ClientFrameStage_t::FRAME_RENDER_START && stage != ClientFrameStage_t::FRAME_RENDER_END)
		return;

	*memesclass->disablePostProcessing = stage == ClientFrameStage_t::FRAME_RENDER_START && g_Options.postproccesing;
}

void Visuals::DrawHitmarker() {

	if (!g_Options.hitmarker)
		return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;


	float hitmarker_size = 5.0f;
	int x, y;
	g_EngineClient->GetScreenSize(x, y);
	if (hitmarker_time > 0)
	{
		if (hitmarker_time > 100)
		{
			if (hitmarker_size < 8.0f)
			{
				hitmarker_size++;
			}
		}

		Render::Get().RenderLine(x / 2, y / 2, (x / 2) + (int)hitmarker_size, (y / 2) + (int)hitmarker_size, Color(255, 255, 255, std::clamp(hitmarker_time,0,255)), 1.0f);
		Render::Get().RenderLine(x / 2, y / 2, (x / 2) - (int)hitmarker_size, (y / 2) - (int)hitmarker_size, Color(255, 255, 255, std::clamp(hitmarker_time,0,255)), 1.0f);
		Render::Get().RenderLine(x / 2, y / 2, (x / 2) + (int)hitmarker_size, (y / 2) - (int)hitmarker_size, Color(255, 255, 255, std::clamp(hitmarker_time,0,255)), 1.0f);
		Render::Get().RenderLine(x / 2, y / 2, (x / 2) - (int)hitmarker_size, (y / 2) + (int)hitmarker_size, Color(255, 255, 255, std::clamp(hitmarker_time,0,255)), 1.0f);

		hitmarker_time -= 2;
		hitmarker_size -= 1.0f;

		if (hitmarker_size < 1.0f)
		{
			hitmarker_size = 0.0f;
		}
	}
}

//velocity graph
std::vector<int> velocity_history;
const int velocity_max_items = 275;
const int velocity_max_speed = 365;
const int velocity_graph_height = 100;
auto test = 200;

//stamina graph
std::vector<int> stamina_history;
const int stamina_max_items = 275;
const int stamina_max_speed = 365;
const int stamina_graph_height = 100;


void Visuals::gatherinfo() {
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
		return;

	//velocity graph
	const Vector velocity = g_LocalPlayer->m_vecVelocity();
	const int speed = floor(min((double)10000, sqrt(pow(velocity.x, 2) + pow(velocity.y, 2)) + 0.5));

	velocity_history.push_back(speed);

	if (velocity_history.size() > velocity_max_items) {
		velocity_history.erase(velocity_history.begin());
	}
	//stamina graph
	const float stamina = g_LocalPlayer->fl_stamina();

	stamina_history.push_back(stamina);

	if (stamina_history.size() > stamina_max_items) {
		stamina_history.erase(stamina_history.begin());
	}
}

void Visuals::velo_graph()
{
	if (!g_LocalPlayer && !g_EngineClient->IsInGame())
		return;

	int xx, yy;
	g_EngineClient->GetScreenSize(xx, yy);

	if (g_Options.stamina_graph) {
		int index2 = -1;
		int last_speed2 = -1;
		for (const auto& speed2 : stamina_history) {
			index2++;

			if (index2 != 0) {
				int cur_x2 = (index2 / (float)stamina_max_items) * g_Options.stm_graph_w + xx / 2 - (g_Options.stm_graph_w / 2);
				int last_x2 = ((index2 - 1) / (float)stamina_max_items) * g_Options.stm_graph_w + xx / 2 - (g_Options.stm_graph_w / 2);

				int cur_y2 = sqrt((((speed2 / (float)stamina_max_speed) * stamina_graph_height) - g_Options.stm_graph_y) * (((speed2 / (float)stamina_max_speed) * stamina_graph_height) - g_Options.stm_graph_y));
				int last_y2 = sqrt((((last_speed2 / (float)stamina_max_speed) * stamina_graph_height) - g_Options.stm_graph_y) * (((last_speed2 / (float)stamina_max_speed) * stamina_graph_height) - g_Options.stm_graph_y));

				if (index2 < 80) {
					test = unsigned char(index2 * 3);
				}
				if (index2 > 195) {
					test = unsigned char(3 * (-index2 + 275));
				}

				Render::Get().RenderLine(last_x2, last_y2, cur_x2, cur_y2, Color(g_Options.stm_graph_clr.r(), g_Options.stm_graph_clr.g(), g_Options.stm_graph_clr.b(), test));
			}

			last_speed2 = speed2;
		}
	}
	if (g_Options.velocity_graph) {
		int index = -1;
		int last_speed = -1;
		for (const auto& speed : velocity_history) {
			index++;

			if (index != 0) {
				int cur_x = (index / (float)velocity_max_items) * g_Options.graph_w + xx / 2 - (g_Options.graph_w / 2);
				int last_x = ((index - 1) / (float)velocity_max_items) * g_Options.graph_w + xx / 2 - (g_Options.graph_w / 2);

				int cur_y = sqrt((((speed / (float)velocity_max_speed) * velocity_graph_height) - g_Options.graph_y) * (((speed / (float)velocity_max_speed) * velocity_graph_height) - g_Options.graph_y));
				int last_y = sqrt((((last_speed / (float)velocity_max_speed) * velocity_graph_height) - g_Options.graph_y) * (((last_speed / (float)velocity_max_speed) * velocity_graph_height) - g_Options.graph_y));

				if (index < 80) {
					test = unsigned char(index * 3);
				}

				if (index > 195) {
					test = unsigned char(3 * (-index + 275));
				}

				Render::Get().RenderLine(last_x, last_y, cur_x, cur_y, Color(g_Options.vel_graph_clr.r(), g_Options.vel_graph_clr.g(), g_Options.vel_graph_clr.b(), test));
			}

			last_speed = speed;
		}
	}

}

//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
auto GetGunName = [](short cw)
{
	switch (cw)
	{
	case 500:
	case 505:
	case 506:
	case 507:
	case 508:
	case 509:
	case 512:
	case 514:
	case 515:
	case 516:
	case 519:
	case 520:
	case 522:
	case 523:
		case WEAPON_KNIFE_T:
			return ("knife t");
		case WEAPON_KNIFE:
			return ("knife ct");
		case WEAPON_KNIFEGG:
			return ("gold knife");
		case WEAPON_DEAGLE:
			return ("desert eagle");
		case WEAPON_AUG:
			return ("aug");
		case WEAPON_G3SG1:
			return ("g3sg1");
		case WEAPON_MAC10:
			return ("mac-10");
		case WEAPON_P90:
			return ("p90");
		case WEAPON_SSG08:
			return ("ssg08");
		case WEAPON_SCAR20:
			return ("scar-20");
		case WEAPON_UMP45:
			return ("ump-45");
		case WEAPON_ELITE:
			return ("dual berettas");
		case WEAPON_FAMAS:
			return ("famas");
		case WEAPON_FIVESEVEN:
			return ("five-seven");
		case WEAPON_GALILAR:
			return ("galilar");
		case WEAPON_M4A1_SILENCER:
			return ("m4a1-s");
		case WEAPON_M4A1:
			return ("m4a4");
		case WEAPON_P250:
			return ("p250");
		case WEAPON_M249:
			return ("m249");
		case WEAPON_XM1014:
			return ("xm1014");
		case WEAPON_GLOCK:
			return ("glock-18");
		case WEAPON_USP_SILENCER:
			return ("usp-s");
		case WEAPON_HKP2000:
			return ("p2000");
		case WEAPON_AK47:
			return ("ak-47");
		case WEAPON_AWP:
			return ("awp");
		case WEAPON_BIZON:
			return ("pp-bizon");
		case WEAPON_MAG7:
			return ("mag-7");
		case WEAPON_NEGEV:
			return ("negev");
		case WEAPON_SAWEDOFF:
			return ("sawed-off");
		case WEAPON_TEC9:
			return ("tec-9");
		case WEAPON_TASER:
			return ("taser");
		case WEAPON_NOVA:
			return ("nova");
		case WEAPON_CZ75A:
			return ("cz75 auto");
		case WEAPON_SG556:
			return ("sg553");
		case WEAPON_REVOLVER:
			return ("r8 revolver");
		case WEAPON_MP7:
			return ("mp7");
		case WEAPON_MP9:
			return ("mp9");
		case WEAPON_MP5:
			return ("mp5-sd");
		case WEAPON_C4:
			return ("c4 explosive");
		case WEAPON_FRAG_GRENADE:
			return ("frag grenade");
		case WEAPON_SMOKEGRENADE:
			return ("smoke grenade");
		case WEAPON_MOLOTOV:
			return ("molotov");
		case WEAPON_INCGRENADE:
			return ("incendiary grenade");
		case WEAPON_FLASHBANG:
			return ("flashbang");
		case WEAPON_DECOY:
			return ("decoy grenade");
		case WEAPON_HEGRENADE:
			return ("high explosive grenade");
		case WEAPON_TAGRENADE:
			return ("tactical awareness grenade");
		default:
			return ("unknown");
		
	}
};
auto icon = [](short cw)
{
	switch (cw)
	{
	case WEAPON_KNIFE:
	case WEAPON_KNIFE_T:
	case 500:
	case 505:
	case 506:
	case 507:
	case 508:
	case 509:
	case 512:
	case 514:
	case 515:
	case 516:
	case 519:
	case 520:
	case 522:
	case 523:
		return "]";
	case WEAPON_DEAGLE:
		return "A";
	case WEAPON_ELITE:
		return "B";
	case WEAPON_FIVESEVEN:
		return "C";
	case WEAPON_GLOCK:
		return "D";
	case WEAPON_HKP2000:
		return "E";
	case WEAPON_P250:
		return "F";
	case WEAPON_USP_SILENCER:
		return "G";
	case WEAPON_TEC9:
		return "H";
	case WEAPON_CZ75A:
		return "I";
	case WEAPON_REVOLVER:
		return "J";
	case WEAPON_MAC10:
		return "K";
	case WEAPON_UMP45:
		return "L";
	case WEAPON_BIZON:
		return "M";
	case WEAPON_MP7:
		return "N";
	case WEAPON_MP9:
		return "O";
	case WEAPON_P90:
		return "P";
	case WEAPON_GALILAR:
		return "Q";
	case WEAPON_FAMAS:
		return "R";
	case WEAPON_M4A1_SILENCER:
		return "T";
	case WEAPON_M4A1:
		return "S";
	case WEAPON_AUG:
		return "U";
	case WEAPON_SG556:
		return "V";
	case WEAPON_AK47:
		return "W";
	case WEAPON_G3SG1:
		return "X";
	case WEAPON_SCAR20:
		return "Y";
	case WEAPON_AWP:
		return "Z";
	case WEAPON_SSG08:
		return "a";
	case WEAPON_XM1014:
		return "b";
	case WEAPON_SAWEDOFF:
		return "c";
	case WEAPON_MAG7:
		return "d";
	case WEAPON_NOVA:
		return "e";
	case WEAPON_NEGEV:
		return "f";
	case WEAPON_M249:
		return "g";
	case WEAPON_TASER:
		return "h";
	case WEAPON_FLASHBANG:
		return "i";
	case WEAPON_HEGRENADE:
		return "j";
	case WEAPON_SMOKEGRENADE:
		return "k";
	case WEAPON_MOLOTOV:
		return "l";
	case WEAPON_DECOY:
		return "m";
	case WEAPON_INCGRENADE:
		return "n";
	case WEAPON_C4:
		return "o";
	default:
		return "";
	}
};
void Visuals::Player::RenderWeaponName(C_BaseEntity* pl)
{
	auto weapon = ctx.pl->m_hActiveWeapon().Get();

	if (!weapon) return;
	if (!weapon->GetCSWeaponData()) return;

	int okay;

	if (g_Options.esp_player_health)
		okay = 20;
	else
		okay = 0;
	auto weapon_text = std::string(GetGunName(weapon->m_Item().m_iItemDefinitionIndex()));
	//Render::Get().RenderText(text, ctx.feet_pos.x, ctx.feet_pos.y, 12.f, Color::White, true,g_pDefaultFont);
	render::draw_text_string(ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) / 2, ctx.bbox.bottom + okay, g_Draw.m_MenuFont, weapon_text, true, Color(g_Options.esp_col.r(), g_Options.esp_col.g(), g_Options.esp_col.b(), flPlayerAlpha[pl->EntIndex()]));

}
void Visuals::Player::weapon_icon(C_BaseEntity* pl) {
	if (!g_Options.iconesp)
		return;

	auto weapon = ctx.pl->m_hActiveWeapon().Get();
	if (!weapon || !weapon->GetCSWeaponData()) return;
	auto text1 = std::string(icon(weapon->m_Item().m_iItemDefinitionIndex()));

	int shit_coding = 0;

	if (g_Options.esp_player_health)
		shit_coding = 20;

	if (g_Options.esp_player_weapons)
		shit_coding = 20;

	if (g_Options.esp_player_weapons && g_Options.esp_player_health)
		shit_coding = 40;

	Render::Get().RenderText(text1, ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) / 2, ctx.bbox.bottom + shit_coding, 11.f, Color(g_Options.esp_col.r(), g_Options.esp_col.g(), g_Options.esp_col.b(), flPlayerAlpha[pl->EntIndex()]), false, fonts::indicator_font);
}

void Visuals::ConsoleColor(vgui::VPANEL panel)
{
	if (!g_Options.custom_console)
		return;

	static bool bShouldRecolorConsole;
	static IMaterial* cMaterial[5];

	if (!cMaterial[0] || !cMaterial[1] || !cMaterial[2] || !cMaterial[3] || !cMaterial[4])
	{
		for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
		{
			auto pMaterial = g_MatSystem->GetMaterial(i);

			if (!pMaterial)
			{
				continue;
			}

			if (strstr(pMaterial->GetName(), "vgui_white"))
			{
				cMaterial[0] = pMaterial;
			}
			else if (strstr(pMaterial->GetName(), "800corner1"))
			{
				cMaterial[1] = pMaterial;
			}
			else if (strstr(pMaterial->GetName(), "800corner2"))
			{
				cMaterial[2] = pMaterial;
			}
			else if (strstr(pMaterial->GetName(), "800corner3"))
			{
				cMaterial[3] = pMaterial;
			}
			else if (strstr(pMaterial->GetName(), "800corner4"))
			{
				cMaterial[4] = pMaterial;
			}
		}
	}
	else
	{
		if (strcmp(g_VGuiPanel->GetName(panel), "GameConsole") && (!strcmp(g_VGuiPanel->GetName(panel), "СHudCrosshair") || !strcmp(g_VGuiPanel->GetName(panel), "MatSystemTopPanel") || !strcmp(g_VGuiPanel->GetName(panel), "FocusOverlayPanel")))
		{
			for (int num = 0; num < 5; num++)
			{
				if (g_EngineClient->Con_IsVisible())
				{
					cMaterial[num]->ColorModulate(g_Options.concole_clr[0], g_Options.concole_clr[1], g_Options.concole_clr[2]);
					cMaterial[num]->AlphaModulate(g_Options.concole_clr[3]);
				}
				else
				{
					cMaterial[num]->ColorModulate(1.f, 1.f, 1.f);
					cMaterial[num]->AlphaModulate(1.0f);
				}
			}
		}
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderSnapline()
{

	int screen_w, screen_h;
	g_EngineClient->GetScreenSize(screen_w, screen_h);

	Render::Get().RenderLine(screen_w / 2.f, (float)screen_h,
		ctx.feet_pos.x, ctx.feet_pos.y, Color::Red);
}
//--------------------------------------------------------------------------------
void Visuals::RenderWeapon(C_BaseCombatWeapon* ent)
{
	auto clean_item_name = [](const char* name) -> const char* {
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	// We don't want to Render weapons that are being held
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	//Render::Get().RenderBox(bbox, g_Options.color_esp_weapons);


	auto drepped_weapon_text = std::string(GetGunName(ent->m_Item().m_iItemDefinitionIndex()));
	render::draw_text_string(bbox.left + (bbox.right - bbox.left) / 2, bbox.bottom + 5, g_Draw.m_MenuFont, drepped_weapon_text, true, Color(255, 255, 255, 255));
}

//--------------------------------------------------------------------------------
void Visuals::RenderDefuseKit(C_BaseEntity* ent)
{
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Options.color_esp_defuse);

	auto name = "Defuse Kit";
	auto sz = fonts::esp_font->CalcTextSizeA(12.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;
	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 12.f, Color::White,false,false, fonts::indicator_font);
}
//--------------------------------------------------------------------------------
void Visuals::RenderPlantedC4(C_BaseEntity* ent)
{
	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;


	Render::Get().RenderBox(bbox, g_Options.color_esp_c4);


	int bombTimer = std::ceil(ent->m_flC4Blow() - g_GlobalVars->curtime);
	std::string timer = std::to_string(bombTimer);

	auto name = (bombTimer < 0.f) ? "Bomb" : timer;
	auto sz = fonts::esp_font->CalcTextSizeA(12.f, FLT_MAX, 0.0f, name.c_str());
	int w = bbox.right - bbox.left;

	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 12.f, g_Options.color_esp_c4,false,false, fonts::indicator_font);
}
//--------------------------------------------------------------------------------
void Visuals::RenderItemEsp(C_BaseEntity* ent)
{
	std::string itemstr = "Undefined";
	const model_t * itemModel = ent->GetModel();
	if (!itemModel)
		return;
	studiohdr_t * hdr = g_MdlInfo->GetStudiomodel(itemModel);
	if (!hdr)
		return;
	itemstr = hdr->szName;
	if (ent->GetClientClass()->m_ClassID == ClassId_CBumpMine)
		itemstr = "";
	else if (itemstr.find("case_pistol") != std::string::npos)
		itemstr = "Pistol Case";
	else if (itemstr.find("case_light_weapon") != std::string::npos)
		itemstr = "Light Case";
	else if (itemstr.find("case_heavy_weapon") != std::string::npos)
		itemstr = "Heavy Case";
	else if (itemstr.find("case_explosive") != std::string::npos)
		itemstr = "Explosive Case";
	else if (itemstr.find("case_tools") != std::string::npos)
		itemstr = "Tools Case";
	else if (itemstr.find("random") != std::string::npos)
		itemstr = "Airdrop";
	else if (itemstr.find("dz_armor_helmet") != std::string::npos)
		itemstr = "Full Armor";
	else if (itemstr.find("dz_helmet") != std::string::npos)
		itemstr = "Helmet";
	else if (itemstr.find("dz_armor") != std::string::npos)
		itemstr = "Armor";
	else if (itemstr.find("upgrade_tablet") != std::string::npos)
		itemstr = "Tablet Upgrade";
	else if (itemstr.find("briefcase") != std::string::npos)
		itemstr = "Briefcase";
	else if (itemstr.find("parachutepack") != std::string::npos)
		itemstr = "Parachute";
	else if (itemstr.find("dufflebag") != std::string::npos)
		itemstr = "Cash Dufflebag";
	else if (itemstr.find("ammobox") != std::string::npos)
		itemstr = "Ammobox";
	else if (itemstr.find("dronegun") != std::string::npos)
		itemstr = "Turrel";
	else if (itemstr.find("exojump") != std::string::npos)
		itemstr = "Exojump";
	else if (itemstr.find("healthshot") != std::string::npos)
		itemstr = "Healthshot";
	else {
		/*May be you will search some missing items..*/
		/*static std::vector<std::string> unk_loot;
		if (std::find(unk_loot.begin(), unk_loot.end(), itemstr) == unk_loot.end()) {
			Utils::ConsolePrint(itemstr.c_str());
			unk_loot.push_back(itemstr);
		}*/
		return;
	}
	
	auto bbox = GetBBox(ent);
	if (bbox.right == 0 || bbox.bottom == 0)
		return;
	auto sz = fonts::esp_font->CalcTextSizeA(12.f, FLT_MAX, 0.0f, itemstr.c_str());
	int w = bbox.right - bbox.left;


	//Render::Get().RenderBox(bbox, g_Options.color_esp_item);
	//Render::Get().RenderText(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 12.f, g_Options.color_esp_item,false,false, g_Draw.AntohaFont);
}
//--------------------------------------------------------------------------------

bool lastvelsaved = false; //saver 
int lastjump, lastvel, lasttick = 0; // last vel holder 
extern int bIntFlag; // surface render
Color interpolate(const Color& first_color, const Color& second_color, const float time)
{
	return Color(
		first_color[0] + time * (second_color[0] - first_color[0]),
		first_color[1] + time * (second_color[1] - first_color[1]),
		first_color[2] + time * (second_color[2] - first_color[2]),
		first_color[3] + time * (second_color[3] - first_color[3])
	);
}

bool element_is_hovering(const ImVec2& min, const ImVec2& max)
{
	const auto mouse_pos = ImGui::GetIO().MousePos;

	return mouse_pos.x >= min.x && mouse_pos.y >= min.y && mouse_pos.x <= max.x && mouse_pos.y <= max.y;
}
#include "../menu.hpp"
ImVec2 movying_element_position(ImVec2 drag_area_size, bool draw_center_line)
{
	static ImVec2 old_mouse_position{ };
	static bool once = false;
	if (!once)
	{
		g_Options.positionsind = { ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2 };
		once = true;
	}
	ImVec2 current_mouse_position = ImGui::GetIO().MousePos;

	static bool start_dragging = false;
	const bool hovering_drag_area = element_is_hovering(g_Options.positionsind, g_Options.positionsind + ImVec2(drag_area_size.x, drag_area_size.y)) && Menu::Get().IsVisible();

	ImVec2 delta = start_dragging ? current_mouse_position - old_mouse_position : ImVec2(0, 0);

	if (hovering_drag_area && ImGui::IsMouseClicked(0)) {
		start_dragging = true;
	}
	else if (start_dragging) {
		if (ImGui::IsMouseDown(0)) {
			g_Options.positionsind += delta;
		}
		else {
			start_dragging = false;
		}
	}

	if (start_dragging) {
		old_mouse_position = current_mouse_position;

		if (draw_center_line)
		{
			Render::Get().RenderLine(0, (int)ImGui::GetIO().DisplaySize.y / 2, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y / 2, Color(255, 255, 255), 1.0f);
			Render::Get().RenderLine((int)ImGui::GetIO().DisplaySize.x / 2, 0, (int)ImGui::GetIO().DisplaySize.x / 2, (int)ImGui::GetIO().DisplaySize.y, Color(255, 255, 255), 1.0f);
		}
	}

	g_Options.positionsind = ImClamp(g_Options.positionsind, ImVec2(ImGui::GetIO().DisplaySize.x / 2, 0.f), ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y - drag_area_size.y));

	return g_Options.positionsind;
}
void Visuals::sunset() {
	static auto rot_override = g_CVar->FindVar("cl_csm_rot_override");
	static auto shadow_dist = g_CVar->FindVar("cl_csm_max_shadow_dist");
	static auto rot_x = g_CVar->FindVar("cl_csm_rot_x");
	static auto rot_y = g_CVar->FindVar("cl_csm_rot_y");

	rot_override->SetValue(1);
	shadow_dist->SetValue(800);
	rot_x->SetValue(g_Options.sunset_x);
	rot_y->SetValue(g_Options.sunset_y);


}
Color hsb(float hue, float saturation, float brightness, float alpha) {
	const float h = std::fmodf(hue, 1.0f) * 6.0f;
	const int i = static_cast<int>(h);
	const float f = h - static_cast<float>(i);
	const float p = brightness * (1.0f - saturation);
	const float q = brightness * (1.0f - saturation * f);
	const float t = brightness * (1.0f - saturation * (1.0f - f));

	float r = 0.0f, g = 0.0f, b = 0.0f;

	switch (i) {
	case 0:
		r = brightness, g = t, b = p;
		break;
	case 1:
		r = q, g = brightness, b = p;
		break;
	case 2:
		r = p, g = brightness, b = t;
		break;
	case 3:
		r = p, g = q, b = brightness;
		break;
	case 4:
		r = t, g = p, b = brightness;
		break;
	case 5:
	default:
		r = brightness, g = p, b = q;
		break;
	}

	return Color(r, g, b, alpha);
}

Color interpolate_hsb(float first_hue, float first_saturation, float first_brightness, float first_alpha, float second_hue, float second_saturation, float second_brightness, float second_alpha, float time) {
	float hue = first_hue + time * (second_hue - first_hue);
	float saturation = first_saturation + time * (second_saturation - first_saturation);
	float brightness = first_brightness + time * (second_brightness - first_brightness);
	float alpha = first_alpha + time * (second_alpha - first_alpha);

	return hsb(hue, saturation, brightness, alpha);
}
void Visuals::Velocity()
{
	if (!g_Options.movement.indicators.velocity.enable)
		return;

	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
		return;

	auto local = g_LocalPlayer;
	if (!local || !local->IsAlive())
		return;

	static int last_speed = 0, last_jump_speed = 0, last_delta = 0;
	static float last_update = 0.0f, take_off_time = 0.0f;
	int current_speed = round(local->m_vecVelocity().Length2D());
	static bool last_onground = false;
	bool current_onground = local->m_fFlags() & FL_ONGROUND;

	if (last_onground && !current_onground)
	{
		last_jump_speed = current_speed;
		take_off_time = g_GlobalVars->curtime + (g_Options.movement.indicators.velocity.disable_takeoff_on_ground ? 0.0f : 3.5f);
	}

	float get_velocity_factor = std::clamp(float(current_speed) / g_Options.movement.indicators.velocity.maximum_value, 0.0f, 1.0f);

	auto delta_velocity_color = [](int velocity) -> Color
		{
			if (velocity > 0)
			{
				return Color(g_Options.movement.indicators.velocity.delta_color.positive[0], g_Options.movement.indicators.velocity.delta_color.positive[1], g_Options.movement.indicators.velocity.delta_color.positive[2], g_Options.movement.indicators.velocity.delta_color.positive[3]);
			}
			else if (velocity < 0)
			{
				return Color(g_Options.movement.indicators.velocity.delta_color.negative[0], g_Options.movement.indicators.velocity.delta_color.negative[1], g_Options.movement.indicators.velocity.delta_color.negative[2], g_Options.movement.indicators.velocity.delta_color.negative[3]);
			}
			else if (velocity > -1 && velocity < 1)
			{
				return Color(g_Options.movement.indicators.velocity.delta_color.neutral[0], g_Options.movement.indicators.velocity.delta_color.neutral[1], g_Options.movement.indicators.velocity.delta_color.neutral[2], g_Options.movement.indicators.velocity.delta_color.neutral[3]);
			}
		};

	Color interpolation_color = Color::lerp_color(Color(g_Options.movement.indicators.velocity.interpolate_color.first[0], g_Options.movement.indicators.velocity.interpolate_color.first[1], g_Options.movement.indicators.velocity.interpolate_color.first[2], g_Options.movement.indicators.velocity.interpolate_color.first[3]), Color(g_Options.movement.indicators.velocity.interpolate_color.second[0], g_Options.movement.indicators.velocity.interpolate_color.second[1], g_Options.movement.indicators.velocity.interpolate_color.second[2], g_Options.movement.indicators.velocity.interpolate_color.second[3]), get_velocity_factor);
	Color hue_saturation_brightness_alpha_color = hsb(get_velocity_factor, g_Options.movement.indicators.velocity.hsb_color.saturation, 1.0f, 1.0f);

	Color selection_color = Color();

	switch (g_Options.movement.indicators.velocity.style)
	{
	case 0:
		
			selection_color[0] =  delta_velocity_color(last_delta)[0];
			selection_color[1] =  delta_velocity_color(last_delta)[1];
			selection_color[2] =  delta_velocity_color(last_delta)[2];
			selection_color[3] =  delta_velocity_color(last_delta)[3];
		break;
	case 1:
			selection_color[0] =   interpolation_color[0];
			selection_color[1] =   interpolation_color[1];
			selection_color[2] =   interpolation_color[2];
			selection_color[3] =   interpolation_color[3];
	
		break;
	case 2:
		selection_color[0] = hue_saturation_brightness_alpha_color[0];
		selection_color[1] = hue_saturation_brightness_alpha_color[1];
		selection_color[2] = hue_saturation_brightness_alpha_color[2];
		selection_color[3] = hue_saturation_brightness_alpha_color[3];
		break;
	}

	last_delta = current_speed - last_speed;

	const bool should_draw_takeoff = (!current_onground || (take_off_time > g_GlobalVars->curtime)) && g_Options.movement.indicators.velocity.takeoff;

	std::string place_holder_velocity = std::to_string(current_speed);

	if (current_onground || local->m_nMoveType() == MOVETYPE_LADDER)
	{
		place_holder_velocity = std::to_string(current_speed);
	}
	else if (should_draw_takeoff)
	{
		place_holder_velocity += " (" + std::to_string(last_jump_speed) + ")";
	}

	ImVec2 position = movying_element_position(ImVec2(50, 50),true);

	lolrendertext(position.x, position.y, g_Options.fonts.indi_size,fonts::indicator_font, place_holder_velocity.c_str(), true, selection_color, g_Options.fonts.indi_font_flag[9], g_Options.fonts.indi_font_flag[10]);

	if (std::fabs(g_GlobalVars->tickcount - last_update) > 5)
	{
		last_speed = current_speed;
		last_update = g_GlobalVars->tickcount + 0.05f;
	}

	last_onground = current_onground;
}
void Visuals::stamina()
{
	if (!g_Options.movement.indicators.stamina.enable)
		return;

	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
		return;

	auto local = g_LocalPlayer;
	if (!local || !local->IsAlive())
		return;

	static float last_speed = 0, last_jump_speed = 0, last_update = 0.0f, take_off_time = 0.0f;
	float current_speed = local->fl_stamina();
	static bool last_onground = false;
	bool current_onground = local->m_fFlags() & FL_ONGROUND;

	if (last_onground && !current_onground)
	{
		last_jump_speed = current_speed;
		take_off_time = g_GlobalVars->curtime + (g_Options.movement.indicators.stamina.disable_takeoff_on_ground ? 0.0f : 3.5f);
	}

	std::string value_str;

	std::stringstream ss;
	ss << std::fixed << std::setprecision(1) << current_speed;
	value_str = ss.str();

	std::string str = value_str;

	std::stringstream ss1;
	ss1 << std::fixed << std::setprecision(1) << last_jump_speed;
	std::string value_str2 = ss1.str();

	const auto should_draw_takeoff = (!current_onground || (take_off_time > g_GlobalVars->curtime)) && g_Options.movement.indicators.stamina.takeoff;

	if (current_onground || local->m_nMoveType() == MOVETYPE_LADDER)
	{
		str = value_str;
	}
	else if (should_draw_takeoff)
	{
		str += " (" + value_str2 + ")";
	}

	float get_stamina_factor = std::clamp(float(current_speed) / g_Options.movement.indicators.stamina.maximum_value, 0.0f, 1.0f);
	Color static_color = Color(g_Options.movement.indicators.stamina.color[0], g_Options.movement.indicators.stamina.color[1], g_Options.movement.indicators.stamina.color[2], g_Options.movement.indicators.stamina.color[3]);
	Color interpolation_color = Color::lerp_color(Color(g_Options.movement.indicators.stamina.interpolate_color.first[0], g_Options.movement.indicators.stamina.interpolate_color.first[1], g_Options.movement.indicators.stamina.interpolate_color.first[2], g_Options.movement.indicators.stamina.interpolate_color.first[3]), Color(g_Options.movement.indicators.stamina.interpolate_color.second[0], g_Options.movement.indicators.stamina.interpolate_color.second[1], g_Options.movement.indicators.stamina.interpolate_color.second[2], g_Options.movement.indicators.stamina.interpolate_color.second[3]), get_stamina_factor);
	Color hue_saturation_brightness_alpha_color = hsb(get_stamina_factor, g_Options.movement.indicators.stamina.hsb_color.saturation, 1.0f, 1.0f);

	Color selection_color;

	switch (g_Options.movement.indicators.stamina.style)
	{
	case 0:
		
		selection_color[0] =		 static_color [0];
			selection_color[1] =	 static_color [1];
			selection_color[2] =	 static_color [2];
			selection_color[3] =	 static_color [3];
		break;
	case 1:
		
			selection_color[0] =  interpolation_color [0];
			selection_color[1] =  interpolation_color [1];
			selection_color[2] =  interpolation_color [2];
			selection_color[3] =  interpolation_color [3];
		break;
	case 2:
		
			selection_color[0] =   hue_saturation_brightness_alpha_color [0];
			selection_color[1] =   hue_saturation_brightness_alpha_color [1];
			selection_color[2] =   hue_saturation_brightness_alpha_color [2];
			selection_color[3] =   hue_saturation_brightness_alpha_color [3];
		break;
	}

	ImVec2 position = movying_element_position(ImVec2(50, 50),true);

	lolrendertext(position.x, position.y + g_Options.fonts.indi_size + 5, g_Options.fonts.indi_size, fonts::indicator_font, str.c_str(), true, selection_color, g_Options.fonts.indi_font_flag[9], g_Options.fonts.indi_font_flag[10]);

	if (g_GlobalVars->tickcount > last_update)
	{
		last_speed = current_speed;
		last_update = g_GlobalVars->tickcount + 0.05f;
	}

	last_onground = current_onground;
}
#define niggaballs 3.14159265358979323846


extern bool isLadder;

void Visuals::RenderIndicators() {



	//if (!g_EngineClient->IsConnected())
	//	return;
	if (!g_Options.misc_indicators)
		return;

	int w, h;

	ImVec2 position = movying_element_position(ImVec2(50, 50), true);
	h = position.y + g_Options.fonts.indi_size + 5;
	w = position.x * 2;
	static Color EdgeBugColor = Color(255, 255, 255, 0);
	static Color PixelsurfColor = Color(255, 255, 255, 0);
	static Color JumpBugColor = Color(255, 255, 255, 0);
	static float fEdgeBugColor = 0.f;
	static float fPixelsurfColor = 0.f;
	static float fJumpBugColor = 0.f;
	clock_t currentTime = clock();

	// Если это первый вызов, инициализируем lastTime
	if (lastTime == 0) {
		lastTime = currentTime;
	}

	// Вычисление прошедшего времени
	double elapsedTime = static_cast<double>(currentTime - lastTime) / CLOCKS_PER_SEC;

	if (g_Options.pixelsurf && g_Options.showps) {
		pixelsurf_alpha += static_cast<float>(alphaChangePerSecond * elapsedTime);
	}
	else {
		pixelsurf_alpha -= static_cast<float>(alphaChangePerSecond * elapsedTime);
	}

	if (pixelsurf_alpha > 1.0f) pixelsurf_alpha = 1.0f;
	if (pixelsurf_alpha < 0.0f) pixelsurf_alpha = 0.0f;

	if (pixelsurf_alpha > 0.f && g_Options.showps) {
		h+= g_Options.fonts.indi_size * pixelsurf_alpha;
		Render::Get().RenderText("ps", ImVec2(w / 2 + 1, h  + 1), g_Options.size_test, should_pixelsurf ? Color(0, 0, 0, static_cast<int>(pixelsurf_alpha * 255)) : Color(0, 0, 0, static_cast<int>(pixelsurf_alpha * 255)), true, false, fonts::indicator_font);
		Render::Get().RenderText("ps", ImVec2(w / 2, h ), g_Options.size_test, should_pixelsurf ? Color(g_Options.psdetect.r(), g_Options.psdetect.g(), g_Options.psdetect.b(), static_cast<int>(pixelsurf_alpha * 255)) : Color(255, 255, 255, static_cast<int>(pixelsurf_alpha * 255)), true, false, fonts::indicator_font);
	}

	// Логика изменения альфа-канала с постоянной скоростью
	if (GetAsyncKeyState(g_Options.edge_bug_key) && g_Options.edge_bug) {
		edgebug_alpha += static_cast<float>(alphaChangePerSecond * elapsedTime);
	}
	else {
		edgebug_alpha -= static_cast<float>(alphaChangePerSecond * elapsedTime);
	}

	if (edgebug_alpha > 1.0f) edgebug_alpha = 1.0f;
	if (edgebug_alpha < 0.0f) edgebug_alpha = 0.0f;


	if (edgebug_alpha > 0.f && g_Options.showeb) {
		h+= g_Options.fonts.indi_size * edgebug_alpha; // heh Я ПРОСТО ГЕНИЙ МАТЕМАТИКИ
		Render::Get().RenderText("eb", ImVec2(w / 2 + 1, h  + 1), g_Options.size_test, NewOne::EdgeBug_Founded ? Color(0, 0, 0, static_cast<int>(edgebug_alpha * 255)) : Color(0, 0, 0, static_cast<int>(edgebug_alpha * 255)), true, false, fonts::indicator_font);
		Render::Get().RenderText("eb", ImVec2(w / 2, h ), g_Options.size_test, NewOne::EdgeBug_Founded ? Color(g_Options.ebdetect.r(), g_Options.ebdetect.g(), g_Options.ebdetect.b(), static_cast<int>(edgebug_alpha * 255)) : Color(255, 255, 255, static_cast<int>(edgebug_alpha * 255)), true, false, fonts::indicator_font);

	}



	// Логика изменения альфа-канала с постоянной скоростью
	if (GetAsyncKeyState(g_Options.FireManKey) && g_Options.FireMan) {
		fr_alpha += static_cast<float>(alphaChangePerSecond * elapsedTime);
	}
	else {
		fr_alpha -= static_cast<float>(alphaChangePerSecond * elapsedTime);
	}

	if (fr_alpha > 1.0f) fr_alpha = 1.0f;
	if (fr_alpha < 0.0f) fr_alpha = 0.0f;


	if (fr_alpha > 0.f && g_Options.FireMan) {
		h += g_Options.fonts.indi_size * fr_alpha; // heh Я ПРОСТО ГЕНИЙ МАТЕМАТИКИ
		Render::Get().RenderText("fr", ImVec2(w / 2 + 1, h + 1), g_Options.size_test, isLadder ? Color(0, 0, 0, static_cast<int>(fr_alpha * 255)) : Color(0, 0, 0, static_cast<int>(fr_alpha * 255)), true, false, fonts::indicator_font);
		Render::Get().RenderText("fr", ImVec2(w / 2, h), g_Options.size_test, isLadder ? Color(255,0,0, static_cast<int>(fr_alpha * 255)) : Color(255, 255, 255, static_cast<int>(fr_alpha * 255)), true, false, fonts::indicator_font);

	}




	if (g_Options.edgejump.edge_jump_duck_in_air && GetAsyncKeyState(g_Options.edgejump.hotkey) && g_Options.showlj) {
		longjump_alpha += static_cast<float>(alphaChangePerSecond * elapsedTime);
	}
	else {
		longjump_alpha -= static_cast<float>(alphaChangePerSecond * elapsedTime);
	}

	if (longjump_alpha > 1.0f) longjump_alpha = 1.0f;
	if (longjump_alpha < 0.0f) longjump_alpha = 0.0f;

	if (longjump_alpha > 0.f && g_Options.showlj) {
		h+= g_Options.fonts.indi_size * longjump_alpha;
		Render::Get().RenderText("lj", ImVec2(w / 2 + 1, h  + 1), g_Options.size_test, should_longjump ? Color(0, 0, 0, static_cast<int>(longjump_alpha * 255)) : Color(0, 0, 0, static_cast<int>(longjump_alpha * 255)), true, false, fonts::indicator_font);
		Render::Get().RenderText("lj", ImVec2(w / 2, h ), g_Options.size_test, should_longjump ? Color(255, 255, 255, static_cast<int>(longjump_alpha * 255)) : Color(255, 255, 255, static_cast<int>(longjump_alpha * 255)), true, false, fonts::indicator_font);
	}

	if (g_Options.misc_minijump && GetAsyncKeyState(g_Options.abobakey) && g_Options.showmj) {
		minijump_alpha += static_cast<float>(alphaChangePerSecond * elapsedTime);
	}
	else {
		minijump_alpha -= static_cast<float>(alphaChangePerSecond * elapsedTime);
	}

	if (minijump_alpha > 1.0f) minijump_alpha = 1.0f;
	if (minijump_alpha < 0.0f) minijump_alpha = 0.0f;

	if (minijump_alpha > 0.f && g_Options.showmj) {
		h+= g_Options.fonts.indi_size * minijump_alpha;
		Render::Get().RenderText("mj", ImVec2(w / 2 + 1, h  + 1), g_Options.size_test, should_minijump ? Color(0, 0, 0, static_cast<int>(minijump_alpha * 255)) : Color(0, 0, 0, static_cast<int>(minijump_alpha * 255)), true, false, fonts::indicator_font);
		Render::Get().RenderText("mj", ImVec2(w / 2, h ), g_Options.size_test, should_minijump ? Color(255, 255, 255, static_cast<int>(minijump_alpha * 255)) : Color(255, 255, 255, static_cast<int>(minijump_alpha * 255)), true, false, fonts::indicator_font);
	}

	int saved_ticks = 0;

	if (should_jumpbug) {
		saved_ticks = g_GlobalVars->tickcount;
	}


	if (g_Options.jump_bug && GetAsyncKeyState(g_Options.jump_bug_key) && g_Options.showjb) {
		jumpbug_alpha += static_cast<float>(alphaChangePerSecond * elapsedTime);
	}
	else {
		jumpbug_alpha -= static_cast<float>(alphaChangePerSecond * elapsedTime);
	}

	if (jumpbug_alpha > 1.0f) jumpbug_alpha = 1.0f;
	if (jumpbug_alpha < 0.0f) jumpbug_alpha = 0.0f;

	if (jumpbug_alpha > 0.f && g_Options.showjb) {
		h+= g_Options.fonts.indi_size * jumpbug_alpha;
		Render::Get().RenderText("jb", ImVec2(w / 2 + 1, h  + 1), g_Options.size_test, g_GlobalVars->tickcount - saved_ticks < 15 ? Color(0, 0, 0, static_cast<int>(jumpbug_alpha * 255)) : Color(0, 0, 0, static_cast<int>(jumpbug_alpha * 255)), true, false, fonts::indicator_font);
		Render::Get().RenderText("jb", ImVec2(w / 2, h ), g_Options.size_test, g_GlobalVars->tickcount - saved_ticks < 15 ? Color(g_Options.jbdetect) : Color(255, 255, 255, static_cast<int>(jumpbug_alpha * 255)), true, false, fonts::indicator_font);
	}

	if (g_Options.CHECKSURFLOLKEK && (GetAsyncKeyState(g_Options.KEYCHECKSURFLOLKEK) || g_Options.togglechecksurflolkek) && g_Options.showcslk) {
		checksurflolkek_alpha += static_cast<float>(alphaChangePerSecond * elapsedTime);
	}
	else {
		checksurflolkek_alpha -= static_cast<float>(alphaChangePerSecond * elapsedTime);
	}

	if (checksurflolkek_alpha > 1.0f) checksurflolkek_alpha = 1.0f;
	if (checksurflolkek_alpha < 0.0f) checksurflolkek_alpha = 0.0f;

	if (checksurflolkek_alpha > 0.f && g_Options.showcslk) {
		h += g_Options.fonts.indi_size * checksurflolkek_alpha;
		Render::Get().RenderText("ast", ImVec2(w / 2 + 1, h + 1), g_Options.size_test,  Color(0, 0, 0, static_cast<int>(checksurflolkek_alpha * 255)), true, false, fonts::indicator_font);
		Render::Get().RenderText("ast", ImVec2(w / 2, h), g_Options.size_test,  Color(255, 255, 255, static_cast<int>(checksurflolkek_alpha * 255)), true, false, fonts::indicator_font);
	}
	lastTime = currentTime;
}

void Visuals::Player::oofarr(C_BasePlayer* pl)
{


	float width = g_Options.oofarcsize;
	QAngle viewangles;
	g_EngineClient->GetViewAngles(&viewangles);
	int w, h;
	g_EngineClient->GetScreenSize(w, h);

	auto clr = ctx.is_visible ? g_Options.arc_visible : g_Options.arc_invisible;

	auto angle = viewangles.yaw - Math::CalcAngle(g_LocalPlayer->GetEyePos(), pl->abs_origin()).yaw - 90;
	Render::Get().arc(w / 2, h / 2, g_Options.radius, angle - width, angle + width, Color(clr), 4.f);

//	render::arc(w / 2, h / 2, 250, angle - width, angle + width, Color(1.f, 1.f, 1.f, 0.5f), 1.5f);
} 


struct velocity_data_t
{
	int speed;
	bool on_ground;
	bool pixelsurfed;
	bool jumpbugged;
	bool edgebugged;
	bool was_in_prediction;
};

struct stamina_data_t
{
	float stamina;
	bool on_ground;
};
ImVec2 graph_position{};
inline std::vector<velocity_data_t> velocity_data;
inline std::vector<stamina_data_t> stamina_data;
extern bool predicred;
void Visuals::graphs_data()
{
	if (!g_Options.graphs.velocity.enable) {
		if (!velocity_data.empty())
		{
			velocity_data.clear();
		}
	}

	if (!g_Options.graphs.stamina.enable) {
		if (!stamina_data.empty())
		{
			stamina_data.clear();
		}
	}

	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
	{
		if (!velocity_data.empty())
		{
			velocity_data.clear();
		}

		if (!stamina_data.empty())
		{
			stamina_data.clear();
		}

		return;
	}

	if (velocity_data.size() > 255)
	{
		velocity_data.erase(velocity_data.begin());
		velocity_data.pop_back();
	}

	if (stamina_data.size() > 255)
	{
		stamina_data.erase(stamina_data.begin());
		stamina_data.pop_back();
	}

	ImVec2 position = movying_element_position(ImVec2(50, 50), true);

	graph_position = ImVec2(position.x + 255 * (g_Options.graphs.size / 2.f), position.y - g_Options.fonts.indi_size);

	velocity_data_t current_velocity_data;
	stamina_data_t current_stamina_data;

	current_velocity_data.speed =g_LocalPlayer->m_vecVelocity().Length2D();
	current_velocity_data.on_ground = g_LocalPlayer->m_fFlags() & 1;
	current_velocity_data.jumpbugged = should_jumpbug;
	current_velocity_data.was_in_prediction = predicred;
	current_stamina_data.stamina = g_LocalPlayer->fl_stamina();
	current_stamina_data.on_ground = g_LocalPlayer->m_fFlags() & 1;

	velocity_data.insert(velocity_data.begin(), current_velocity_data);
	stamina_data.insert(stamina_data.begin(), current_stamina_data);
}

void Visuals::velocity_graph_indicator()
{
	if (!g_Options.graphs.velocity.enable)
	{
		if (!velocity_data.empty())
		{
			velocity_data.clear();
		}


		return;
	}

	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
		if (!velocity_data.empty())
		{
			velocity_data.clear();
		}

		return;
	}

	if (velocity_data.size() < 2)
	{
		return;
	}

	for (auto i = 0; i < velocity_data.size() - 1; i++)
	{
		const auto current = velocity_data[i];
		const auto next = velocity_data[i + 1];

		const auto clamped_current_speed = std::clamp(current.speed, 0, 400);
		const auto clamped_next_speed = std::clamp(next.speed, 0, 400);

		bool jumped = !current.on_ground && next.on_ground;
		bool landed = current.on_ground && !next.on_ground;
		const auto jump_bug_detected = !current.was_in_prediction && current.jumpbugged;

		float current_speed = (clamped_current_speed * 75 / 400);
		float next_speed = (clamped_next_speed * 75 / 400);

		float max_val = velocity_data.size() - 1;
		float val = i;

		int fade_alpha = g_Options.graphs.fade ? static_cast<int>(fabs(max_val - fabs(val - max_val / 2) * 2.0f)) : 255;

		Color color = Color(g_Options.graphs.velocity.color[0], g_Options.graphs.velocity.color[1], g_Options.graphs.velocity.color[2], (fade_alpha / 255.f));

		Render::Get().RenderLine(graph_position.x - (i - 1) * g_Options.graphs.size, graph_position.y - current_speed, graph_position.x - i * g_Options.graphs.size, graph_position.y - next_speed, color, 1.0f);

		if (jumped && g_Options.graphs.velocity.draw_velocity)
		{
			lolrendertext(graph_position.x - (i - 1) * g_Options.graphs.size, graph_position.y - next_speed - (g_Options.fonts.sub_indi_size + 9), g_Options.fonts.sub_indi_size, fonts::sub_indicator_font, std::to_string((int)round(current.speed)).c_str(), true, Color(1.0f, 1.0f, 1.0f, (fade_alpha / 255.f)), g_Options.fonts.sub_indi_font_flag[9], g_Options.fonts.sub_indi_font_flag[10]);
			//im_render.text(graph_position.x - (i - 1) * c::movement::indicators::graphs::size, graph_position.y - current_speed - (g_Options.fonts.sub_indi_size + 21), g_Options.fonts.sub_indi_size, fonts::sub_indicator_font, std::to_string((int)round(next.speed)).c_str(), true, color_t(1.0f, 1.0f, 1.0f, (alpha / 255.f)), g_Options.fonts.sub_indi_font_flag[9], g_Options.fonts.sub_indi_font_flag[10]);
		}

		if (jump_bug_detected && g_Options.graphs.velocity.draw_jumpbug)
		{
			lolrendertext(graph_position.x - (i - 1) * g_Options.graphs.size, graph_position.y - current_speed - (g_Options.fonts.sub_indi_size + 9), g_Options.fonts.sub_indi_size, fonts::sub_indicator_font, "jb", true, Color(1.0f, 1.0f, 1.0f, (fade_alpha / 255.f)), g_Options.fonts.sub_indi_font_flag[9], g_Options.fonts.sub_indi_font_flag[10]);
		}
	}
}

void Visuals::stamina_graph_indicator()
{
	if (!g_Options.graphs.stamina.enable) {
		if (!stamina_data.empty())
		{
			stamina_data.clear();
		}

		return;
	}

	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
		if (!stamina_data.empty())
		{
			stamina_data.clear();
		}

		return;
	}

	if (stamina_data.size() < 2)
	{
		return;
	}

	for (auto i = 0; i < stamina_data.size() - 1; i++)
	{
		const auto current = stamina_data[i];
		const auto next = stamina_data[i + 1];

		const auto clamped_current_speed = std::clamp(current.stamina, 0.0f, 35.0f);
		const auto clamped_next_speed = std::clamp(next.stamina, 0.0f, 35.0f);

		float current_speed = (clamped_current_speed * 25 / 35);
		float next_speed = (clamped_next_speed * 25 / 35);

		float max_val = velocity_data.size() - 1;
		float val = i;

		int alpha = g_Options.graphs.fade ? fabs(max_val - fabs(val - max_val / 2) * 2.f) : 255;

		Color color = Color(g_Options.graphs.stamina.color[0], g_Options.graphs.stamina.color[1], g_Options.graphs.stamina.color[2], (alpha / 255.f));

		Render::Get().RenderLine(graph_position.x - (i - 1) * g_Options.graphs.size, graph_position.y - current_speed, graph_position.x - i * g_Options.graphs.size, graph_position.y - next_speed, color, 1.0f);
	}
}
void draw_screen_effect(IMaterial* material) {
	static auto fn = Utils::PatternScan2("client.dll", "55 8B EC 83 E4 ? 83 EC ? 53 56 57 8D 44 24 ? 89 4C 24 ?");
	int w, h;
	g_EngineClient->GetScreenSize(w, h);
	__asm {
		push h
		push w
		push 0
		xor edx, edx
		mov ecx, material
		call fn
		add esp, 12
	}
}

struct motion_blur_history {
	motion_blur_history() {
		last_time_update = 0.0f;
		previous_pitch = 0.0f;
		previous_yaw = 0.0f;
		previous_pos = Vector{ 0.0f, 0.0f, 0.0f };
		no_rotational_mb_until = 0.0f;
	}

	float last_time_update;
	float previous_pitch;
	float previous_yaw;
	Vector previous_pos;
	float no_rotational_mb_until;
};

void Visuals::motion_blur(CViewSetup* setup)
{
	if (!g_Options.motionblur.enable)
	{
		return;
	}

	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
	{
		return;
	}

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
	{
		return;
	}

	static motion_blur_history history;
	static float motion_blur_values[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (setup)
	{
		const float time_elapsed = g_GlobalVars->realtime - history.last_time_update;

		const auto view_angles = setup->angles;

		float current_pitch = view_angles.pitch;

		while (current_pitch > 180.0f)
			current_pitch -= 360.0f;
		while (current_pitch < -180.0f)
			current_pitch += 360.0f;

		float current_yaw = view_angles.yaw;

		while (current_yaw > 180.0f)
			current_yaw -= 360.0f;
		while (current_yaw < -180.0f)
			current_yaw += 360.0f;

		Vector current_side_vector;
		Vector current_forward_vector;
		Vector current_up_vector;
		Math::AngleVectors(setup->angles, &current_forward_vector, &current_side_vector, &current_up_vector);

		Vector current_position = setup->origin;
		Vector position_change = history.previous_pos - current_position;

		if ((position_change.Length() > 30.0f) && (time_elapsed >= 0.5f))
		{
			motion_blur_values[0] = 0.0f;
			motion_blur_values[1] = 0.0f;
			motion_blur_values[2] = 0.0f;
			motion_blur_values[3] = 0.0f;
		}
		else if (time_elapsed > (1.0f / 15.0f))
		{
			motion_blur_values[0] = 0.0f;
			motion_blur_values[1] = 0.0f;
			motion_blur_values[2] = 0.0f;
			motion_blur_values[3] = 0.0f;
		}
		else if (position_change.Length() > 50.0f)
		{
			history.no_rotational_mb_until = g_GlobalVars->realtime + 1.0f;
		}
		else
		{
			const float horizontal_fov = setup->fov;
			const float vertical_fov = (setup->aspect_ratio <= 0.0f) ? (setup->fov) : (setup->fov / setup->aspect_ratio);
			const float viewdot_motion = current_forward_vector.Dot(position_change);

			if (g_Options.motionblur.forward_move_blur)
			{
				motion_blur_values[2] = viewdot_motion;
			}

			const float sidedot_motion = current_side_vector.Dot(position_change);
			float yawdiff_original = history.previous_yaw - current_yaw;

			if (((history.previous_yaw - current_yaw > 180.0f) || (history.previous_yaw - current_yaw < -180.0f)) && ((history.previous_yaw + current_yaw > -180.0f) && (history.previous_yaw + current_yaw < 180.0f)))
			{
				yawdiff_original = history.previous_yaw + current_yaw;
			}

			float yawdiff_adjusted = yawdiff_original + (sidedot_motion / 3.0f);

			if (yawdiff_original < 0.0f)
			{
				yawdiff_adjusted = std::clamp(yawdiff_adjusted, yawdiff_original, 0.0f);
			}
			else
			{
				yawdiff_adjusted = std::clamp(yawdiff_adjusted, 0.0f, yawdiff_original);
			}

			const float undampened_yaw = yawdiff_adjusted / horizontal_fov;
			motion_blur_values[0] = undampened_yaw * (1.0f - (fabsf(current_pitch) / 90.0f));

			const float pitch_compensate_mask = 1.0f - ((1.0f - fabsf(current_forward_vector[2])) * (1.0f - fabsf(current_forward_vector[2])));
			const float pitchdiff_original = history.previous_pitch - current_pitch;
			float pitchdiff_adjusted = pitchdiff_original;

			if (current_pitch > 0.0f)
			{
				pitchdiff_adjusted = pitchdiff_original - ((viewdot_motion / 2.0f) * pitch_compensate_mask);
			}
			else
			{
				pitchdiff_adjusted = pitchdiff_original + ((viewdot_motion / 2.0f) * pitch_compensate_mask);
			}


			if (pitchdiff_original < 0.0f)
			{
				pitchdiff_adjusted = std::clamp(pitchdiff_adjusted, pitchdiff_original, 0.0f);
			}
			else
			{
				pitchdiff_adjusted = std::clamp(pitchdiff_adjusted, 0.0f, pitchdiff_original);
			}

			motion_blur_values[1] = pitchdiff_adjusted / vertical_fov;
			motion_blur_values[3] = undampened_yaw;
			motion_blur_values[3] *= (fabs(current_pitch) / 90.0f) * (fabs(current_pitch) / 90.0f) * (fabs(current_pitch) / 90.0f);

			if (time_elapsed > 0.0f)
			{
				motion_blur_values[2] /= time_elapsed * 30.0f;
			}
			else
			{
				motion_blur_values[2] = 0.0f;
			}

			motion_blur_values[2] = std::clamp((fabsf(motion_blur_values[2]) - g_Options.motionblur.falling_minimum) / (g_Options.motionblur.falling_maximum - g_Options.motionblur.falling_minimum), 0.0f, 1.0f) * (motion_blur_values[2] >= 0.0f ? 1.0f : -1.0f);
			motion_blur_values[2] /= 30.0f;
			motion_blur_values[0] *= g_Options.motionblur.rotate_intensity * .15f * g_Options.motionblur.strength;
			motion_blur_values[1] *= g_Options.motionblur.rotate_intensity * .15f * g_Options.motionblur.strength;
			motion_blur_values[2] *= g_Options.motionblur.rotate_intensity * .15f * g_Options.motionblur.strength;
			motion_blur_values[3] *= g_Options.motionblur.falling_intensity * .15f * g_Options.motionblur.strength;
		}

		if (g_GlobalVars->realtime < history.no_rotational_mb_until)
		{
			motion_blur_values[0] = 0.0f;
			motion_blur_values[1] = 0.0f;
			motion_blur_values[3] = 0.0f;
		}
		else
		{
			history.no_rotational_mb_until = 0.0f;
		}

		history.previous_pos = current_position;
		history.previous_pitch = current_pitch;
		history.previous_yaw = current_yaw;
		history.last_time_update = g_GlobalVars->realtime;

		return;
	}

	IMaterial* material = g_MatSystem->FindMaterial("dev/motion_blur", "RenderTargets", false);

	if (material->IsErrorMaterial())
	{
		return;
	}

	const auto motion_blur_internal = material->find_var("$MotionBlurInternal");

	motion_blur_internal->set_vector_component(motion_blur_values[0], 0);
	motion_blur_internal->set_vector_component(motion_blur_values[1], 1);
	motion_blur_internal->set_vector_component(motion_blur_values[2], 2);
	motion_blur_internal->set_vector_component(motion_blur_values[3], 3);

	const auto motion_blur_view_port_internal = material->find_var("$MotionBlurViewportInternal");

	motion_blur_view_port_internal->set_vector_component(0.0f, 0);
	motion_blur_view_port_internal->set_vector_component(0.0f, 1);
	motion_blur_view_port_internal->set_vector_component(1.0f, 2);
	motion_blur_view_port_internal->set_vector_component(1.0f, 3);

	if (g_Options.motionblur.video_adapter == 0 || g_Options.motionblur.video_adapter == 3)
	{
		static ConVar* mat_res = g_CVar->FindVar("mat_resolveFullFrameDepth");
		mat_res->SetValue(0);
	}

	draw_screen_effect(material);
}
void Visuals::AddToDrawList() {
	stamina_graph_indicator();
	velocity_graph_indicator();
	player::run();
	
}
