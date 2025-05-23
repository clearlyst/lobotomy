#pragma once

#include "../Math/VMatrix.hpp"
class bf_read;
class bf_write;
class INetMessage
{
public:
    virtual	~INetMessage() {};

    virtual void SetNetChannel(void* netchan) = 0;
    virtual void SetReliable(bool state) = 0;

    virtual bool Process(void) = 0;

    virtual	bool ReadFromBuffer(bf_read& buffer) = 0;
    virtual	bool WriteToBuffer(bf_write& buffer) = 0;

    virtual bool IsReliable(void) const = 0;

    virtual int GetType(void) const = 0;
    virtual int GetGroup(void) const = 0;
    virtual const char* GetName(void) const = 0;
    virtual void* GetNetChannel(void) const = 0;
    virtual const char* ToString(void) const = 0;
};

class cnetmsg_clientinfo {
private:
    char __PAD0[0x8];
public:
    uint32_t send_table_crc;
    uint32_t server_count;
    bool is_hltv;
    bool is_replay;
    uint32_t friends_id;
};

template<typename T>
class CNetMessagePB : public INetMessage, public T {};
using cclcmsg_clientinfo_t = CNetMessagePB<cnetmsg_clientinfo>;
class IPanel;
class C_BaseEntity;

enum class ClearFlags_t
{
    VIEW_CLEAR_COLOR = 0x1,
    VIEW_CLEAR_DEPTH = 0x2,
    VIEW_CLEAR_FULL_TARGET = 0x4,
    VIEW_NO_DRAW = 0x8,
    VIEW_CLEAR_OBEY_STENCIL = 0x10,
    VIEW_CLEAR_STENCIL = 0x20,
};


enum class MotionBlurMode_t
{
    MOTION_BLUR_DISABLE = 1,
    MOTION_BLUR_GAME = 2,
    MOTION_BLUR_SFM = 3
};

class CViewSetup
{
public:
    __int32   x;                  //0x0000 
    __int32   x_old;              //0x0004 
    __int32   y;                  //0x0008 
    __int32   y_old;              //0x000C 
    __int32   width;              //0x0010 
    __int32   width_old;          //0x0014 
    __int32   height;             //0x0018 
    __int32   height_old;         //0x001C 
    char      pad_0x0020[0x90];   //0x0020
    float     fov;                //0x00B0 
    float     viewmodel_fov;      //0x00B4 
    Vector    origin;             //0x00B8 
    QAngle    angles;             //0x00C4 
    float		near_z;
    float		far_z;
    float		near_viewmodel_z;
    float		far_viewmodel_z;
    float		aspect_ratio;
    float		near_blur_depth;
    float		near_focus_depth;
    float		far_focus_depth;
    float		far_blur_depth;
    float		near_blur_radius;
    float		far_blur_radius;
    float		do_f_quality;
    int			motion_blur_mode;
    float		shutter_time;
    Vector		shutter_open_position;
    QAngle		shutter_open_angles;
    Vector		shutter_close_position;
    QAngle		shutter_close_angles;
    float		off_center_top;
    float		off_center_bottom;
    float		off_center_left;
    float		off_center_right;
    bool		off_center : 1;
    bool		render_to_subrect_of_larger_screen : 1;
    bool		do_bloom_and_tone_mapping : 1;
    bool		do_depth_of_field : 1;
    bool		hdr_target : 1;
    bool		draw_world_normal : 1;
    bool		cull_font_faces : 1;
    bool		cache_full_scene_state : 1;
    bool		csm_view : 1;
    char      pad_0x00D0[0x7C];   //0x00D0

};//Size=0x014C

class IClientMode
{
public:
    virtual             ~IClientMode() {}
    virtual int         ClientModeCSNormal(void *) = 0;
    virtual void        Init() = 0;
    virtual void        InitViewport() = 0;
    virtual void        Shutdown() = 0;
    virtual void        Enable() = 0;
    virtual void        Disable() = 0;
    virtual void        Layout() = 0;
    virtual IPanel*     GetViewport() = 0;
    virtual void*       GetViewportAnimationController() = 0;
    virtual void        ProcessInput(bool bActive) = 0;
    virtual bool        ShouldDrawDetailObjects() = 0;
    virtual bool        ShouldDrawEntity(C_BaseEntity *pEnt) = 0;
    virtual bool        ShouldDrawLocalPlayer(C_BaseEntity *pPlayer) = 0;
    virtual bool        ShouldDrawParticles() = 0;
    virtual bool        ShouldDrawFog(void) = 0;
    virtual void        OverrideView(CViewSetup *pSetup) = 0;
    virtual int         KeyInput(int down, int keynum, const char *pszCurrentBinding) = 0;
    virtual void        StartMessageMode(int iMessageModeType) = 0;
    virtual IPanel*     GetMessagePanel() = 0;
    virtual void        OverrideMouseInput(float *x, float *y) = 0;
    virtual bool        CreateMove(float flInputSampleTime, void* usercmd) = 0;
    virtual void        LevelInit(const char *newmap) = 0;
    virtual void        LevelShutdown(void) = 0;
};