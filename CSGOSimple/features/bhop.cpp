#include "bhop.hpp"
#include "bhop.hpp"
#include "../options.hpp"

#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include <filesystem>
#include "../../CSGOSimple/helpers/prediction.hpp"
#include "chat_hud.h"
#include "../imgui/imgui.h"


float flZVelBackup;

float flBugSpeed;

Vector wheredafeetat[4];
bool greenfoot[4];
int iEdgebugButtons;

bool jumpBugAGFix = false;
Vector backupvelocity;
static bool crouched = false;

int savedbuttons;
PlayerBackup PrePredBackup{};
Vector backuporigin;
static Vector ebpos = { 0,0,0 };
void BunnyHop::PastedMovementCorrection(CUserCmd* cmd, QAngle old_angles, QAngle wish_angle)
{// pasted from only god knows where
	if (old_angles.pitch != wish_angle.pitch || old_angles.yaw != wish_angle.yaw || old_angles.roll != wish_angle.roll)
	{
		Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

		auto viewangles = old_angles;
		auto movedata = Vector(cmd->forwardmove, cmd->sidemove, cmd->upmove);
		viewangles.Normalize();

		if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && viewangles.roll != 0.f)
			movedata.y = 0.f;

		Math::AngleVectors(wish_angle, &wish_forward, &wish_right, &wish_up);
		Math::AngleVectors(viewangles, &cmd_forward, &cmd_right, &cmd_up);

		auto v8 = sqrt(wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y), v10 = sqrt(wish_right.x * wish_right.x + wish_right.y * wish_right.y), v12 = sqrt(wish_up.z * wish_up.z);

		Vector wish_forward_norm(1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f),
			wish_right_norm(1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f),
			wish_up_norm(0.f, 0.f, 1.0f / v12 * wish_up.z);

		auto v14 = sqrt(cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y), v16 = sqrt(cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y), v18 = sqrt(cmd_up.z * cmd_up.z);

		Vector cmd_forward_norm(1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f),
			cmd_right_norm(1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f),
			cmd_up_norm(0.f, 0.f, 1.0f / v18 * cmd_up.z);

		auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

		Vector correct_movement;
		correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 + (cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28) + (cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27);
		correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 + (cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28) + (cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27);
		correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 + (cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28) + (cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27);

		static ConVar* cl_forwardspeed = g_CVar->FindVar(("cl_forwardspeed"));

		if (cl_forwardspeed == nullptr)
			return;

		static ConVar* cl_sidespeed = g_CVar->FindVar(("cl_sidespeed"));

		if (cl_sidespeed == nullptr)
			return;

		static ConVar* cl_upspeed = g_CVar->FindVar(("cl_upspeed"));

		if (cl_upspeed == nullptr)
			return;
		// get max speed limits by convars
		const float flMaxForwardSpeed = cl_forwardspeed->GetFloat();
		const float flMaxSideSpeed = cl_sidespeed->GetFloat();
		const float flMaxUpSpeed = cl_upspeed->GetFloat();

		correct_movement.x = std::clamp(correct_movement.x, -flMaxForwardSpeed, flMaxForwardSpeed);
		correct_movement.y = std::clamp(correct_movement.y, -flMaxSideSpeed, flMaxSideSpeed);
		correct_movement.z = std::clamp(correct_movement.z, -flMaxUpSpeed, flMaxUpSpeed);

		cmd->forwardmove = correct_movement.x;
		cmd->sidemove = correct_movement.y;
		cmd->upmove = correct_movement.z;

		cmd->buttons &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);
		if (cmd->sidemove != 0.0) {
			if (cmd->sidemove <= 0.0)
				cmd->buttons |= IN_MOVELEFT;
			else
				cmd->buttons |= IN_MOVERIGHT;
		}

		if (cmd->forwardmove != 0.0) {
			if (cmd->forwardmove <= 0.0)
				cmd->buttons |= IN_BACK;
			else
				cmd->buttons |= IN_FORWARD;
		}
	}
}
void BunnyHop::OnCreateMove(CUserCmd* cmd)
{
  static bool jumped_last_tick = false;
  static bool should_fake_jump = false;
  if (!g_Options.misc_bhop)
	  return;

  if ((g_Options.jump_bug && GetAsyncKeyState(g_Options.jump_bug_key)) || AlertJB)
	  return;
  if (!g_LocalPlayer)
	  return;

  static bool wasLastTimeOnGround=( g_LocalPlayer->m_fFlags() & 1 );

  if (!(g_LocalPlayer->m_fFlags() & 1) && g_LocalPlayer->m_nMoveType() != MOVETYPE_LADDER && !wasLastTimeOnGround)
	  cmd->buttons &= ~IN_JUMP;

  wasLastTimeOnGround = (g_LocalPlayer->m_fFlags() & 1);
}
static auto prediction = new PredictionSystem();


bool bApplyStrafe;
static Vector vecMoveLastStrafe;
static QAngle angViewLastStrafe;
static QAngle angViewDeltaStrafe;
 bool eblolkek;
void BunnyHop::egbag(CUserCmd* pCmd, QAngle& angView)
{// this method of doing it is kinda autistic and I know the code is a bit of clusterfuck but whatever
	
	if (!GetAsyncKeyState(g_Options.edge_bug_key) )
		return;
	auto pLocal = g_LocalPlayer;

	if (iFlagsBackup & FL_ONGROUND)
		return; // imagine trying to edgebug while we on the ground lol (protip: u cunt)

	bShouldEdgebug = backupvelocity.z < -flBugSpeed && round(pLocal->m_vecVelocity().z) == -round(flBugSpeed) && pLocal->m_nMoveType() != MOVETYPE_LADDER;
	if (bShouldEdgebug) // we literally boutta bug on da edge lol
		return;

	int nCommandsPredicted = g_Prediction->Split->nCommandsPredicted;

	// backup original stuff that we change so we can restore later if no edgebug detek
	QAngle angViewOriginal = angView;
	QAngle angCmdViewOriginal = pCmd->viewangles;
	int iButtonsOriginal = pCmd->buttons;
	Vector vecMoveOriginal;
	vecMoveOriginal.x = pCmd->sidemove;
	vecMoveOriginal.y = pCmd->forwardmove;

	// static static static static static
	static Vector vecMoveLastStrafe;
	static QAngle angViewLastStrafe;
	static QAngle angViewOld = angView;
	static QAngle angViewDeltaStrafe;
	static bool bAppliedStrafeLast = false;
	if (!bAppliedStrafeLast)
	{// we didn't strafe last time so it's safe to update these, if we did strafe we don't want to change them ..
		angViewLastStrafe = angView;
		vecMoveLastStrafe = vecMoveOriginal;
	
			angViewDeltaStrafe = QAngle(angView.pitch - angViewOld.pitch, std::clamp(pCmd->viewangles.yaw - angViewOld.yaw, -(180.f / 128.f), 180.f / 128.f), 0.f);
		angViewDeltaStrafe;
	}
	bAppliedStrafeLast = false;
	angViewOld = angView;
	
	for (int t = 0; t < 4; t++)
	{
		static int iLastType;
		if (iLastType)
		{
			t = iLastType;
			iLastType = 0;
		}
		memesclass->restoreEntityToPredictedFrame(0, nCommandsPredicted - 1); // reset player to before engine prediction was ran (-1 because this whole function is only called after pred in cmove)
		if (iButtonsOriginal& IN_DUCK&& t < 2) // if we already unducking then don't unduck pusi
			t = 2;
		bool bApplyStrafe = !(t % 2); // t == 0 || t == 2
		bool bApplyDuck = t > 1;

		// set base cmd values that we need
		pCmd->viewangles = angViewLastStrafe;
		pCmd->buttons = iButtonsOriginal;
		pCmd->sidemove = vecMoveLastStrafe.x;
		pCmd->forwardmove = vecMoveLastStrafe.y;

		for (int i = 0; i < 64; i++)
		{
			// apply cmd changes for prediction
			if (bApplyDuck)
				pCmd->buttons |= IN_DUCK;
			else
				pCmd->buttons &= ~IN_DUCK;
			if (bApplyStrafe)
			{
				pCmd->viewangles += angViewDeltaStrafe;
				pCmd->viewangles.normalize();
				pCmd->viewangles.Clamp();
			}
			else
			{
				pCmd->sidemove = 0.f;
				pCmd->forwardmove = 0.f;
			}
			Vector PrePredVelocity = g_LocalPlayer->m_vecVelocity();
			// run prediction

			prediction->StartPrediction(pCmd);
			prediction->EndPrediction();
			float RaznVelocity = PrePredVelocity.z - g_LocalPlayer->m_vecVelocity().z;
			Vector VeloDelta = (g_LocalPlayer->m_vecVelocity().toAngle() - PrePredVelocity.toAngle()).normalize();
			bShouldEdgebug = floor(g_LocalPlayer->m_vecVelocity().z) > floor(PrePredVelocity.z) && PrePredVelocity.z * 0.25f > RaznVelocity && VeloDelta.y < 45.f && !(pLocal->m_fFlags() & 1) && backupvelocity.z < -round(flBugSpeed) && pLocal->m_vecVelocity().z < 0.f && hypotf(PrePredVelocity.x, PrePredVelocity.y) < hypotf(g_LocalPlayer->m_vecVelocity().x, g_LocalPlayer->m_vecVelocity().y);
			backupvelocity.z = pLocal->m_vecVelocity().z;
			if (!bShouldEdgebug)
				bShouldEdgebug = backupvelocity.z < -flBugSpeed && round(pLocal->m_vecVelocity().z) == -round(flBugSpeed) && pLocal->m_nMoveType() != MOVETYPE_LADDER;
			if (bShouldEdgebug)
			{
				
				iEdgebugButtons = pCmd->buttons; // backup iButtons state
				if (bApplyStrafe)
				{// if we hit da bug wit da strafe we gotta make sure we strafe right
					bAppliedStrafeLast = true;
					angView = (angViewLastStrafe + angViewDeltaStrafe);
					angView.normalize();
					angView.Clamp();
					angViewLastStrafe = angView;
					pCmd->sidemove = vecMoveLastStrafe.x;
					pCmd->forwardmove = vecMoveLastStrafe.y;
				}
				/* restore angViewPoint back to what it was, we only modified it for prediction purposes
				*  we use movefix instead of changing angViewPoint directly
				*  so we have pSilent pEdgebug (perfect-silent; prediction-edgebug) ((lol))
				*/
				pCmd->viewangles = angCmdViewOriginal;
				iLastType = t;
				return;
			}

			if (pLocal->m_fFlags() & FL_ONGROUND || pLocal->m_nMoveType() == MOVETYPE_LADDER)
				break;
		}
	}

	/* if we got this far in the function then we won't hit an edgebug in any of the predicted scenarios
	*  so we gotta restore everything back to what it was originally
	*/
	pCmd->viewangles = angCmdViewOriginal;
	angView = angViewOriginal;
	pCmd->buttons = iButtonsOriginal;
	pCmd->sidemove = vecMoveOriginal.x;
	pCmd->forwardmove = vecMoveOriginal.y;
}


inline bool edgebug_test;
QAngle backupforfreelook;
float backupbtforfreelook[2];
int JBBACKUPBUTTONS;
int bIMoveType;
void BunnyHop::PrePred(CUserCmd* cmd)
{
	if (should_pixelsurf && !GetAsyncKeyState(g_Options.edge_bug_key))
		cmd->buttons |= IN_DUCK;

	g_LocalPlayer->collect(PrePredBackup);
	iFlagsBackup = g_LocalPlayer->m_fFlags();
	// gotta backup dat z vel
	flZVelBackup = g_LocalPlayer->m_vecVelocity().z;
	auto sv_gravity = g_CVar->FindVar(("sv_gravity"));
	flBugSpeed = (sv_gravity->GetFloat() * 0.5f * g_GlobalVars->interval_per_tick);
	backuporigin = g_LocalPlayer->m_vecOrigin();
	backupvelocity = g_LocalPlayer->m_vecVelocity();
	bIMoveType = g_LocalPlayer->m_nMoveType();
	if (GetAsyncKeyState(g_Options.edge_bug_key) )
	{
		if (bShouldEdgebug) // if we predicted an edgebug while crouching last tick then crouch again (mainly to reduce weird crouchspam shit)
			cmd->buttons = iEdgebugButtons;

	}
	if (iFlagsBackup & 1)
		should_pixelsurf = false;
	static float flZVelPrev = flZVelBackup;
	bShouldEdgebug = flZVelPrev < -flBugSpeed && round(g_LocalPlayer->m_vecVelocity().z) == -round(flBugSpeed) && g_LocalPlayer->m_nMoveType() != MOVETYPE_LADDER;
	JBBACKUPBUTTONS = cmd->buttons;
	flZVelPrev = flZVelBackup;
}
 
Vector durackeh{};
Vector armiya;
size_t faketicks;
size_t faketicks1;
Vector pointvec = Vector();
bool check(float a, float b)
{
	int a1 = (int)a;
	int b1 = (int)b;
	if (b < 0)
	{
		if (a1 == b1)
		{
			int a2 = (a - a1) * 100;
			int b2 = (b - b1) * 100;
			int a3 = (a - a1) * 1000;
			if (b2 == a2 || a2 + 1 == b2 || a2 + 2 == b2)
				return true;
			else
				return false;
		}
	}
	else
	{
		if (a1 == b1)
		{
			int a2 = (a - a1) * 100;
			int b2 = (b - b1) * 100;
			int a3 = (a - a1) * 1000;
			if (b2 == a2 || a2 == b2 - 1 || a2 == b2 - 2)
				return true;
			else
				return false;
		}
	}

	return false;
}
#include "../render.hpp"
#include <deque>
#include "Notify.h"





void BunnyHop::sw(CUserCmd* cmd)
{

	if (g_Options.edgejump.enabled && GetAsyncKeyState(g_Options.edgejump.hotkey))
	{
		if (g_Options.edgejump.edge_jump_duck_in_air)
		{
			bool do_lj;
			static float btime;
			if (btime < g_GlobalVars->curtime)
				should_longjump = false;
			if (!g_LocalPlayer)
				btime = 0.f;
			static int saved_tick_count;


			if (!g_LocalPlayer)
				return;

			const auto move_type = g_LocalPlayer->m_nMoveType();

			if (move_type == MOVETYPE_LADDER || move_type == MOVETYPE_NOCLIP) {
				return;
			}

			if (g_LocalPlayer->m_fFlags() & 1) {
				saved_tick_count = g_GlobalVars->tickcount;
			}

			if (g_GlobalVars->tickcount - saved_tick_count > 2) {
				do_lj = false;
			}
			else {
				do_lj = true;
			}

			if (do_lj && !(g_LocalPlayer->m_fFlags() & 1)) {
				cmd->buttons |= IN_DUCK;
				btime = g_GlobalVars->curtime + .2f;
				should_longjump = true;

			}
		}
	}
}
void set_clantag(const char* name) {
	using Fn = int(__fastcall*)(const char*, const char*);
	static auto apply_clan_tag_fn = reinterpret_cast<Fn>(Utils::PatternScan("engine.dll", "53 56 57 8B DA 8B F9 FF 15"));
	apply_clan_tag_fn(name, name);
}

void BunnyHop::clantag()
{
	static bool restore = false;

	if (g_Options.clantag && g_LocalPlayer) {
		static float LastChangeTime = 0.f;
		if (g_GlobalVars->realtime - LastChangeTime >= 0.5f) {
			LastChangeTime = g_GlobalVars->realtime;
			static std::string text = "lobotomy        ";

			LastChangeTime = g_GlobalVars->realtime;

			std::string temp = text;
			text.erase(0, 1);
			text += temp[0];

			Utils::SetClantag(text.data());
			restore = true;
		}
	}
	else if (restore) {
		restore = false;
		Utils::SetClantag("");

	}
}


