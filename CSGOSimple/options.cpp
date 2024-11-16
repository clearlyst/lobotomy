#include <ShlObj.h>
#include <filesystem>
#include "options.hpp"
#include "valve_sdk/misc/Enums.hpp"
#include "valve_sdk/sdk.hpp"

const std::map<int, const char*> config_names = {
	{0, "Pistols"},
	{1, "HeavyPistols"},
	{2, "ShotGun"},
	{3, "Heavy"},
	{4, "Smg"},
	{5, "Rifle"},
	{6, "Sniper"},
	{7, "ASniper"},
	
};

const std::map<int, const char*> k_weapon_names = {
	{WEAPON_KNIFE, "knife ct"},
	{WEAPON_KNIFE_T, "knife t"},
	{GLOVE_CT_SIDE, "glove ct"},
	{GLOVE_T_SIDE, "glove t"},
	{61, "usp"},
	{32, "p2000"},
	{4, "glock-18"},
	{2, "dual berettas"},
	{36, "p250"},
	{3, "five-seven"},
	{30, "tec-9"},
	{63, "cz75 auto"},
	{64, "r8 revolver"},
	{1, "deagle"},

	{34, "mp9"},
	{17, "mac-10"},
	{23, "mp5-sd"},
	{33, "mp7"},
	{24, "ump-45"},
	{19, "p90"},
	{26, "pp-bizon"},

	{7, "ak-47"},
	{60, "m4a1-s"},
	{16, "m4a4"},
	{8, "aug"},
	{39, "sg553"},
	{10, "famas"},
	{13, "galil"},

	{40, "ssg08"},
	{38, "scar-20"},
	{9, "awp"},
	{11, "g3sg1"},

	{14, "m249"},
	{28, "negev"},

	{27, "mag-7"},
	{35, "nova"},
	{29, "sawed-off"},
	{25, "xm1014"},
};

void Options::SetupValue(int& value, std::string category, std::string name)
{
	ints.push_back(new ConfigValue<int>(std::move(category), std::move(name), &value));
}
/*void Options::SetupValue(char value, std::string category, std::string name)
{
	chars.push_back(new ConfigValue<char>(std::move(category), std::move(name), &value));
}*/

void Options::SetupValue(bool& value, std::string category, std::string name)
{
	bools.push_back(new ConfigValue<bool>(std::move(category), std::move(name), &value));
}

void Options::SetupValue(float& value, std::string category, std::string name)
{
	floats.push_back(new ConfigValue<float>(std::move(category), std::move(name), &value));
}

void Options::SetupColor(Color& value, const std::string& name)
{
	SetupValue(value._CColor[0], "Colors", name + "_r");
	SetupValue(value._CColor[1], "Colors", name + "_g");
	SetupValue(value._CColor[2], "Colors", name + "_b");
	SetupValue(value._CColor[3], "Colors", name + "_a");
}


#define save(name) SetupValue(g_Options.##name,"Options", #name);
#define saveclr(name) SetupColor(g_Options.##name,#name);

void Options::SetupWeapons()
{
	for (auto& [val, key] : config_names)
	{
		auto aimbot = &g_Options.weapons[val].legit;

		if (val != WEAPON_KNIFE && val != WEAPON_KNIFE_T && val != GLOVE_T_SIDE)
		{
			SetupValue(aimbot->enabled, key, "Enabled");
			//SetupValue(aimbot->silent, key, "pSilent");
			SetupValue(aimbot->silent2, key, "Silent");

			SetupValue(aimbot->flash_check, key, "Flash Check");
			SetupValue(aimbot->smoke_check, key, "Smoke Check");
			SetupValue(aimbot->autopistol, key, "AutoPistol");
			SetupValue(g_Options.backtrack, key, "backtrack");
			SetupValue(g_Options.aim_at_bt, key, "aimatbt");
			SetupValue(g_Options.extend_bt, key, "extendbt");
			SetupValue(aimbot->fov, key, "Fov");
			SetupValue(aimbot->silent_fov, key, "pSilent Fov");
			SetupValue(aimbot->smooth, key, "Smooth");
			SetupValue(aimbot->silent_s, key, "silent aim");
			SetupValue(aimbot->kill_delay, key, "Kill Delay");
			SetupValue(aimbot->shot_delay, key, "Shot Delay");

			SetupValue(aimbot->hitboxes.head, key, "Head Hitbox");
			SetupValue(aimbot->hitboxes.stomach, key, "Stomach Hitbox");
			SetupValue(aimbot->hitboxes.chest, key, "Chest Hitbox");
			SetupValue(aimbot->hitboxes.hands, key, "Hands Hitbox");
			SetupValue(aimbot->hitboxes.legs, key, "Legs Hitbox");

			SetupValue(aimbot->rcs.enabled, key, "RCS Enabled");
			SetupValue(aimbot->rcs.start, key, "RCS Start");
			SetupValue(aimbot->rcs.type, key, "RCS Type");
			SetupValue(aimbot->rcs.x, key, "RCS X");
			SetupValue(aimbot->rcs.y, key, "RCS Y");
			SetupValue(aimbot->silent_s, key, "silentaim");
			SetupValue(aimbot->autowall.enabled, key, "AutoWall Enabled");
			SetupValue(aimbot->autowall.min_damage, key, "AutoWall MinDamage");

			SetupValue(aimbot->autofire.enabled, key, "AutoFire Enabled");
			SetupValue(aimbot->autofire.hotkey, key, "AutoFire Hotkey");

		}
	}
	for (auto& [key, val] : k_weapon_names) {
		auto& option = g_Options.changers.skin.m_items[key];
		SetupValue(option.definition_vector_index, val, "d_vec_index");
		SetupValue(option.definition_index, val, "d_index");
		SetupValue(option.paint_kit_vector_index, val, "pk_vec_index");
		SetupValue(option.paint_kit_index, val, "pk_index");
		SetupValue(option.definition_override_index, val, "do_index");
		SetupValue(option.definition_override_vector_index, val, "do_vec_index");
		SetupValue(option.seed, val, "seed");
		//SetupValue(option.enable_stat_track, val, "stattrack");
		//SetupValue(option.stat_trak, val, "stat_trak_val");
		SetupValue(option.wear, val, "wear");
	}

}

