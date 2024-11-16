#include "PlayerVisual.h"
#include "../options.hpp"
#include "bt.h"
#include "../renderer.h"
bool player::get_player_box(C_BasePlayer* entity, bbox_t& in)
{
	if (!entity)
	{
		return false;
	}

	auto trans = entity->m_rgflCoordinateFrame();

	auto collideable = entity->GetCollideable();

	if (!collideable)
	{
		return false;
	}

	Vector min = collideable->OBBMins();
	Vector max = collideable->OBBMaxs();

	Vector points[] =
	{
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector points_transformed[8];

	for (int i = 0; i < 8; i++)
	{
		Math::VectorTransform(points[i], trans, points_transformed[i]);
	}

	Vector pos = entity->abs_origin(), flb, brt, blb, frt, frb, brb, blt, flt;

	if (!Math::WorldToScreen(points_transformed[3], flb) ||
		!Math::WorldToScreen(points_transformed[5], brt) ||
		!Math::WorldToScreen(points_transformed[0], blb) ||
		!Math::WorldToScreen(points_transformed[4], frt) ||
		!Math::WorldToScreen(points_transformed[2], frb) ||
		!Math::WorldToScreen(points_transformed[1], brb) ||
		!Math::WorldToScreen(points_transformed[6], blt) ||
		!Math::WorldToScreen(points_transformed[7], flt))
	{
		return false;
	}

	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	float left = flb.x;
	float top = flb.y;
	float right = flb.x;
	float bottom = flb.y;

	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
		{
			left = arr[i].x;
		}

		if (bottom < arr[i].y)
		{
			bottom = arr[i].y;
		}

		if (right < arr[i].x)
		{
			right = arr[i].x;
		}

		if (top > arr[i].y)
		{
			top = arr[i].y;
		}
	}

	in.x = static_cast<int>(left);
	in.y = static_cast<int>(top);
	in.w = static_cast<int>(right) - static_cast<int>(left);
	in.h = static_cast<int>(bottom) - static_cast<int>(top);

	return true;
}

bool player::get_entity_box(C_BaseEntity* entity, bbox_t& in)
{
	if (!entity)
	{
		return false;
	}

	Vector min, max, flb, brt, blb, frt, frb, brb, blt, flt;

	auto collideable = entity->GetCollideable();

	if (!collideable)
	{
		return false;
	}

	min = collideable->OBBMins();
	max = collideable->OBBMaxs();

	matrix3x4_t trans = entity->m_rgflCoordinateFrame();

	Vector points[] =
	{
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector points_transformed[8];

	for (int i = 0; i < 8; i++)
	{
		Math::VectorTransform(points[i], trans, points_transformed[i]);
	}


	if (!Math::WorldToScreen(points_transformed[3], flb) ||
		!Math::WorldToScreen(points_transformed[5], brt) ||
		!Math::WorldToScreen(points_transformed[0], blb) ||
		!Math::WorldToScreen(points_transformed[4], frt) ||
		!Math::WorldToScreen(points_transformed[2], frb) ||
		!Math::WorldToScreen(points_transformed[1], brb) ||
		!Math::WorldToScreen(points_transformed[6], blt) ||
		!Math::WorldToScreen(points_transformed[7], flt))
	{
		return false;
	}

	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	float left = flb.x;
	float top = flb.y;
	float right = flb.x;
	float bottom = flb.y;

	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
		{
			left = arr[i].x;
		}

		if (bottom < arr[i].y)
		{
			bottom = arr[i].y;
		}

		if (right < arr[i].x)
		{
			right = arr[i].x;
		}

		if (top > arr[i].y)
		{
			top = arr[i].y;
		}
	}

	in.x = static_cast<int>(left);
	in.y = static_cast<int>(top);
	in.w = static_cast<int>(right) - static_cast<int>(left);
	in.h = static_cast<int>(bottom) - static_cast<int>(top);

	return true;
}

