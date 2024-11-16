#pragma once

#include "..//options.hpp"
#include "..//valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"

class MegaPrivateAimbotOtAntohi {
public:
	void RCS(QAngle& angle);
	void run(CUserCmd* cmd);
#define CHECK_VALID( _v ) 0
	inline void VectorSubtract(const Vector& a, const Vector& b, Vector& c)
	{
		CHECK_VALID(a);
		CHECK_VALID(b);
		c.x = a.x - b.x;
		c.y = a.y - b.y;
		c.z = a.z - b.z;
	}

	inline void sinCos(float radians, PFLOAT sine, PFLOAT cosine)
	{
		__asm
		{
			fld dword ptr[radians]
			fsincos
			mov edx, dword ptr[cosine]
			mov eax, dword ptr[sine]
			fstp dword ptr[edx]
			fstp dword ptr[eax]
		}
	}
	//Чучуть асэма не помешает Приват епта // antosha (rip)
	void angleVectors(QAngle& angles, Vector& forward)
	{
		float	sp, sy, cp, cy;

		DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
		DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));

		forward.x = cp * cy;
		forward.y = cp * sy;
		forward.z = -sp;
	}
	legitbot_s settings;
	legitbot_s option;
	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle)
	{
		QAngle delta = aimAngle - viewAngle;
		Math::FixAngles(delta);
		return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
	}

	inline float GetFov(Vector vLocalOrigin, Vector vPosition, Vector vForward)
	{
		Vector vLocal;

		VectorSubtract(vPosition, vLocalOrigin, vLocal);

		vLocal.NormalizeInPlace();

		float fValue = vForward.Dot(vLocal);

		//np for kolo's math skills
		if (fValue < -1.0f)
			fValue = -1.0f;

		if (fValue > 1.0f)
			fValue = 1.0f;

		return RAD2DEG(acos(fValue));
	}


	QAngle current_punch = { 0, 0, 0 };
	QAngle last_punch = { 0, 0, 0 };
};
extern MegaPrivateAimbotOtAntohi g_Aimbot;