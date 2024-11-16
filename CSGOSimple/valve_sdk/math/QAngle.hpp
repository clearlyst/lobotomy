#pragma once
#include <filesystem>

class QAngle
{
public:
    QAngle(void)
    {
        Init();
    }
    QAngle(float X, float Y, float Z)
    {
        Init(X, Y, Z);
    }
    QAngle(const float* clr)
    {
        Init(clr[0], clr[1], clr[2]);
    }

    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f)
    {
        pitch = ix;
        yaw = iy;
        roll = iz;
    }

    float operator[](int i) const
    {
        return ((float*)this)[i];
    }
    float& operator[](int i)
    {
        return ((float*)this)[i];
    }
    QAngle clamped() {
        QAngle clamped = *this;
        clamped.Clamp();
        return clamped;
    }
    QAngle& operator+=(const QAngle& v)
    {
        pitch += v.pitch; yaw += v.yaw; roll += v.roll;
        return *this;
    }
    QAngle& operator-=(const QAngle& v)
    {
        pitch -= v.pitch; yaw -= v.yaw; roll -= v.roll;
        return *this;
    }
    QAngle& operator*=(float fl)
    {
        pitch *= fl;
        yaw *= fl;
        roll *= fl;
        return *this;
    }
    QAngle& operator*=(const QAngle& v)
    {
        pitch *= v.pitch;
        yaw *= v.yaw;
        roll *= v.roll;
        return *this;
    }
    QAngle& normalize() noexcept // норм тема
    {
        this->pitch = std::isfinite(this->pitch) ? std::remainder(this->pitch, 360.f) : 0.0f;
        this->yaw = std::isfinite(this->yaw) ? std::remainder(this->yaw, 360.f) : 0.0f;
        this->roll = 0.0f;
        return *this;
    }
    QAngle& operator/=(const QAngle& v)
    {
        pitch /= v.pitch;
        yaw /= v.yaw;
        roll /= v.roll;
        return *this;
    }
    QAngle& operator+=(float fl)
    {
        pitch += fl;
        yaw += fl;
        roll += fl;
        return *this;
    }
    void Clamp()
    {
        this->pitch = std::clamp(this->pitch, -89.f, 89.f);
        this->yaw = std::clamp(std::remainder(this->yaw, 360.0f), -180.0f, 180.0f);
        this->roll = std::clamp(this->roll, -50.f, 50.f);
    }

    QAngle& operator/=(float fl)
    {
        pitch /= fl;
        yaw /= fl;
        roll /= fl;
        return *this;
    }
    QAngle& operator-=(float fl)
    {
        pitch -= fl;
        yaw -= fl;
        roll -= fl;
        return *this;
    }

    QAngle& operator=(const QAngle &vOther)
    {
        pitch = vOther.pitch; yaw = vOther.yaw; roll = vOther.roll;
        return *this;
    }

    QAngle operator-(void) const
    {
        return QAngle(-pitch, -yaw, -roll);
    }
    QAngle operator+(const QAngle& v) const
    {
        return QAngle(pitch + v.pitch, yaw + v.yaw, roll + v.roll);
    }
    QAngle operator-(const QAngle& v) const
    {
        return QAngle(pitch - v.pitch, yaw - v.yaw, roll - v.roll);
    }
    QAngle operator*(float fl) const
    {
        return QAngle(pitch * fl, yaw * fl, roll * fl);
    }
    QAngle operator*(const QAngle& v) const
    {
        return QAngle(pitch * v.pitch, yaw * v.yaw, roll * v.roll);
    }
    QAngle operator/(float fl) const
    {
        return QAngle(pitch / fl, yaw / fl, roll / fl);
    }
    QAngle operator/(const QAngle& v) const
    {
        return QAngle(pitch / v.pitch, yaw / v.yaw, roll / v.roll);
    }

    float Length() const
    {
        return sqrt(pitch*pitch + yaw*yaw + roll*roll);
    }
    float LengthSqr(void) const
    {
        return (pitch*pitch + yaw*yaw + roll*roll);
    }
    bool IsZero(float tolerance = 0.01f) const
    {
        return (pitch > -tolerance && pitch < tolerance &&
            yaw > -tolerance && yaw < tolerance &&
            roll > -tolerance && roll < tolerance);
    }
    bool validator()
    {
        return this->pitch == 404.f && this->yaw == 404.f && this->roll == 404.f;
    }
	float Normalize() const // долбоебизм
	{
		QAngle res = *this;
		float l = res.Length();
		if (l != 0.0f)
		{
			res /= l;
		}
		else
		{
			res[0] = res[1] = res[2] = 0.0f;
		}
		return l;
	}

    float pitch;
    float yaw;
    float roll;
};

inline QAngle operator*(float lhs, const QAngle& rhs)
{
    return rhs * lhs;
}
inline QAngle operator/(float lhs, const QAngle& rhs)
{
    return rhs / lhs;
}