void player::run()
{

	{
		if (!g_Options.players.enable)
		{
			return;
		}

		if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
		{
			return;
		}

		for (int i = 1; i <= g_GlobalVars->maxClients; i++)
		{
			auto entity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));

			if (!entity || !entity->IsPlayer() || entity == g_LocalPlayer || entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
			{
				continue;
			}

			if (g_Options.players.death_history.enable)
			{
				draw_death_history(entity);
			}
		}

		for (int i = 1; i <= g_GlobalVars->maxClients; i++)
		{
			auto entity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));

			if (!entity || !entity->IsPlayer() || entity == g_LocalPlayer || entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
			{
				continue;
			}

			const int index = entity->EntIndex();

			if (g_Options.players.fade.enable)
			{
				if (entity->IsDormant() || !entity->IsAlive())
				{
					player_alpha[index] = std::max(player_alpha[index] - g_GlobalVars->frametime * g_Options.players.fade.time, 0.f);
				}
				else
				{
					player_alpha[index] = min(player_alpha[index] + g_GlobalVars->frametime * g_Options.players.fade.time, 1.f);
				}

				player_alpha[index] = std::clamp(player_alpha[index], 0.0f, 1.0f);
			}
			else
			{
				if (!entity->IsAlive() || entity->IsDormant())
				{
					continue;
				}
			}

			bool check_on_see = g_LocalPlayer->CanSeePlayer(entity, entity->GetEyePos());

			Color visible = g_Options.players.fade.enable ? Color(g_Options.players.colors.visible[0], g_Options.players.colors.visible[1], g_Options.players.colors.visible[2], float(player_alpha[index] * g_Options.players.colors.visible[3])) : Color(g_Options.players.colors.visible[0], g_Options.players.colors.visible[1], g_Options.players.colors.visible[2], g_Options.players.colors.visible[3]);
			Color invisible = g_Options.players.fade.enable ? Color(g_Options.players.colors.invisible[0], g_Options.players.colors.invisible[1], g_Options.players.colors.invisible[2], float(player_alpha[index] * g_Options.players.colors.invisible[3])) : Color(g_Options.players.colors.invisible[0], g_Options.players.colors.invisible[1], g_Options.players.colors.invisible[2], g_Options.players.colors.invisible[3]);
			Color name = g_Options.players.fade.enable ? Color(g_Options.players.colors.name[0], g_Options.players.colors.name[1], g_Options.players.colors.name[2], float(player_alpha[index] * g_Options.players.colors.name[3])) : Color(g_Options.players.colors.name[0], g_Options.players.colors.name[1], g_Options.players.colors.name[2], g_Options.players.colors.name[3]);
			Color box = g_Options.players.fade.enable ? Color(g_Options.players.colors.box[0], g_Options.players.colors.box[1], g_Options.players.colors.box[2], float(player_alpha[index] * g_Options.players.colors.box[3])) : Color(g_Options.players.colors.box[0], g_Options.players.colors.box[1], g_Options.players.colors.box[2], g_Options.players.colors.box[3]);
			Color box_outline = g_Options.players.fade.enable ? Color(g_Options.players.colors.box_outline[0], g_Options.players.colors.box_outline[1], g_Options.players.colors.box_outline[2], float(player_alpha[index] * g_Options.players.colors.box_outline[3])) : Color(g_Options.players.colors.box_outline[0], g_Options.players.colors.box_outline[1], g_Options.players.colors.box_outline[2], g_Options.players.colors.box_outline[3]);
			Color health_bar = g_Options.players.fade.enable ? Color(g_Options.players.colors.health_bar[0], g_Options.players.colors.health_bar[1], g_Options.players.colors.health_bar[2], float(player_alpha[index] * g_Options.players.colors.health_bar[3])) : Color(g_Options.players.colors.health_bar[0], g_Options.players.colors.health_bar[1], g_Options.players.colors.health_bar[2], g_Options.players.colors.health_bar[3]);
			Color health_bar_upper = g_Options.players.fade.enable ? Color(g_Options.players.colors.health_bar_upper[0], g_Options.players.colors.health_bar_upper[1], g_Options.players.colors.health_bar_upper[2], float(player_alpha[index] * g_Options.players.colors.health_bar_upper[3])) : Color(g_Options.players.colors.health_bar_upper[0], g_Options.players.colors.health_bar_upper[1], g_Options.players.colors.health_bar_upper[2], g_Options.players.colors.health_bar_upper[3]);
			Color health_bar_lower = g_Options.players.fade.enable ? Color(g_Options.players.colors.health_bar_lower[0], g_Options.players.colors.health_bar_lower[1], g_Options.players.colors.health_bar_lower[2], float(player_alpha[index] * g_Options.players.colors.health_bar_lower[3])) : Color(g_Options.players.colors.health_bar_lower[0], g_Options.players.colors.health_bar_lower[1], g_Options.players.colors.health_bar_lower[2], g_Options.players.colors.health_bar_lower[3]);
			Color health_bar_outline = g_Options.players.fade.enable ? Color(g_Options.players.colors.health_bar_outline[0], g_Options.players.colors.health_bar_outline[1], g_Options.players.colors.health_bar_outline[2], float(player_alpha[index] * g_Options.players.colors.health_bar_outline[3])) : Color(g_Options.players.colors.health_bar_outline[0], g_Options.players.colors.health_bar_outline[1], g_Options.players.colors.health_bar_outline[2], g_Options.players.colors.health_bar_outline[3]);
			Color skeleton = g_Options.players.fade.enable ? Color(g_Options.players.colors.skeleton[0], g_Options.players.colors.skeleton[1], g_Options.players.colors.skeleton[2], float(player_alpha[index] * g_Options.players.colors.skeleton[3])) : Color(g_Options.players.colors.skeleton[0], g_Options.players.colors.skeleton[1], g_Options.players.colors.skeleton[2], g_Options.players.colors.skeleton[3]);
			Color backtrack_skeleton = g_Options.players.fade.enable ? Color(g_Options.players.colors.backtrack_skeleton[0], g_Options.players.colors.backtrack_skeleton[1], g_Options.players.colors.backtrack_skeleton[2], float(player_alpha[index] * g_Options.players.colors.backtrack_skeleton[3])) : Color(g_Options.players.colors.backtrack_skeleton[0], g_Options.players.colors.backtrack_skeleton[1], g_Options.players.colors.backtrack_skeleton[2], g_Options.players.colors.backtrack_skeleton[3]);
			Color backtrack_dot = g_Options.players.fade.enable ? Color(g_Options.players.colors.backtrack_dot[0], g_Options.players.colors.backtrack_dot[1], g_Options.players.colors.backtrack_dot[2], float(player_alpha[index] * g_Options.players.colors.backtrack_dot[3])) : Color(g_Options.players.colors.backtrack_dot[0], g_Options.players.colors.backtrack_dot[1], g_Options.players.colors.backtrack_dot[2], g_Options.players.colors.backtrack_dot[3]);
			Color view_angle = g_Options.players.fade.enable ? Color(g_Options.players.colors.view_angle[0], g_Options.players.colors.view_angle[1], g_Options.players.colors.view_angle[2], float(player_alpha[index] * g_Options.players.colors.view_angle[3])) : Color(g_Options.players.colors.view_angle[0], g_Options.players.colors.view_angle[1], g_Options.players.colors.view_angle[2], g_Options.players.colors.view_angle[3]);
			Color snap_line = g_Options.players.fade.enable ? Color(g_Options.players.colors.snapline[0], g_Options.players.colors.snapline[1], g_Options.players.colors.snapline[2], float(player_alpha[index] * g_Options.players.colors.snapline[3])) : Color(g_Options.players.colors.snapline[0], g_Options.players.colors.snapline[1], g_Options.players.colors.snapline[2], g_Options.players.colors.snapline[3]);
			Color ammo_bar = g_Options.players.fade.enable ? Color(g_Options.players.colors.ammo_bar[0], g_Options.players.colors.ammo_bar[1], g_Options.players.colors.ammo_bar[2], float(player_alpha[index] * g_Options.players.colors.ammo_bar[3])) : Color(g_Options.players.colors.ammo_bar[0], g_Options.players.colors.ammo_bar[1], g_Options.players.colors.ammo_bar[2], g_Options.players.colors.ammo_bar[3]);
			Color ammo_bar_outline = g_Options.players.fade.enable ? Color(g_Options.players.colors.ammo_bar_outline[0], g_Options.players.colors.ammo_bar_outline[1], g_Options.players.colors.ammo_bar_outline[2], float(player_alpha[index] * g_Options.players.colors.ammo_bar_outline[3])) : Color(g_Options.players.colors.ammo_bar_outline[0], g_Options.players.colors.ammo_bar_outline[1], g_Options.players.colors.ammo_bar_outline[2], g_Options.players.colors.ammo_bar_outline[3]);
			Color weapon_text = g_Options.players.fade.enable ? Color(g_Options.players.colors.weapon_text[0], g_Options.players.colors.weapon_text[1], g_Options.players.colors.weapon_text[2], float(player_alpha[index] * g_Options.players.colors.weapon_text[3])) : Color(g_Options.players.colors.weapon_text[0], g_Options.players.colors.weapon_text[1], g_Options.players.colors.weapon_text[2], g_Options.players.colors.weapon_text[3]);
			Color weapon_icon = g_Options.players.fade.enable ? Color(g_Options.players.colors.weapon_icon[0], g_Options.players.colors.weapon_icon[1], g_Options.players.colors.weapon_icon[2], float(player_alpha[index] * g_Options.players.colors.weapon_icon[3])) : Color(g_Options.players.colors.weapon_icon[0], g_Options.players.colors.weapon_icon[1], g_Options.players.colors.weapon_icon[2], g_Options.players.colors.weapon_icon[3]);
			Color distance = g_Options.players.fade.enable ? Color(g_Options.players.colors.distance[0], g_Options.players.colors.distance[1], g_Options.players.colors.distance[2], float(player_alpha[index] * g_Options.players.colors.distance[3])) : Color(g_Options.players.colors.distance[0], g_Options.players.colors.distance[1], g_Options.players.colors.distance[2], g_Options.players.colors.distance[3]);
			Color health_text = g_Options.players.fade.enable ? Color(g_Options.players.colors.health_text[0], g_Options.players.colors.health_text[1], g_Options.players.colors.health_text[2], float(player_alpha[index] * g_Options.players.colors.health_text[3])) : Color(g_Options.players.colors.health_text[0], g_Options.players.colors.health_text[1], g_Options.players.colors.health_text[2], g_Options.players.colors.health_text[3]);
			Color ammo_text = g_Options.players.fade.enable ? Color(g_Options.players.colors.ammo_text[0], g_Options.players.colors.ammo_text[1], g_Options.players.colors.ammo_text[2], float(player_alpha[index] * g_Options.players.colors.ammo_text[3])) : Color(g_Options.players.colors.ammo_text[0], g_Options.players.colors.ammo_text[1], g_Options.players.colors.ammo_text[2], g_Options.players.colors.ammo_text[3]);
			Color out_of_view = g_Options.players.fade.enable ? Color(g_Options.players.colors.out_of_view[0], g_Options.players.colors.out_of_view[1], g_Options.players.colors.out_of_view[2], float(player_alpha[index] * g_Options.players.colors.out_of_view[3])) : Color(g_Options.players.colors.out_of_view[0], g_Options.players.colors.out_of_view[1], g_Options.players.colors.out_of_view[2], g_Options.players.colors.out_of_view[3]);

			if (g_Options.players.engine_radar)
			{
				entity->m_bSpotted() = true;
			}

			if (g_Options.players.out_of_view.enable)
			{
				draw_outoffov(entity, g_Options.players.colors.custom ? out_of_view : check_on_see ? visible : invisible);
			}

		

			bbox_t _box;

			if (!get_player_box(entity, _box))
			{
				continue;
			}

			if (g_Options.players.name.enable)
			{
				draw_name(entity, _box, g_Options.players.colors.custom ? name : check_on_see ? visible : invisible);
			}

			if (g_Options.players.box.enable)
			{
				draw_box(entity, _box, g_Options.players.colors.custom ? box : check_on_see ? visible : invisible, box_outline);
			}

			if (g_Options.players.health_bar.enable)
			{
				draw_health_bar(entity, _box, g_Options.players.colors.custom ? health_bar : check_on_see ? visible : invisible, health_bar_upper, health_bar_lower, health_bar_outline);
			}

			if (g_Options.players.skeleton.enable)
			{
				draw_skeleton(entity, g_Options.players.colors.custom ? skeleton : check_on_see ? visible : invisible);
			}

			if (g_Options.players.backtrack_skeleton.enable)
			{
				draw_backtrack_skeleton(entity, g_Options.players.colors.custom ? backtrack_skeleton : check_on_see ? visible : invisible);
			}

			if (g_Options.players.backtrack_dot.enable)
			{
				draw_backtrack_dot(entity, g_Options.players.colors.custom ? backtrack_dot : check_on_see ? visible : invisible);
			}

			if (g_Options.players.view_angle.enable)
			{
				draw_viewangle(entity, g_Options.players.colors.custom ? view_angle : check_on_see ? visible : invisible);
			}

			if (g_Options.players.snapline.enable)
			{
				draw_snapline(entity, _box, g_Options.players.colors.custom ? snap_line : check_on_see ? visible : invisible);
			}

			if (g_Options.players.flags.enable)
			{
				draw_flags(entity, _box, g_Options.players.colors.custom ? snap_line : check_on_see ? visible : invisible);
			}

			draw_bottom_bar(entity, _box, g_Options.players.colors.custom ? ammo_bar : check_on_see ? visible : invisible, g_Options.players.colors.custom ? health_text : check_on_see ? visible : invisible, g_Options.players.colors.custom ? weapon_icon : check_on_see ? visible : invisible, g_Options.players.colors.custom ? weapon_text : check_on_see ? visible : invisible, g_Options.players.colors.custom ? ammo_text : check_on_see ? visible : invisible, g_Options.players.colors.custom ? distance : check_on_see ? visible : invisible, ammo_bar_outline);

			
		}

		for (int i = 1; i < g_EntityList->GetHighestEntityIndex(); i++)
		{
			auto entity = reinterpret_cast<C_BaseEntity*>(g_EntityList->GetClientEntity(i));

			if (!entity || entity->IsPlayer() || entity->IsDormant() || entity == g_LocalPlayer)
			{
				continue;
			}

			if (g_Options.players.thrown_grenade.text.enable || g_Options.players.thrown_grenade.icon.enable)
			{
				//throwed_grenade(entity);
			}

			if (g_Options.players.dropped_weapon.box.enable ||
				g_Options.players.dropped_weapon.ammo_bar.enable ||
				g_Options.players.dropped_weapon.ammo_text.enable ||
				g_Options.players.dropped_weapon.text.enable ||
				g_Options.players.dropped_weapon.icon.enable)
			{

				bbox_t entity_box;

				if (!get_entity_box(entity, entity_box))
				{
					continue;
				}

				//dropped_weapon(entity, entity_box);
			}
		}
	}
}
void rendertext(const float x, const float y, const float fontSize, ImFont* font, const std::string& text, const bool centered, const Color& color, const bool dropShadow, const bool outline)
{
	
	Render::Get().RenderText(text, ImVec2(x, y), fontSize, color, centered, outline, font,dropShadow);
}
void rendertext2(const float x, const float y, const float fontSize, ImFont* font, const std::string& text, const bool centered, const Color& color, const bool dropShadow, const bool outline)
{

	Render::Get().RenderText(text, ImVec2(x, y), fontSize, color, centered, outline, font);
}
void player::draw_name(C_BasePlayer* entity, bbox_t bbox, Color color)
{
	player_info_t info;
	g_EngineClient->GetPlayerInfo(entity->EntIndex(), &info);

	std::string print((g_Options.players.name.bot_check && info.fakeplayer) ? std::string("[bot] ").append(info.szName).c_str() : info.szName);

	ImVec2 size = Render::Get().GetTextSize(print, fonts::esp_font, g_Options.fonts.esp_size);

	rendertext(bbox.x + (bbox.w / 2), bbox.y - static_cast<int>(size.y) - 2, g_Options.fonts.esp_size, fonts::esp_font, print, true, color, g_Options.fonts.esp_flag[9], g_Options.fonts.esp_flag[10]);
}
void drawrect(const float x, const float y, const float w, const float h, const Color& color, const int flags, const float thickness)
{
	Render::Get().RenderBox(x, y, x + w, y + h, color, thickness);
}
void renderrectcorner(const float x, const float y, const float w, const float h, const float lenght, const Color& colour) {

	Render::Get().RenderLine(x, y, x, y + (h / lenght), Color(colour));
	Render::Get().RenderLine(x, y, x + (w / lenght), y, Color(colour));
	Render::Get().RenderLine(x + w, y, x + w - (w / lenght), y, Color(colour));
	Render::Get().RenderLine(x + w, y, x + w, y + (h / lenght), Color(colour));
	Render::Get().RenderLine(x, y + h, x + (w / lenght), y + h, Color(colour));
	Render::Get().RenderLine(x, y + h, x, y + h - (h / lenght), Color(colour));
	Render::Get().RenderLine(x + w, y + h, x + w - (w / lenght), y + h, Color(colour));
	Render::Get().RenderLine(x + w, y + h, x + w, y + h - (h / lenght), Color(colour));
}
void player::draw_box(C_BasePlayer* entity, bbox_t bbox, Color color, Color color_second)
{
	switch (g_Options.players.box.type)
	{
	case 0:
		if (g_Options.players.box.outline[0])
		{
			drawrect(bbox.x - 1, bbox.y - 1, bbox.w + 2, bbox.h + 2, color_second,0,1.f);
		}

		if (g_Options.players.box.outline[1])
		{
			drawrect(bbox.x + 1, bbox.y + 1, bbox.w - 2, bbox.h - 2, color_second, 0, 1.f);
		}

		drawrect(bbox.x, bbox.y, bbox.w, bbox.h, color, 0, 1.f);

		break;
	case 1:
		if (g_Options.players.box.outline[0])
		{
			renderrectcorner(bbox.x - 1, bbox.y - 1, bbox.w + 2, bbox.h + 2, g_Options.players.box.lenght, color_second);
		}

		if (g_Options.players.box.outline[1])
		{
			renderrectcorner(bbox.x + 1, bbox.y + 1, bbox.w - 2, bbox.h - 2, g_Options.players.box.lenght, color_second);
		}

		renderrectcorner(bbox.x, bbox.y, bbox.w, bbox.h, g_Options.players.box.lenght, color);

		break;
	}
}

