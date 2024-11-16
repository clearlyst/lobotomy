#include "discord_rpc_init.h"
#include <ctime>
#include <string>
#include "valve_sdk/csgostructs.hpp"

void Discord::Initialize()
{
    DiscordEventHandlers Handle;
    memset(&Handle, 0, sizeof(Handle));
    Discord_Initialize("1251192197999755277", &Handle, 1, NULL);
}

void Discord::Update()
{
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    static auto elapsed = std::time(nullptr);

    discordPresence.state = "playing cs:go"; /* тут типо можешь поменять чет типо playing with gloss вся хуйня */
    discordPresence.largeImageKey = "photo_2024-06-14_18-19-26";
    discordPresence.largeImageText = u8"Пошел нахуй!";
    discordPresence.startTimestamp = elapsed;
    Discord_UpdatePresence(&discordPresence);
}
Discord g_Discord;