void BunnyHop::Minijump(CUserCmd* cmd) {
	static float btime;
	if (btime < g_GlobalVars->curtime)
		should_minijump = false;
	if (!g_LocalPlayer)
		btime = 0.f;

	if (!g_Options.misc_minijump || !GetAsyncKeyState(g_Options.abobakey))
		return;

	if (!g_LocalPlayer)
		return;

	if (const auto mt = g_LocalPlayer->m_nMoveType(); mt == MOVETYPE_LADDER || mt == MOVETYPE_NOCLIP)
		return;

	static int longjump_tick = 0;
	static bool minijumpbool = false;

	if ((iFlagsBackup & FL_ONGROUND) && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
	{
		cmd->buttons |= IN_JUMP;
		minijumpbool = true;
		btime = g_GlobalVars->curtime + .2f;
		should_minijump = true;
		longjump_tick = cmd->tick_count + 1;
		cmd->buttons |= IN_DUCK;
	}


}
float Convert(float degree)
{
	return (degree * (M_PI / 180));
}
float Convert2(float radians)
{
	return radians * (180 / M_PI);
}
void BunnyHop::ps_fix(CUserCmd* cmd)
{
	if (!g_LocalPlayer)
		return;
	if (!g_LocalPlayer->IsAlive())
		return;
	if (flZVelBackup > 0.f)
		return;
	if (!g_Options.ps_fix)
		return;

	if (backupvelocity.Length2D() >= 285.91f)
	{

		if (g_LocalPlayer->m_fFlags() & 1)
		{

			auto sv_airAcelerate = g_CVar->FindVar("sv_airaccelerate");
			float Razn = ((backupvelocity.Length2D() + 2.f - 285.91f) / 12.f * 64.f);
			Vector velocity = backupvelocity * -1.f;
			velocity.z = 0.f;
			float rotation = Convert(velocity.toAngle().y - cmd->viewangles.yaw);
			float cos_rot = cos(rotation);
			float sin_rot = sin(rotation);

			float forwardmove = cos_rot * Razn;
			float sidemove = -sin_rot * Razn;
			cmd->forwardmove = forwardmove;
			cmd->sidemove = sidemove;
		}
	}
}


void MovementFix(CUserCmd* m_Cmd, QAngle wish_angle, QAngle old_angles) {
	if (old_angles.pitch != wish_angle.pitch || old_angles.yaw != wish_angle.yaw || old_angles.roll != wish_angle.roll) {
		Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

		auto viewangles = old_angles;
		auto movedata = Vector(m_Cmd->forwardmove, m_Cmd->sidemove, m_Cmd->upmove);
		viewangles.Normalize();

		if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && viewangles.roll != 0.f)
			movedata.y = 0.f;

		Math::AngleVectors(wish_angle, wish_forward, wish_right, wish_up);
		Math::AngleVectors(viewangles, cmd_forward, cmd_right, cmd_up);

		auto v8 = sqrt(wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y), v10 = sqrt(wish_right.x * wish_right.x + wish_right.y * wish_right.y), v12 = sqrt(wish_up.z * wish_up.z);

		Vector wish_forward_norm(1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f),
			wish_right_norm(1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f),
			wish_up_norm(0.f, 0.f, 1.0f / v12 * wish_up.z);

		auto v14 = sqrt(cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y), v16 = sqrt(cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y), v18 = sqrt(cmd_up.z * cmd_up.z);

		Vector cmd_forward_norm(1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f),
			cmd_right_norm(1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f),
			cmd_up_norm(0.f, 0.f, 1.0f / v18 * cmd_up.z);

		auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

		Vector correct_movement;
		correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 + (cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28) + (cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27);
		correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 + (cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28) + (cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27);
		correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 + (cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28) + (cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27);

		correct_movement.x = std::clamp(correct_movement.x, -450.f, 450.f);
		correct_movement.y = std::clamp(correct_movement.y, -450.f, 450.f);
		correct_movement.z = std::clamp(correct_movement.z, -320.f, 320.f);

		m_Cmd->forwardmove = correct_movement.x;
		m_Cmd->sidemove = correct_movement.y;
		m_Cmd->upmove = correct_movement.z;

		m_Cmd->buttons &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);
		if (m_Cmd->sidemove != 0.0) {
			if (m_Cmd->sidemove <= 0.0)
				m_Cmd->buttons |= IN_MOVELEFT;
			else
				m_Cmd->buttons |= IN_MOVERIGHT;
		}

		if (m_Cmd->forwardmove != 0.0) {
			if (m_Cmd->forwardmove <= 0.0)
				m_Cmd->buttons |= IN_BACK;
			else
				m_Cmd->buttons |= IN_FORWARD;
		}
	}
}
bool flvl;

inline bool should_ps_crouch;
inline bool should_ps_standing;
bool hb{};
bool Awall = false;
int GlobalTick = 0;
void BunnyHop::autoalign(CUserCmd* cmd)
{

	if (!g_LocalPlayer)
		return;

	if (iFlagsBackup & 1)
	{
		jumpBugAGFix = false;
		return;
	}
	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
		return;
	if (g_LocalPlayer->m_vecVelocity().z > 0)
		jumpBugAGFix = false;
	if (jumpBugAGFix)
		return;
	if (GetAsyncKeyState(g_Options.FireManKey) && GlobalTick == 0)
		return;
	auto colidable = g_LocalPlayer->GetCollideable();
	if (!colidable)
		return;

	CGameTrace trace;
	float step = (float)M_PI * 2.0f / 16.f;
	Awall = false;
	static float StartCircle = 0.f;
	if (g_LocalPlayer->m_vecVelocity().z != -6.25f && flZVelBackup != -6.25f)
		StartCircle = 0.f;
	Vector WallPosition{};
	for (float a = StartCircle; a < (M_PI * 2.0f); a += step)
	{
		Vector wishdir = Vector(cosf(a), sinf(a), 0.f);
		const auto startPos = g_LocalPlayer->abs_origin();
		const auto endPos = startPos + wishdir;


		CTraceFilterWorldOnly flt;

		Ray_t ray;
		ray.Init(startPos, endPos, colidable->OBBMins(), colidable->OBBMaxs());
		g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &trace);
		if ((trace.fraction < 1.f) && (trace.plane.normal.z == 0.f))
		{
			WallPosition = trace.endpos;
			StartCircle = a;
			Awall = true;

			break;

		}
	}




	if (Awall && !GetAsyncKeyState(g_Options.airstuckkey))
	{

		int nCommandsPredicted = g_Prediction->Split->nCommandsPredicted;
		Vector NormalPlane = Vector(trace.plane.normal.x * -1.f, trace.plane.normal.y * -1.f, 0.f);
		Vector WallAngle = NormalPlane.toAngle();
		if (g_LocalPlayer->m_vecVelocity().Length2D() > 280 && !(cmd->buttons & IN_DUCK))
		{
			if (g_LocalPlayer->m_vecVelocity().z == -6.25f || flZVelBackup == -6.25f)
			{
				float mVel = hypotf(g_LocalPlayer->m_vecVelocity().x, g_LocalPlayer->m_vecVelocity().y);
				float ideal = RAD2DEG(atanf(25.f / mVel));
				Vector dvelo = g_LocalPlayer->m_vecVelocity();
				dvelo.z = 0.f;
				Vector velo_angle = dvelo.toAngle();
				Vector delta = velo_angle - WallAngle;
				delta.normalize();
				if (delta.y >= 0.f)
					WallAngle.y += ideal;
				else
					WallAngle.y -= ideal;
			}
		}
		float rotation = Convert(WallAngle.y - cmd->viewangles.yaw);
		float cos_rot = cos(rotation);
		float sin_rot = sin(rotation);
		float multiplayer = 0.f;
		if (g_LocalPlayer->m_vecVelocity().z == -6.25f || flZVelBackup == -6.25f)
			multiplayer = 45.f;
		else
			multiplayer = 6.f;
		float forwardmove = cos_rot * multiplayer;
		float sidemove = -sin_rot * multiplayer;
		CGameTrace SecTrace;
		Ray_t ray;
		CTraceFilterWorldOnly flt;
		Vector StartPos = Vector(g_LocalPlayer->abs_origin().x, g_LocalPlayer->abs_origin().y, WallPosition.z);
		Vector endPos = StartPos + NormalPlane * 64.f;
		ray.Init(g_LocalPlayer->abs_origin(), endPos);

		g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &SecTrace);
		if (SecTrace.fraction > 0.249715533f || !(cmd->buttons & IN_FORWARD) && !(cmd->buttons & IN_BACK) && !(cmd->buttons & IN_MOVELEFT) && !(cmd->buttons & IN_MOVERIGHT))
		{
			cmd->forwardmove = forwardmove;
			cmd->sidemove = sidemove;
		}
		if (g_LocalPlayer->m_vecVelocity().Length2D() > 280 && !(cmd->buttons & IN_DUCK))
		{
			if (g_LocalPlayer->m_vecVelocity().z == -6.25f || flZVelBackup == -6.25f)
			{
				if (cmd->forwardmove < 0.f && cmd->buttons & IN_FORWARD)
					cmd->forwardmove = 450.f;
				if (cmd->forwardmove > 0.f && cmd->buttons & IN_BACK)
					cmd->forwardmove = -450.f;
				if (cmd->sidemove < 0.f && cmd->buttons & IN_MOVERIGHT)
					cmd->sidemove = 450.f;
				if (cmd->sidemove > 0.f && cmd->buttons & IN_MOVELEFT)
					cmd->sidemove = -450.f;
				return;
			}
		}
		if (multiplayer == 45.f && (cmd->buttons & IN_FORWARD) || (cmd->buttons & IN_BACK) || (cmd->buttons & IN_MOVELEFT) || (cmd->buttons & IN_MOVERIGHT))
		{

			int buttons = cmd->buttons;
			float forwardmove = cmd->forwardmove;
			float sidemove = cmd->sidemove;

			for (int i = 0; i < 450; i += 45)
			{
				memesclass->restoreEntityToPredictedFrame(0, nCommandsPredicted - 1);
				if (buttons & IN_FORWARD)
					cmd->forwardmove = i;
				if (buttons & IN_BACK)
					cmd->forwardmove = -i;
				if (buttons & IN_MOVELEFT)
					cmd->sidemove = -i;
				if (buttons & IN_MOVERIGHT)
					cmd->sidemove = i;

				prediction->StartPrediction(cmd);
				prediction->EndPrediction();
				float zvelo = g_LocalPlayer->m_vecVelocity().z;
				if (zvelo == -flBugSpeed)
				{
					forwardmove = cmd->forwardmove;
					sidemove = cmd->sidemove;



				}


			}
			cmd->forwardmove = forwardmove;
			cmd->sidemove = sidemove;
		}


		if (cmd->forwardmove < 0.f && cmd->buttons & IN_FORWARD)
			cmd->forwardmove = 450.f;
		if (cmd->forwardmove > 0.f && cmd->buttons & IN_BACK)
			cmd->forwardmove = -450.f;
		if (cmd->sidemove < 0.f && cmd->buttons & IN_MOVERIGHT)
			cmd->sidemove = 450.f;
		if (cmd->sidemove > 0.f && cmd->buttons & IN_MOVELEFT)
			cmd->sidemove = -450.f;



	}
	static bool hit = false;
	if (Awall && GetAsyncKeyState(g_Options.airstuckkey) && g_Options.airstuck)
	{
		int nCommandsPredicted = g_Prediction->Split->nCommandsPredicted;
		Vector NormalPlane = Vector(trace.plane.normal.x * -1.f, trace.plane.normal.y * -1.f, 0.f);
		Vector WallAngle = NormalPlane.toAngle();

		float BackupForwardMove = cmd->forwardmove;
		float BackupSideMove = cmd->sidemove;
		static float FoundedForwardMove{};
		static float FoundedSideMove{};
		static QAngle FoundedViewAngle{};
		if (hit)
		{
			cmd->viewangles = FoundedViewAngle;
			cmd->forwardmove = FoundedForwardMove;
			cmd->sidemove = FoundedSideMove;

		}
		if (g_LocalPlayer->m_vecVelocity().z < -40.f || g_LocalPlayer->m_vecVelocity().z > 0.f)
			hit = false;
		if (!hit)
		{



			float mVel = hypotf(g_LocalPlayer->m_vecVelocity().x, g_LocalPlayer->m_vecVelocity().y);
			float ideal = RAD2DEG(atanf(-1.f / mVel));
			Vector dvelo = g_LocalPlayer->m_vecVelocity();
			dvelo.z = 0.f;
			Vector velo_angle = dvelo.toAngle();
			Vector delta = velo_angle - WallAngle;
			delta.normalize();
			if (delta.y >= 0.f)
				WallAngle.y += ideal;
			else
				WallAngle.y -= ideal;
			float rotation = Convert(WallAngle.y - cmd->viewangles.yaw);
			float cos_rot = cos(rotation);
			float sin_rot = sin(rotation);
			float multiplayer = 12.f;

			float forwardmove = cos_rot * multiplayer;
			float sidemove = -sin_rot * multiplayer;






			for (float j = -180.f; j < 180.f; j += 1.f)
			{
				memesclass->restoreEntityToPredictedFrame(0, nCommandsPredicted - 1);

				cmd->forwardmove = forwardmove;
				cmd->sidemove = sidemove;
				cmd->viewangles.yaw = j;
				bool Stable = false;
				Vector OldVelo = g_LocalPlayer->m_vecVelocity();
				prediction->StartPrediction(cmd);
				prediction->StartPrediction(cmd);
				if (g_LocalPlayer->m_vecVelocity().z > OldVelo.z )
				{

					hit = true;
					FoundedForwardMove = cmd->forwardmove;
					FoundedSideMove = cmd->sidemove;
					FoundedViewAngle = cmd->viewangles;
					break;
				}

					

			}
		}
		
		







		if (!hit)
		{
			cmd->forwardmove = BackupForwardMove;
			cmd->sidemove = BackupSideMove;
		}
		if (hit)
		{
			cmd->viewangles = FoundedViewAngle;
			cmd->forwardmove = FoundedForwardMove;
			cmd->sidemove = FoundedSideMove;
			return;
		}


	}
}
int saved_buttons;
int predictedtickamount = -1;
void BunnyHop::pixel_surf(CUserCmd* cmd) {
	if (g_Options.pixelsurf) {
		
		static int ticks = 0;
		if (!g_LocalPlayer)
			return;
		if (!g_LocalPlayer->IsAlive())
		{
			ticks = 0;
			return;
		}
		if (g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER)
			return;
		if (g_LocalPlayer->m_fFlags() & 1)
			return;
		if (GetAsyncKeyState(g_Options.edge_bug_key))
			return;
		if (!Awall)
			return;


		if (!should_pixelsurf)
		{
			int nCommandsPredicted = g_Prediction->Split->nCommandsPredicted;

			int BackupButtons = cmd->buttons;
			for (int i = 0; i < 2; i++)
			{
				memesclass->restoreEntityToPredictedFrame(0, nCommandsPredicted - 1);
				if (i == 0)
					cmd->buttons &= ~IN_DUCK;
				else
					cmd->buttons |= IN_DUCK;
				for (int z = 0; z < 8; z++)
				{

					prediction->StartPrediction(cmd);
					prediction->EndPrediction();
					if (g_LocalPlayer->m_fFlags() & 1)
					{
						break;
					}
					float zVelo = g_LocalPlayer->m_vecVelocity().z;
					should_pixelsurf = flZVelBackup < 10.f && zVelo == -6.25f;
					if (should_pixelsurf && i == 0)
					{
						should_pixelsurf = false;
						cmd->buttons = BackupButtons;
						return;
					}
					if (should_pixelsurf)
					{
						ticks = cmd->tick_count + z + 16;
						BackupButtons = cmd->buttons;
						break;
					}
				}
			}
			cmd->buttons = BackupButtons;
			memesclass->restoreEntityToPredictedFrame(0, nCommandsPredicted);
		}
		else
		{
			cmd->buttons |= IN_DUCK;
			if (cmd->tick_count > ticks)
			{

				if (flZVelBackup != -6.25f)
				{
					should_pixelsurf = false;
				}
			}
		}



	}
}





template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}
bool ducking = false;
int tick, eb_counter = 0;
float last_set = 0;
Vector eb_pos;

bool edgebugged;

void BunnyHop::edgebug_notify(float unpred_z, int unpred_flags) {
	float zvelo = floor(g_LocalPlayer->m_vecVelocity().z);
	bool should_edgebug2 = true;
	edgebugs = 0;
	if (unpred_flags & 1)
		return;
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
	{
		should_edgebug2 = false;
		return;
	}
	if (!g_LocalPlayer->IsAlive() || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
	{
		should_edgebug2 = false;
		return;
	}
	if (!g_Options.ebinfo)
		return;

	if (!GetAsyncKeyState(g_Options.edge_bug_key) && !g_Options.edge_bug)
		return;

	if (edgebugs < 0)
		edgebugs = 0;

	int nCommandsPredicted = g_Prediction->Split->nCommandsPredicted;


	if (g_Options.edge_bug && GetAsyncKeyState(g_Options.edge_bug_key))
	{
		if (unpred_z < -flBugSpeed && round(unpred_z) == -round(flBugSpeed) && g_LocalPlayer->m_nMoveType() != MOVETYPE_LADDER)
		{


				edgebugged = true;
				edgebugs++;

				auto g_ChatElement = FindHudElement<c_hudchat>("CHudChat");

				g_ChatElement->chatprintf(0, 0, std::string("").
					append(" \x08").
					append("lobotomy |  ").
					append(" \x01").
					append("eb check").c_str());

				
			}
		else if (floor(g_LocalPlayer->m_vecVelocity().z) > floor(unpred_z) && !(g_LocalPlayer->m_fFlags() & 1) && unpred_z < 0.f && g_LocalPlayer->m_vecVelocity().z < 0.f) {
			edgebugged = true;
			edgebugs++;

			auto g_ChatElement = FindHudElement<c_hudchat>("CHudChat");

			g_ChatElement->chatprintf(0, 0, std::string("").
				append(" \x08").
				append("lobotomy |  ").
				append(" \x01").
				append("eb check").c_str());

		}

		else {
			edgebugged = false;
		}
	}
}


float BunnyHop::lerpa(float a, float b, float t)
{
	return a + t * (b - a);
}

void BunnyHop::hitbox_invo(IGameEvent* hibox)
{
	if (!g_Options.hitbones)
		return;
	auto user_id = g_EngineClient->GetPlayerForUserID(hibox->GetInt("userid"));
	auto attacker_id = g_EngineClient->GetPlayerForUserID(hibox->GetInt("attacker"));
	auto local_id = g_EngineClient->GetLocalPlayer(); hibox->GetInt("attacker") == local_id && hibox->GetInt("userid") != local_id;

	int iHealth = hibox->GetInt("health");
	int dmgHealth = hibox->GetInt("dmg_health");
	int iHitGroup = hibox->GetInt("hitgroup");


	const auto entity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(user_id));

	if (!entity)
		return;

	if (!entity->EntIndex())
		return;

	matrix3x4_t bone_matrix[MAXSTUDIOBONES];

	if (!entity->SetupBones(bone_matrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, g_GlobalVars->curtime))
		return;

	studiohdr_t* studio_hdr = g_MdlInfo->GetStudiomodel(entity->GetModel());

	if (!studio_hdr)
		return;

	const auto hitbox_set = studio_hdr->GetHitboxSet(entity->m_nHitboxSet());

	if (!hitbox_set)
		return;

	for (int i = 0; i < hitbox_set->numhitboxes; i++)
	{
		mstudiobbox_t* hitbox = hitbox_set->GetHitbox(i);

		if (!hitbox)
			continue;

		Vector min_actual, max_actual;

		Math::vector_transform(hitbox->bbmin, bone_matrix[hitbox->bone], min_actual);
		Math::vector_transform(hitbox->bbmax, bone_matrix[hitbox->bone], max_actual);

		g_DebugOverlay->add_capsule_overlay(min_actual, max_actual, hitbox->m_flRadius, Color(g_Options.hit_bones_col.r(), g_Options.hit_bones_col.g(), g_Options.hit_bones_col.b(), g_Options.hit_bones_col.a()), 2.5f);
	}
}
std::deque<std::pair<Vector, Vector>> checkpoints;