constexpr float anim_time = 255 / 2.0f;
void drawrectfilled(const float x, const float y, const float w, const float h, const Color& color, const int flags) {
	Render::Get().RenderBoxFilled(x, y, x + w, y + h, Color(color), 1.f);

}

void player::draw_health_bar(C_BasePlayer* entity, bbox_t bbox, Color color, Color upper, Color lower, Color outline)
{
	static float prev_player_hp[65];

	if (prev_player_hp[entity->EntIndex()] > entity->m_iHealth())
	{
		prev_player_hp[entity->EntIndex()] -= anim_time * g_GlobalVars->frametime;
	}
	else
	{
		prev_player_hp[entity->EntIndex()] = entity->m_iHealth();
	}

	int g = static_cast<int>(entity->m_iHealth() * 2.55f);
	int r = 255 - g;

	Color rgb_health_color = Color(r, g, 0, 255);

	if (g_Options.players.health_bar.background)
	{
		drawrectfilled(bbox.x - 2, bbox.y - 1, (-g_Options.players.health_bar.size - 2), bbox.h + 2, outline,0);
	}
	else
	{
		drawrect(bbox.x - 3, bbox.y - 1, (-g_Options.players.health_bar.size), bbox.h + 2, outline,0,1.f);
	}


	if (!g_Options.players.health_bar.baseonhealth && !g_Options.players.health_bar.gradient)
	{
		drawrectfilled(
			bbox.x - 3,
			bbox.y + (bbox.h - bbox.h * (prev_player_hp[entity->EntIndex()]) / 100.f),
			(-g_Options.players.health_bar.size),
			(bbox.h * (prev_player_hp[entity->EntIndex()]) / 100.f),
			color,0);
	}

	if (g_Options.players.health_bar.baseonhealth)
	{
		drawrectfilled(
			bbox.x - 3,
			bbox.y + (bbox.h - bbox.h * (prev_player_hp[entity->EntIndex()]) / 100.f),
			(-g_Options.players.health_bar.size),
			(bbox.h * (prev_player_hp[entity->EntIndex()]) / 100.f),
			rgb_health_color,0);
	}

	if (g_Options.players.health_bar.gradient)
	{
		Render::Get().RenderGradientBox(
			bbox.x - 3,
			bbox.y + (bbox.h - bbox.h * (prev_player_hp[entity->EntIndex()]) / 100.f),
			(-g_Options.players.health_bar.size),
			(bbox.h * (prev_player_hp[entity->EntIndex()]) / 100.f),
			upper, upper, lower, lower);
	}

	if (g_Options.players.health_bar.text && entity->m_iHealth() < 99)
	{
		ImVec2 size = Render::Get().GetTextSize(std::to_string(entity->m_iHealth()), fonts::sub_esp_font, g_Options.fonts.esp_sub_size);

		rendertext(bbox.x - (size.x + g_Options.players.health_bar.size), bbox.y, g_Options.fonts.esp_sub_size, fonts::sub_esp_font, std::to_string(static_cast<int>(prev_player_hp[entity->EntIndex()])), true, Color(1.0f, 1.0f, 1.0f, color.a()/255.f), g_Options.fonts.esp_sub_flag[9], g_Options.fonts.esp_sub_flag[10]);
	}
}

