#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include "valve_sdk/Misc/Color.hpp"
#include "imgui/imgui.h"
#define A( s ) #s
#define OPTION(type, var, val) Var<type> var = {A(var), val}
template <typename T>
class ConfigValue
{
public:
	ConfigValue(std::string category_, std::string name_, T* value_)
	{
		category = category_;
		name = name_;
		value = value_;
	}

	std::string category, name;
	T* value;
};

struct legitbot_s
{
	bool enabled = false;
	bool deathmatch = false;
	//bool silent = false;
	int silent2 = false;
	bool silent_s = false;
	bool flash_check = false;
	bool smoke_check = false;
	bool autopistol = false;

	float fov = 0.f;
	float silent_fov = 0.f;
	float smooth = 1.f;

	int shot_delay = 0;
	int kill_delay = 0;

	struct
	{
		bool head = true;
		bool chest = true;
		bool hands = true;
		bool legs = true;
		bool stomach = true;
	} hitboxes;

	struct
	{
		bool enabled = false;
		int start = 1;
		int type = 0;
		int x = 100;
		int y = 100;
	} rcs;

	struct
	{
		bool enabled = false;
		int min_damage = 1;
	} autowall;

	struct
	{
		bool enabled = false;
		int hotkey = 0;
	} autofire;
};

struct weapons
{
	legitbot_s legit;
};
struct statrack_setting
{
	int definition_index = 1;
	struct
	{
		int counter = 0;
	}statrack_new;
};
struct item_setting
{
	char name[32] = "Default";
	//bool enabled = false;
	int stickers_place = 0;
	int definition_vector_index = 0;
	int definition_index = 0;
	bool   enabled_stickers = 0;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	bool stat_trak = 0;
	float wear = FLT_MIN;
	char custom_name[32] = "";
};

class Options
{
public:
	std::map<short, weapons> weapons;
	struct
	{
		/*struct
		{
			std::map<int, profilechanger_settings> profile_items = { };
		}profile;*/
		struct
		{
			bool skin_preview = false;
			bool show_cur = true;

			std::map<int, statrack_setting> statrack_items = { };
			std::map<int, item_setting> m_items = { };
			std::map<std::string, std::string> m_icon_overrides = { };
		}skin;
	}changers;
		bool SkiNChanger{};
		bool esp_enemies_only = false;
		bool esp_player_boxes = false;
		bool esp_player_boxesOccluded = false;
		bool esp_player_names = false;
		bool esp_player_health = false;
		bool skele_ton_esp = false;
		bool esp_player_armour = false;
		bool esp_player_weapons = false;
		bool esp_player_snaplines = false;
		bool esp_dropped_weapons = false;
		bool esp_dropped_weapons_icon = false;
		bool esp_defuse_kit = false;
		bool esp_planted_c4 = false;
		bool esp_items = false;
		bool detectchat = false;
		float lockfactor = 0.f;
		bool glow_enabled = false;
		bool glow_enemies_only = false;
		bool glow_players = false;
		bool glow_chickens = false;
		bool glow_c4_carrier = false;
		bool glow_planted_c4 = false;
		bool glow_defuse_kits = false;
		bool glow_weapons = false;
		bool glow_enemiesOC = false;
		bool aimatbt = false;
		bool chams_player_enabled = false;
		bool player_enemies_shine = false;
		bool chams_player_enemies_only = false;
		bool chams_player_wireframe = false;
		int chams_player_flat = false;
		bool chams_player_ignorez = false;
		bool chams_player_glass = false;
		bool chams_arms_enabled = false;
		bool chams_arms_wireframe = false;
		bool chams_arms_flat = false;
		bool chams_arms_ignorez = false;
		bool chams_arms_glass = false;
		bool CHECKSURFLOLKEK = false;
		int KEYCHECKSURFLOLKEK = 0;
		int KEYCHECKSURFLOLKEKPoint = 0;
		int keychecksurf2 = VK_F2;
		int type = 0;
		bool togglechecksurflolkek{};
		bool showcslk;
		bool showonscreen;
		bool misc_bhop = false;
		bool misc_bhop2 = false;
		int playerModelT{ 0 };
		int playerModelCT{ 0 };
		bool misc_showranks = false;
		bool misc_watermark = true;
		bool Velocity = false;
		bool outline = false;
		bool lastjump = false;
		bool lastjumpoutline = false;
		bool autoaccept = false;
		bool no_flash = false;
		bool no_smoke = false;
		bool spectator_list = false;
		int AutoStafe_key;
		bool killss;
		bool autostrafe = false;
		int veloposy = 170;
		float oofarcsize;
		int staminaposy = 145;
		bool FireMan = false;
		int FireManKey = 0;
		int FireManType;
		bool airstuck;
		int airstuckkey;