int currentCheckpoint{ 0 };
float pressDelay{ 0.f };
void BunnyHop::checkPoint(CUserCmd* cmd)
{
	if (!g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_Options.checkpoint)
	{
		checkpoints.clear();
		currentCheckpoint = 0;
		pressDelay = 0;
		return;
	}

	if (g_LocalPlayer->IsAlive() && pressDelay < g_GlobalVars->realtime)
	{
		auto g_ChatElement = FindHudElement<c_hudchat>("CHudChat");
		if (!g_ChatElement)
			return;
		if (GetAsyncKeyState(g_Options.checkpoint_key))
		{
			
			checkpoints.push_front({ Vector(cmd->viewangles.pitch, cmd->viewangles.yaw, 0.f), g_LocalPlayer->m_vecOrigin() });
			currentCheckpoint = 0;
			if (checkpoints.size() > 99)
				checkpoints.pop_back();
			pressDelay = g_GlobalVars->realtime + 0.2f;

		
			notification::run(" ", "#_print_created_checkpoint", true, true, true);
		}

		if (!checkpoints.empty())
		{
			if (GetAsyncKeyState(g_Options.teleport_key))
			{
				
				auto& cur = checkpoints.at(currentCheckpoint);
				std::string cmd = "setpos " + std::to_string(cur.second.x) + " " + std::to_string(cur.second.y) + " " + std::to_string(cur.second.z) + ";";
				g_EngineClient->execute_cmd(cmd.c_str());
				cmd = "setang " + std::to_string(cur.first.x) + " " + std::to_string(cur.first.y) + " " + std::to_string(cur.first.z) + ";";
				g_EngineClient->execute_cmd(cmd.c_str());
				pressDelay = g_GlobalVars->realtime + 0.2f;

				notification::run(" ", "#_print_teleport_checkpoint", true, true, true);
			}

			if (GetAsyncKeyState(g_Options.teleport_nextkey))
			{
				
				if (currentCheckpoint > 0)
					currentCheckpoint--;

				auto& cur = checkpoints.at(currentCheckpoint);
				std::string cmd = "setpos " + std::to_string(cur.second.x) + " " + std::to_string(cur.second.y) + " " + std::to_string(cur.second.z) + ";";
				g_EngineClient->execute_cmd(cmd.c_str());
				cmd = "setang " + std::to_string(cur.first.x) + " " + std::to_string(cur.first.y) + " " + std::to_string(cur.first.z) + ";";
				g_EngineClient->execute_cmd(cmd.c_str());

				pressDelay = g_GlobalVars->realtime + 0.2f;

				
				notification::run(" ", "#_print_teleport_checkpoint", true, true, true);
			}

			if (GetAsyncKeyState(g_Options.teleport_prevkey))
			{
				
				if (currentCheckpoint + 1 < checkpoints.size())
					currentCheckpoint++;

				auto& cur = checkpoints.at(currentCheckpoint);
				std::string cmd = "setpos " + std::to_string(cur.second.x) + " " + std::to_string(cur.second.y) + " " + std::to_string(cur.second.z) + ";";
				g_EngineClient->execute_cmd(cmd.c_str());
				cmd = "setang " + std::to_string(cur.first.x) + " " + std::to_string(cur.first.y) + " " + std::to_string(cur.first.z) + ";";
				g_EngineClient->execute_cmd(cmd.c_str());

				pressDelay = g_GlobalVars->realtime + 0.2f;

			
				notification::run(" ", "#_print_teleport_checkpoint", true, true, true);
			}

			if (GetAsyncKeyState(g_Options.teleport_undokey))
			{
				
				checkpoints.erase(checkpoints.begin() + currentCheckpoint);
				if (currentCheckpoint >= checkpoints.size())
				{
					#undef max
					currentCheckpoint = std::max(0, static_cast<int>(checkpoints.size()) - 1);
				}
				pressDelay = g_GlobalVars->realtime + 0.2f;

				
			}
		}
	}
}


bool InCrosshair(int x, int y, int w, int h)
{
	int ScreenHeight, ScreenWidth;
	g_EngineClient->GetScreenSize(ScreenHeight, ScreenWidth);
	ScreenHeight /= 2;
	ScreenWidth /= 2;
	return (ScreenHeight > x && w + x > ScreenHeight && ScreenWidth > y && h + y > ScreenWidth);
}
struct windowdetect
{

	int alpha;
	std::string info;
	float time = -999.f;

	windowdetect(const int& Alpha = {}, const std::string& Info = "")
		: alpha(Alpha), info(Info) {}
};
std::vector<windowdetect> WindowDetect;
bool seccheckcros = false;
float backupmyaw{};
void BunnyHop::CHECKSURFLOLKEK(CUserCmd* cmd)
{

	seccheckcros = false;
	static bool HitJump{ false };
	static bool HitMiniJump{ false };
	static bool HitLongJump{ false };
	static bool HitCHop{ false };
	static bool HitJumpBug{ false };
	if (!g_Options.CHECKSURFLOLKEK)
		return;

	if (!g_LocalPlayer)
		return;
	if (!g_EngineClient->IsInGame())
		return;
	if (!g_LocalPlayer->IsAlive())
		return;

	if (GetAsyncKeyState(g_Options.KEYCHECKSURFLOLKEKPoint) & 0x01 && !seccheckcros)
	{
		Vector viewangle = Vector(cmd->viewangles.pitch, cmd->viewangles.yaw, 0.f);
		const auto endPos = g_LocalPlayer->GetEyePos() + Vector::fromAngle(viewangle) * 2000;
		CGameTrace trace;

		Ray_t ray;
		ray.Init(g_LocalPlayer->GetEyePos(), endPos);
		CTraceFilterWorldOnly flt;

		g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &trace);
		if (trace.fraction == 1.f)
		{
			g_EngineClient->ExecuteClientCmd("play error");


			return;
		}


		PointsCheck.emplace_back(g_LocalPlayer->GetEyePos() + (endPos - g_LocalPlayer->GetEyePos()) * trace.fraction, std::string(g_EngineClient->GetLevelNameShort()));	//NormalPosVec = trace.plane.normal;



	}
	
	static int toggletime = 0;
	if (g_Options.type)
	{
		if (GetAsyncKeyState(g_Options.KEYCHECKSURFLOLKEK) && toggletime < GetTickCount64())
		{
			toggletime = GetTickCount64() + 500;
			g_Options.togglechecksurflolkek = !g_Options.togglechecksurflolkek;
		}
	}
	if (!GetAsyncKeyState(g_Options.KEYCHECKSURFLOLKEK) && !g_Options.togglechecksurflolkek)
	{
		HITGODA = false;
		HitJump = false;
		HitMiniJump = false;
		HitLongJump = false;
		HitCHop = false;
		HitJumpBug = false;
		return;
	}

	if (PointsCheck.size() > 0)
	{
		int index = 0;

		{
			float Nearest = 99999.f;
			for (int i = 0; i < PointsCheck.size(); i++)
			{
				if(g_EngineClient->GetLevelNameShort() == PointsCheck.at(i).Map)
				if (Nearest > g_LocalPlayer->m_vecOrigin().DistTo(PointsCheck.at(i).Pos))
				{
					Nearest = g_LocalPlayer->m_vecOrigin().DistTo(PointsCheck.at(i).Pos);
					index = i;
				}

			}
		}
		Vector Surf = PointsCheck.at(index).Pos;

		Vector Calculate{};
		float iCalcilate{};

		int BackupButtons = cmd->buttons;
		float ForwaMove = cmd->forwardmove;
		float SideMove = cmd->sidemove;

		Vector BackupVelo = g_LocalPlayer->m_vecVelocity();
		Vector Origin = g_LocalPlayer->m_vecOrigin();
		float stamina = g_LocalPlayer->fl_stamina();
		int BackupPredicted = g_Prediction->Split->nCommandsPredicted;
		static int timer= 0;
		static int ticks = 0;
		static int ljticks = 0;
		if (flZVelBackup < 0.f)
			ljticks = 0;
		int g = (iFlagsBackup & 1) ? 3 : 5;
		int hitgroudtick{};
		Vector ChatPositions{};
		bool ChatStand{};
		bool ChatDuck{};
		for (int v = 0; v < g; v++)
		{
			cmd->sidemove = 0.f;
			cmd->forwardmove = 0.f;
			cmd->buttons = BackupButtons;
			bool jb = false;
			bool secjb = false;
			bool thrjb = false;
			bool forthjb = false;
			memesclass->restoreEntityToPredictedFrame(0, BackupPredicted - 1);
			g_LocalPlayer->apply(PrePredBackup);
			int backflags = g_LocalPlayer->m_fFlags();
			Vector OldLocalPlaerOrigin = g_LocalPlayer->m_vecOrigin();
			Vector OldLocalPlayerVelocity = g_LocalPlayer->m_vecVelocity();
			float stamina2 = g_LocalPlayer->fl_stamina();
			int BackupPredicted2 = 0;
			PlayerBackup backup{};
			int once = 0;
			if (!HitJump && !HitMiniJump && !HitLongJump && !HitCHop)
			{
				for (int i = 0; i < 48; i++)
				{
					//g_LocalPlayer->m_vecVelocity() = Vector(0.f, 0.f, g_LocalPlayer->m_vecVelocity().z);
					if (g_LocalPlayer->m_fFlags() & 1)
					{
						once += 1;
						
					}
					if(once == 1)
						ChatPositions = g_LocalPlayer->m_vecOrigin();
					if (v == 0)
					{
						if (g_LocalPlayer->m_fFlags() & 1)
							cmd->buttons |= IN_JUMP;
					}
					if (v == 1)
					{
						if (g_LocalPlayer->m_fFlags() & 1)
						{
							cmd->buttons |= IN_JUMP;
							cmd->buttons |= IN_DUCK;
						}
					}
					if (v == 2)
					{
						if (g_LocalPlayer->m_fFlags() & 1)
						{
							cmd->buttons |= IN_JUMP;
							cmd->buttons |= IN_DUCK;
						}
					}
					if (!(g_LocalPlayer->m_fFlags() & 1) && v != 3 && v != 4)
					{
						cmd->buttons &= ~IN_DUCK;
						cmd->buttons &= ~IN_JUMP;
					}
					if (v == 3)
					{
						if (g_LocalPlayer->m_fFlags() & 1)
						{
							cmd->buttons |= IN_JUMP;
						}
						cmd->buttons |= IN_DUCK;
					}
					if (v == 4)
					{
						
						
						if (i == hitgroudtick)
						{
							cmd->buttons |= IN_DUCK;
							cmd->buttons &= ~IN_JUMP;
							once += 1;
						}
						else
						{
							cmd->buttons &= ~IN_DUCK;
							cmd->buttons |= IN_JUMP;

						}
						
					}
					
					backflags = g_LocalPlayer->m_fFlags();
					OldLocalPlaerOrigin = g_LocalPlayer->m_vecOrigin();
					OldLocalPlayerVelocity = g_LocalPlayer->m_vecVelocity();
					g_LocalPlayer->collect(backup);
					Vector BackupVelo2 = g_LocalPlayer->m_vecVelocity();
					Vector Origin2 = g_LocalPlayer->m_vecOrigin();
					stamina2 = g_LocalPlayer->fl_stamina();
					

					prediction->StartPrediction(cmd);
					prediction->EndPrediction();
					BackupPredicted = g_Prediction->Split->nCommandsPredicted;
					if (v == 2)
					{
						if (g_LocalPlayer->m_fFlags() & 1 && !(backflags & 1))
						{
							if(!jb)
							hitgroudtick = i;
							jb = true;
						}
					}
					
					if (!(g_LocalPlayer->m_fFlags() & 1) && v != 3 && v != 4)
					{
						cmd->buttons &= ~IN_DUCK;
						cmd->buttons &= ~IN_JUMP;
					}
					if (backflags & 1 && !(g_LocalPlayer->m_fFlags() & 1) && v == 2)
					{
						g_LocalPlayer->m_vecOrigin() = Vector(g_LocalPlayer->m_vecOrigin().x, g_LocalPlayer->m_vecOrigin().y, g_LocalPlayer->m_vecOrigin().z + 8.9999704f);
					}
					Vector PredictedLocalPlayerOrigin = g_LocalPlayer->m_vecOrigin();
					Vector PredictedLocalPlayerVelocity = g_LocalPlayer->m_vecVelocity();
					iCalcilate = 0.f;
					if (OldLocalPlayerVelocity.z > 0.f && PredictedLocalPlayerVelocity.z < 0.f && !iCalcilate && once == 1)
					{
						Calculate = OldLocalPlaerOrigin;
						iCalcilate = OldLocalPlaerOrigin.z - PredictedLocalPlayerOrigin.z;
						once += 1;
					}

					if (iCalcilate )
					{
						float z = 0;
						while (Calculate.z > Surf.z - 20.f)
						{
							if (check(Calculate.z, Surf.z))
							{
								if (v == 0)
									HitJump = true;
								if (v == 1)
								{
									HitMiniJump = true;

								}
								if (v == 2)
								{
									HitLongJump = true;
								}
								if (v == 3)
								{
									HitCHop = true;
								}
								if (v == 4)
									HitJumpBug = true;
								ChatStand = true;
								ticks = cmd->tick_count + i + 2;
							}
							if (check(Calculate.z + 9.f, Surf.z))
							{
								if (v == 0)
									HitJump = true;
								if (v == 1)
								{
									HitMiniJump = true;

								}
								if (v == 2)
								{
									HitLongJump = true;
								}
								if (v == 4)
									HitJumpBug = true;
								ChatDuck = true;
								ticks = cmd->tick_count + i+2;
							
							}
							if (v == 3)
							if (check(Calculate.z - 9.f, Surf.z))
							{
								ChatStand = true;
								{
									HitCHop = true;
								}
								ticks = cmd->tick_count + i + 2;
							}

							float o = z * (z + 1) / 2;
							float p = iCalcilate + (0.1953125 * z);
							Calculate.z = Calculate.z - p;
							z += 1;
						}
					}


				}
			}

		}
		cmd->buttons = BackupButtons;
		cmd->forwardmove = ForwaMove;
		cmd->sidemove = SideMove;
		memesclass->restoreEntityToPredictedFrame(0, BackupPredicted - 1);
		static int TicksOut = 0;
		if (flZVelBackup > -19.f)
		{
			ticks = 0;
			HITGODA = false;

		}
		if (cmd->tick_count < ticks)
		{
			HITGODA = true;
			
			cmd->forwardmove = 0.f;
			cmd->sidemove = 0.f;
			cmd->buttons = 0;
			cmd->mousedx = 0;
			cmd->mousedy = 0;
			
			
		}
		else HITGODA = false;
		if (flZVelBackup > 0.f)
		{
			if (HitJumpBug)
			{
				if (!ChatPositions.is_zero())
				{
					if (TicksOut < GetTickCount64())
					{
						TicksOut = GetTickCount64() + 500;
						notification::run(" ", "#_print_psaisit", true, true, true);
						WindowDetect.emplace_back(5, std::string("jumpbug to ").append(std::to_string(Surf.z)).append(" with ").append(std::to_string(backuporigin.z)).append(ChatDuck ? " (crouch)" : " (stand)"));
					}
				}
			}
			HitCHop = false;
			HitJumpBug = false;
		}
		cmd->buttons = BackupButtons;
		if (HitCHop)
		{
			cmd->buttons |= IN_DUCK;
			if (iFlagsBackup & 1)
				cmd->buttons |= IN_JUMP;
			if (!ChatPositions.is_zero())
			{
				if (TicksOut < GetTickCount64())
				{
					TicksOut = GetTickCount64() + 500;
					notification::run(" ", "#_print_psaisit", true, true, true);
					WindowDetect.emplace_back(5, std::string("crouch hop to ").append(std::to_string(Surf.z)).append(" with ").append(std::to_string(backuporigin.z)).append(ChatDuck ? " (crouch)" : " (stand)"));

				}
			}
		}
		if (HitJump && iFlagsBackup & 1)
		{
			cmd->buttons |= IN_JUMP;
			HitJump = false;
			if (!ChatPositions.is_zero())
			{
				if (TicksOut < GetTickCount64())
				{
					TicksOut = GetTickCount64() + 500;
					notification::run(" ", "#_print_psaisit", true, true, true);
					WindowDetect.emplace_back(5, std::string("jump to ").append(std::to_string(Surf.z)).append(" with ").append(std::to_string(backuporigin.z)).append(ChatDuck ? " (crouch)" : " (stand)"));

				}
			}
		}
		if (HitMiniJump && iFlagsBackup & 1)
		{
			cmd->buttons |= IN_JUMP;
			cmd->buttons |= IN_DUCK;
			HitMiniJump = false;
			if (!ChatPositions.is_zero())
			{
				if (TicksOut < GetTickCount64())
				{
					TicksOut = GetTickCount64() + 500;
					notification::run(" ", "#_print_psaisit", true, true, true);
					WindowDetect.emplace_back(5, std::string("minijump to ").append(std::to_string(Surf.z)).append(" with ").append(std::to_string(backuporigin.z)).append(ChatDuck ? " (crouch)" : " (stand)"));

				}
			}
		}
		if (HitLongJump && iFlagsBackup & 1)
		{
			cmd->buttons |= IN_JUMP;
			cmd->buttons |= IN_DUCK;
			ljticks = g_GlobalVars->tickcount + 3;
			HitLongJump = false;
			if (!ChatPositions.is_zero())
			{
				if (TicksOut < GetTickCount64())
				{
					TicksOut = GetTickCount64() + 500;
					notification::run(" ", "#_print_psaisit", true, true, true);
					WindowDetect.emplace_back(5, std::string("longjump to ").append(std::to_string(Surf.z)).append(" with ").append(std::to_string(backuporigin.z)).append(ChatDuck ? " (crouch)" : " (stand)"));

				}
			}
		}
		if (ljticks > g_GlobalVars->tickcount)
			cmd->buttons |= IN_DUCK;
		AlertJB = HitJumpBug;
		
	}
}
bool isLadder = false;
bool isBackspacePressed = false;
void BunnyHop::FireMan(CUserCmd* cmd)
{
	isLadder = false;
	if (g_Options.FireManType == 0)
	{
		if (!g_LocalPlayer)
			return;
		if (!g_EngineClient->IsInGame())
			return;
		if (!g_LocalPlayer->IsAlive())
			return;
		if (!g_Options.FireMan || !GetAsyncKeyState(g_Options.FireManKey))
			return;
		if (bIMoveType == MOVETYPE_LADDER)
		{
			cmd->buttons |= IN_JUMP;
			cmd->forwardmove = 0.f;
			cmd->sidemove = 0.f;
		}


		auto colidable = g_LocalPlayer->GetCollideable();
		if (!colidable)
			return;
		memesclass->restoreEntityToPredictedFrame(0, g_Prediction->Split->nCommandsPredicted - 1);
		CGameTrace trace;
		float step = (float)M_PI * 2.0f / 16.f;
		Awall = false;
		static float StartCircle = 0.f;
		if (g_LocalPlayer->m_vecVelocity().z != -6.25f && flZVelBackup != -6.25f)
			StartCircle = 0.f;
		Vector WallPosition{};
		for (float a = StartCircle; a < (M_PI * 2.0f); a += step)
		{
			Vector wishdir = Vector(cosf(a), sinf(a), 0.f);
			const auto startPos = g_LocalPlayer->abs_origin();
			const auto endPos = startPos + wishdir;


			CTraceFilterWorldAndPropsOnly flt;

			Ray_t ray;
			ray.Init(startPos, endPos, colidable->OBBMins(), colidable->OBBMaxs());
			g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &trace);
			if ((trace.fraction < 1.f) && (trace.plane.normal.z == 0.f))
			{

				WallPosition = trace.endpos;
				StartCircle = a;
				Awall = true;

				break;

			}
		}




		if (Awall)
		{

			int nCommandsPredicted = g_Prediction->Split->nCommandsPredicted;
			Vector NormalPlane = Vector(trace.plane.normal.x * -1.f, trace.plane.normal.y * -1.f, 0.f);
			Vector WallAngle = NormalPlane.toAngle();


			Vector dvelo = g_LocalPlayer->m_vecVelocity();
			dvelo.z = 0.f;
			Vector velo_angle = dvelo.toAngle();
			Vector delta = velo_angle - WallAngle;
			delta.normalize();


			float rotation = Convert(WallAngle.y - cmd->viewangles.yaw);
			float cos_rot = cos(rotation);
			float sin_rot = sin(rotation);
			float multiplayer = 450.f;
			float bakcupFormardMove = cmd->forwardmove;
			float backupSideMpve = cmd->sidemove;
			float forwardmove = cos_rot * multiplayer;
			float sidemove = -sin_rot * multiplayer;


			int backupbuttons = cmd->buttons;
			memesclass->restoreEntityToPredictedFrame(0, g_Prediction->Split->nCommandsPredicted - 1);
			cmd->forwardmove = forwardmove;
			cmd->sidemove = sidemove;
			cmd->buttons &= ~IN_JUMP;
			prediction->StartPrediction(cmd);
			if (!isLadder)
			{
				if (bIMoveType != MOVETYPE_LADDER && g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || bIMoveType == MOVETYPE_LADDER)
				{
					isLadder = true;
				}
				else
				{
					cmd->buttons = backupbuttons;
					cmd->forwardmove = bakcupFormardMove;
					cmd->sidemove = backupSideMpve;


				}
			}

			if (isLadder)
			{

				memesclass->restoreEntityToPredictedFrame(0, g_Prediction->Split->nCommandsPredicted - 1);
				int oldflags = g_LocalPlayer->m_fFlags();
				int oldMoveType = g_LocalPlayer->m_nMoveType();
				float v1 = cmd->forwardmove;
				float v2 = cmd->sidemove;
				cmd->forwardmove = 0.f;
				cmd->sidemove = 0.f;

				prediction->StartPrediction(cmd);
				if (!(oldflags & 1) && g_LocalPlayer->m_fFlags() & 1)
				{

					cmd->forwardmove = forwardmove;
					cmd->sidemove = sidemove;
					cmd->buttons &= ~IN_JUMP;


				}
				else
				{
					cmd->buttons |= IN_JUMP;
					cmd->forwardmove = 0.f;
					cmd->sidemove = 0.f;
				}
			}
		}
	}
	if (g_Options.FireManType == 1)
	{
		if (!g_LocalPlayer)
			return;
		if (!g_EngineClient->IsInGame())
			return;
		if (!g_LocalPlayer->IsAlive())
			return;
		
		static bool FrHit = false;
		if (!g_Options.FireMan || !GetAsyncKeyState(g_Options.FireManKey))
		{
			FrHit = false;
			return;
		}




		auto colidable = g_LocalPlayer->GetCollideable();
		if (!colidable)
			return;
		memesclass->restoreEntityToPredictedFrame(0, g_Prediction->Split->nCommandsPredicted - 1);
		CGameTrace trace;
		float step = (float)M_PI * 2.0f / 16.f;
		Awall = false;
		static float StartCircle = 0.f;
		if (g_LocalPlayer->m_vecVelocity().z != -6.25f && flZVelBackup != -6.25f)
			StartCircle = 0.f;
		Vector WallPosition{};
		for (float a = StartCircle; a < (M_PI * 2.0f); a += step)
		{
			Vector wishdir = Vector(cosf(a), sinf(a), 0.f);
			const auto startPos = g_LocalPlayer->abs_origin();
			const auto endPos = startPos + wishdir;


			CTraceFilterWorldAndPropsOnly flt;

			Ray_t ray;
			ray.Init(startPos, endPos, colidable->OBBMins(), colidable->OBBMaxs());
			g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &trace);
			if ((trace.fraction < 1.f) && (trace.plane.normal.z == 0.f))
			{

				WallPosition = trace.endpos;
				StartCircle = a;
				Awall = true;

				break;

			}
		}




		if (Awall)
		{

			int nCommandsPredicted = g_Prediction->Split->nCommandsPredicted;
			Vector NormalPlane = Vector(trace.plane.normal.x * -1.f, trace.plane.normal.y * -1.f, 0.f);
			Vector WallAngle = NormalPlane.toAngle();


			Vector dvelo = g_LocalPlayer->m_vecVelocity();
			dvelo.z = 0.f;
			Vector velo_angle = dvelo.toAngle();
			Vector delta = velo_angle - WallAngle;
			delta.normalize();


			float rotation = Convert(WallAngle.y - cmd->viewangles.yaw);
			float cos_rot = cos(rotation);
			float sin_rot = sin(rotation);
			float multiplayer = 450.f;
			float bakcupFormardMove = cmd->forwardmove;
			float backupSideMpve = cmd->sidemove;
			float forwardmove = cos_rot * multiplayer;
			float sidemove = -sin_rot * multiplayer;


			int backupbuttons = cmd->buttons;
			memesclass->restoreEntityToPredictedFrame(0, g_Prediction->Split->nCommandsPredicted - 1);
			cmd->forwardmove = forwardmove;
			cmd->sidemove = sidemove;
			cmd->buttons &= ~IN_JUMP;
			prediction->StartPrediction(cmd);
			if (!isLadder)
			{
				if (bIMoveType != MOVETYPE_LADDER && g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || bIMoveType == MOVETYPE_LADDER)
				{
					isLadder = true;
				}
				else
				{
					cmd->buttons = backupbuttons;
					cmd->forwardmove = bakcupFormardMove;
					cmd->sidemove = backupSideMpve;


				}
			}

			if (isLadder)
			{

				memesclass->restoreEntityToPredictedFrame(0, g_Prediction->Split->nCommandsPredicted - 1);
				int oldflags = g_LocalPlayer->m_fFlags();
				int oldMoveType = g_LocalPlayer->m_nMoveType();

				float v1 = cmd->forwardmove;
				float v2 = cmd->sidemove;
				cmd->forwardmove = 0.f;
				cmd->sidemove = 0.f;

				for (int i = 0; i < 12; i++)
				{
					prediction->StartPrediction(cmd);
					if (!(oldflags & 1) && g_LocalPlayer->m_fFlags() & 1)
					{

						FrHit = true;



					}
				}
				if (!FrHit)
				{
					cmd->buttons &= ~IN_JUMP;
					cmd->forwardmove = 0.f;
					cmd->sidemove = 0.f;
				}
				else
				{
					if (bIMoveType != MOVETYPE_LADDER)
					{
						cmd->forwardmove = forwardmove;
						cmd->sidemove = sidemove;
						cmd->buttons |= IN_JUMP;
					}
					else
					{
						cmd->forwardmove = 0.f;
						cmd->sidemove = 0.f;
						cmd->buttons |= IN_JUMP;
					}

				}
			}
			else
				FrHit = false;
		}
	}
}