void player::draw_backtrack_dot(C_BasePlayer* entity, Color color)
{
	if (Backtrack::records[entity->EntIndex()].size() < 1)
	{
		return;
	}

	Vector previous_screenpos;

	for (int i = 1; i < Backtrack::records[entity->EntIndex()].size(); i++)
	{
		Vector screen_pos;

		auto head = Backtrack::records[entity->EntIndex()][i].head;

		if (!Math::WorldToScreen(head, screen_pos))
		{
			continue;
		}

		if (previous_screenpos.IsValid() && !previous_screenpos.is_zero())
		{

			drawrect(screen_pos.x, screen_pos.y, 2, 2, color, 0,1.f);

		}

		previous_screenpos = screen_pos;
	}
}

void player::draw_backtrack_skeleton(C_BasePlayer* entity, Color color)
{
	auto hdr = g_MdlInfo->GetStudiomodel(entity->GetModel());

	if (!hdr)
	{
		return;
	}

	if (Backtrack::records[entity->EntIndex()].size() < 1)
	{
		return;
	}

	auto record = &Backtrack::records[entity->EntIndex()].back();

	Vector v_parent, v_child, s_parent, s_child;

	for (int j = 0; j < hdr->numbones; j++)
	{
		mstudiobone_t* bone = hdr->GetBone(j);

		if (!bone)
		{
			continue;
		}

		if ((bone->flags & BONE_USED_BY_HITBOX) && (bone->parent != -1))
		{
			v_child = entity->GetBonePos(j);
			v_parent = entity->GetBonePos(bone->parent);

			v_child = Vector(record->matrix[j][0][3], record->matrix[j][1][3], record->matrix[j][2][3]);
			v_parent = Vector(record->matrix[bone->parent][0][3], record->matrix[bone->parent][1][3], record->matrix[bone->parent][2][3]);

			if (Math::WorldToScreen(v_parent, s_parent) && Math::WorldToScreen(v_child, s_child))
			{
				Render::Get().RenderLine(s_parent[0], s_parent[1], s_child[0], s_child[1], color, g_Options.players.backtrack_skeleton.thickness);
			}
		}
	}
}