		struct
		{
			bool enabled = false;
			bool edge_jump_duck_in_air = false;
			bool lg_on_ej = false;
			int hotkey = 0;
		} edgejump;
		struct
		{
			 bool edgebug = false;
			 bool edgebug_asissttoedge = false;
			 int edgebug_key = 0;
			 bool chatinfo_edgebug;
			 int edgebug_key_type = 0;
			 int edgebug_ticks = 64;
			 float edgebug_mouselock = 10.0f;
			 bool edgebug_indicator = false;
			 float edgebug_indicator_clr[3]{ 1.0f, 1.0f, 1.0f };
			 float edgebug_indicator_detect_clr[3]{ 0.0f, 1.0f, 0.0f };
			 bool edgebug_sound = false;
			 int edgebug_sound_type = 0;
			 bool edgebug_counter = false;
			 int edgebug_counter_pos = 650;
			 float edgebug_counter_clr[3]{ 1.0f, 1.0f, 1.0f };
			 bool edgebug_effect = false;
		} eb;
		bool edge_bug;



		int firstloop1 = 1;
		float firstloop2 = 30.f;
		float firstloop3 = 15.f;
		float secondloop1 = -180.f;
		float secondloop2 = 180.f;
		float secondloop3 = 10.f;
		float AdvanceSearchRange;
		bool OffSilent;
		bool takeoffstam;
		bool rintarovel = false;
		bool defstylevelo = false;
		int edge_bug_key;
		int EdgeBugTicks = 1;
		int EdgeBugCircle = 32;
		bool EdgeBugAdvanceSearch;
		bool EdgeBugDetectChat;
		float EdgeBugMouseLock;
		float deltaScaler;
		int DeltaType;
		bool MegaEdgeBug;
		bool AutoStrafeEdgeBug = false;
		bool SiletEdgeBug = false;
		bool jump_bug = false;
		bool misc_minijump = false;
		bool AutoBounce = false;
		int KeyAutoBounce = 0;
		int abobakey;
		bool pixelsurf = false;
		bool ps_fix = false;
		bool autoalign = false;
		bool unlockcvars = false;
		int indicatorsposy = 120;
		int size_test = 28.f;
		int size_test_esp = 11.f;
		bool hitbones = false;
		int jump_bug_key;
		int dhkey;
		int glow_enemies_type;
		bool sniper_xhair = false;
		bool takeoffvel = false;
		bool esp_world_fog = false;
		bool pxcalcbl;
		int posloska;
		float esp_world_fog_start = false;
		float esp_world_fog_end = false;
		float esp_world_fog_destiny = false;
		Color fog_color = { 0,0,0 };
		bool backtrack = false;
		bool stamina = false;
		bool jump_on_jb = false;
		bool aim_at_bt = false;
		bool misc_indicators = false;
		bool esp_enabled = false;
		bool showlj = false;
		bool showps = false;
		bool delayhop = false;
		bool iconesp = false;
		int delayhopticks;
		bool fastduck = false;
		bool panoramablur = false;
		bool postproccesing = false;
		bool showag = false;
		bool showmj = false;
		bool autoduck = false;
		int stm_graph_w = 10;
		int stm_graph_y = 0;
		int graph_w = 10;
		int graph_y = 0;
		float autoducktick;
		int edgebugtick;
		bool showjb = false;
		bool eblock = false;
		bool esp_player_healthbar = false;
		bool oof_arc = false;
		bool freelook = false;
		bool stamina_graph = false;
		bool money_flags = false;
		bool hk_flags = false;
		bool pixel_surf_calc = false;
		bool drawclip = false;
		bool wirefame = false;
		bool kits_flags = false;
		bool velocity_graph = false;
		int specposx = 5;
		int specposy = 55;
		int specsizex = 0;
		bool showeb = false;
		bool noviewbob = false;
		int setpointps;
		int calculatekey;
		bool ebinfo = false;
		bool extend_bt = false;
		int btcount = 1;
		int box_type;
		bool clantag = false;
		bool zeusbug = false;
		int radius;
		bool custom_console = false;
		bool hitinfo = false;
		int spec_type;
		int skybox_num;
		bool scaleform = false;
		bool esp_backtrack = false;
		bool ladderbugfix = false;
		bool flags = false;
		bool exploit = false;
		bool ljend = false;
		bool nulls3 = false;
		bool back = false;
		bool yaw_changer = false;
		bool velotag = false;
		float yaw1;
		float yaw2;
		bool moveleft = false;
		bool moveright = false;
		bool forwardd = false;
		bool left = false;
		bool right = false;
		bool nulls3_lj = false;
		bool nulls3_mj = false;
		bool nulls3_ej = false;
		Color color_esp_ally_visible= {255, 255, 255};
		Color color_esp_enemy_visible= {255, 255, 255};
		Color color_esp_ally_occluded= {255, 255, 255};
		Color color_esp_enemy_occluded= {255, 255, 255};
		Color color_esp_weapons= {255, 255, 255};
		Color color_esp_defuse= {255, 255, 255};
		Color color_esp_c4= {255, 255, 255};
		Color color_esp_item= {255, 255, 255};
		Color Velocitycol = { 0, 0, 0 };
		Color color_glow_ally= {255, 255, 255};
		Color color_glow_allyOC= {255, 255, 255};
		Color Coloresp_ally_visible = Color(255, 255, 255);
		Color arc_visible = Color(255, 255, 255);
	    float concole_clr[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
		Color arc_invisible = Color(255, 255, 255);
		Color Coloresp_enemy_visible = Color(255, 255, 255);
		Color skybox_color = Color(255, 255, 255);
		Color Coloresp_ally_occluded = Color(255, 255, 255);
		Color hit_bones_col = Color(255, 255, 255);
		Color Coloresp_enemy_occluded = Color(255, 255, 255);
		Color color_glow_enemy= {255, 255, 255};
		Color color_glow_enemyOC= {255, 255, 255};
		float fuckingaccent[4] = { 0.34f, 0.34f, 0.34f, 1.f }; // ACCENT COLOR ESHKERE
		Color color_glow_chickens= {255, 255, 255};
		Color color_glow_c4_carrier= {255, 255, 255};
		Color color_glow_planted_c4= {255, 255, 255};
		Color color_glow_defuse= {255, 255, 255};
		Color color_glow_weapons= {255, 255, 255};
		Color jbdetect = Color(255, 0, 0);
		Color ebdetect = Color(0, 255, 0);
		Color psdetect = Color(255, 255, 255);
		Color btdot = Color(255, 255, 255);
		Color aldetect = Color(255, 255, 255);
		Color accentcolsl = Color(255, 255, 255);
		Color esp_col = Color(255, 255, 255);
		Color Colorespbox = Color(255, 255, 255);
		Color accentcol = Color(255, 255, 255, 255);
		Color accentcoll = Color(255, 255, 255);
		Color spec_color = Color(255, 255, 255);
		Color accentcolssl = Color(255, 255, 255);
		Color skeleton_col = Color(255, 255, 255);
		Color color_chams_player_ally_visible= {255, 255, 255};
		Color color_chams_player_ally_occluded= {255, 255, 255};
		Color color_chams_player_enemy_visible= {255, 255, 255};
		Color color_chams_player_enemy_occluded= {255, 255, 255};
		Color color_chams_arms_visible= {255, 255, 255};
		Color color_chams_arms_occluded= {255, 255, 255};
		Color color_watermark= {255, 255, 255}; // no menu config cuz its useless
		Color player_enemy_visible_shine = { 255,255,255 };
		Color vel_graph_clr = Color(255, 255, 255);
		Color stm_graph_clr = Color(255, 255, 255);
		bool hitmarker;
		bool hit_sound;
		bool checkpoint = true;
		int checkpoint_choices;
		int checkpoint_key = VK_NUMPAD0;
		int checkpoint_key_s;
		int teleport_key= VK_NUMPAD1;
		int teleport_key_s;
		int teleport_nextkey= VK_NUMPAD2;
		int teleport_nextkey_s;
		int teleport_prevkey= VK_NUMPAD3;
		int teleport_prevkey_s;
		int teleport_undokey= VK_NUMPAD4;
		int teleport_undokey_s;
		int typeind{};


		struct 
		{
			  bool fade = true;
			  float size = 1.00f;

			struct 
			{
				  bool enable = false;
				  bool draw_velocity = false;
				  bool draw_jumpbug = false;
				  bool draw_edgebug = false;
				  bool draw_pixelsurf = false;
				  float color[3]{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f };
			}velocity;

			struct 
			{
				 bool enable = false;
				 float color[3]{ 166 / 255.0f, 143 / 255.0f, 242 / 255.0f };
			}stamina;
		}graphs;


		struct 
		{
			struct 
			{
				 bool enable = false;
				 bool hide_original_model = false;
				 int type = 0;
				 float color[4]{ 1.f, 1.f, 1.f, 1.f };

				struct 
				{
					 bool enable = false;
					 int index = 0;
					 int type = 0;
					 float color[4]{ 1.f, 1.f, 1.f, 1.f };
				} overlay;
			} visible;

			struct 
			{
				 bool enable = false;
				 bool hide_original_model = false;
				 int type = 0;
				 float color[4]{ 1.f, 1.f, 1.f, 1.f };

				struct 
				{
					 bool enable = false;
					 int index = 0;
					 int type = 0;
					 float color[4]{ 1.f, 1.f, 1.f, 1.f };
				}overlay;
			}invisible;

			struct 
			{
				 bool enable = false;
				 bool hide_original_model = false;
				 int type = 0;
				 float color[4]{ 1.f, 1.f, 1.f, 1.f };

				struct 
				{
					bool enable = false;
					int index = 0;
					int type = 0;
					float color[4]{ 1.f, 1.f, 1.f, 1.f };
				}overlay;
			}visible_attachment;

			struct 
			{
				 bool enable = false;
				 bool hide_original_model = false;
				 int type = 0;
				 float color[4]{ 1.f, 1.f, 1.f, 1.f };

				struct 
				{
					 bool enable = false;
					 int index = 0;
					 int type = 0;
					 float color[4]{ 1.f, 1.f, 1.f, 1.f };
				}overlay;
			}invisible_attachment;

			struct 
			{
				 bool enable = false;
				 bool hide_original_model = false;
				 int type = 0;
				 float color[4]{ 1.f, 1.f, 1.f, 1.f };

				struct 
				{
					 bool enable = false;
					 int index = 0;
					 int type = 0;
					 float color[4]{ 1.f, 1.f, 1.f, 1.f };
				}overlay;
			}sleeves;

			struct 
			{
				 bool enable = false;
				 bool hide_original_model = false;
				 int type = 0;
				 float color[4]{ 1.f, 1.f, 1.f, 1.f };

				struct 
				{
					 bool enable = false;
					 int index = 0;
					 int type = 0;
					 float color[4]{ 1.f, 1.f, 1.f, 1.f };
				}overlay;
			}weapon;

			struct 
			{
				 bool enable = false;
				 bool old_shaders = false;
				 bool hide_original_model = false;
				 int type = 0;
				 float color[4]{ 1.f, 1.f, 1.f, 1.f };

				struct 
				{
					 bool enable = false;
					 int index = 0;
					 int type = 0;
					 float color[4]{ 1.f, 1.f, 1.f, 1.f };
				}overlay;
			}hands;

			struct 
			{
				 bool enable = false;
				 bool hide_original_model = false;
				 bool gradient = false;
				 bool invisible = false;
				 bool draw_all_ticks = false;
				 int type = 0;
				 float color_1[4]{ 1.f, 1.f, 1.f, 1.f };
				 float color_2[4]{ 1.f, 1.f, 1.f, 1.f };

				struct 
				{
					 bool enable = false;
					 int index = 0;
					 int type = 0;
					 float color[4]{ 1.f, 1.f, 1.f, 1.f };
				}overlay;
			}backtrack;
		}chams;

		struct 
		{
			 bool engine_radar = false;
			 bool enable = false;
			struct 
			{
				 bool custom = false;
				 float visible[4]{ 0.0f, 1.0f, 0.0f, 1.0f };
				 float invisible[4]{ 1.0f, 0.0f, 0.0f, 1.0f };
				 float box[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float box_outline[4]{ 0.0f, 0.0f, 0.0f, 1.0f };
				 float name[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float health_bar[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float health_bar_upper[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float health_bar_lower[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float health_bar_outline[4]{ 0.0f, 0.0f, 0.0f, 1.0f };
				 float health_text[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float ammo_text[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float ammo_bar[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float ammo_bar_outline[4]{ 0.0f, 0.0f, 0.0f, 1.0f };
				 float weapon_text[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float weapon_icon[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float view_angle[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float skeleton[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float backtrack_skeleton[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float backtrack_dot[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float snapline[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float distance[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float dlight[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float out_of_view[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
				 float sounds[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
			}colors;

			struct 
			{
				 bool enable = false;
				 float time = 0.0f;
			}fade;

			struct 
			{
				 bool enable = false;
				 int type = 0;
				 bool outline[2] = { false, false };
				 float lenght = 5.f;
			}box;

			struct 
			{
				 bool enable = false;
				 bool bot_check = false;
			}name;

			struct 
			{
				 bool enable = false;
				 bool baseonhealth = false;
				 bool gradient = false;
				 bool background = false;
				 bool text = false;
				 int size = 1;
			}health_bar;

			struct 
			{
				 bool enable = false;
				 bool baseonhealth = false;
				 bool hide_suffix = false;
				 bool hide_at_max = false;
			}health_text;

			struct 
			{
				 bool enable = false;
			}ammo_text;

			struct 
			{
				 bool enable = false;
				 bool background = false;
				 int size = 1;
			}ammo_bar;

			struct 
			{
				 bool enable = false;
			}weapon_text;

			struct 
			{
				 bool enable = false;
			}weapon_icon;

			struct 
			{
				 bool enable = false;
				 float thickness = 0.5f;
				 float distance = 50.0f;
			}view_angle;

			struct 
			{
				 bool enable = false;
				 float thickness = 0.5f;
			}skeleton;

			struct 
			{
				 bool enable = false;
				 float thickness = 0.5f;
			}backtrack_skeleton;

			struct 
			{
				 bool enable = false;
			}backtrack_dot;

			struct 
			{
				 bool enable = false;
				 float size = 7.5f;
				 float distance = 30.0f;
			}out_of_view;

			struct 
			{
				 bool enable = false;
				 bool flags[7] = { false, false, false, false, false, false, false };
			}flags;

			struct 
			{
				 bool enable = false;
			}distance;

			struct 
			{
				 bool enable = false;
				 bool type[3] = { false, false, false };
				 float thickness = 0.5f;
			}snapline;

			struct 
			{
				 bool enable = false;
			}emitted_sound;

			struct 
			{
				 bool enable = false;
			}death_history;

			struct 
			{
				 bool enable = false;
				 float radius = 50.0f;
				 float color[4]{ 1.f, 1.f, 1.f, 1.f };
			}dlight;

			struct 
			{
				 float distance = 1500.0f;

				struct 
				{
					 bool enable = false;
					 int type = 0;
					 bool outline[2] = { false, false };
					 float lenght = 5.f;
					 float color[4]{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 200 / 255.0f };
				}box;

				struct 
				{
					 bool enable = false;
					 float color[4]{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 200 / 255.0f };
				}text;

				struct 
				{
					 bool enable = false;
					 float color[4]{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 200 / 255.0f };
				}icon;

				struct 
				{
					 bool enable = false;
					 float color[4]{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 200 / 255.0f };
				}ammo_text;

				struct 
				{
					 bool enable = false;
					 float color[4]{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 200 / 255.0f };
				}ammo_bar;
			}dropped_weapon;
			
			struct 
			{
				struct 
				{
					 bool enable = false;
					 float color[4]{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 200 / 255.0f };
				}text;

				struct 
				{
					 bool enable = false;
					 float color[4]{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 200 / 255.0f };
				}icon;
			}thrown_grenade;
		}players;
		struct {

			 int indi_size = 28;
			 int indi_font = 0;
			 bool indi_font_flag[11] = { false, false, false, false, false, false, false, false, false, true, false };

			 int sub_indi_size = 18;
			 int sub_indi_font = 0;
			 bool sub_indi_font_flag[11] = { false, false, false, false, false, false, false, false, false, true, false };

			 int scene_font = 0;
			 int scene_size = 12;
			 bool scene_flag[11] = { false, false, false, true, false, false, true, false, false, true, false };

			 int scene_sub_font = 0;
			 int scene_sub_size = 12;
			 bool scene_sub_flag[11] = { false, false, false, true, false, false, true, false, false, true, false };

			 int esp_font = 0;
			 int esp_size = 9;
			 bool esp_flag[11] = { false, false, false, true, false, false, true, false, true, true, false };

			 int esp_sub_font = 0;
			 int esp_sub_size = 9;
			 bool esp_sub_flag[11] = { false, false, false, true, false, false, true, false, true, true, false };

			 int debug_information_font = 0;
			 int debug_information_size = 32;
			 bool debug_information_flag[11] = { false, false, false, true, false, false, true, false, true, true, false };
		}fonts;
		struct {
			struct
			{
				struct
				{
					 bool enable = false;
					 bool disable_takeoff_on_ground = false;
					 bool takeoff = false;
					 float maximum_value = 400.0f;
					 int style = 0;

					struct 
					{
						 float saturation = 0.50f;
					}hsb_color;

					struct 
					{
						 float first[4]{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 0 / 255.0f };
						 float second[4]{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f };
					}interpolate_color;

					struct 
					{
						 float positive[4]{ 23 / 255.0f, 238 / 255.0f, 103 / 255.0f, 255 / 255.0f };
						 float neutral[4]{ 255 / 255.0f, 200 / 255.0f, 100 / 255.0f, 255 / 255.0f };
						 float negative[4]{ 225 / 255.0f, 100 / 255.0f, 100 / 255.0f, 255 / 255.0f };
					}delta_color;
				}velocity;
				struct
				{
					 bool enable = false;
					 bool disable_takeoff_on_ground = false;
					 bool takeoff = false;
					 float maximum_value = 40.0f;
					 int style = 0;

					 float color[4]{ 200 / 255.0f, 200 / 255.0f, 200 / 255.0f, 255 / 255.0f };

					struct  
					{
						 float saturation = 0.50f;
					}hsb_color;

					struct
					{
						 float first[4]{ 200 / 255.0f, 200 / 255.0f, 200 / 255.0f, 255 / 255.0f };
						 float second[4]{ 128 / 255.0f, 255 / 255.0f, 128 / 255.0f, 255 / 255.0f };
					}interpolate_color;
				}stamina;
			}indicators;
		}movement;
		struct 
		{
			 bool enable = false;
			 int video_adapter = 1;
			 bool forward_move_blur = false;
			 float falling_minimum = 10.0f;
			 float falling_maximum = 20.0f;
			 float falling_intensity = 1.0f;
			 float rotate_intensity = 1.0f;
			 float strength = 1.0f;
		}motionblur;
		ImVec2 positionsind{};
		bool sunset = false;
		int sunset_x = 0;
		int sunset_y = 0;
		bool m_enabled{ false };
		bool forwardEnabled{ false };
		float wear = FLT_MIN;
		float fallingMin{ 10.0f };
		float fallingMax{ 20.0f };
		float fallingIntensity{ 1.0f };
		float rotationIntensity{ 1.0f };
		float strength{ 1.0f };
		bool watermark = true;
		bool trackdispay;

		std::wstring artist, title;
		bool pause = false;
		bool next = false;
		bool pervious = false;
		std::string albumArtPath;
		int64_t trackPosition;
		int64_t trackDuration = 0;
protected:
	//std::vector<ConfigValue<char>*> chars;
	std::vector<ConfigValue<int>*> ints;
	std::vector<ConfigValue<bool>*> bools;
	std::vector<ConfigValue<float>*> floats;
private:
//	void SetupValue(char value, std::string category, std::string name);
	void SetupValue(int& value, std::string category, std::string name);
	void SetupValue(bool& value, std::string category, std::string name);
	void SetupValue(float& value, std::string category, std::string name);
	void SetupColor(Color& value, const std::string& name);
	void SetupWeapons();
	void SetupVisuals();
	void SetupMisc();
	void SetupColors();
public:
	void Initialize();
	void LoadSettings(const std::string& szIniFile);
	void SaveSettings(const std::string& szIniFile);
	void DeleteSettings(const std::string& szIniFile);

	std::string folder;
	std::string HWID;
};

inline Options g_Options;
inline bool   g_Unload;