void BunnyHop::renderCheckSurf()
{
	if (!g_Options.CHECKSURFLOLKEK)
		return;
	if (!g_LocalPlayer)
		return;
	if (!g_EngineClient->IsInGame())
		return;
	if (!g_LocalPlayer->IsAlive())
		return;

	static bool isBackspacePressed = false;
	int scx, scy;
	g_EngineClient->GetScreenSize(scx, scy);
	// Обработка нажатия клавиши Backspace
	if (GetAsyncKeyState(VK_BACK) & 0x8000) {
		isBackspacePressed = true;
	}
	else {
		isBackspacePressed = false;
	}

	if (PointsCheck.size() > 0) {
		if (g_Options.showonscreen)
		{
			if (WindowDetect.size() > 0)
			{
				// Iterate using an index-based loop, but handle erasing carefully
				for (int i = 0; i < WindowDetect.size(); ) {
					auto& inf = WindowDetect.at(i);
					float linesize = inf.time > 0.5f ? 1.f : inf.alpha/255; // Assuming inf.alpha is 0-255
					if (inf.time == -999.f)
						inf.time = 1.f;
					// Get the size of the text
					auto TextSize = Render::Get().GetTextSize(inf.info, fonts::sub_indicator_font, 18.f);

					// Render the box, text, and lines
					Render::Get().RenderBoxFilled(scx / 2 - TextSize.x / 2 - 5, scy * 0.8f - 10 - (i * 25), scx / 2 + TextSize.x / 2 + 5, scy * 0.8f + 25 - (i * 25), Color(25, 25, 25, inf.alpha));
					Render::Get().RenderText(inf.info, ImVec2(scx / 2, scy * 0.8 - (i * 25)), 18.f, Color(255, 255, 255, inf.alpha), true, false, fonts::sub_indicator_font, true);
					
					// Decrement alpha value, but make it slower for better visibility
					if (GetTickCount64() % 10 == 0) { // Change this to decrement less frequently
						inf.time -= 0.01f;
					}
					if (inf.time > 0.5f)
					{
						if (GetTickCount64() % 5 == 0) {
							inf.alpha += 5;
						}
					}
					else
					{
						if (GetTickCount64() % 5 == 0) {
							inf.alpha -= 5;
						}
					}
					if (inf.alpha > 255) inf.alpha = 255;
					if (inf.alpha < 1) inf.alpha = 1;
					// Check if alpha value is low enough to remove the element
					if (inf.alpha < 3) {
						WindowDetect.erase(WindowDetect.begin() + i);
					}
					else {
						++i; // Only increment i if no element was erased
					}
				}
			}
		}

		for (int i = 0; i < PointsCheck.size(); i++) {
			Vector render{};
			if(g_EngineClient->GetLevelNameShort() == PointsCheck.at(i).Map)
			if (!g_DebugOverlay->ScreenPosition(PointsCheck.at(i).Pos, render)) {
				 
				// Вычисление расстояния до игрока
				float distance = g_LocalPlayer->GetEyePos().DistTo(PointsCheck.at(i).Pos);

				// Определение размера круга в зависимости от расстояния
				float circleSize = std::clamp(12.0f - (distance / 200.0f), 5.0f, 12.0f);

				// Определение прозрачности в зависимости от расстояния
				int alpha = std::clamp(static_cast<int>(255 - (distance / 2.0f)), 50, 255);

				seccheckcros = InCrosshair(render.x - 6.f, render.y - 6.f, 12.f, 12.f);

				// Отрисовка круга с учетом прозрачности
				Render::Get().RenderCircleFilled(render.x, render.y, seccheckcros ? circleSize * 1.5f : circleSize, 128, Color(50, 50, 50, alpha));
				Render::Get().RenderCircle(render.x, render.y, seccheckcros ? circleSize * 1.5f : circleSize, 128, seccheckcros ? Color(g_Options.fuckingaccent[0] * 100, g_Options.fuckingaccent[1] * 100, g_Options.fuckingaccent[2] * 100, alpha/255.f) : Color(g_Options.fuckingaccent[0] , g_Options.fuckingaccent[1] , g_Options.fuckingaccent[2] , alpha/255.f), 1.f);
				// Отрисовка крестика на весь круг
				Render::Get().RenderLine(render.x - circleSize, render.y, render.x + circleSize-1, render.y, seccheckcros ? Color(g_Options.fuckingaccent[0] * 100, g_Options.fuckingaccent[1] * 100, g_Options.fuckingaccent[2] * 100, alpha/255.f) : Color(g_Options.fuckingaccent[0] , g_Options.fuckingaccent[1] , g_Options.fuckingaccent[2] , alpha/255.f));
				Render::Get().RenderLine(render.x, render.y - circleSize, render.x, render.y + circleSize-1, seccheckcros ? Color(g_Options.fuckingaccent[0] * 100, g_Options.fuckingaccent[1] * 100, g_Options.fuckingaccent[2] * 100, alpha/255.f) : Color(g_Options.fuckingaccent[0] , g_Options.fuckingaccent[1] , g_Options.fuckingaccent[2] , alpha/255.f)); //vrode

				if (seccheckcros && isBackspacePressed) {
					PointsCheck.erase(PointsCheck.begin() + i);
					isBackspacePressed = false; // сбрасываем флаг после удаления
					break; // выходим из цикла, чтобы избежать проблем с индексами
				}
			}
		}
	}
}
float dotProduct(const Vector& v1, const Vector& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float length(const Vector& v) {
	return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float angleBetweenVectors(const Vector& b, const Vector& newvelo) {
	float dot = dotProduct(b, newvelo);
	float len_b = length(b);
	float len_newvelo = length(newvelo);

	float cosTheta = dot / (len_b * len_newvelo);
	float theta = std::acos(cosTheta);

	// Convert to degrees
	float degrees = theta * 180.0f / M_PI;
	return degrees;
}
void BunnyHop::AutoBounce(CUserCmd* cmd)
{
	if (!g_Options.AutoBounce)
		return;

	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
		return;

	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->m_nMoveType() == MOVETYPE_OBSERVER || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || iFlagsBackup & 1)
		return;


	if (iFlagsBackup & FL_ONGROUND || iFlagsBackup & FL_DUCKING)
		return;

	if (should_pixelsurf)
		return;
	if (bShouldEdgebug)
		return;
	if (GetAsyncKeyState(g_Options.edge_bug_key) || GetAsyncKeyState(g_Options.jump_bug_key) || GetAsyncKeyState(g_Options.edgejump.hotkey) || GetAsyncKeyState(g_Options.abobakey))
		return;
	if (GetAsyncKeyState(g_Options.dhkey))
		return;
	float timepredicted = 0.f;
	float savedzpos = -999999.f;
	bool foundground = false;
	CUserCmd savedcmd = *cmd;

	//finding ground on crouch
	memesclass->restoreEntityToPredictedFrame(0, g_Prediction->Split->nCommandsPredicted - 1);
	//engine_prediction->restoreprediction();
	while (timepredicted < 0.05f && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
	{
		cmd->buttons |= IN_DUCK;
		prediction->StartPrediction(cmd);
		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
		{
			foundground = true;
			savedzpos = g_LocalPlayer->m_vecOrigin().z;
			
			break;
		}
		else
		{
			
			timepredicted += g_GlobalVars->interval_per_tick;
		}
	}


	bool foundgstanding = false;
	float savedzposstanding = -999999.f;

	if (foundground && savedzpos != -999999.f)
	{
		*cmd = savedcmd;
		memesclass->restoreEntityToPredictedFrame(0,g_Prediction->Split->nCommandsPredicted - 1);
		//engine_prediction->restoreprediction();

		timepredicted = 0.f;

		while (timepredicted < 0.05f && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		{
			cmd->buttons &= ~IN_DUCK;
			prediction->StartPrediction( cmd);
			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
			{
				foundgstanding = true;
				savedzposstanding = g_LocalPlayer->m_vecOrigin().z;
				
				break;
			}
			else
			{
				
				timepredicted += g_GlobalVars->interval_per_tick;
			}
		}
	}

	if (foundground && foundgstanding)
	{
		if (savedzpos > savedzposstanding)
		{
			cmd->buttons |= IN_DUCK;
		}
	}
	else if (foundground && !foundgstanding)
	{
		cmd->buttons |= IN_DUCK;
	}
	else
	{
		*cmd = savedcmd;
	}


}


void BunnyHop::jump_bug(CUserCmd* cmd, int pre_flags, int pre_velocity)
{
	static bool jumpbug_triggered = false;
	static bool crouched = false;

	if (!AlertJB)
	{
		if (!g_Options.jump_bug || !GetAsyncKeyState(g_Options.jump_bug_key))
		{
			jumpbug_triggered = false;
			crouched = false;
			return;
		}
	}

	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
	{
		jumpbug_triggered = false;
		crouched = false;
		return;
	}

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
	{
		jumpbug_triggered = false;
		crouched = false;
		return;
	}

	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_OBSERVER)
	{
		jumpbug_triggered = false;
		crouched = false;
		return;
	}

	if (!(pre_flags & FL_ONGROUND) && g_LocalPlayer->m_fFlags() & FL_ONGROUND)
	{
		cmd->buttons |= IN_DUCK;
		cmd->buttons &= ~IN_JUMP;
	}

	bool should = (!(pre_flags & FL_ONGROUND) && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && (g_LocalPlayer->m_vecVelocity().z > pre_velocity) && (g_LocalPlayer->m_nMoveType() != MOVETYPE_NOCLIP || g_LocalPlayer->m_nMoveType() != MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() != MOVETYPE_OBSERVER) ? true : false);
	
	if (!should) {
		if (!jumpbug_triggered) {
			if (!(pre_flags & FL_ONGROUND) && g_LocalPlayer->m_fFlags() & FL_ONGROUND) {
				if (cmd->buttons & IN_DUCK) {
					if (g_LocalPlayer->m_fFlags() & FL_DUCKING) {
						cmd->buttons &= ~IN_DUCK;
						crouched = true;
					}
					else {
						cmd->buttons |= IN_DUCK;
					}
				}

				cmd->buttons &= ~IN_JUMP;

				jumpbug_triggered = true;
			}
		}
		else {
			if (crouched) {
				cmd->buttons &= ~IN_DUCK;
				crouched = false;
			}
			else {
				cmd->buttons |= IN_DUCK;
				jumpbug_triggered = false;
			}
		}

		should_jumpbug = false;
	}
	else {
		should_jumpbug = true;
	}
}

int bktick;
int bktick1;
int bktick2;
size_t tbr;
void BunnyHop::delayhop(CUserCmd* cmd)
{
	static float btime = 0.f;
	if (btime < g_GlobalVars->curtime)
		should_delayhop = false;

	if (!g_LocalPlayer)
		return;

	if (!g_Options.delayhop)
		return;

	if (!(iFlagsBackup & 1))
	{
		bktick = g_GlobalVars->tickcount + g_Options.delayhopticks;

	}

	if (GetAsyncKeyState(g_Options.dhkey))
	{

		if (iFlagsBackup & 1)
		{
			btime = g_GlobalVars->curtime + .2f;
			should_delayhop = true;
			if (bktick >= g_GlobalVars->tickcount)
				cmd->buttons &= ~IN_JUMP;
			else
				cmd->buttons |= IN_JUMP;
		}

	}

}
void AutoDuck(CUserCmd* cmd)
{
	if (!g_Options.chams_player_glass)
		return;

	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
		return;

	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->m_nMoveType() == MOVETYPE_OBSERVER || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || iFlagsBackup & 1)
		return;

	if (iFlagsBackup & FL_ONGROUND || iFlagsBackup & FL_DUCKING)
		return;

	if (should_pixelsurf)
		return;

	if (GetAsyncKeyState(g_Options.edge_bug_key) || GetAsyncKeyState(g_Options.jump_bug_key) || GetAsyncKeyState(g_Options.edgejump.hotkey))
		return;
	if (GetAsyncKeyState(g_Options.delayhop))
		return;

	float timepredicted = 0.f;
	float savedzpos = -999999.f;
	bool foundground = false;
	CUserCmd savedcmd = *cmd;


	memesclass->restoreEntityToPredictedFrame(0, g_Prediction->Split->nCommandsPredicted - 1);

	while (timepredicted < 0.03f && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
	{
		cmd->buttons |= IN_DUCK;
		prediction->StartPrediction(cmd);
		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
		{
			foundground = true;
			savedzpos = g_LocalPlayer->m_vecOrigin().z;
			prediction->EndPrediction();
			break;
		}
		else
		{
			prediction->EndPrediction();
			timepredicted += g_GlobalVars->interval_per_tick;
		}
	}


	bool foundgstanding = false;
	float savedzposstanding = -999999.f;

	if (foundground && savedzpos != -999999.f)
	{
		*cmd = savedcmd;
		memesclass->restoreEntityToPredictedFrame(0, g_Prediction->Split->nCommandsPredicted - 1);


		timepredicted = 0.f;

		while (timepredicted < 0.03f && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		{
			cmd->buttons &= ~IN_DUCK;
			prediction->StartPrediction(cmd);
			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
			{
				foundgstanding = true;
				savedzposstanding = g_LocalPlayer->m_vecOrigin().z;

				break;
			}
			else
			{

				timepredicted += g_GlobalVars->interval_per_tick;
			}
		}
	}

	if (foundground && foundgstanding)
	{
		if (savedzpos > savedzposstanding)
		{
			cmd->buttons |= IN_DUCK;
		}
	}
	else if (foundground && !foundgstanding)
	{
		cmd->buttons |= IN_DUCK;
	}
	else
	{
		*cmd = savedcmd;
	}


}


void BunnyHop::ljend(CUserCmd* cmd) {

	if (!g_Options.ljend)
		return;

	static int saved_tick;

	int duck_pre_flags = g_LocalPlayer->m_fFlags();
	int duck_post_flags;
	if (!(iFlagsBackup & FL_ONGROUND) && (g_LocalPlayer->m_fFlags() & FL_ONGROUND))
	{
		cmd->buttons |= in_duck;

	}

	if (duck_post_flags & (1 << 0) && !(duck_pre_flags & (1 << 0))) {
		if (g_GlobalVars->tickcount - saved_tick > 0 && g_GlobalVars->tickcount - saved_tick < 40)
			cmd->buttons |= in_duck;
	}
}
void BunnyHop::yaw_changer()
{
	if (!g_LocalPlayer)
		return;

	const int pre = g_LocalPlayer->m_nMoveType();
	if (g_Options.yaw_changer)
	{
		if (g_LocalPlayer->m_fFlags() & 1 || pre == MOVETYPE_LADDER)
			return;
		float m_pitch = g_CVar->FindVar("m_pitch")->GetFloat();
		float m_yaw = g_CVar->FindVar("m_yaw")->GetFloat();

		// Modify the values of m_pitch and m_yaw

		m_yaw = g_Options.yaw1;
		m_pitch = g_Options.yaw2;

		m_pitch += 0.010f;
		m_yaw += 0.010f;

		// Set the new values of m_pitch and m_yaw
		g_CVar->FindVar("m_pitch")->SetValue(m_pitch);
		g_CVar->FindVar("m_yaw")->SetValue(m_yaw);
	}
	else
	{
		if (!g_Options.yaw_changer || (!(g_LocalPlayer->m_fFlags() & 1)) || (!(pre == MOVETYPE_LADDER)))
		{
			float m_pitch = g_CVar->FindVar("m_pitch")->GetFloat();
			float m_yaw = g_CVar->FindVar("m_yaw")->GetFloat();

			m_pitch = 0.022f;
			m_yaw = 0.022f;

			g_CVar->FindVar("m_pitch")->SetValue(m_pitch);
			g_CVar->FindVar("m_yaw")->SetValue(m_yaw);

		}
	}
}
void BunnyHop::Ladderjump(CUserCmd* cmd) {
	static int saved_tick;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
		return;
	}
	if (GetAsyncKeyState(g_Options.edgejump.hotkey) && g_LocalPlayer->m_fFlags() & 1)
		return;

	if (g_Options.edgejump.lg_on_ej && GetAsyncKeyState(g_Options.edgejump.hotkey)) {
		const int pre = g_LocalPlayer->m_nMoveType();
		prediction->StartPrediction(cmd);
		prediction->EndPrediction();


		const int post = g_LocalPlayer->m_nMoveType();
		if (pre == MOVETYPE_LADDER)
		{
			if (post != MOVETYPE_LADDER)
			{
				saved_tick = g_GlobalVars->tickcount;
				cmd->buttons |= IN_JUMP;
				cmd->forwardmove = 0.f;
				cmd->sidemove = 0.f;
				cmd->buttons = cmd->buttons & ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);
			}
		}

		if (g_GlobalVars->tickcount - saved_tick > 1 && g_GlobalVars->tickcount - saved_tick < 15)
		{
			cmd->forwardmove = 0.f;
			cmd->sidemove = 0.f;
			cmd->buttons = cmd->buttons & ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);
			cmd->buttons |= IN_DUCK;
		}
	}
}
void BunnyHop::AutoStafe(CUserCmd* cmd)
{
	if (!g_Options.autostrafe)
		return;

	if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
		return;

	static auto side = 1.f;
	side = -side;

	auto velocity = g_LocalPlayer->m_vecVelocity();

	QAngle wish_angle = cmd->viewangles;

	auto speed = velocity.Length2D();
	auto ideal_strafe = std::clamp(RAD2DEG(atan(15.f / speed)), 0.f, 90.f);

	cmd->forwardmove = 0.f;

	static auto cl_sidespeed = g_CVar->FindVar("cl_sidespeed");

	static float old_yaw = 0.f;
	auto yaw_delta = std::remainderf(wish_angle.yaw - old_yaw, 360.f);
	auto abs_yaw_delta = abs(yaw_delta);
	old_yaw = wish_angle.yaw;

	const auto cl_sidespeed_value = cl_sidespeed->GetFloat();

	if (abs_yaw_delta <= ideal_strafe || abs_yaw_delta >= 30.f)
	{
		QAngle velocity_direction;
		Math::VectorAngles(velocity, velocity_direction);
		auto velocity_delta = std::remainderf(wish_angle.yaw - velocity_direction.yaw, 360.0f);
		auto retrack = std::clamp(RAD2DEG(atan(30.f / speed)), 0.f, 90.f) * 2.f;
		if (velocity_delta <= retrack || speed <= 15.f)
		{
			if (-retrack <= velocity_delta || speed <= 15.0f)
			{
				wish_angle.yaw += side * ideal_strafe;
				cmd->sidemove = cl_sidespeed_value * side;
			}
			else
			{
				wish_angle.yaw = velocity_direction.yaw - retrack;
				cmd->sidemove = cl_sidespeed_value;
			}
		}
		else
		{
			wish_angle.yaw = velocity_direction.yaw + retrack;
			cmd->sidemove = -cl_sidespeed_value;
		}

		Math::CorrectMovement(cmd, wish_angle, cmd->viewangles);
	}
	else if (yaw_delta > 0.f)
		cmd->sidemove = -cl_sidespeed_value;
	else
		cmd->sidemove = cl_sidespeed_value;
}


