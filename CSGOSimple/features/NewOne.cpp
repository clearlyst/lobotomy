#include "NewOne.h"
#include "../options.hpp"

#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include <filesystem>
#include "../../CSGOSimple/helpers/prediction.hpp"
#include "chat_hud.h"
#include "../imgui/imgui.h"
#include <deque>
#include "Notify.h"
static auto prediction = new PredictionSystem();

Vector edgebug_velocity_backup;
void NewOne::PrePredictionEdgeBug(CUserCmd* cmd)
{
	if (!g_LocalPlayer)
		return;
	if (!g_LocalPlayer->IsAlive())
		return;
	BackupVelocity   = g_LocalPlayer->m_vecVelocity();
	BackupOrigin     = g_LocalPlayer->abs_origin();
	BackupMoveType    = g_LocalPlayer->m_nMoveType();
	BackupFlags     = g_LocalPlayer->m_fFlags();
	BackupQAngle     = cmd->viewangles;
	edgebug_velocity_backup = g_LocalPlayer->m_vecVelocity();

}



bool NewOne::EdgeBugCheck(CUserCmd* cmd)
{

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || !g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
	{
		return false;
	}

	if (const auto mt = g_LocalPlayer->m_nMoveType(); mt == MOVETYPE_LADDER || mt == MOVETYPE_NOCLIP)
	{
		return false;
	}
	static auto sv_gravity = g_CVar->FindVar("sv_gravity")->GetFloat();

	float gravity_vel = (sv_gravity * 0.5f * g_GlobalVars->interval_per_tick);

	if (edgebug_velocity_backup.z < -gravity_vel && round(g_LocalPlayer->m_vecVelocity().z) == -round(gravity_vel))
	{
		return true;
	}

	if (edgebug_velocity_backup.z < -6.0f && g_LocalPlayer->m_vecVelocity().z > edgebug_velocity_backup.z && g_LocalPlayer->m_vecVelocity().z < -6.0f)
	{
		float velocty_before_engineprediction = g_LocalPlayer->m_vecVelocity().z;

		prediction->StartPrediction(cmd);

		static auto sv_gravity_after_engineprediction = g_CVar->FindVar("sv_gravity");
		const float gravity_velocity_constant = roundf(-sv_gravity_after_engineprediction->GetFloat() * g_GlobalVars->interval_per_tick + velocty_before_engineprediction);

		if (gravity_velocity_constant == round(g_LocalPlayer->m_vecVelocity().z))
		{
			return true;
		}
	}

	

	return false;
}
int edgebug_tick;
int edgebug_prediction_ticks = 0;
int edgebug_prediction_timestamp = 0;
int edgebug_mouse_offset = 0;
void NewOne::ReStorePrediction()
{
	memesclass->restoreEntityToPredictedFrame(0, g_Prediction->Split->nCommandsPredicted - 1);
}
struct NewCmd1
{
	QAngle viewangle;
	float forwardmove;
	float sideMove;
	int buttons;
	Vector origin;
};
NewCmd1 NewCmd[128];
static bool DuckEd = false;
void NewOne::EdgeBugPostPredict(CUserCmd* cmd)
{
	if (!g_Options.edge_bug || !GetAsyncKeyState(g_Options.edge_bug_key))
	{
		EdgeBug_Founded = false;
		EdgeBug_Ticks = 0;
		DuckEd = false;
		return;
	}

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
	{
		EdgeBug_Founded = false;
		EdgeBug_Ticks = 0;
		return;
	}

	if (DuckEd)
	{
		if (!(iFlagsBackup & 1))
			cmd->buttons &= ~IN_JUMP;
		else
			cmd->buttons |= IN_JUMP;
		cmd->buttons |= IN_DUCK;
	}

	if (BackupFlags & 1)
	{
		DuckEd = false;
		return;
	}
	if (g_Options.MegaEdgeBug)
		g_Options.deltaScaler = 179.f;

	auto isLargeAngleDifference = [](float yaw1, float yaw2) -> bool {
		float yawDifference = std::abs(yaw1 - yaw2);

		// Приводим yawDifference в диапазон от -180 до 180
		if (yawDifference > 180.0f)
			yawDifference = 360.0f - yawDifference;

		return yawDifference >= g_Options.deltaScaler; // Проверяем, превышает ли разница 90 градусов
		};
	static int LastType = 0;
	static QAngle NewAngle;
	int BackupButtons = cmd->buttons;
	float BackupForwardMove = cmd->forwardmove;
	float BackupSideMove = cmd->sidemove;
	QAngle angViewPointBackup = cmd->viewangles;
	float DeltaSalier = 0.f;
	if (g_Options.DeltaType == 0)
		DeltaSalier = 1.f;
	if (g_Options.DeltaType == 1)
		DeltaSalier = 0.8f;
	if (g_Options.DeltaType == 2)
		DeltaSalier = 0.5f;
	if (g_Options.DeltaType == 3)
		DeltaSalier = 0.25f;
	if (g_Options.MegaEdgeBug)
		DeltaSalier = 1.f;
	QAngle DeltaAngle = (angViewPointBackup - NewAngle) * DeltaSalier;
	NewAngle = cmd->viewangles;

	if (!EdgeBug_Founded)
	{
		for (int v = 0; v < g_Options.EdgeBugCircle; v++)
		{
			if (LastType)
			{
				v = LastType;
				LastType = 0;
			}
			ReStorePrediction();
			edgebug_velocity_backup = g_LocalPlayer->m_vecVelocity();

			cmd->viewangles = angViewPointBackup;
			cmd->forwardmove = BackupForwardMove;
			cmd->sidemove = BackupSideMove;
			cmd->buttons = BackupButtons;
			QAngle currentAngle = cmd->viewangles;
			for (int i = 0; i < g_Options.EdgeBugTicks; i++)
			{
				if (EdgeBug_Founded || g_LocalPlayer->m_fFlags() & 1 || g_LocalPlayer->m_vecVelocity().z > 0.f)
					break;

				if (g_Options.MegaEdgeBug)
				{
					if (v == 0)
					{
						cmd->buttons &= ~IN_DUCK;
						cmd->forwardmove = 0;
						cmd->sidemove = 0;
					}
					if (v == 1)
					{
						cmd->buttons |= IN_DUCK;
						cmd->forwardmove = 0;
						cmd->sidemove = 0;
					}
					if (v == 2)
					{
						cmd->buttons &= ~IN_DUCK;
					
						{
							cmd->forwardmove = BackupForwardMove;
							cmd->sidemove = BackupSideMove;

							if (!isLargeAngleDifference(currentAngle.yaw, angViewPointBackup.yaw))
								currentAngle = (currentAngle + DeltaAngle).normalize().clamped();
							cmd->viewangles = currentAngle;
							if (g_Options.SiletEdgeBug)
							{
								Math::start_movement_fix(cmd);
								cmd->viewangles = angViewPointBackup;
								Math::end_movement_fix(cmd);
							}
						}

					}
					if (v == 3)
					{
						cmd->buttons |= IN_DUCK;
						
						{
							cmd->forwardmove = BackupForwardMove;
							cmd->sidemove = BackupSideMove;
							if (!isLargeAngleDifference(currentAngle.yaw, angViewPointBackup.yaw))
								currentAngle = (currentAngle + DeltaAngle).normalize().clamped();
							cmd->viewangles = currentAngle;
							if (g_Options.SiletEdgeBug)
							{
								Math::start_movement_fix(cmd);
								cmd->viewangles = angViewPointBackup;
								Math::end_movement_fix(cmd);
							}
						}
					}
					if (v == 4)
					{
						cmd->buttons &= ~IN_DUCK;
					
						{
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
					}
					if (v == 5)
					{
						cmd->buttons |= IN_DUCK;

						{
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
					}
				}
				else
				{
					if (v == 0)
					{
						cmd->buttons &= ~IN_DUCK;
						cmd->forwardmove = 0;
						cmd->sidemove = 0;
					}
					if (v == 1)
					{
						cmd->buttons |= IN_DUCK;
						cmd->forwardmove = 0;
						cmd->sidemove = 0;
					}
					if (v == 2)
					{
						cmd->buttons &= ~IN_DUCK;
						if (g_Options.AutoStrafeEdgeBug)
						{
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
						else
						{
							cmd->forwardmove = BackupForwardMove;
							cmd->sidemove = BackupSideMove;

							if (!isLargeAngleDifference(currentAngle.yaw, angViewPointBackup.yaw))
								currentAngle = (currentAngle + DeltaAngle).normalize().clamped();
							cmd->viewangles = currentAngle;
							if (g_Options.SiletEdgeBug)
							{
								Math::start_movement_fix(cmd);
								cmd->viewangles = angViewPointBackup;
								Math::end_movement_fix(cmd);
							}
						}

					}
					if (v == 3)
					{
						cmd->buttons |= IN_DUCK;
						if (g_Options.AutoStrafeEdgeBug)
						{
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
						else
						{
							cmd->forwardmove = BackupForwardMove;
							cmd->sidemove = BackupSideMove;
							if (!isLargeAngleDifference(currentAngle.yaw, angViewPointBackup.yaw))
								currentAngle = (currentAngle + DeltaAngle).normalize().clamped();
							cmd->viewangles = currentAngle;
							if (g_Options.SiletEdgeBug)
							{
								Math::start_movement_fix(cmd);
								cmd->viewangles = angViewPointBackup;
								Math::end_movement_fix(cmd);
							}
						}
					}
				}

				NewCmd[i].forwardmove = cmd->forwardmove;
				NewCmd[i].sideMove = cmd->sidemove;
				NewCmd[i].buttons = cmd->buttons;
				NewCmd[i].viewangle = cmd->viewangles;
				NewCmd[i].origin = g_LocalPlayer->m_vecOrigin();

				if (EdgeBug_Founded || g_LocalPlayer->m_fFlags() & 1 || g_LocalPlayer->m_vecVelocity().z > 0.f)
					break;

				prediction->StartPrediction(cmd);
				EdgeBug_Founded = EdgeBugCheck(cmd);
				edgebug_velocity_backup = g_LocalPlayer->m_vecVelocity();

				if (EdgeBug_Founded)
				{
					EdgeBug_Ticks = i;
					LastType = v;
					edgebug_tick = g_GlobalVars->tickcount + (i + 1);
					edgebug_prediction_ticks = i;
					edgebug_prediction_timestamp = g_GlobalVars->tickcount;
					edgebug_mouse_offset = std::abs(cmd->mousedx);
				}
			}
			cmd->viewangles = angViewPointBackup;
			cmd->forwardmove = BackupForwardMove;
			cmd->sidemove = BackupSideMove;
			cmd->buttons = BackupButtons;
		}
	}
	if(EdgeBug_Founded)
	{
		ReStorePrediction();
		static int Hits = 0;
		Vector org = g_LocalPlayer->m_vecOrigin();
		float dist = org.DistTo(NewCmd[Hits].origin);
	
		if (Hits > EdgeBug_Ticks || dist > 1.f)
		{
			

			Hits = 0;
			EdgeBug_Founded = false;
			
			return;
		}
		if (NewCmd[Hits].buttons & IN_DUCK)
		{
			cmd->buttons |= IN_DUCK;
			DuckEd = true;
		}
		else
			cmd->buttons &= ~IN_DUCK;
		if (iFlagsBackup & 1)
			cmd->buttons |= IN_JUMP;
		else
			cmd->buttons &= ~IN_JUMP;
		cmd->forwardmove = NewCmd[Hits].forwardmove;
		cmd->sidemove = NewCmd[Hits].sideMove;
		cmd->viewangles = NewCmd[Hits].viewangle;
		if (!g_Options.SiletEdgeBug && LastType > 2 && !g_Options.AutoStrafeEdgeBug)
			g_EngineClient->SetViewAngles(&cmd->viewangles);
		Hits += 1;
	}

}

struct DATAFORDETECT
{
	Vector velocity;
	bool onground;
};

std::deque<DATAFORDETECT> VelocitiesForDetection;

void NewOne::edgebug_detect(CUserCmd* cmd)
{
	if (!g_Options.EdgeBugDetectChat)
		return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || !g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
	{
		
		return;
	}

	if (const auto fg = g_LocalPlayer->m_fFlags(); fg == FL_INWATER || fg == FL_WATERJUMP)
	{
		
		return;
	}

	if (const auto mt = g_LocalPlayer->m_nMoveType(); mt == MOVETYPE_LADDER || mt == MOVETYPE_NOCLIP)
	{
		
		return;
	}

	DATAFORDETECT d;
	d.velocity = g_LocalPlayer->m_vecVelocity();
	d.onground = g_LocalPlayer->m_fFlags() & FL_ONGROUND;

	VelocitiesForDetection.push_front(d);

	if (VelocitiesForDetection.size() > 2)
		VelocitiesForDetection.pop_back();

	static auto sv_Gravity = g_CVar->FindVar("sv_gravity");
	float negativezvel = sv_Gravity->GetFloat() * -0.5f * g_GlobalVars->interval_per_tick;

	if (VelocitiesForDetection.size() == 2 && ((round(negativezvel * 100.f) == round(VelocitiesForDetection.at(0).velocity.z * 100.f) && VelocitiesForDetection.at(1).velocity.z < negativezvel && !VelocitiesForDetection.at(1).onground && !VelocitiesForDetection.at(0).onground)))
	{
		VelocitiesForDetection.clear();
		notification::run(" ", "#_print_edgebug", true, true, true);
		
	}
}

void NewOne::edgebug_lock(float& x, float& y)
{
	

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || !g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
	{
		
		return;
	}

	if (const auto fg = g_LocalPlayer->m_fFlags(); fg == FL_INWATER || fg == FL_WATERJUMP)
	{
		
		return;
	}

	if (const auto mt = g_LocalPlayer->m_nMoveType(); mt == MOVETYPE_LADDER || mt == MOVETYPE_NOCLIP)
	{
		
		return;
	}

	if (EdgeBug_Founded)
	{
		if (x != 0.0)
		{
			float v3 = (float)(edgebug_prediction_timestamp + edgebug_prediction_ticks - g_GlobalVars->tickcount) / x;
			float v4 = 100.0 / (atan2f(edgebug_prediction_ticks, v3) + 100.0 + (float)(g_Options.EdgeBugMouseLock * edgebug_mouse_offset));
			if (!isnan(v4))
				x = x * v4;
		}
	}
}