void player::draw_skeleton(C_BasePlayer* entity, Color color)
{
	auto p_studio_hdr = g_MdlInfo->GetStudiomodel(entity->GetModel());

	if (!p_studio_hdr)
	{
		return;
	}

	Vector v_parent, v_child, s_parent, s_child;

	for (int i = 0; i < p_studio_hdr->numbones; i++)
	{
		mstudiobone_t* bone = p_studio_hdr->GetBone(i);

		if (!bone)
		{
			continue;
		}

		if (bone && (bone->flags & BONE_USED_BY_HITBOX) && (bone->parent != -1))
		{
			v_child = entity->GetBonePos(i);
			v_parent = entity->GetBonePos(bone->parent);

			if (Math::WorldToScreen(v_parent, s_parent) && Math::WorldToScreen(v_child, s_child))
			{
				Render::Get().RenderLine(s_parent[0], s_parent[1], s_child[0], s_child[1], color, g_Options.players.skeleton.thickness);
			}
		}
	}
}

void player::draw_snapline(C_BasePlayer* entity, bbox_t bbox, Color color)
{
	int x, y;
	g_EngineClient->GetScreenSize(x, y);

	if (g_Options.players.snapline.type[0])
	{
		Render::Get().RenderLine(bbox.x + bbox.w / 2, bbox.y, x / 2, 0, color, g_Options.players.snapline.thickness);
	}

	if (g_Options.players.snapline.type[1])
	{
		Render::Get().RenderLine(bbox.x + bbox.w / 2, bbox.y + bbox.h / 2,x / 2, y / 2, color, g_Options.players.snapline.thickness);
	}

	if (g_Options.players.snapline.type[2])
	{
		Render::Get().RenderLine(bbox.x + bbox.w / 2, bbox.y + bbox.h, x / 2,y, color, g_Options.players.snapline.thickness);
	}
}

