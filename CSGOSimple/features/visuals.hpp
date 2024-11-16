#pragma once

#include "../singleton.hpp"

#include "../render.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"




class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;
    float timer = 0.f;
	float flHurtTime;
	
	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
		} ctx;
		bool Begin(C_BasePlayer * pl);
		bool vis_begin(C_BasePlayer* pl);
		void RenderBox(C_BaseEntity* pl);
		void oof_arrows(C_BasePlayer* pl);
		void RenderName(C_BaseEntity* pl);
		void RenderWeaponName(C_BaseEntity* pl);
		void weapon_icon(C_BaseEntity* pl);
		void RenderHealthBar(C_BaseEntity* pl);
		void RenderBacktrack(C_BaseEntity* ent);
		void RenderHealth(C_BaseEntity* pl);
		void flags(C_BaseEntity* pl);
		void oofarr(C_BasePlayer* peremenaya);
		void RenderSkeleton(C_BaseEntity* ent);
		void RenderArmour();
		void RenderSnapline();
	};
	int hitmarker_time = 0;
	void ConsoleColor(vgui::VPANEL panel);
	void RenderWeapon(C_BaseCombatWeapon* ent);
	void Render_Weapon_Icon(C_BaseCombatWeapon* ent);
	void RenderDefuseKit(C_BaseEntity* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void RenderItemEsp(C_BaseEntity* ent);
	void sunset();
	void Velocity();
	void speclik();
	void out_of_fov();
public:
	void TryingFunc();
	void motion_blur(CViewSetup* setup);
	void AddToDrawList();
	void d3d9_list();
	void Render();
	void paint_traverse();
	void HIT_LOGGER(IGameEvent* BANUS);
	void playerModel(ClientFrameStage_t stage);
	void RenderIndicators();
	void graphs_data();
	void velocity_graph_indicator();
	void stamina_graph_indicator();
	void stamina();
	void FireGameEvent(IGameEvent* event);
	void Postprocess(ClientFrameStage_t stage);
	void DrawHitmarker();
	void gatherinfo();
	void velo_graph();
};
