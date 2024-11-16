#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../valve_sdk/sdk.hpp"

#include <DirectXMath.h>

#define RAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define DEG2RAD(x) DirectX::XMConvertToRadians(x)
#define M_PI 3.14159265358979323846
#define PI_F	((float)(M_PI)) 

namespace Math
{
	inline float FASTSQRT(float x)
	{
		unsigned int i = *(unsigned int*)&x;

		i += 127 << 23;
		// approximation of square root
		i >>= 1;
		return *(float*)&i;
	}

	float VectorDistance(const Vector& v1, const Vector& v2);
	QAngle CalcAngle(const Vector& src, const Vector& dst);
	float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle);
	template<class T>
	void Normalize3(T& vec)
	{
		for (auto i = 0; i < 2; i++) {
			while (vec[i] < -180.0f) vec[i] += 360.0f;
			while (vec[i] >  180.0f) vec[i] -= 360.0f;
		}
		vec[2] = 0.f;
	}

	QAngle Vpenivatel(const Vector& source, const Vector& destination, const QAngle& vievAngles);
    void ClampAngles(QAngle& angles);
	void FixAngles(QAngle& angles);
    void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
	void AngleVectors(const QAngle& angles, Vector& forward);
	void AngleVectors(const QAngle& angles, Vector& forward, Vector& right, Vector& up);
	void AngleVectors(const QAngle& angView, Vector* pForward, Vector* pRight, Vector* pUp);
    void VectorAngles(const Vector& forward, QAngle& angles);
    bool WorldToScreen(const Vector& in, Vector& out);
	void CorrectMovement(CUserCmd* cmd, QAngle wish_angle, QAngle old_angles);
	void vector_transform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
	inline QAngle originalAngle;
	inline float originalForwardMove, originalSideMove;

	inline void start_movement_fix(CUserCmd* cmd)
	{
		originalAngle = cmd->viewangles;
		originalForwardMove = cmd->forwardmove;
		originalSideMove = cmd->sidemove;
	}

	inline void end_movement_fix(CUserCmd* cmd)
	{
		float deltaViewAngles;
		float f1;
		float f2;

		if (originalAngle.yaw < 0.f)
			f1 = 360.0f + originalAngle.yaw;
		else
			f1 = originalAngle.yaw;

		if (cmd->viewangles.yaw < 0.0f)
			f2 = 360.0f + cmd->viewangles.yaw;
		else
			f2 = cmd->viewangles.yaw;

		if (f2 < f1)
			deltaViewAngles = abs(f2 - f1);
		else
			deltaViewAngles = 360.0f - abs(f1 - f2);

		deltaViewAngles = 360.0f - deltaViewAngles;

		cmd->forwardmove = cos(DEG2RAD(deltaViewAngles)) * originalForwardMove + cos(DEG2RAD(deltaViewAngles + 90.f)) * originalSideMove;
		cmd->sidemove = sin(DEG2RAD(deltaViewAngles)) * originalForwardMove + sin(DEG2RAD(deltaViewAngles + 90.f)) * originalSideMove;
	}
}