void player::draw_viewangle(C_BasePlayer* entity, Color color)
{
	Vector src;
	Vector pos = entity->GetBonePos(8);
	Math::AngleVectors(entity->m_angEyeAngles(), src);
	src *= g_Options.players.view_angle.distance;
	Vector dst = pos + src;
	Vector start, end;

	if (Math::WorldToScreen(pos, start) && Math::WorldToScreen(dst, end))
	{
		Render::Get().RenderLine(start.x, start.y, end.x, end.y, color, g_Options.players.view_angle.thickness);
	}
}

void player::draw_flags(C_BasePlayer* entity, bbox_t bbox, Color color)
{
	std::vector<std::pair<std::string, Color>> flags;

	if (g_Options.players.flags.flags[0] && entity->armor() > 0)
	{
		flags.push_back({ entity->has_helmet() ? "hk" : "k", color });
	}

	if (g_Options.players.flags.flags[1])
	{
		flags.push_back({ std::string(("$")).append(std::to_string(entity->money())), color });
	}

	if (g_Options.players.flags.flags[2] && entity->IsFlashed())
	{
		flags.push_back({ "flashed", color });
	}

	if (g_Options.players.flags.flags[3] && entity->m_bIsScoped())
	{
		flags.push_back({ "scoped", color });
	}

	if (g_Options.players.flags.flags[4] && ((entity->GetSequenceActivity(entity->GetAnimOverlay(1)->m_nSequence) == 967) && (entity->GetAnimOverlay(1)->m_flWeight != 0.f)))
	{
		flags.push_back({ "reloading...", color });
	}

	if (g_Options.players.flags.flags[5] && entity->m_bHasDefuser())
	{
		flags.push_back({ "defusing...", color });
	}

	if (g_Options.players.flags.flags[6] && entity->is_rescuing())
	{
		flags.push_back({ "rescuing...", color });
	}

	auto position = 0;

	for (auto text : flags)
	{
		rendertext(bbox.x + bbox.w + 3, bbox.y + position, g_Options.fonts.esp_sub_size, fonts::sub_esp_font, text.first, false, text.second, g_Options.fonts.esp_sub_flag[9], g_Options.fonts.esp_sub_flag[10]);

		position += g_Options.fonts.esp_sub_size;
	}
}