float ret(float hk)
{

	bool flag = false;
	if (flag && 1 == 0) {
		// никакой код здесь не выполнится член сосать
		int x = 5;
		int y = 7;
		int z = x + y * 2 - 1 / 3;
		z = (z + 1) * (z - 1) / (z + 2) - (z - 2) * (z + 3) / (z - 4);
	}
	else {
		flag = true;
		int x = 5;
		int y = 7;
		int z = x + y * 2 - 1 / 3;
		z = (z + 1) * (z - 1) / (z + 2) - (z - 2) * (z + 3) / (z - 4);
	}
	if (flag || 1 == 1) {
		int x = 5;
		int y = 7;
		int z = x + y * 2 - 1 / 3;
		z = (z + 1) * (z - 1) / (z + 2) - (z - 2) * (z + 3) / (z - 4);
		// никакой код здесь не выполнится
	}
	else {
		int x = 5;
		int y = 7;
		int z = x + y * 2 - 1 / 3;
		z = (z + 1) * (z - 1) / (z + 2) - (z - 2) * (z + 3) / (z - 4);
		flag = false;
	}
	std::string Global = std::to_string(atoi("2"));
	return hk - (float)atoi(Global.c_str());
}
Vector NormalPosVec{};

void BunnyHop::pxcalc(CUserCmd* cmd)
{
	if (!g_LocalPlayer)
		return;
	if (!g_Options.pxcalcbl)
		return;
	if (!GetAsyncKeyState(g_Options.calculatekey) && !GetAsyncKeyState(g_Options.setpointps))
		return;
	static Vector render = {};
	static Vector viewangle = {};
	static 	CGameTrace trace = { };
	static int ticksforcalc = 0;
	static std::vector<std::string> combo = {};
	static int wrticks = 0;
	static float time = 0.f;
	if (!g_LocalPlayer)
		time = 0.f;
	float ad = ret(44.49794);
	float iad = ret(2.12471);
	float ae = ret(44.49794);
	float iae = ret(2.12471);
	float af = ret(46.585155);
	float iaf = ret(2.097992);
	float ag = ret(46.328964);
	float iag = ret(2.112167);
	float ak = ret(46.349533);
	float iak = ret(2.05442);
	float aq = ret(37.306175);
	float iaq = ret(2.03862);
	float aw = ret(46.306168);
	float iaw = ret(2.03862);
	float as = ret(45.505562);
	float ias = ret(2.076744);
	float ax = ret(45.694935);
	float iax = ret(2.114349);
	float az = ret(45.928982);
	float iaz = ret(2.05658);
	float zx = ret(44.292542);
	float izx = ret(2.134491);
	float xc = ret(48.405243);
	float ixc = ret(2.011292);
	float cv = ret(39.405258);
	float icv = ret(2.011307);
	float vb = ret(37.443584);
	float ivb = ret(2.032089);
	float bn = ret(46.4436);
	float ibn = ret(2.032074);
	float nm = ret(39.222183);
	float inm = ret(2.162125);
	float ass = ret(48.142784);
	float iass = ret(2.025795);
	float sd = ret(48.222176);
	float isd = ret(2.162125);
	float df = ret(39.142769);
	float idf = ret(2.02578);
	float fg = ret(47.50787);
	float ifg = ret(2.028015);
	float gh = ret(38.345467);
	float igh = ret(2.184952);
	float hj = ret(46.096443);
	float ihj = ret(2.048607);
	float zzz = ret(38.798359);
	float izzz = ret(2.164352);
	float kl = ret(47.798344);
	float ikl = ret(2.164368);
	float qw = ret(37.096436);
	float iqw = ret(2.048592);
	float we = ret(35.36142);
	float iwe = ret(2.168922);
	float er = ret(37.303425);
	float ier = ret(2.10994);
	float rt = ret(35.835266);
	float irt = ret(2.15416);
	float ty = ret(37.388512);
	float ity = ret(2.034714);
	float yu = ret(37.148033);
	float iyu = ret(2.046165);
	float ui = ret(37.111656);
	float iui = ret(2.047867);
	float io = ret(47.575405);
	float iio = ret(2.034225);
	float op = ret(36.067513);
	float iop = ret(2.097588);
	float po = ret(38.507915);
	float ipo = ret(2.02803);
	float oi = ret(37.169167);
	float ioi = ret(2.045128);
	float uy = ret(39.834682);
	float iuy = ret(2.007446);
	float yt = ret(38.78841);
	float iyt = ret(2.042053);
	float re = ret(37.124794);
	float ire = ret(2.047276);
	float angkjsdfhgjhksdfj = ret(49.997486);
	float ia = ret(2.066513);
	float b = ret(58.997456);
	float ib = ret(2.066498);
	float c = ret(56.653721);
	float ic = ret(2.1641690000000002);
	float d = ret(44.798252);
	float id = ret(2.18013);
	float e = ret(44.526211);
	float ie = ret(2.194565);
	float f = ret(45.661243);
	float iif = ret(2.13681);
	float g = ret(37.581924);
	float ig = ret(2.095177);
	float k = ret(46.581947);
	float ik = ret(2.095207);
	float l = ret(37.303394);
	float il = ret(2.10994);
	float aa = ret(47.464214);
	float iaa = ret(2.050949);
	float ab = ret(46.303379);
	float iab = ret(2.109924);
	float ac = ret(38.464184);
	float iac = ret(2.050949);
	float sa = ret(36.30547);
	float isa = ret(2.091064);
	float ds = ret(39.142776);
	float ids = ret(2.025795);
	float fd = ret(37.096451);
	float ifd = ret(2.048607);
	float hg = ret(36.021801);
	float ihg = ret(2.099792);
	float jh = ret(35.717518);
	float ijh = ret(2.114273);
	float kj = ret(37.650806);
	float ikj = ret(2.087006);
	float lk = ret(38.50787);
	float ilk = ret(2.028);
	float xz = ret(35.857803);
	float ixz = ret(2.10759);
	float cx = ret(45.928982);
	float icx = ret(2.056595);
	float vc = ret(40.477528);
	float ivc = ret(2.141525);
	float bv = ret(38.798359);
	float ibv = ret(2.164352);
	float nb = ret(46.790314);
	float inb = ret(2.036423);
	float mn = ret(49.477528);
	float imn = ret(2.141541);
	float qq = ret(48.689182);
	float iqq = ret(2.143753);
	float ww = ret(47.798344);
	float iww = ret(2.164352);
	float ee = ret(39.689213);
	float iee = ret(2.143768);
	float rr = ret(58.117512);
	float irr = ret(2.1641690000000002);
	float tt = ret(58.070759);
	float itt = ret(2.1641690000000002);
	float yy = ret(58.148655);
	float iyy = ret(2.1641690000000002);
	float uu = ret(57.831043);
	float iuu = ret(2.1641690000000002);
	float oo = ret(57.984516);
	float ioo = ret(2.1641690000000002);
	float pp = ret(57.526463);
	float ipp = ret(2.1641690000000002);
	float ss = ret(57.457188);
	float iss = ret(2.1641690000000002);
	float dd = ret(57.028584);
	float idd = ret(2.1641690000000002);
	float ff = ret(57.827671);
	float iff = ret(2.1641690000000002);
	float gg = ret(57.397099);
	float igg = ret(2.1641690000000002);
	float hh = ret(58.179234);
	float ihh = ret(2.1641690000000002);
	float jj = ret(58.221012);
	float ijj = ret(2.1641690000000002);
	float kk = ret(57.632755);
	float ikk = ret(2.1641690000000002);
	float ll = ret(57.11182);
	float ill = ret(2.1641690000000002);
	float nf = ret(58.165714);
	float inf = ret(2.1641690000000002);
	float br = ret(57.538944);
	float ibr = ret(2.1641690000000002);
	float cty = ret(56.821491);
	float icty = ret(2.1641690000000002);
	float svv = ret(56.723469);
	float isvv = ret(2.1641690000000002);
	float mr = ret(58.37888);
	float imr = ret(2.1641690000000002);
	float me = ret(58.08942);
	float ime = ret(2.1641690000000002);
	float ah = ret(57.532032);
	float iah = ret(2.1641690000000002);
	float dada = ret(42.74173);
	float idada = ret(2.013695);
	float wfw = ret(35.498852);
	float iwfw = ret(2.12468);
	float bot = ret(44.498882);
	float ibot = ret(2.12468);
	float wx = ret(45.014797);
	float iwx = ret(2.100113);
	float xgd = ret(44.746452);
	float ixgd = ret(2.112892);
	float xye = ret(44.706718);
	float ixye = ret(2.114784);
	float dfj = ret(34.260181);
	float idfj = ret(2.18367);
	float nsdbge = ret(38.391796);
	float insdbge = ret(2.009659);
	float wfrh = ret(37.289478);
	float iwfrh = ret(2.042374);
	float vs = ret(37.12368);
	float ivs = ret(2.04731);
	float dhr = ret(37.108013);
	float idhr = ret(2.04805);
	float wgww = ret(46.482052);
	float iwgww = ret(2.030243);
	float xte = ret(40.769958);
	float ixte = ret(2.137085);
	float fbaa = ret(49.769958);
	float ifbaa = ret(2.137085);
	float jhe = ret(37.482037);
	float ijhe = ret(2.030243);
	float xhe = ret(47.861366);
	float ixhe = ret(2.029869);
	float xheh = ret(37.833923);
	float ixheh = ret(2.013489);
	float assss = ret(44.670063);
	float iassss = ret(2.116531);
	float dfjr = ret(46.833923);
	float idfjr = ret(2.013489);
	float fdrth = ret(46.306488);
	float ifdrth = ret(2.038605);
	float sm = ret(36.607971);
	float ism = ret(2.072472);
	float bfgt = ret(39.202259);
	float ibfgt = ret(2.145996);
	float ehr = ret(48.202229);
	float iehr = ret(2.145996);
	float jdr = ret(37.482037);
	float ijdr = ret(2.030258);
	float cey = ret(36.243351);
	float icey = ret(2.089249);
	float qa = ret(44.670029);
	float iqa = ret(2.116531);
	float ws = ret(44.695377);
	float iws = ret(2.115326);
	float qs = ret(46.265015);
	float iqs = ret(2.112579);
	float wd = ret(45.592903);
	float iwd = ret(2.114738);
	float ef = ret(44.868847);
	float ief = ret(2.115051);
	float rg = ret(44.699188);
	float irg = ret(2.1151429999999998);
	float yj = ret(44.699795);
	float iyj = ret(2.115112);
	float uk = ret(45.666786);
	float iuk = ret(2.114677);
	float ol = ret(44.330849);
	float iol = ret(2.132683);
	float rf = ret(46.481861);
	float irf = ret(2.030258);
	float tg = ret(44.98024);
	float itg = ret(2.114998);
	float yh = ret(44.363728);
	float iyh = ret(2.131119);
	float uj = ret(46.169182);
	float iuj = ret(2.045143);
	float sw = ret(44.82885);
	float isw = ret(2.130783);
	float de = ret(46.135601);
	float ide = ret(2.046738);
	float fr = ret(46.634338);
	float ifr = ret(2.044807);
	float gt = ret(47.479485);
	float igt = ret(2.028358);
	float ryu = ret(48.078674);
	float iryu = ret(2.026215);
	float hy = ret(45.45496);
	float ihy = ret(2.128647);
	float ju = ret(47.261536);
	float iju = ret(2.042618);
	float ki = ret(44.337719);
	float iki = ret(2.132355);
	float qe = ret(46.217319);
	float iqe = ret(2.070312);
	float wr = ret(45.960419);
	float iwr = ret(2.055084);
	float et = ret(37.560932);
	float iet = ret(2.026489);
	float ry = ret(35.903938);
	float iry = ret(2.105392);
	float tu = ret(44.344452);
	float itu = ret(2.132019);
	float yi = ret(47.493374);
	float iyi = ret(2.045456);
	float uo = ret(35.910835);
	float iuo = ret(2.10508);
	float ip = ret(37.570156);
	float iip = ret(2.026169);
	float sf = ret(45.967182);
	float isf = ret(2.054779);
	float dg = ret(45.802128);
	float idg = ret(2.062622);
	float fh = ret(44.004311);
	float ifh = ret(2.148216);
	float hk = ret(46.142567);
	float ihk = ret(2.046391);
	float jl = ret(44.01125);
	float ijl = ret(2.147903);
	float zc = ret(45.809193);
	float izc = ret(2.062271);
	float xv = ret(46.14946);
	float ixv = ret(2.046066);
	float bm = ret(45.960419);
	float ibm = ret(2.055099);
	float mb = ret(48.299812);
	float imb = ret(2.028343);
	float nv = ret(47.839298);
	float inv = ret(2.162491);
	float bc = ret(50.210602);
	float ibc = ret(2.135513);
	float vx = ret(47.479485);
	float ivx = ret(2.028358);
	float cz = ret(48.473785);
	float icz = ret(2.160873);
	float da = ret(47.83218);
	float ida = ret(2.162811);
	float fs = ret(48.066753);
	float ifs = ret(2.17836);
	float gd = ret(49.379147);
	float igd = ret(2.152969);
	float hf = ret(37.61808);
	float ihf = ret(2.087341);
	float jg = ret(35.896915);
	float ijg = ret(2.105728);
	float kh = ret(36.555054);
	float ikh = ret(2.10379);
	float eq = ret(35.930542);
	float ieq = ret(2.104126);
	float rw = ret(35.563519);
	float irw = ret(2.121597);
	float te = ret(35.570614);
	float ite = ret(2.121246);
	float yr = ret(39.409531);
	float iyr = ret(2.024544);
	float ut = ret(37.515041);
	float iut = ret(2.028671);
	float keeel = ret(46.79277);
	float ikeeel = ret(2.028687);
	float cb = ret(2.0);
	float icb = ret(2.195312);
	float zs = ret(45.697342);
	float izs = ret(2.109436);
	float xd = ret(45.69606);
	float ixd = ret(2.067673);
	float cf = ret(38.549225);
	float icf = ret(2.175674);
	float vg = ret(38.585472);
	float ivg = ret(2.174026);
	float bh = ret(47.606373);
	float ibh = ret(2.045074);
	float nj = ret(37.502224);
	float inj = ret(2.029282);
	float mk = ret(44.281822);
	float imk = ret(2.135017);
	float sz = ret(47.324383);
	float isz = ret(2.046036);
	float cd = ret(37.481876);
	float icd = ret(2.030258);
	float vf = ret(46.510254);
	float ivf = ret(2.064301);
	float bg = ret(40.209267);
	float ibg = ret(2.15358);
	float nh = ret(39.36842);
	float inh = ret(2.172226);
	float jm = ret(37.085548);
	float ijm = ret(2.0491260000000002);
	float eu = ret(38.512302);
	float ieu = ret(2.02301);
	float en = ret(40.492733);
	float ien = ret(2.152603);
	float rm = ret(46.606709);
	float irm = ret(2.106461);
	float eh = ret(35.840012);
	float ieh = ret(2.108437);
	float wv = ret(45.987576);
	float iwv = ret(2.053802);
	float nd = ret(45.576809);
	float ind = ret(2.073334);
	float dh = ret(37.966171);
	float idh = ret(2.025169);
	float xf = ret(38.894909);
	float ixf = ret(2.172562);
	float ch = ret(46.037247);
	float ich = ret(2.064629);
	float cy = ret(39.423092);
	float icy = ret(2.019943);
	float dt = ret(39.776566);
	float idt = ret(2.155106);
	float dy = ret(38.861069);
	float idy = ret(2.161484);
	float xy = ret(39.776566);
	float ixy = ret(2.155106);
	float rh = ret(37.737106);
	float irh = ret(2.03933);
	float xg = ret(37.721718);
	float ixg = ret(2.082008);
	float xb = ret(37.156357);
	float ixb = ret(2.045769);
	float wh = ret(38.421661);
	float iwh = ret(2.181488);
	float di = ret(37.103939);
	float idi = ret(2.048248);
	float ehh = ret(38.047909);
	float iehh = ret(2.003296);
	float ar = ret(45.224331);
	float iar = ret(2.090118);
	float ay = ret(37.14946);
	float iay = ret(2.046066);
	float ux = ret(38.429054);
	float iux = ret(2.181152);
	float sn = ret(45.583729);
	float isn = ret(2.073021);
	float bj = ret(38.839314);
	float ibj = ret(2.162506);
	float ge = ret(39.16518);
	float ige = ret(2.035904);
	float mu = ret(35.846893);
	float imu = ret(2.108109);
	float nn = ret(37.135555);
	float inn = ret(2.046738);
	float rn = ret(40.379131);
	float irn = ret(2.152985);
	float wf = ret(37.634323);
	float iwf = ret(2.044807);
	float wg = ret(46.606709);
	float iwg = ret(2.106461);
	float fff = ret(45.698483);
	float ifff = ret(2.070877);
	float wewe = ret(39.078674);
	float iwewe = ret(2.026215);
	float eg = ret(46.037247);
	float ieg = ret(2.064629);
	float vj = ret(38.546967);
	float ivj = ret(2.178947);
	float jt = ret(38.894909);
	float ijt = ret(2.172562);
	float wvv = ret(47.464214);
	float iwvv = ret(2.050949);
	float wj = ret(44.351196);
	float iwj = ret(2.131714);
	float wff = ret(44.299892);
	float iwff = ret(2.134155);
	float gn = ret(46.156364);
	float ign = ret(2.045753);
	float kek = ret(46.10397);
	float ikek = ret(2.048248);
	float awd = ret(48.16518);
	float iawd = ret(2.035904);
	float sb = ret(46.085548);
	float isb = ret(2.0491260000000002);
	float sv = ret(46.355804);
	float isv = ret(2.122063);
	float dn = ret(38.429054);
	float idn = ret(2.181137);
	float sr = ret(45.583748);
	float isr = ret(2.073021);
	float cht = ret(38.479485);
	float icht = ret(2.028358);
	float vm = ret(44.351196);
	float ivm = ret(2.131714);
	float wb = ret(37.135586);
	float iwb = ret(2.046738);
	float aer = ret(44.929222);
	float iaer = ret(2.125435);
	float bs = ret(39.165195);
	float ibs = ret(2.035904);
	float enn = ret(37.737114);
	float ienn = ret(2.039345);
	float em = ret(46.502239);
	float iem = ret(2.029282);
	float fw = ret(46.085548);
	float ifw = ret(2.0491260000000002);
	float egg = ret(49.209267);
	float iegg = ret(2.15358);
	float wdd = ret(47.512287);
	float iwdd = ret(2.02301);
	float st = ret(48.368435);
	float ist = ret(2.172226);
	float cg = ret(47.549225);
	float icg = ret(2.175674);
	float xff = ret(37.634338);
	float ixff = ret(2.044807);
	float hr = ret(37.103954);
	float ihr = ret(2.048248);
	float wdwd = ret(46.156364);
	float iwdwd = ret(2.045753);
	float awdawd = ret(39.423107);
	float iawdawd = ret(2.019958);
	float qz = ret(46.450844);
	float iqz = ret(2.031731);
	float vh = ret(46.737099);
	float ivh = ret(2.03933);
	float dyy = ret(48.776558);
	float idyy = ret(2.15509);
	float xt = ret(38.68755);
	float ixt = ret(2.03624);
	float wt = ret(47.687534);
	float iwt = ret(2.03624);
	float ng = ret(47.894924);
	float ing = ret(2.172562);
	float wvw = ret(47.546967);
	float iwvw = ret(2.178947);
	float wfff = ret(48.423138);
	float iwfff = ret(2.019958);
	float wggg = ret(45.878326);
	float iwggg = ret(2.12241);
	float zf = ret(38.261536);
	float izf = ret(2.042633);
	float wsw = ret(45.576809);
	float iwsw = ret(2.073334);
	float wfu = ret(37.79277);
	float iwfu = ret(2.028702);
	float du = ret(38.421646);
	float idu = ret(2.181458);
	float hs = ret(37.156342);
	float ihs = ret(2.045738);
	float cyu = ret(37.450829);
	float icyu = ret(2.031715);
	float srr = ret(47.421646);
	float isrr = ret(2.181458);
	float xtt = ret(37.169182);
	float ixtt = ret(2.045158);
	float ph = ret(46.893822);
	float iph = ret(2.047516);
	float wgw = ret(49.492733);
	float iwgw = ret(2.152573);
	float xh = ret(39.776566);
	float ixh = ret(2.155106);
	float shhr = ret(40.245232);
	float ishhr = ret(2.169456);
	float rehr = ret(47.383125);
	float irehr = ret(2.061584);
	float fdh = ret(37.142578);
	float ifdh = ret(2.04641);
	float nfdfr = ret(38.421661);
	float infdfr = ret(2.181473);
	float htgrt = ret(45.576824);
	float ihtgrt = ret(2.073349);
	float ngr = ret(38.832165);
	float ingr = ret(2.162811);
	float htgrs = ret(38.68758);
	float ihtgrs = ret(2.03624);
	float gnfdggg = ret(35.840012);
	float ignfdggg = ret(2.108437);
	float msu = ret(38.261536);
	float imsu = ret(2.042618);
	float nrhc = ret(39.066738);
	float inrhc = ret(2.17836);
	float qn = ret(37.169182);
	float iqn = ret(2.045143);
	float qg = ret(36.802143);
	float iqg = ret(2.062622);
	float xur = ret(36.802143);
	float ixur = ret(2.062622);
	float bfnh = ret(49.245232);
	float ibfnh = ret(2.169456);
	float zpw = ret(39.473785);
	float izpw = ret(2.160889);
	float whr = ret(47.421646);
	float iwhr = ret(2.181458);
	float wmfj = ret(37.169182);
	float iwmfj = ret(2.045143);
	float ejt = ret(37.721828);
	float iejt = ret(2.095215);
	float ejtyyy = ret(45.878326);
	float iejtyyy = ret(2.122391);
	float mwfu = ret(48.423138);
	float imwfu = ret(2.019958);
	float jkdkdkdk = ret(47.687534);
	float ijkdkdkdk = ret(2.03624);
	float aaaaaaa = ret(47.806198);
	float iaaaaaaa = ret(2.163994);
	float siiif = ret(45.936226);
	float isiiif = ret(2.056236);
	float cieop = ret(47.861084);
	float icieop = ret(2.161499);
	float fpdo = ret(35.865238);
	float ifpdo = ret(2.107239);
	float cueo = ret(36.212204);
	float icueo = ret(2.090729);
	float xieu = ret(35.917717);
	float ixieu = ret(2.104736);
	float riep = ret(47.421661);
	float iriep = ret(2.181473);
	float cnhfy = ret(37.536232);
	float icnhfy = ret(2.027664);
	float xisjg = ret(37.721813);
	float ixisjg = ret(2.0952);
	float fsw = ret(38.68755);
	float ifsw = ret(2.03624);
	float beu = ret(47.894924);
	float ibeu = ret(2.172562);
	float nwr = ret(38.806198);
	float inwr = ret(2.163994);
	float vksiy = ret(48.776558);
	float ivksiy = ret(2.155106);
	float smaiu = ret(36.730148);
	float ismaiu = ret(2.098312);
	float cnydu = ret(37.103954);
	float icnydu = ret(2.048248);
	float csury = ret(47.047939);
	float icsury = ret(2.003311);
	float cibuy = ret(36.809223);
	float icibuy = ret(2.062286);
	float vbdyft = ret(41.210602);
	float ivbdyft = ret(2.135529);
	float cvsyr = ret(47.429047);
	float icvsyr = ret(2.181152);
	float cbcbcbcbcbc = ret(35.505325);
	float icbcbcbcbcbc = ret(2.12439);
	float jfudyr = ret(37.634338);
	float ijfudyr = ret(2.044807);

	int x, y;
	static int alpha = 0;
	if (alpha < 0)
		alpha = 0;
	if (alpha > 255)
		alpha = 255;
	g_EngineClient->GetScreenSize(x, y);
	if (cmd == nullptr)
	{


		auto draw = ImGui::GetOverlayDrawList();

		auto m_vecSize = ImGui::GetWindowSize();
		auto m_vecPos = ImGui::GetWindowPos();
		auto m_vecSize2 = ImGui::GetWindowSize();
		auto m_vecPos2 = ImGui::GetWindowPos();
		m_vecPos2.x += 5;
		m_vecPos2.y += 5;
		m_vecSize2.x -= 10;
		m_vecSize2.y -= 10;
		std::string combos;
		if (combo.size() > 0)
		{


			//draw->AddRectFilled(ImVec2(x / 2, y / 2), ImVec2(x + x - 2, y + y - 2 + 100), ImColor(27, 27, 27, alpha), 4.f);
			//draw->AddRectFilled(ImVec2(2,2), ImVec2(2,y / 2 + 100), ImColor(21,20,21,alpha), 0.f);
			draw->AddText(fonts::indicator_font, 20.f, ImVec2(x / 2 - fonts::indicator_font->CalcTextSizeA(20, FLT_MAX, NULL, "created solutions").x / 2, y / 2 + 100), ImColor(255, 255, 255, alpha), "created solutions");

			int z = 0;
			for (int i = 0; i < combo.size(); i++)
			{
				draw->AddText(fonts::indicator_font, 20.f, ImVec2(x / 2 - fonts::indicator_font->CalcTextSizeA(20, FLT_MAX, NULL, combo[i].c_str()).x / 2, y / 2 + 130 + z * 20), ImColor(255, 255, 255, alpha), combo[i].c_str());
				z += 1;
			}
		}
		if (time > g_GlobalVars->curtime)
		{

			alpha++;
		}
		else
		{
			alpha -= 1;
		}

	}
	if (cmd)
	{
		viewangle = Vector(cmd->viewangles.pitch, cmd->viewangles.yaw, 0);

		if (GetAsyncKeyState(g_Options.setpointps) & 0x01)
		{

			const auto endPos = g_LocalPlayer->GetEyePos() + Vector::fromAngle(viewangle) * 2000;


			Ray_t ray;
			ray.Init(g_LocalPlayer->GetEyePos(), endPos);
			CTraceFilterWorldOnly flt;

			g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &trace);
			if (trace.fraction == 1.f)
			{
				g_EngineClient->ExecuteClientCmd("play error");
				pointvec.Zero();

				return;
			}


			pointvec = g_LocalPlayer->GetEyePos() + (endPos - g_LocalPlayer->GetEyePos()) * trace.fraction;
			NormalPosVec = trace.plane.normal;



		}
	}


	if (GetAsyncKeyState(g_Options.calculatekey) & 0x01)
	{
		float point;

		if (pointvec.z < 0.f)
			point = (int)pointvec.z - 0.97125f;
		else
			point = (int)pointvec.z + 0.03125f;



		Vector backuporigin = g_LocalPlayer->m_vecOrigin();
		bool dropdown = (int)point == (int)backuporigin.z;
		combo.clear();

		float  a = backuporigin.z + cb;
		int i = 0;


		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("drop down");
			if (check(a + 9.f, point))
				combo.emplace_back("drop down");
			float o = i * (i + 1) / 2;
			float p = icb + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ad;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iad + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ae;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iae + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + af;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iaf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ag;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iag + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ak;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iak + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + aq;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iaq + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + aw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iaw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + as;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ias + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ax;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iax + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + az;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iaz + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + zx;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = izx + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xc;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixc + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cv;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icv + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + vb;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ivb + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bn;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibn + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nm;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = inm + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ass;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iass + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + sd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = id + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + df;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + gh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = igh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + hj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ihj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + zzz;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = izzz + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + kl;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ikl + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + qw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iqw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + we;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwe + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + er;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ier + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + rt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = irt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ty;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ity + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + yu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("3 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("3 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iyu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ui;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("4 (or more) crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("4 (or more) crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iui + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + io;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iio + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + op;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iop + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + po;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ipo + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + oi;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 3 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 3 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ioi + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + uy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iuy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + yt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 3 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 3 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iyt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + re;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 4 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 4 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ire + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + angkjsdfhgjhksdfj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch");
			float o = i * (i + 1) / 2;
			float p = ia + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + b;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch");
			float o = i * (i + 1) / 2;
			float p = ib + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + c;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jump");
			if (check(a + 9.f, point))
				combo.emplace_back("jump + crouch");
			float o = i * (i + 1) / 2;
			float p = ic + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + d;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + crouch");
			float o = i * (i + 1) / 2;
			float p = id + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + e;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ie + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + f;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iif + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + g;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ig + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + k;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ik + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + l;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = il + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + aa;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iaa + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ab;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iab + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ac;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iac + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + sa;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = isa + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ds;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ids + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifd + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + hg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ihg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + jh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ijh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + kj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ikj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + lk;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ilk + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xz;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixz + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cx;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icx + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + vc;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ivc + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bv;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibv + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nb;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = inb + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + mn;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = imn + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + qq;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iqq + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ww;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iww + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ee;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iee + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cey;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icey + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + jdr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ijdr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ehr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iehr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bfgt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibfgt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + sm;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ism + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fdrth;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifdrth + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dfjr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idfjr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + assss;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iassss + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xheh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixheh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xhe;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixhe + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + jhe;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ijhe + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fbaa;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifbaa + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xte;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixte + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wgww;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwgww + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dhr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + 5 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + 5 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idhr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + vs;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + 4 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + 4 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ivs + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wfrh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + 3 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + 3 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwfrh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nsdbge;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = insdbge + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dfj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idfj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xye;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + 4 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + 4 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixye + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xgd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + 3 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + 3 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixgd + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wx;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwx + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bot;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibot + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dada;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idada + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wfw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwfw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + rr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("jb");
			if (check(a + 9.f, point))
				combo.emplace_back("jb + crouch");
			float o = i * (i + 1) / 2;
			float p = irr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + tt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = itt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + yy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = iyy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + uu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = iuu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + oo;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 crouch hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("2 crouch hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = ioo + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + pp;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("3 crouch hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("3 crouch hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = ipp + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ss;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("4 crouch hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("4 crouch hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = iss + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = idd + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ff;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + mj hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + mj hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = iff + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + gg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 mj hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("2 mj hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = igg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + hh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = ihh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + jj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = ijj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + kk;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = ikk + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ll;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 lj hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("2 lj hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = ill + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ah;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = iah + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + me;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + mj hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + mj hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = ime + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + mr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = imr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + svv;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + mj hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + mj hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = isvv + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cty;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 4 hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 4 hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = icty + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + br;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 3 hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 3 hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = ibr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nf;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 hop + jb");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 hop + jb + crouch");
			float o = i * (i + 1) / 2;
			float p = inf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + qa;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("3 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("3 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iqa + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ws;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("4 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("4 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iws + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + qs;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 3 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 3 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iqs + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 4 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 4 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwd + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ef;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 5 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 5 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ief + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + rg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("5 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("5 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = irg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + yj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("6 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("6 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iyj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + uk;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iuk + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ol;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iol + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + rf;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = irf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + tg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = itg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + yh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iyh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + uj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 3 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 3 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iuj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + sw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = isw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + de;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ide + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("3 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("3 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + gt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = igt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ryu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iryu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + hy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ihy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ju;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iju + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + keeel;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ikeeel + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ki;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iki + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + qe;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iqe + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + et;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iet + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ry;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iry + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + tu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = itu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + yi;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iyi + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + uo;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iuo + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ip;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iip + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + sf;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = isf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + hk;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ihk + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + jl;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ijl + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + zc;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = izc + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xv;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixv + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bm;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibm + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + mb;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = imb + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nv;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = inv + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bc;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibc + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + vx;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ivx + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cz;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icz + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + da;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ida + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fs;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifs + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + gd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = igd + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + hf;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ihf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + jg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ijg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + kh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ikh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + eq;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ieq + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + rw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = irw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + te;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ite + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + yr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iyr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ut;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iut + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + zs;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = izs + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixd + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cf;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 3 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 3 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + vg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 4 (or more) mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 4 (or more) mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ivg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = inj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + mk;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = imk + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + sz;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = isz + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icd + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + vf;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ivf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("3 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("3 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = inh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + jm;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ijm + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + eu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ieu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + en;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ien + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + rm;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = irm + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + eh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ieh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wv;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwv + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ind + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xf;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ch;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ich + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + rh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = irh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xb;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixb + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + di;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idi + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ehh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iehh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ar;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iar + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ay;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iay + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ux;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iux + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + sn;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = isn + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ge;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ige + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + mu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = imu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nn;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = inn + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + rn;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = irn + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wf;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + xh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wgw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwgw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ph;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iph + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xtt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixtt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + srr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = isrr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cyu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icyu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + hs;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ihs + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + du;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wfu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwfu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wsw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwsw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + zf;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = izf + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wggg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwggg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wfff;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwfff + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wvw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwvw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ng;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ing + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dyy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idyy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + vh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ivh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + qz;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iqz + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + awdawd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iawd + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + hr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ihr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xff;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixff + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + st;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("2 mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("2 mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ist + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wdd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwdd + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + egg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iegg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + em;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iem + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + enn;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ienn + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bs;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibs + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + aer;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iaer + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wb;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("hop + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("hop + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwb + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + cht;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icht + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + sr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = isr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + dn;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = idn + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + sv;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = isv + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + sb;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = isb + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + awd;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iawd + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + kek;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 3 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 3 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ikek + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + gn;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ign + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wff;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwff + (0.1953125 * i);
			a = a - p;
			i += 1;
		}

		a = backuporigin.z + wj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wvv;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwvv + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + jt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 3 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 3 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ijt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + vj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ivj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + eg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ieg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wewe;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwewe + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fff;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifff + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + 2 hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + 2 hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + shhr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + 2 mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + 2 mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ishhr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + rehr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + mj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + mj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = irehr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fdh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifdh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nfdfr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = infdfr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + htgrt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ihtgrt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ngr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ingr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + htgrs;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ihtgrs + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + gnfdggg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ignfdggg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + msu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + hop + mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + hop + mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = imsu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nrhc;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + 2 crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + 2 crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = inrhc + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + qn;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iqn + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + qg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + couch hop + mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + couch hop + mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iqg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xur;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixur + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + bfnh;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + crouch hop + mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + crouch hop + mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibfnh + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + zpw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = izpw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + whr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwhr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + wmfj;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + lj hop + mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + lj hop + mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iwmfj + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + ejt;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj + mj hop + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj + mj hop + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iejt + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + ejtyyy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + lj hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + lj hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iejtyyy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + mwfu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = imwfu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + jkdkdkdk;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + 2 lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + 2 lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ijkdkdkdk + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + aaaaaaa;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iaaaaaaa + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + siiif;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + crouch hop + hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + crouch hop + hop + crouch");
			float o = i * (i + 1) / 2;
			float p = isiiif + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cieop;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icieop + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fpdo;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifpdo + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cueo;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icueo + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xieu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + hop + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + hop + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixieu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + riep;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + 2 crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + 2 crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = iriep + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cnhfy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + 2 crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + 2 crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icnhfy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + xisjg;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ixisjg + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + fsw;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ifsw + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + beu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + crouch hop + mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + crouch hop + mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ibeu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + nwr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = inwr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + vksiy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ivksiy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + smaiu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + mj hop + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + mj hop + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ismaiu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cnydu;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj + lj hop + mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj + lj hop + mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icnydu + (0.1953125 * i);
			a = a - p;
			i += 1;
		}



		a = backuporigin.z + csury;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + mj hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + mj hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icsury + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cibuy;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("crouch hop + lj hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("crouch hop + lj hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icibuy + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + vbdyft;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + crouch hop + mj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + crouch hop + mj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ivbdyft + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cvsyr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + crouch hop + lj hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + crouch hop + lj hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icvsyr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + cbcbcbcbcbc;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("mj hop + lj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("mj hop + lj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = icbcbcbcbcbc + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		a = backuporigin.z + jfudyr;
		i = 0;
		while (a > point - 20.f)
		{
			if (check(a, point))
				combo.emplace_back("lj hop + mj hop + crouch hop");
			if (check(a + 9.f, point))
				combo.emplace_back("lj hop + mj hop + crouch hop + crouch");
			float o = i * (i + 1) / 2;
			float p = ijfudyr + (0.1953125 * i);
			a = a - p;
			i += 1;
		}
		std::sort(combo.begin(), combo.end(),
			[](const std::string& a, const std::string& b)
			{
				return a.length() < b.length();
			});
		time = g_GlobalVars->curtime + 4.5f;
		ticksforcalc = GetTickCount64() + 400;
	}

}

void BunnyHop::pxcalcrender()
{
	if (!g_LocalPlayer)
		return;
	if (g_Options.pxcalcbl)
	{
		Vector render;
		if (!pointvec.is_zero())
		{
			if (g_LocalPlayer->CanSeePlayer(0, pointvec))
			{
				if (!g_DebugOverlay->ScreenPosition(pointvec, render))
				{

					Render::Get().RenderCircleFilled(render.x, render.y, 12.f, 128, Color(100, 100, 100, 100));

					Render::Get().RenderCircle(render.x, render.y, 12.f, 128, Color(g_Options.accentcoll[0], g_Options.accentcoll[1], g_Options.accentcoll[2]), 1.f);


				}
				//Render::Get().RenderCircle3D(pointvec, Vector(), 128, 16.f, Color(255, 255, 255));
			}
		}
	}
}

void BunnyHop::PixelFinder(CUserCmd* cmd)
{
	auto g_ChatElement = FindHudElement<c_hudchat>("CHudChat");
	if (!g_ChatElement)
		return;
	static Vector StartPos, EndPos{};
	static Vector ViewAngles{};
	static Vector pixelsurfPoint{};
	static Vector WallNormal{};
	static bool redLine = false;
	static std::vector<Vector> Points{};
	int to_muchPoints = 0;
	if (cmd != nullptr)
	{
		ViewAngles = Vector(cmd->viewangles.pitch, cmd->viewangles.yaw, 0.f);
		static bool FirstPoint = FALSE;
		if (GetAsyncKeyState(g_Options.posloska))
		{

			if (!FirstPoint)
			{
				if (Points.size() > 0)
					Points.clear();
				const auto endPos = g_LocalPlayer->GetEyePos() + Vector::fromAngle(ViewAngles) * 6000;

				CGameTrace trace = { };
				Ray_t ray;
				ray.Init(g_LocalPlayer->GetEyePos(), endPos);
				CTraceFilter flt;
				flt.pSkip = g_LocalPlayer;
				g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &trace);


				WallNormal = trace.plane.normal;

				StartPos = g_LocalPlayer->GetEyePos() + (endPos - g_LocalPlayer->GetEyePos()) * trace.fraction;
				EndPos = g_LocalPlayer->GetEyePos() + (endPos - g_LocalPlayer->GetEyePos()) * trace.fraction;
				FirstPoint = true;
			}
			else
			{
				const auto endPos = g_LocalPlayer->GetEyePos() + Vector::fromAngle(ViewAngles) * 6000;

				CGameTrace trace = { };
				Ray_t ray;
				ray.Init(g_LocalPlayer->GetEyePos(), endPos);
				CTraceFilter flt;
				flt.pSkip = g_LocalPlayer;
				g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &trace);

				Vector SecPlane = trace.plane.normal;
				if (SecPlane.x != WallNormal.x || SecPlane.y != WallNormal.y)
				{
					redLine = true;
				}

				EndPos = Vector(StartPos.x, StartPos.y, Vector(g_LocalPlayer->GetEyePos() + (endPos - g_LocalPlayer->GetEyePos()) * trace.fraction).z);
			}

		}
		else if (!GetAsyncKeyState(g_Options.posloska) && FirstPoint)
		{
			if (redLine)
			{
				redLine = false;
				FirstPoint = false;

				return;
			}

			Vector origin = g_LocalPlayer->m_vecOrigin();
			int buttons = cmd->buttons;
			float ForwardMove = cmd->forwardmove;
			float Lerp = 0;


			int PredictionFrames = g_Prediction->Split->nCommandsPredicted - 1;

			for (int i = 0; i < 10000; i++)
			{
				float align = 15.97803f;
				*(int*)((uintptr_t)(C_BasePlayer*)g_LocalPlayer + 0x25C) = 2;
				//if (to_muchPoints > 4)
				//{
				//	Points.clear();
				//	return;
				//}


				cmd->buttons |= IN_JUMP;
				cmd->buttons |= IN_DUCK;

				Vector Angles{ WallNormal.x * -1.f, WallNormal.y * -1.f,0.f };
				Vector to_wall = Angles.toAngle();
				float rotation = DEG2RAD(to_wall.y - cmd->viewangles.yaw);
				float cos_rot = cos(rotation);
				float sin_rot = sin(rotation);
				float forwardmove = cos_rot * 10.f;
				float sidemove = -sin_rot * 10.f;
				cmd->forwardmove = forwardmove;
				cmd->sidemove = sidemove;
				if (g_LocalPlayer->m_vecVelocity().z < 0)
				{
					static float Height = 0.f;
					if (StartPos.z < 0.f)
						Height = (int)StartPos.z - 0.972092f - Lerp;
					else
						Height = (int)StartPos.z + 0.0287018f - Lerp;
					int maxLerp;

					maxLerp = StartPos.z - EndPos.z;

					if (Lerp > maxLerp)
					{
						cmd->buttons = buttons;
						cmd->forwardmove = ForwardMove;
						FirstPoint = false;

						break;
					}
					else
					{

						Lerp += 1.f;

					}

					if (WallNormal.x < 0 && WallNormal.y < 0.f)
					{
						g_LocalPlayer->m_vecOrigin() = Vector(StartPos.x - 15.97803f, StartPos.y - 15.97803f, Height);
						memesclass->setAbsOrigin(g_LocalPlayer, Vector(StartPos.x, StartPos.y, Height));
					}
					else if (WallNormal.x < 0 && WallNormal.y > 0.f)
					{
						g_LocalPlayer->m_vecOrigin() = Vector(StartPos.x - 15.97803f, StartPos.y + 15.97803f, Height);
						memesclass->setAbsOrigin(g_LocalPlayer, Vector(StartPos.x, StartPos.y, Height));
					}
					else if (WallNormal.x > 0 && WallNormal.y < 0.f)
					{
						g_LocalPlayer->m_vecOrigin() = Vector(StartPos.x + 15.97803f, StartPos.y - 15.97803f, Height);
						memesclass->setAbsOrigin(g_LocalPlayer, Vector(StartPos.x, StartPos.y, Height));
					}
					else if (WallNormal.x > 0 && WallNormal.y > 0.f)
					{
						g_LocalPlayer->m_vecOrigin() = Vector(StartPos.x + 15.97803f, StartPos.y + 15.97803f, Height);
						memesclass->setAbsOrigin(g_LocalPlayer, Vector(StartPos.x, StartPos.y, Height));
					}
					else if (WallNormal.x == 0.f && WallNormal.y > 0.f)
					{
						g_LocalPlayer->m_vecOrigin() = Vector(StartPos.x, StartPos.y + 15.97803f, Height);
						memesclass->setAbsOrigin(g_LocalPlayer, Vector(StartPos.x, StartPos.y, Height));
					}
					else if (WallNormal.x == 0.f && WallNormal.y < 0.f)
					{
						g_LocalPlayer->m_vecOrigin() = Vector(StartPos.x, StartPos.y - 15.97803f, Height);
						memesclass->setAbsOrigin(g_LocalPlayer, Vector(StartPos.x, StartPos.y, Height));
					}
					else if (WallNormal.x < 0 && WallNormal.y == 0.f)
					{
						g_LocalPlayer->m_vecOrigin() = Vector(StartPos.x - 15.97803f, StartPos.y, Height);
						memesclass->setAbsOrigin(g_LocalPlayer, Vector(StartPos.x, StartPos.y, Height));
					}
					else if (WallNormal.x > 0 && WallNormal.y == 0.f)
					{
						g_LocalPlayer->m_vecOrigin() = Vector(StartPos.x + 15.97803f, StartPos.y, Height);
						memesclass->setAbsOrigin(g_LocalPlayer, Vector(StartPos.x, StartPos.y, Height));
					}



				}

				prediction->StartPrediction(cmd);
				if (g_LocalPlayer->m_vecVelocity().z == -flBugSpeed)
				{

					if (WallNormal.x < 0 && WallNormal.y < 0.f)
					{
						//	pointvec = Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	g_Routes.emplace_back(routes(Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y+ 15.97803f, g_LocalPlayer->m_vecOrigin().z), "ps"));
						Points.emplace_back(Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z));
						to_muchPoints += 1;
						FirstPoint = false;
					}
					else if (WallNormal.x < 0 && WallNormal.y > 0.f)
					{
						//	pointvec = Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	g_Routes.emplace_back(routes(Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z), "ps"));
						Points.emplace_back(Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z));
						to_muchPoints += 1; FirstPoint = false;
					}
					else if (WallNormal.x > 0 && WallNormal.y < 0.f)
					{
						//	pointvec = Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	g_Routes.emplace_back(routes(Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z), "ps"));
						Points.emplace_back(Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z));
						to_muchPoints += 1; FirstPoint = false;
					}
					else if (WallNormal.x > 0 && WallNormal.y > 0.f)
					{
						//	pointvec = Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	g_Routes.emplace_back(routes(Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z), "ps"));
						Points.emplace_back(Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z));
						to_muchPoints += 1; FirstPoint = false;
					}
					else if (WallNormal.x == 0.f && WallNormal.y > 0.f)
					{
						//	pointvec = Vector(g_LocalPlayer->m_vecOrigin().x, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	g_Routes.emplace_back(routes(Vector(g_LocalPlayer->m_vecOrigin().x, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z), "ps"));
						Points.emplace_back(Vector(g_LocalPlayer->m_vecOrigin().x, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z));
						to_muchPoints += 1; FirstPoint = false;
					}
					else if (WallNormal.x == 0.f && WallNormal.y < 0.f)
					{
						//	pointvec = Vector(g_LocalPlayer->m_vecOrigin().x, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	g_Routes.emplace_back(routes(Vector(g_LocalPlayer->m_vecOrigin().x, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z), "ps"));
						Points.emplace_back(Vector(g_LocalPlayer->m_vecOrigin().x, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z));
						to_muchPoints += 1; FirstPoint = false;
					}
					else if (WallNormal.x < 0 && WallNormal.y == 0.f)
					{
						//		pointvec = Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y , g_LocalPlayer->m_vecOrigin().z);
						//		g_Routes.emplace_back(routes(Vector(g_LocalPlayer->m_vecOrigin().x+ 15.97803f, g_LocalPlayer->m_vecOrigin().y , g_LocalPlayer->m_vecOrigin().z), "ps"));
						Points.emplace_back(Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y, g_LocalPlayer->m_vecOrigin().z));
						to_muchPoints += 1; FirstPoint = false;
					}
					else if (WallNormal.x > 0 && WallNormal.y == 0.f)
					{
						//	pointvec = Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y, g_LocalPlayer->m_vecOrigin().z);
						//	g_Routes.emplace_back(routes(Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y, g_LocalPlayer->m_vecOrigin().z), "ps"));
						Points.emplace_back(Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y, g_LocalPlayer->m_vecOrigin().z));
						to_muchPoints += 1;	FirstPoint = false;
					}

				}
				if (g_LocalPlayer->m_fFlags() & 1 && i > 4)
				{
					if (WallNormal.x < 0 && WallNormal.y < 0.f)
					{
						pixelsurfPoint = Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	to_muchPoints += 1;			FirstPoint = false;
					}
					else if (WallNormal.x < 0 && WallNormal.y > 0.f)
					{
						pixelsurfPoint = Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	to_muchPoints += 1;			FirstPoint = false;
					}
					else if (WallNormal.x > 0 && WallNormal.y < 0.f)
					{
						pixelsurfPoint = Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	to_muchPoints += 1;		FirstPoint = false;
					}
					else if (WallNormal.x > 0 && WallNormal.y > 0.f)
					{
						pixelsurfPoint = Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	to_muchPoints += 1;		FirstPoint = false;
					}
					else if (WallNormal.x == 0.f && WallNormal.y > 0.f)
					{
						pixelsurfPoint = Vector(g_LocalPlayer->m_vecOrigin().x, g_LocalPlayer->m_vecOrigin().y - 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						////	to_muchPoints += 1;		FirstPoint = false;
					}
					else if (WallNormal.x == 0.f && WallNormal.y < 0.f)
					{
						pixelsurfPoint = Vector(g_LocalPlayer->m_vecOrigin().x, g_LocalPlayer->m_vecOrigin().y + 15.97803f, g_LocalPlayer->m_vecOrigin().z);
						//	to_muchPoints += 1;		FirstPoint = false;
					}
					else if (WallNormal.x < 0 && WallNormal.y == 0.f)
					{
						pixelsurfPoint = Vector(g_LocalPlayer->m_vecOrigin().x + 15.97803f, g_LocalPlayer->m_vecOrigin().y, g_LocalPlayer->m_vecOrigin().z);
						//	to_muchPoints += 1;		FirstPoint = false;
					}
					else if (WallNormal.x > 0 && WallNormal.y == 0.f)
					{
						pixelsurfPoint = Vector(g_LocalPlayer->m_vecOrigin().x - 15.97803f, g_LocalPlayer->m_vecOrigin().y, g_LocalPlayer->m_vecOrigin().z);
						//	to_muchPoints += 1;		FirstPoint = false;
					}

				}
			}


			cmd->buttons = buttons;
			cmd->forwardmove = ForwardMove;
			FirstPoint = false;
			if (Points.size() > 0)
			{
				for (int i = 0; i < Points.size() - 1; ) {
					if ((int)Points[i].z - (int)Points[i + 1].z == 1) {
						Points.erase(Points.begin() + i);
					}
					else {
						++i;
					}
				}

			}

		}
	}
	else
	{
		if (Points.size() > 0)
		{
			for (int i = 0; i < Points.size(); i++)
			{
				Vector render{};
				if (!g_DebugOverlay->ScreenPosition(Points.at(i), render))
				{

					InCross = InCrosshair(render.x - 6.f, render.y - 6.f, 12.f, 12.f);

					Render::Get().RenderCircleFilled(render.x, render.y, InCross ? 18.f : 12.f, 128, Color(100, 100, 100, 100));
					Render::Get().RenderCircle(render.x, render.y, InCross ? 18.f : 12.f, 128, InCross ? Color(255, 255, 255, 255) : Color(g_Options.accentcoll), 1.f);

					if (InCross)
					{
						if (GetAsyncKeyState(g_Options.setpointps) & 0x01)
						{
							pointvec = Points.at(i);
						}
						if (GetAsyncKeyState(g_Options.KEYCHECKSURFLOLKEKPoint) & 0x01)
						{
							PointsCheck.emplace_back(Points.at(i), g_EngineClient->GetLevelNameShort());
						}

					}


				}
			}
		}
		if (GetAsyncKeyState(g_Options.posloska))
		{
			if (!StartPos.is_zero() && !EndPos.is_zero())
			{
				Vector Start2d, End2d{};
				if (!g_DebugOverlay->ScreenPosition(StartPos, Start2d))
				{
					if (!g_DebugOverlay->ScreenPosition(EndPos, End2d))
					{
						Render::Get().RenderLine(Start2d.x, Start2d.y, End2d.x, End2d.y, redLine ? Color(255, 0, 0) : Color(255, 255, 255, 255), 1.f);

					}
				}
			}
		}
	}

}


#include "mmsystem.h"
#include "../helpers/Firstk.h"
#include "../helpers/Seck.h"
#include "../helpers/Thrk.h"
#include "../helpers/forthk.h"
#include "../helpers/fithhk.h"
#include "../helpers/sixk.h"
#include "../helpers/sevenk.h"
#pragma comment(lib,"Winmm.lib")
#include <random>

void GetKillSound(int kill)
{
	if (kill == 1)
	{
		PlaySound((LPCSTR)firs1, NULL, SND_MEMORY | SND_ASYNC);
	}
	if (kill == 2)
	{
		std::random_device rd;  // Источник энтропии
		std::mt19937 gen(rd()); // Генератор случайных чисел на основе Mersenne Twister
		std::uniform_int_distribution<> distrib(0, 6); // Диапазон от 0 до 6 включительно

		// Генерация случайного числа
		int random_number = distrib(gen);
		int random = random_number;
		if (random == 0)
			PlaySound((LPCSTR)sec1, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 1)
			PlaySound((LPCSTR)sec2, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 2)
			PlaySound((LPCSTR)sec3, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 3)
			PlaySound((LPCSTR)sec4, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 4)
			PlaySound((LPCSTR)sec5, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 5)
			PlaySound((LPCSTR)sec6, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 6)
			PlaySound((LPCSTR)sec7, NULL, SND_MEMORY | SND_ASYNC);
	}
	if (kill == 3)
	{
		std::random_device rd;  // Источник энтропии
		std::mt19937 gen(rd()); // Генератор случайных чисел на основе Mersenne Twister
		std::uniform_int_distribution<> distrib(0, 3); // Диапазон от 0 до 6 включительно

		// Генерация случайного числа
		int random_number = distrib(gen);
		int random = random_number;
		if (random == 0)
			PlaySound((LPCSTR)thr1, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 1)
			PlaySound((LPCSTR)thr2, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 2)
			PlaySound((LPCSTR)thr3, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 3)
			PlaySound((LPCSTR)thr4, NULL, SND_MEMORY | SND_ASYNC);
	}
	if (kill == 4)
	{
		std::random_device rd;  // Источник энтропии
		std::mt19937 gen(rd()); // Генератор случайных чисел на основе Mersenne Twister
		std::uniform_int_distribution<> distrib(0, 5); // Диапазон от 0 до 6 включительно

		// Генерация случайного числа
		int random_number = distrib(gen);
		int random = random_number;
		if (random == 0)
			PlaySound((LPCSTR)forth1, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 1)
			PlaySound((LPCSTR)forth2, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 2)
			PlaySound((LPCSTR)forth3, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 3)
			PlaySound((LPCSTR)forth4, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 4)
			PlaySound((LPCSTR)forth5, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 5)
			PlaySound((LPCSTR)forth6, NULL, SND_MEMORY | SND_ASYNC);
	}
	if (kill == 5)
	{
		std::random_device rd;  // Источник энтропии
		std::mt19937 gen(rd()); // Генератор случайных чисел на основе Mersenne Twister
		std::uniform_int_distribution<> distrib(0, 3); // Диапазон от 0 до 6 включительно

		// Генерация случайного числа
		int random_number = distrib(gen);
		int random = random_number;
		if (random == 0)
			PlaySound((LPCSTR)fiv1, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 1)
			PlaySound((LPCSTR)fiv2, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 2)
			PlaySound((LPCSTR)fiv3, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 3)
			PlaySound((LPCSTR)fiv4, NULL, SND_MEMORY | SND_ASYNC);
	}
	if (kill == 6)
	{
		std::random_device rd;  // Источник энтропии
		std::mt19937 gen(rd()); // Генератор случайных чисел на основе Mersenne Twister
		std::uniform_int_distribution<> distrib(0, 3); // Диапазон от 0 до 6 включительно

		// Генерация случайного числа
		int random_number = distrib(gen);
		int random = random_number;
		if (random == 0)
			PlaySound((LPCSTR)sixh1, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 1)
			PlaySound((LPCSTR)sixh2, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 2)
			PlaySound((LPCSTR)sixh3, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 3)
			PlaySound((LPCSTR)sixh4, NULL, SND_MEMORY | SND_ASYNC);
	}
	if (kill == 7)
	{
		std::random_device rd;  // Источник энтропии
		std::mt19937 gen(rd()); // Генератор случайных чисел на основе Mersenne Twister
		std::uniform_int_distribution<> distrib(0, 3); // Диапазон от 0 до 6 включительно

		// Генерация случайного числа
		int random_number = distrib(gen);
		int random = random_number;
		if (random == 0)
			PlaySound((LPCSTR)seven1, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 1)
			PlaySound((LPCSTR)seven2, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 2)
			PlaySound((LPCSTR)seven3, NULL, SND_MEMORY | SND_ASYNC);
		if (random == 3)
			PlaySound((LPCSTR)seven4, NULL, SND_MEMORY | SND_ASYNC);
	}
}
void BunnyHop::KillSound(IGameEvent* pEvent)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
	{
		killCounter = 1;
		return;
	}
	
	int attackerid = pEvent->GetInt("attacker");
	int userid = pEvent->GetInt("userid");
	C_BasePlayer* attacker = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(attackerid)));
	C_BasePlayer* target = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(userid)));

	if (!attacker  && !target)
		return;
	int v1 = attacker->EntIndex();
	int v2 = g_LocalPlayer->EntIndex();
	if ((v1 != v2))
		return;

	GetKillSound(killCounter);
	killCounter += 1;
	if (killCounter > 7)
		killCounter = 7;
}