void Options::SetupVisuals()
{
	

	

}

void Options::SetupMisc()
{
	
	save(delayhopticks);
	save(fastduck);
	save(panoramablur);
	save(postproccesing);
	save(showag);
	save(showmj);
	save(autoduck);
	save(stm_graph_w);
	save(stm_graph_y);
	save(graph_w);
	save(graph_y);
	save(autoducktick);
	save(edgebugtick);
	save(showjb);
	save(eblock);
	save(esp_player_healthbar);
	save(oof_arc);
	save(freelook);
	save(stamina_graph);
	save(money_flags);
	save(hk_flags);
	save(pixel_surf_calc);
	save(drawclip);
	save(wirefame);
	save(kits_flags);
	save(velocity_graph);
	save(specposx);
	save(specposy);
	save(specsizex);
	save(showeb);
	save(noviewbob);
	save(setpointps);
	save(calculatekey);
	save(ebinfo);
	save(extend_bt);
	save(btcount);
	save(box_type);
	save(clantag);
	save(zeusbug);
	save(radius);
	save(custom_console);
	save(hitinfo);
	save(spec_type);
	save(skybox_num);
	save(scaleform);
	save(esp_backtrack);
	save(ladderbugfix);
	save(flags);
	save(exploit);
	save(ljend);
	save(nulls3);
	save(back);
	save(yaw_changer);
	save(velotag);
	save(yaw1);
	save(yaw2);
	save(moveleft);
	save(moveright);
	save(forwardd);
	save(left);
	save(right);
	save(nulls3_lj);
	save(nulls3_mj);
	save(nulls3_ej);
	save(SkiNChanger);
	save(esp_enemies_only);
	save(esp_player_boxes);
	save(esp_player_boxesOccluded);
	save(esp_player_names);
	save(esp_player_health);
	save(skele_ton_esp);
	save(esp_player_armour);
	save(esp_player_weapons);
	save(esp_player_snaplines);
	save(esp_dropped_weapons);
	save(esp_dropped_weapons_icon);
	save(esp_defuse_kit);
	save(esp_planted_c4);
	save(esp_items);
	save(detectchat);
	save(lockfactor);
	save(glow_enabled);
	save(glow_enemies_only);
	save(glow_players);
	save(glow_chickens);
	save(glow_c4_carrier);
	save(glow_planted_c4);
	save(glow_defuse_kits);
	save(glow_weapons);
	save(glow_enemiesOC);
	save(aimatbt);
	save(chams_player_enabled);
	save(player_enemies_shine);
	save(chams_player_enemies_only);
	save(chams_player_wireframe);
	save(chams_player_flat);
	save(chams_player_ignorez);
	save(chams_player_glass);
	save(chams_arms_enabled);
	save(chams_arms_wireframe);
	save(chams_arms_flat);
	save(chams_arms_ignorez);
	save(chams_arms_glass);
	save(CHECKSURFLOLKEK);
	save(KEYCHECKSURFLOLKEK);
	save(KEYCHECKSURFLOLKEKPoint);
	save(misc_bhop);
	save(misc_bhop2);
	save(playerModelT);
	save(playerModelCT);
	save(misc_showranks);
	save(misc_watermark);
	save(Velocity);
	save(outline);
	save(lastjump);
	save(lastjumpoutline);
	save(autoaccept);
	save(no_flash);
	save(no_smoke);
	save(spectator_list);
	save(AutoStafe_key);
	save(autostrafe);
	save(veloposy);
	save(oofarcsize);
	save(staminaposy);

	// edgejump struct
	save(edgejump.enabled);
	save(edgejump.edge_jump_duck_in_air);
	save(edgejump.lg_on_ej);
	save(edgejump.hotkey);

	// eb struct
	save(eb.edgebug);
	save(eb.edgebug_asissttoedge);
	save(eb.edgebug_key);
	save(eb.chatinfo_edgebug);
	save(eb.edgebug_key_type);
	save(eb.edgebug_ticks);
	save(eb.edgebug_mouselock);
	save(eb.edgebug_indicator);
	save(eb.edgebug_indicator_clr[0]);
	save(eb.edgebug_indicator_clr[1]);
	save(eb.edgebug_indicator_clr[2]);
	save(eb.edgebug_indicator_detect_clr[0]);
	save(eb.edgebug_indicator_detect_clr[1]);
	save(eb.edgebug_indicator_detect_clr[2]);
	save(eb.edgebug_sound);
	save(eb.edgebug_sound_type);
	save(eb.edgebug_counter);
	save(eb.edgebug_counter_pos);
	save(eb.edgebug_counter_clr[0]);
	save(eb.edgebug_counter_clr[1]);
	save(eb.edgebug_counter_clr[2]);
	save(eb.edgebug_effect);

	save(edge_bug);
	save(OffSilent);
	save(takeoffstam);
	save(rintarovel);
	save(defstylevelo);
	save(edge_bug_key);
	save(jump_bug);
	save(misc_minijump);
	save(AutoBounce);
	save(KeyAutoBounce);
	save(abobakey);
	save(pixelsurf);
	save(ps_fix);
	save(autoalign);
	save(unlockcvars);
	save(indicatorsposy);
	save(size_test);
	save(size_test_esp);
	save(hitbones);
	save(jump_bug_key);
	save(dhkey);
	save(glow_enemies_type);
	save(sniper_xhair);
	save(takeoffvel);
	save(esp_world_fog);
	save(pxcalcbl);
	save(posloska);
	save(esp_world_fog_start);
	save(esp_world_fog_end);
	save(esp_world_fog_destiny);
	saveclr(fog_color);
	save(backtrack);
	save(stamina);
	save(jump_on_jb);
	save(aim_at_bt);
	save(misc_indicators);
	save(esp_enabled);
	save(showlj);
	save(showps);
	save(delayhop);
	save(iconesp);
	save(chams.visible_attachment.overlay.enable);
	save(chams.visible_attachment.overlay.index);
	save(chams.visible_attachment.overlay.type);
	save(chams.visible_attachment.overlay.color[0]);
	save(chams.visible_attachment.overlay.color[1]);
	save(chams.visible_attachment.overlay.color[2]);
	save(chams.visible_attachment.overlay.color[3]);

	save(chams.invisible_attachment.enable);
	save(chams.invisible_attachment.hide_original_model);
	save(chams.invisible_attachment.type);
	save(chams.invisible_attachment.color[0]);
	save(chams.invisible_attachment.color[1]);
	save(chams.invisible_attachment.color[2]);
	save(chams.invisible_attachment.color[3]);
	save(chams.invisible_attachment.overlay.enable);
	save(chams.invisible_attachment.overlay.index);
	save(chams.invisible_attachment.overlay.type);
	save(chams.invisible_attachment.overlay.color[0]);
	save(chams.invisible_attachment.overlay.color[1]);
	save(chams.invisible_attachment.overlay.color[2]);
	save(chams.invisible_attachment.overlay.color[3]);

	save(chams.sleeves.enable);
	save(chams.sleeves.hide_original_model);
	save(chams.sleeves.type);
	save(chams.sleeves.color[0]);
	save(chams.sleeves.color[1]);
	save(chams.sleeves.color[2]);
	save(chams.sleeves.color[3]);
	save(chams.sleeves.overlay.enable);
	save(chams.sleeves.overlay.index);
	save(chams.sleeves.overlay.type);
	save(chams.sleeves.overlay.color[0]);
	save(chams.sleeves.overlay.color[1]);
	save(chams.sleeves.overlay.color[2]);
	save(chams.sleeves.overlay.color[3]);

	save(chams.weapon.enable);
	save(chams.weapon.hide_original_model);
	save(chams.weapon.type);
	save(chams.weapon.color[0]);
	save(chams.weapon.color[1]);
	save(chams.weapon.color[2]);
	save(chams.weapon.color[3]);
	save(chams.weapon.overlay.enable);
	save(chams.weapon.overlay.index);
	save(chams.weapon.overlay.type);
	save(chams.weapon.overlay.color[0]);
	save(chams.weapon.overlay.color[1]);
	save(chams.weapon.overlay.color[2]);
	save(chams.weapon.overlay.color[3]);

	save(chams.hands.enable);
	save(chams.hands.hide_original_model);
	save(chams.hands.old_shaders);
	save(chams.hands.type);
	save(chams.hands.color[0]);
	save(chams.hands.color[1]);
	save(chams.hands.color[2]);
	save(chams.hands.color[3]);
	save(chams.hands.overlay.enable);
	save(chams.hands.overlay.index);
	save(chams.hands.overlay.type);
	save(chams.hands.overlay.color[0]);
	save(chams.hands.overlay.color[1]);
	save(chams.hands.overlay.color[2]);
	save(chams.hands.overlay.color[3]);

	save(chams.backtrack.enable);
	save(chams.backtrack.hide_original_model);
	save(chams.backtrack.gradient);
	save(chams.backtrack.invisible);
	save(chams.backtrack.draw_all_ticks);
	save(chams.backtrack.type);
	save(chams.backtrack.color_1[0]);
	save(chams.backtrack.color_1[1]);
	save(chams.backtrack.color_1[2]);
	save(chams.backtrack.color_1[3]);
	save(chams.backtrack.color_2[0]);
	save(chams.backtrack.color_2[1]);
	save(chams.backtrack.color_2[2]);
	save(chams.backtrack.color_2[3]);
	save(chams.backtrack.overlay.enable);
	save(chams.backtrack.overlay.index);
	save(chams.backtrack.overlay.type);
	save(chams.backtrack.overlay.color[0]);
	save(chams.backtrack.overlay.color[1]);
	save(chams.backtrack.overlay.color[2]);
	save(chams.backtrack.overlay.color[3]);
	saveclr(color_esp_ally_visible);
	saveclr(color_esp_enemy_visible);
	saveclr(color_esp_ally_occluded);
	saveclr(color_esp_enemy_occluded);
	saveclr(color_esp_weapons);
	saveclr(color_esp_defuse);
	saveclr(color_esp_c4);
	saveclr(color_esp_item);
	saveclr(Velocitycol);
	saveclr(color_glow_ally);
	saveclr(color_glow_allyOC);
	saveclr(Coloresp_ally_visible);
	saveclr(arc_visible);
	saveclr(arc_invisible);
	saveclr(Coloresp_enemy_visible);
	saveclr(skybox_color);
	saveclr(Coloresp_ally_occluded);
	saveclr(hit_bones_col);
	saveclr(Coloresp_enemy_occluded);
	saveclr(color_glow_enemy);
	saveclr(color_glow_enemyOC);
	save(fuckingaccent[0]);
	save(fuckingaccent[1]);
	save(fuckingaccent[2]);
	save(fuckingaccent[3]);
	saveclr(color_glow_chickens);
	saveclr(color_glow_c4_carrier);
	saveclr(color_glow_planted_c4);
	saveclr(color_glow_defuse);
	saveclr(color_glow_weapons);
	saveclr(jbdetect);
	saveclr(ebdetect);
	saveclr(psdetect);
	saveclr(btdot);
	saveclr(aldetect);
	saveclr(accentcolsl);
	saveclr(esp_col);
	saveclr(Colorespbox);
	saveclr(accentcol);
	saveclr(accentcoll);
	saveclr(spec_color);
	saveclr(accentcolssl);
	saveclr(skeleton_col);
	saveclr(color_chams_player_ally_visible);
	saveclr(color_chams_player_ally_occluded);
	saveclr(color_chams_player_enemy_visible);
	saveclr(color_chams_player_enemy_occluded);
	saveclr(color_chams_arms_visible);
	saveclr(color_chams_arms_occluded);
	saveclr(color_watermark);
	saveclr(player_enemy_visible_shine);
	saveclr(vel_graph_clr);
	saveclr(stm_graph_clr);

	// Booleans
	save(hitmarker);
	save(hit_sound);
	save(checkpoint);
	save(checkpoint_choices);

	// Integers
	save(checkpoint_key);
	save(checkpoint_key_s);
	save(teleport_key);
	save(teleport_key_s);
	save(teleport_nextkey);
	save(teleport_nextkey_s);
	save(teleport_prevkey);
	save(teleport_prevkey_s);
	save(teleport_undokey);
	save(teleport_undokey_s);
	save(typeind);

	// Structs
	save(graphs.fade);
	save(graphs.size);
	save(graphs.velocity.enable);
	save(graphs.velocity.draw_velocity);
	save(graphs.velocity.draw_jumpbug);
	save(graphs.velocity.draw_edgebug);
	save(graphs.velocity.draw_pixelsurf);
	save(graphs.velocity.color[0]);
	save(graphs.velocity.color[1]);
	save(graphs.velocity.color[2]);
	save(graphs.stamina.enable);
	save(graphs.stamina.color[0]);
	save(graphs.stamina.color[1]);
	save(graphs.stamina.color[2]);

	save(chams.visible.enable);
	save(chams.visible.hide_original_model);
	save(chams.visible.type);
	save(chams.visible.color[0]);
	save(chams.visible.color[1]);
	save(chams.visible.color[2]);
	save(chams.visible.color[3]);
	save(chams.visible.overlay.enable);
	save(chams.visible.overlay.index);
	save(chams.visible.overlay.type);
	save(chams.visible.overlay.color[0]);
	save(chams.visible.overlay.color[1]);
	save(chams.visible.overlay.color[2]);
	save(chams.visible.overlay.color[3]);

	save(chams.invisible.enable);
	save(chams.invisible.hide_original_model);
	save(chams.invisible.type);
	save(chams.invisible.color[0]);
	save(chams.invisible.color[1]);
	save(chams.invisible.color[2]);
	save(chams.invisible.color[3]);
	save(chams.invisible.overlay.enable);
	save(chams.invisible.overlay.index);
	save(chams.invisible.overlay.type);
	save(chams.invisible.overlay.color[0]);
	save(chams.invisible.overlay.color[1]);
	save(chams.invisible.overlay.color[2]);
	save(chams.invisible.overlay.color[3]);

	save(chams.visible_attachment.enable);
	save(chams.visible_attachment.hide_original_model);
	save(chams.visible_attachment.type);
	save(chams.visible_attachment.color[0]);
	save(chams.visible_attachment.color[1]);
	save(chams.visible_attachment.color[2]);
	save(chams.visible_attachment.color[3]);
	save(movement.indicators.velocity.delta_color.negative[1]);
	save(movement.indicators.velocity.delta_color.negative[2]);
	save(movement.indicators.velocity.delta_color.negative[3]);
	save(fonts.sub_indi_font_flag[2]);
	save(fonts.sub_indi_font_flag[3]);
	save(fonts.sub_indi_font_flag[4]);
	save(fonts.sub_indi_font_flag[5]);
	save(fonts.sub_indi_font_flag[6]);
	save(fonts.sub_indi_font_flag[7]);
	save(fonts.sub_indi_font_flag[8]);
	save(fonts.sub_indi_font_flag[9]);
	save(fonts.sub_indi_font_flag[10]);

	save(fonts.scene_font);
	save(fonts.scene_size);
	save(fonts.scene_flag[0]);
	save(fonts.scene_flag[1]);
	save(fonts.scene_flag[2]);
	save(fonts.scene_flag[3]);
	save(fonts.scene_flag[4]);
	save(fonts.scene_flag[5]);
	save(fonts.scene_flag[6]);
	save(fonts.scene_flag[7]);
	save(fonts.scene_flag[8]);
	save(fonts.scene_flag[9]);
	save(fonts.scene_flag[10]);

	save(fonts.scene_sub_font);
	save(fonts.scene_sub_size);
	save(fonts.scene_sub_flag[0]);
	save(fonts.scene_sub_flag[1]);
	save(fonts.scene_sub_flag[2]);
	save(fonts.scene_sub_flag[3]);
	save(fonts.scene_sub_flag[4]);
	save(fonts.scene_sub_flag[5]);
	save(fonts.scene_sub_flag[6]);
	save(fonts.scene_sub_flag[7]);
	save(fonts.scene_sub_flag[8]);
	save(fonts.scene_sub_flag[9]);
	save(fonts.scene_sub_flag[10]);

	save(fonts.esp_font);
	save(fonts.esp_size);
	save(fonts.esp_flag[0]);
	save(fonts.esp_flag[1]);
	save(fonts.esp_flag[2]);
	save(fonts.esp_flag[3]);
	save(fonts.esp_flag[4]);
	save(fonts.esp_flag[5]);
	save(fonts.esp_flag[6]);
	save(fonts.esp_flag[7]);
	save(fonts.esp_flag[8]);
	save(fonts.esp_flag[9]);
	save(fonts.esp_flag[10]);

	save(fonts.esp_sub_font);
	save(fonts.esp_sub_size);
	save(fonts.esp_sub_flag[0]);
	save(fonts.esp_sub_flag[1]);
	save(fonts.esp_sub_flag[2]);
	save(fonts.esp_sub_flag[3]);
	save(fonts.esp_sub_flag[4]);
	save(fonts.esp_sub_flag[5]);
	save(fonts.esp_sub_flag[6]);
	save(fonts.esp_sub_flag[7]);
	save(fonts.esp_sub_flag[8]);
	save(fonts.esp_sub_flag[9]);
	save(fonts.esp_sub_flag[10]);

	save(fonts.debug_information_font);
	save(fonts.debug_information_size);
	save(fonts.debug_information_flag[0]);
	save(fonts.debug_information_flag[1]);
	save(fonts.debug_information_flag[2]);
	save(fonts.debug_information_flag[3]);
	save(fonts.debug_information_flag[4]);
	save(fonts.debug_information_flag[5]);
	save(fonts.debug_information_flag[6]);
	save(fonts.debug_information_flag[7]);
	save(fonts.debug_information_flag[8]);
	save(fonts.debug_information_flag[9]);
	save(fonts.debug_information_flag[10]);

	save(movement.indicators.velocity.enable);
	save(movement.indicators.velocity.disable_takeoff_on_ground);
	save(movement.indicators.velocity.takeoff);
	save(movement.indicators.velocity.maximum_value);
	save(movement.indicators.velocity.style);

	save(movement.indicators.velocity.hsb_color.saturation);

	save(movement.indicators.velocity.interpolate_color.first[0]);
	save(movement.indicators.velocity.interpolate_color.first[1]);
	save(movement.indicators.velocity.interpolate_color.first[2]);
	save(movement.indicators.velocity.interpolate_color.first[3]);

	save(movement.indicators.velocity.interpolate_color.second[0]);
	save(movement.indicators.velocity.interpolate_color.second[1]);
	save(movement.indicators.velocity.interpolate_color.second[2]);
	save(movement.indicators.velocity.interpolate_color.second[3]);

	save(movement.indicators.velocity.delta_color.positive[0]);
	save(movement.indicators.velocity.delta_color.positive[1]);
	save(movement.indicators.velocity.delta_color.positive[2]);
	save(movement.indicators.velocity.delta_color.positive[3]);

	save(movement.indicators.velocity.delta_color.neutral[0]);
	save(movement.indicators.velocity.delta_color.neutral[1]);
	save(movement.indicators.velocity.delta_color.neutral[2]);
	save(movement.indicators.velocity.delta_color.neutral[3]);

	save(movement.indicators.velocity.delta_color.negative[0]);
	save(players.health_bar.text);
	save(players.health_bar.size);

	save(players.health_text.enable);
	save(players.health_text.baseonhealth);
	save(players.health_text.hide_suffix);
	save(players.health_text.hide_at_max);

	save(players.ammo_text.enable);

	save(players.ammo_bar.enable);
	save(players.ammo_bar.background);
	save(players.ammo_bar.size);

	save(players.weapon_text.enable);

	save(players.weapon_icon.enable);

	save(players.view_angle.enable);
	save(players.view_angle.thickness);
	save(players.view_angle.distance);

	save(players.skeleton.enable);
	save(players.skeleton.thickness);

	save(players.backtrack_skeleton.enable);
	save(players.backtrack_skeleton.thickness);

	save(players.backtrack_dot.enable);

	save(players.out_of_view.enable);
	save(players.out_of_view.size);
	save(players.out_of_view.distance);

	save(players.flags.enable);
	save(players.flags.flags[0]);
	save(players.flags.flags[1]);
	save(players.flags.flags[2]);
	save(players.flags.flags[3]);
	save(players.flags.flags[4]);
	save(players.flags.flags[5]);
	save(players.flags.flags[6]);

	save(players.distance.enable);

	save(players.snapline.enable);
	save(players.snapline.type[0]);
	save(players.snapline.type[1]);
	save(players.snapline.type[2]);
	save(players.snapline.thickness);

	save(players.emitted_sound.enable);

	save(players.death_history.enable);

	save(players.dlight.enable);
	save(players.dlight.radius);
	save(players.dlight.color[0]);
	save(players.dlight.color[1]);
	save(players.dlight.color[2]);
	save(players.dlight.color[3]);

	save(players.dropped_weapon.distance);

	save(players.dropped_weapon.box.enable);
	save(players.dropped_weapon.box.type);
	save(players.dropped_weapon.box.outline[0]);
	save(players.dropped_weapon.box.outline[1]);
	save(players.dropped_weapon.box.lenght);
	save(players.dropped_weapon.box.color[0]);
	save(players.dropped_weapon.box.color[1]);
	save(players.dropped_weapon.box.color[2]);
	save(players.dropped_weapon.box.color[3]);

	save(players.dropped_weapon.text.enable);
	save(players.dropped_weapon.text.color[0]);
	save(players.dropped_weapon.text.color[1]);
	save(players.dropped_weapon.text.color[2]);
	save(players.dropped_weapon.text.color[3]);

	save(players.dropped_weapon.icon.enable);
	save(players.dropped_weapon.icon.color[0]);
	save(players.dropped_weapon.icon.color[1]);
	save(players.dropped_weapon.icon.color[2]);
	save(players.dropped_weapon.icon.color[3]);

	save(players.dropped_weapon.ammo_text.enable);
	save(players.dropped_weapon.ammo_text.color[0]);
	save(players.dropped_weapon.ammo_text.color[1]);
	save(players.dropped_weapon.ammo_text.color[2]);
	save(players.dropped_weapon.ammo_text.color[3]);

	save(players.dropped_weapon.ammo_bar.enable);
	
	save(players.dropped_weapon.ammo_bar.color[0]);
	save(players.dropped_weapon.ammo_bar.color[1]);
	save(players.dropped_weapon.ammo_bar.color[2]);
	save(players.dropped_weapon.ammo_bar.color[3]);

	save(players.thrown_grenade.text.enable);
	save(players.thrown_grenade.text.color[0]);
	save(players.thrown_grenade.text.color[1]);
	save(players.thrown_grenade.text.color[2]);
	save(players.thrown_grenade.text.color[3]);

	save(players.thrown_grenade.icon.enable);
	save(players.thrown_grenade.icon.color[0]);
	save(players.thrown_grenade.icon.color[1]);
	save(players.thrown_grenade.icon.color[2]);
	save(players.thrown_grenade.icon.color[3]);

	save(fonts.indi_size);
	save(fonts.indi_font);
	save(fonts.indi_font_flag[0]);
	save(fonts.indi_font_flag[1]);
	save(fonts.indi_font_flag[2]);
	save(fonts.indi_font_flag[3]);
	save(fonts.indi_font_flag[4]);
	save(fonts.indi_font_flag[5]);
	save(fonts.indi_font_flag[6]);
	save(fonts.indi_font_flag[7]);
	save(fonts.indi_font_flag[8]);
	save(fonts.indi_font_flag[9]);
	save(fonts.indi_font_flag[10]);

	save(fonts.sub_indi_size);
	save(fonts.sub_indi_font);
	save(fonts.sub_indi_font_flag[0]);
	save(fonts.sub_indi_font_flag[1]);
	save(players.engine_radar);
	save(players.enable);

	save(players.colors.custom);
	save(players.colors.visible[0]);
	save(players.colors.visible[1]);
	save(players.colors.visible[2]);
	save(players.colors.visible[3]);
	save(players.colors.invisible[0]);
	save(players.colors.invisible[1]);
	save(players.colors.invisible[2]);
	save(players.colors.invisible[3]);
	save(players.colors.box[0]);
	save(players.colors.box[1]);
	save(players.colors.box[2]);
	save(players.colors.box[3]);
	save(players.colors.box_outline[0]);
	save(players.colors.box_outline[1]);
	save(players.colors.box_outline[2]);
	save(players.colors.box_outline[3]);
	save(players.colors.name[0]);
	save(players.colors.name[1]);
	save(players.colors.name[2]);
	save(players.colors.name[3]);
	save(players.colors.health_bar[0]);
	save(players.colors.health_bar[1]);
	save(players.colors.health_bar[2]);
	save(players.colors.health_bar[3]);
	save(players.colors.health_bar_upper[0]);
	save(players.colors.health_bar_upper[1]);
	save(players.colors.health_bar_upper[2]);
	save(players.colors.health_bar_upper[3]);
	save(players.colors.health_bar_lower[0]);
	save(players.colors.health_bar_lower[1]);
	save(players.colors.health_bar_lower[2]);
	save(players.colors.health_bar_lower[3]);
	save(players.colors.health_bar_outline[0]);
	save(players.colors.health_bar_outline[1]);
	save(players.colors.health_bar_outline[2]);
	save(players.colors.health_bar_outline[3]);
	save(players.colors.health_text[0]);
	save(players.colors.health_text[1]);
	save(players.colors.health_text[2]);
	save(players.colors.health_text[3]);
	save(players.colors.ammo_text[0]);
	save(players.colors.ammo_text[1]);
	save(players.colors.ammo_text[2]);
	save(players.colors.ammo_text[3]);
	save(players.colors.ammo_bar[0]);
	save(players.colors.ammo_bar[1]);
	save(players.colors.ammo_bar[2]);
	save(players.colors.ammo_bar[3]);
	save(players.colors.ammo_bar_outline[0]);
	save(players.colors.ammo_bar_outline[1]);
	save(players.colors.ammo_bar_outline[2]);
	save(players.colors.ammo_bar_outline[3]);
	save(players.colors.weapon_text[0]);
	save(players.colors.weapon_text[1]);
	save(players.colors.weapon_text[2]);
	save(players.colors.weapon_text[3]);
	save(players.colors.weapon_icon[0]);
	save(players.colors.weapon_icon[1]);
	save(players.colors.weapon_icon[2]);
	save(players.colors.weapon_icon[3]);
	save(players.colors.view_angle[0]);
	save(players.colors.view_angle[1]);
	save(players.colors.view_angle[2]);
	save(players.colors.view_angle[3]);
	save(players.colors.skeleton[0]);
	save(players.colors.skeleton[1]);
	save(players.colors.skeleton[2]);
	save(players.colors.skeleton[3]);
	save(players.colors.backtrack_skeleton[0]);
	save(players.colors.backtrack_skeleton[1]);
	save(players.colors.backtrack_skeleton[2]);
	save(players.colors.backtrack_skeleton[3]);
	save(players.colors.backtrack_dot[0]);
	save(players.colors.backtrack_dot[1]);
	save(players.colors.backtrack_dot[2]);
	save(players.colors.backtrack_dot[3]);
	save(players.colors.snapline[0]);
	save(players.colors.snapline[1]);
	save(players.colors.snapline[2]);
	save(players.colors.snapline[3]);
	save(players.colors.distance[0]);
	save(players.colors.distance[1]);
	save(players.colors.distance[2]);
	save(players.colors.distance[3]);
	save(players.colors.dlight[0]);
	save(players.colors.dlight[1]);
	save(players.colors.dlight[2]);
	save(players.colors.dlight[3]);
	save(players.colors.out_of_view[0]);
	save(players.colors.out_of_view[1]);
	save(players.colors.out_of_view[2]);
	save(players.colors.out_of_view[3]);
	save(players.colors.sounds[0]);
	save(players.colors.sounds[1]);
	save(players.colors.sounds[2]);
	save(players.colors.sounds[3]);

	save(players.fade.enable);
	save(players.fade.time);

	save(players.box.enable);
	save(players.box.type);
	save(players.box.outline[0]);
	save(players.box.outline[1]);
	save(players.box.lenght);

	save(players.name.enable);
	save(players.name.bot_check);

	save(players.health_bar.enable);
	save(players.health_bar.baseonhealth);
	save(players.health_bar.gradient);
	save(players.health_bar.background);
	save(movement.indicators.stamina.enable);
	save(movement.indicators.stamina.disable_takeoff_on_ground);
	save(movement.indicators.stamina.takeoff);
	save(movement.indicators.stamina.maximum_value);
	save(movement.indicators.stamina.style);

	save(movement.indicators.stamina.color[0]);
	save(movement.indicators.stamina.color[1]);
	save(movement.indicators.stamina.color[2]);
	save(movement.indicators.stamina.color[3]);

	save(movement.indicators.stamina.hsb_color.saturation);

	save(movement.indicators.stamina.interpolate_color.first[0]);
	save(movement.indicators.stamina.interpolate_color.first[1]);
	save(movement.indicators.stamina.interpolate_color.first[2]);
	save(movement.indicators.stamina.interpolate_color.first[3]);

	save(movement.indicators.stamina.interpolate_color.second[0]);
	save(movement.indicators.stamina.interpolate_color.second[1]);
	save(movement.indicators.stamina.interpolate_color.second[2]);
	save(movement.indicators.stamina.interpolate_color.second[3]);
	save(type);
	save(showcslk);
	save(showonscreen);
	save(positionsind.x);
	save(positionsind.y);
	save(sunset);
	save(sunset_x);
	save(sunset_y);
	save(motionblur.enable);
	save(motionblur.video_adapter);
	save(motionblur.forward_move_blur);
	save(motionblur.falling_minimum);
	save(motionblur.falling_maximum);
	save(motionblur.falling_intensity);
	save(motionblur.rotate_intensity);
	save(motionblur.strength);
	save(watermark);
	save(trackdispay);
	save(FireMan);
	save(FireManKey);
	save(FireManType);
	save(airstuck);
	save(airstuckkey);
	save(EdgeBugAdvanceSearch);
	save(EdgeBugDetectChat);
	save(EdgeBugMouseLock);
	save(EdgeBugTicks);
	save(deltaScaler);
	save(SiletEdgeBug);
	save(AutoStrafeEdgeBug);
	save(MegaEdgeBug);
	save(killss);
	// ... and so on for the rest of the struct members
}

void Options::Initialize()
{
	CHAR my_documents[MAX_PATH];
	SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, my_documents);
	folder = std::string("C:\\lobotomy\\");
	CreateDirectoryA(folder.c_str(), nullptr);
	SetupWeapons();
	SetupVisuals();
	SetupMisc();
}

void Options::SaveSettings(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	for (auto value : ints)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : bools)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());

	for (auto value : floats)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
}

void Options::LoadSettings(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	if (!std::filesystem::exists(file))
		return;

	char value_l[32] = { '\0' };

	for (auto value : ints)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = atoi(value_l);
	}

	for (auto value : bools)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = !strcmp(value_l, "true");
	}

	for (auto value : floats)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = float(atof(value_l));
	}
	g_ClientState->ForceFullUpdate();
}

void Options::DeleteSettings(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	if (!std::filesystem::exists(file))
		return;

	remove(file.c_str());
}