void player::draw_bottom_bar(C_BasePlayer* entity, bbox_t bbox, Color ammo_bar, Color health_text, Color weapon_icon, Color weapon_text, Color ammo_text, Color distance, Color ammo_bar_outline)
{
	int offset_bottom = 0;

	auto weapon = entity->m_hActiveWeapon();

	if (!weapon)
	{
		return;
	}


	int g = static_cast<int>(entity->m_iHealth() * 2.55f);
	int r = 255 - g;

	Color rgb_health_color = Color(r, g, 0, 255);

	auto weapon_data = weapon->GetCSWeaponData();

	if (!weapon_data)
	{
		return;
	}

	if (g_Options.players.ammo_bar.enable)
	{
		if (weapon->IsGun())
		{
			float box_w = (float)fabs(bbox.w - bbox.x);

			int width = ((box_w * weapon->m_iClip1()) / weapon_data->iMaxClip1);

			if (g_Options.players.ammo_bar.background)
			{
				drawrectfilled(bbox.x - 1, bbox.y + bbox.h + 2, bbox.w + 2, g_Options.players.ammo_bar.size + 2, ammo_bar_outline,0);
			}
			else
			{
				drawrect(bbox.x - 1, bbox.y + bbox.h + 2, bbox.w + 2, g_Options.players.ammo_bar.size + 2, ammo_bar_outline,9,1.f);
			}

			drawrectfilled(bbox.x, bbox.y + bbox.h + 3, bbox.w, g_Options.players.ammo_bar.size, ammo_bar,0);

			offset_bottom += 3 + g_Options.players.ammo_bar.size;
		}
	}

	if (g_Options.players.health_text.enable)
	{
		if (g_Options.players.health_text.hide_at_max)
		{
			if (entity->m_iHealth() < 98)
			{
				if (g_Options.players.health_text.baseonhealth)
				{
					rendertext(bbox.x + (bbox.w / 2), bbox.y + bbox.h + 2 + offset_bottom, g_Options.fonts.esp_sub_size, fonts::sub_esp_font, g_Options.players.health_text.hide_suffix ? std::to_string(entity->m_iHealth()) : std::to_string(entity->m_iHealth()) + " hp", true, rgb_health_color, g_Options.fonts.esp_sub_flag[9], g_Options.fonts.esp_sub_flag[10]);
				}
				else
				{
					rendertext(bbox.x + (bbox.w / 2), bbox.y + bbox.h + 2 + offset_bottom, g_Options.fonts.esp_sub_size, fonts::sub_esp_font, g_Options.players.health_text.hide_suffix ? std::to_string(entity->m_iHealth()) : std::to_string(entity->m_iHealth()) + " hp", true, health_text, g_Options.fonts.esp_sub_flag[9], g_Options.fonts.esp_sub_flag[10]);
				}

				offset_bottom += 4 + 11.f;
			}
		}
		else
		{
			if (g_Options.players.health_text.baseonhealth)
			{
				rendertext(bbox.x + (bbox.w / 2), bbox.y + bbox.h + 2 + offset_bottom, g_Options.fonts.esp_sub_size, fonts::sub_esp_font, g_Options.players.health_text.hide_suffix ? std::to_string(entity->m_iHealth()) : std::to_string(entity->m_iHealth()) + " hp", true, rgb_health_color, g_Options.fonts.esp_sub_flag[9], g_Options.fonts.esp_sub_flag[10]);
			}
			else
			{
				rendertext(bbox.x + (bbox.w / 2), bbox.y + bbox.h + 2 + offset_bottom, g_Options.fonts.esp_sub_size, fonts::sub_esp_font, g_Options.players.health_text.hide_suffix ? std::to_string(entity->m_iHealth()) : std::to_string(entity->m_iHealth()) + " hp", true, health_text, g_Options.fonts.esp_sub_flag[9], g_Options.fonts.esp_sub_flag[10]);
			}

			offset_bottom += 4 + 11.f;
		}
	}

	if (g_Options.players.weapon_icon.enable)
	{
		rendertext2(bbox.x + (bbox.w / 2), bbox.y + bbox.h + offset_bottom, 12.0f, fonts::icon_font, weapon->get_wpn_icon(), true, weapon_icon, true,true);

		offset_bottom += 13;
	}

	if (g_Options.players.weapon_text.enable)
	{
		rendertext(bbox.x + (bbox.w / 2), bbox.y + bbox.h + offset_bottom, g_Options.fonts.esp_sub_size, fonts::sub_esp_font, weapon->m_Item().weapon_name_definition(), true, weapon_text, g_Options.fonts.esp_sub_flag[9], g_Options.fonts.esp_sub_flag[10]);

		offset_bottom += 2 + 11.f;
	}

	if (g_Options.players.ammo_text.enable)
	{
		if (weapon->IsGun())
		{
			rendertext(bbox.x + (bbox.w / 2), bbox.y + bbox.h + offset_bottom, g_Options.fonts.esp_sub_size, fonts::sub_esp_font, "[" + std::to_string(weapon->m_iClip1()) + "|" + std::to_string(weapon_data->iMaxClip1) + "]", true, ammo_text, g_Options.fonts.esp_sub_flag[9], g_Options.fonts.esp_sub_flag[10]);

			offset_bottom += 2 + 11.f;
		}
	}

	if (g_Options.players.distance.enable)
	{
		rendertext(bbox.x + (bbox.w / 2), bbox.y + bbox.h + offset_bottom, g_Options.fonts.esp_sub_size, fonts::sub_esp_font, std::to_string((int)(g_LocalPlayer->m_vecOrigin().DistTo(entity->m_vecOrigin()) * 0.0254f)) + " m", true, distance, g_Options.fonts.esp_sub_flag[9], g_Options.fonts.esp_sub_flag[10]);
	}
}

