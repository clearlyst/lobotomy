#pragma once

#include "../Math/Vector.hpp"
#include "../Math/QAngle.hpp"
#include "../Math/VMatrix.hpp"

class OverlayText_t;
#define get_virtual_fn( index, function_name, type, ... )  auto function_name { return CallVFunction<type>( this, index )( this, __VA_ARGS__ ); };
template< typename T >
T getvfunc(void* vTable, int iIndex)
{
    return (*(T**)vTable)[iIndex];
}
class IVDebugOverlay
{
public:
    virtual void            __unkn() = 0;
    virtual void            AddEntityTextOverlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char *format, ...) = 0;
    virtual void            AddBoxOverlay(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration) = 0;
    virtual void            AddSphereOverlay(const Vector& vOrigin, float flRadius, int nTheta, int nPhi, int r, int g, int b, int a, float flDuration) = 0;
    virtual void            AddTriangleOverlay(const Vector& p1, const Vector& p2, const Vector& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
    virtual void            AddLineOverlay(const Vector& origin, const Vector& dest, int r, int g, int b, bool noDepthTest, float duration) = 0;
    virtual void            AddTextOverlay(const Vector& origin, float duration, const char *format, ...) = 0;
    virtual void            AddTextOverlay(const Vector& origin, int line_offset, float duration, const char *format, ...) = 0;
    virtual void            AddScreenTextOverlay(float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char *text) = 0;
    virtual void            AddSweptBoxOverlay(const Vector& start, const Vector& end, const Vector& mins, const Vector& max, const QAngle & angles, int r, int g, int b, int a, float flDuration) = 0;
    virtual void            AddGridOverlay(const Vector& origin) = 0;
    virtual void            AddCoordFrameOverlay(const matrix3x4_t& frame, float flScale, int vColorTable[3][3] = NULL) = 0;
    virtual int             ScreenPosition(const Vector& point, Vector& screen) = 0;
    virtual int             ScreenPosition(float flXPos, float flYPos, Vector& screen) = 0;
    virtual OverlayText_t*  GetFirst(void) = 0;
    virtual OverlayText_t*  GetNext(OverlayText_t *current) = 0;
    virtual void            ClearDeadOverlays(void) = 0;
    virtual void            ClearAllOverlays() = 0;
    virtual void            AddTextOverlayRGB(const Vector& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char *format, ...) = 0;
    virtual void            AddTextOverlayRGB(const Vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char *format, ...) = 0;
    virtual void            AddLineOverlayAlpha(const Vector& origin, const Vector& dest, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
    virtual void            AddBoxOverlay2(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, const uint8_t* faceColor, const uint8_t* edgeColor, float duration) = 0;
    virtual void            PurgeTextOverlays() = 0;
    virtual void            DrawPill(const Vector& mins, const Vector& max, float& diameter, int r, int g, int b, int a, float duration) = 0;
    bool world_to_screen(const Vector& in, Vector& out) {
        using original_fn = int(__thiscall*)(IVDebugOverlay*, const Vector&, Vector&);
        int return_value = (*(original_fn**)this)[13](this, in, out);
        return static_cast<bool>(return_value != 1);
    }

    get_virtual_fn(24, add_capsule_overlay(Vector& mins, Vector& maxs, float pillradius, Color color, float duration), void(__thiscall*)(void*, Vector&, Vector&, float&, int, int, int, int, float), mins, maxs, pillradius, color.r(), color.g(), color.b(), color.a(), duration);

    void BoxOverlay(const Vector& origin, const Vector& mins, const Vector& max, Vector const& orientation, int r, int g, int b, int a, float duration)
    {
        using Fn = void(__thiscall*)(void*, const Vector&, const Vector&, const Vector&, Vector const&, int, int, int, int, float);
        getvfunc <Fn>(this, 1)(this, origin, mins, max, orientation, r, g, b, a, duration);
    }
};