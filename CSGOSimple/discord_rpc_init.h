#pragma once
#include "../CSGOSimple/discord-rpc-sdk-main/discord_register.h"
#include "discord-rpc-sdk-main/discord_rpc.h"
#include <Windows.h>

class Discord {
public:
    void Initialize();
    void Update();
};
extern Discord g_Discord;