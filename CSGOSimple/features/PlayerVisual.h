#pragma once
#pragma once

#include "../singleton.hpp"

#include "../render.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"
struct bbox_t {
	int x, y, w, h;
	bbox_t() = default;
	bbox_t(const int x, const int y, const int w, const int h) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}
};
namespace player
{
	inline float player_alpha[65] = { };
	bool get_player_box(C_BasePlayer* entity, bbox_t& in);
	bool get_entity_box(C_BaseEntity* entity, bbox_t& in);
	void run();
	void draw_name(C_BasePlayer* entity, bbox_t bbox, Color color);
	void draw_box(C_BasePlayer* entity, bbox_t bbox, Color color, Color color_second);
	void draw_health_bar(C_BasePlayer* entity, bbox_t bbox, Color color, Color upper, Color lower, Color outline);
	void draw_backtrack_dot(C_BasePlayer* entity, Color color);
	void draw_backtrack_skeleton(C_BasePlayer* entity, Color color);
	void draw_skeleton(C_BasePlayer* entity, Color color);
	void draw_snapline(C_BasePlayer* entity, bbox_t bbox, Color color);
	void draw_viewangle(C_BasePlayer* entity, Color color);
	void draw_flags(C_BasePlayer* entity, bbox_t bbox, Color color);
	void draw_bottom_bar(C_BasePlayer* entity, bbox_t bbox, Color ammo_bar, Color health_text, Color weapon_icon, Color weapon_text, Color ammo_text, Color distance, Color ammo_bar_outline);
	void draw_outoffov(C_BasePlayer* entity, Color color);
	void draw_death_history(C_BasePlayer* entity);
	void draw_sounds();
}