void player::draw_outoffov(C_BasePlayer* entity, Color color)
{
	int x, y;
	g_EngineClient->GetScreenSize(x, y);
	const Vector2D screen = Vector2D(x, y);
	const auto screen_center = Vector2D(screen.x / 2, screen.y / 2);

	Vector screen_point2;
	Vector player_pos = entity->m_vecOrigin();

	Math::WorldToScreen(player_pos, screen_point2);

	Vector2D screen_point = Vector2D(screen_point2.x, screen_point2.y);

	if (screen_point.x < 0 || screen_point.y < 0 || screen_point.x > screen.x || screen_point.y > screen.y)
	{
		const auto angle_yaw_rad = std::atan2f(screen_point.y - screen_center.y, screen_point.x - screen_center.x);
		auto angle = RAD2DEG(angle_yaw_rad);

		const auto new_point_x = screen_center.x + g_Options.players.out_of_view.distance * cosf(angle_yaw_rad);
		const auto new_point_y = screen_center.y + g_Options.players.out_of_view.distance * sinf(angle_yaw_rad);

		float size = g_Options.players.out_of_view.size;

		std::array<Vector2D, 3> points
		{
		  Vector2D(new_point_x - size, new_point_y + size),
		  Vector2D(new_point_x, new_point_y - size),
		  Vector2D(new_point_x + size, new_point_y + size)
		};

		float kek = DEG2RAD(std::remainderf(angle + 90.0f, 360.0f));

		const auto c = cosf(kek);
		const auto s = sinf(kek);

		const auto points_center = (points.at(0) + points.at(1) + points.at(2)) / 3;
		for (auto& point : points)
		{
			point -= points_center;

			const auto x = point.x, y = point.y;

			point.x = x * c - y * s;
			point.y = x * s + y * c;

			point += points_center;
		}

		Render::Get().Triangle((ImVec2&)points[0], (ImVec2&)points[1], (ImVec2&)points[2], color);
	}
}

void player::draw_death_history(C_BasePlayer* entity)
{
	if (entity->IsAlive())
	{
		return;
	}

	if (entity->IsDormant())
	{
		return;
	}

	bool check_on_see = g_LocalPlayer->CanSeePlayer(entity, entity->GetEyePos());

	if (!check_on_see)
	{
		return;
	}

	player_info_t info;
	g_EngineClient->GetPlayerInfo(entity->EntIndex(), &info);

	Vector previous_screen_position;
	Vector screen_position;
	Vector get_head_position = entity->GetBonePos(8);
	ImVec2 size = Render::Get().GetTextSize(info.szName, fonts::sub_esp_font, g_Options.fonts.esp_sub_size);

	if (Math::WorldToScreen(get_head_position, screen_position))
	{
		drawrectfilled(screen_position.x - 4, screen_position.y - 4, 4, 4, Color(0, 255, 0, 255),0);
		rendertext(screen_position.x + 5, screen_position.y, g_Options.fonts.esp_sub_size, fonts::sub_esp_font, info.szName, true, Color(255, 255, 255, 255), g_Options.fonts.esp_flag[9], g_Options.fonts.esp_flag[10]);
	}
}

void player::draw_sounds()
{
	return;
}
