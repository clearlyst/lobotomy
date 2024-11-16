#include "aimbot.hpp"
#include "autowall.hpp"
#include "bt.h"
#include "../helpers/input.hpp"


QAngle calculateRelativeAngle(const Vector& source, const Vector& destination, const QAngle& viewAngles) noexcept
{
	Vector delta = destination - source;
	QAngle angles{ RAD2DEG(atan2f(-delta.z, std::hypotf(delta.x, delta.y))) - viewAngles.pitch,
				   RAD2DEG(atan2f(delta.y, delta.x)) - viewAngles.yaw ,
					0.f };
	angles.normalize();
	return angles;
}
bool is_pistol(C_BaseCombatWeapon* weapon) {
	if (!weapon)
		return false;

	
	int id = weapon->m_Item().m_iItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_GLOCK, WEAPON_ELITE, WEAPON_P250, WEAPON_TEC9, WEAPON_CZ75A, WEAPON_USP_SILENCER, WEAPON_HKP2000, WEAPON_FIVESEVEN };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool is_heavy_pistol(C_BaseCombatWeapon* weapon) {
	if (!weapon)
		return false;

	int id = weapon->m_Item().m_iItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_DEAGLE, WEAPON_REVOLVER };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool is_shotgun(C_BaseCombatWeapon* weapon) {
	if (!weapon)
		return false;

	int id = weapon->m_Item().m_iItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_NOVA, WEAPON_XM1014, WEAPON_SAWEDOFF, WEAPON_MAG7 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool is_heavy(C_BaseCombatWeapon* weapon) {
	if (!weapon)
		return false;

	int id = weapon->m_Item().m_iItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_M249, WEAPON_NEGEV };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool is_smg(C_BaseCombatWeapon* weapon) {
	if (!weapon)
		return false;

	int id = weapon->m_Item().m_iItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_MAC10, WEAPON_MP7, WEAPON_UMP45, WEAPON_P90, WEAPON_BIZON, WEAPON_MP9 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool is_rifle(C_BaseCombatWeapon* weapon) {
	if (!weapon)
		return false;

	int id = weapon->m_Item().m_iItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_AK47,WEAPON_AUG,WEAPON_FAMAS,WEAPON_GALILAR,WEAPON_M4A1,WEAPON_M4A1_SILENCER, WEAPON_SG556 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool is_sniper(C_BaseCombatWeapon* weapon) {
	if (!weapon)
		return false;


	int id = weapon->m_Item().m_iItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_AWP, WEAPON_SSG08 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}

bool is_auto_sniper(C_BaseCombatWeapon* weapon) {
	if (!weapon)
		return false;

	int id = weapon->m_Item().m_iItemDefinitionIndex();
	static const std::vector<int> v = { WEAPON_G3SG1, WEAPON_SCAR20 };
	return (std::find(v.begin(), v.end(), id) != v.end());
}
void MegaPrivateAimbotOtAntohi::run(CUserCmd* cmd)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;
	auto pukolki = g_LocalPlayer->m_hActiveWeapon();
	if (!pukolki || pukolki->IsGrenade() || pukolki->IsKnife())
		return;
	static int type{0};
	if (is_pistol(pukolki))
		type = 0;
	if (is_heavy_pistol(pukolki))
		type = 1;
	if (is_shotgun(pukolki))
		type = 2;
	if (is_heavy(pukolki))
		type = 3;
	if (is_smg(pukolki))
		type = 4;
	if (is_rifle(pukolki))
		type = 5;
	if (is_sniper(pukolki))
		type = 6;
	if (is_auto_sniper(pukolki))
		type = 7;
	
	settings = g_Options.weapons[type].legit;
	if (!settings.enabled)
		return;
	if (!(cmd->buttons & IN_ATTACK))
		return;
	if (!pukolki->CanFire())
		return;
	if (pukolki->IsMiscWeapon())
		return;
	//float ebatshasvpenu = 0.f;
	float typofovchik = settings.fov;
	Vector NuKudastrealyat = Vector(0, 0, 0);
	Vector NashiOkiEpta = g_LocalPlayer->GetEyePos();
	std::vector<int> hitboxes;

	if (settings.hitboxes.head)
		hitboxes.emplace_back(HITBOX_HEAD);

	if (settings.hitboxes.stomach)
		hitboxes.emplace_back(HITBOX_STOMACH);

	if (settings.hitboxes.chest)
	{
		hitboxes.emplace_back(HITBOX_CHEST);
		hitboxes.emplace_back(HITBOX_LOWER_CHEST);
		hitboxes.emplace_back(HITBOX_UPPER_CHEST);
	}


	if (settings.hitboxes.hands)
	{
		hitboxes.emplace_back(HITBOX_LEFT_FOREARM);
		hitboxes.emplace_back(HITBOX_LEFT_HAND);
		hitboxes.emplace_back(HITBOX_LEFT_UPPER_ARM);

		hitboxes.emplace_back(HITBOX_RIGHT_FOREARM);
		hitboxes.emplace_back(HITBOX_RIGHT_HAND);
		hitboxes.emplace_back(HITBOX_RIGHT_UPPER_ARM);
	}

	if (settings.hitboxes.legs)
	{
		hitboxes.emplace_back(HITBOX_LEFT_FOOT);
		hitboxes.emplace_back(HITBOX_LEFT_CALF);
		hitboxes.emplace_back(HITBOX_LEFT_THIGH);

		hitboxes.emplace_back(HITBOX_RIGHT_FOOT);
		hitboxes.emplace_back(HITBOX_LEFT_CALF);
		hitboxes.emplace_back(HITBOX_LEFT_THIGH);
	}

	const auto aimPunch = g_LocalPlayer->m_aimPunchAngle();
	float ebuchiirngexploit = g_CVar->FindVar("weapon_recoil_scale")->GetFloat();
	static Vector vAngle;
	static QAngle vClientViewAngles;
	g_EngineClient->GetViewAngles(&vClientViewAngles);
	angleVectors(vClientViewAngles, vAngle);
	for (auto i = 1; i <= g_GlobalVars->maxClients; i++)
	{
		C_BasePlayer* eblanchik = C_BasePlayer::GetPlayerByIndex(i);
		if (!eblanchik || eblanchik == g_LocalPlayer || eblanchik->IsDormant() || !eblanchik->IsAlive())
			continue;
		if (eblanchik->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
			continue;
		if (!eblanchik->IsAlive())
			continue;
		auto localPlayerEyePosition = g_LocalPlayer->GetEyePos();
		for (const auto AhuetYapernul : hitboxes)
		{

			Vector GdeKostyanchik = eblanchik->GetHitboxPos(AhuetYapernul);
			const auto angle = calculateRelativeAngle(NashiOkiEpta, GdeKostyanchik, cmd->viewangles + aimPunch);
			const auto fovss = std::hypot(angle.yaw, angle.pitch);

			if (fovss > typofovchik)
				continue;

			if (!g_LocalPlayer->CanSeePlayer(eblanchik, GdeKostyanchik))
			{

				//const auto damage = c_autowall::get()->autowall(g_LocalPlayer->GetEyePos(), hitboxPos, g_LocalPlayer, player).damage;
				const auto damage = Autowall::GetDamage(GdeKostyanchik);

				if (damage < settings.autowall.min_damage)
					continue;
			}
			if (pukolki->IsKnife())
				continue;
			QAngle NenuzhnayaZalupaDlyaFov = calculateRelativeAngle(NashiOkiEpta, GdeKostyanchik, cmd->viewangles + aimPunch);
			float avotetouzhenuzhno = GetFov(NashiOkiEpta, GdeKostyanchik, vAngle);
			if (fovss < typofovchik)
			{
				// KOSTYAN DETECTED
				typofovchik = fovss;
				NuKudastrealyat = GdeKostyanchik;
			}
			if(Backtrack::records[i].size() > 2)
			if (g_Options.aimatbt)
			{
				auto records = &Backtrack::records[i];
				if (!records || records->empty() || !Backtrack::valid(records->front().simulationTime) || records->size() <= 3 || !g_Options.backtrack)
					continue;
				int bestRecord{ };

				for (size_t p = 0; p < Backtrack::records[i].size()-2; p++) {
					auto& record = Backtrack::records[i][p];
					if (!Backtrack::valid(record.simulationTime))
						continue;

					const float fov = std::hypot(angle.yaw, angle.pitch);

					if (fov < typofovchik) {
						typofovchik = fov;
						bestRecord = p;
					}
				}
				auto currentRecord = Backtrack::records[i][bestRecord];


				if (!g_Options.aim_at_bt)
					continue;

				if (settings.hitboxes.legs)
				{
					if (!g_LocalPlayer->CanSeePlayer(eblanchik, currentRecord.Pelvis))
					{


						float ebatshasvpenu = Autowall::GetDamage(currentRecord.Pelvis);
						if (ebatshasvpenu < settings.autowall.min_damage)
							continue;
					}
					NuKudastrealyat = currentRecord.Pelvis;
				}


				if (settings.hitboxes.chest)
				{
					if (!g_LocalPlayer->CanSeePlayer(eblanchik, currentRecord.LChest))
					{

						float ebatshasvpenu = Autowall::GetDamage(currentRecord.LChest);
						if (ebatshasvpenu < settings.autowall.min_damage)
							continue;
					}
					NuKudastrealyat = currentRecord.LChest;
				}
				if (settings.hitboxes.chest)
				{
					if (!g_LocalPlayer->CanSeePlayer(eblanchik, currentRecord.Chest))
					{


						float ebatshasvpenu = Autowall::GetDamage(currentRecord.Chest);
						if (ebatshasvpenu < settings.autowall.min_damage)
							continue;
					}
					NuKudastrealyat = currentRecord.Chest;
				}
				if (settings.hitboxes.head)
				{
					if (!g_LocalPlayer->CanSeePlayer(eblanchik, currentRecord.Neck))
					{


						float ebatshasvpenu = Autowall::GetDamage(currentRecord.Neck);
						if (ebatshasvpenu < settings.autowall.min_damage)
							continue;
					}
					NuKudastrealyat = currentRecord.Neck;
				}
				if (settings.hitboxes.head)
				{
					if (!g_LocalPlayer->CanSeePlayer(eblanchik, currentRecord.head))
					{


						float ebatshasvpenu = Autowall::GetDamage(currentRecord.head);
						if (ebatshasvpenu < settings.autowall.min_damage)
							continue;
					}
					NuKudastrealyat = currentRecord.head;
				}
				if (settings.hitboxes.head)
				{
					if (!g_LocalPlayer->CanSeePlayer(eblanchik, currentRecord.head))
					{


						float ebatshasvpenu = Autowall::GetDamage(currentRecord.head);
						if (ebatshasvpenu > settings.autowall.min_damage)
							continue;
					}
					NuKudastrealyat = currentRecord.head;
				}
			}
		}
	}
	//auto bestFov = 180;


	if (NuKudastrealyat.wtf())
		return;



	QAngle NeNushasBuduebat = Math::Vpenivatel(NashiOkiEpta, NuKudastrealyat, cmd->viewangles).normalize();



	auto ZV = float(settings.rcs.x) / 50.f;
	if (settings.rcs.enabled)
	{
		cmd->viewangles += (NeNushasBuduebat.clamped() - g_LocalPlayer->m_aimPunchAngle() * ZV) / settings.smooth;
	}
	else
	{
		cmd->viewangles += (NeNushasBuduebat.clamped()) / settings.smooth;
	}

	if (!settings.smoke_check)
	{
		g_EngineClient->SetViewAngles(&cmd->viewangles);
	}

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	auto weapon_data = weapon->GetCSWeaponData();



}

MegaPrivateAimbotOtAntohi g_Aimbot;