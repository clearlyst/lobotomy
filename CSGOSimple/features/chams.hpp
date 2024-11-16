#pragma once

#include "../singleton.hpp"
#include "../valve_sdk/sdk.hpp"

class IMatRenderContext;
struct DrawModelState_t;
struct ModelRenderInfo_t;
class matrix3x4_t;
class IMaterial;
class Color;

class Chams
    : public Singleton<Chams>
{
    friend class Singleton<Chams>;


public:
    
    void initialize();

    void run(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix);

    void run_bt(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix);

};