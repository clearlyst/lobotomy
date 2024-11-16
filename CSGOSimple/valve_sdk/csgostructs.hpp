#pragma once

#include "sdk.hpp"
#include <array>
#include <vector>
#include "../helpers/utils.hpp"

#define NETVAR(type, name, table, netvar)                           \
    type& name##() const {                                          \
        static int _##name = NetvarSys::Get().GetOffset(table, netvar);     \
        return *(type*)((uintptr_t)this + _##name);                 \
    }

#define PNETVAR(type, name, table, netvar)                           \
    type* name##() const {                                          \
        static int _##name = NetvarSys::Get().GetOffset(table, netvar);     \
        return (type*)((uintptr_t)this + _##name);                 \
    }

#define NETPROP(name, table, netvar) static RecvProp* name() \
{ \
	static auto prop_ptr = NetvarSys::Get().GetNetvarProp(table,netvar); \
	return prop_ptr; \
}

struct datamap_t;
class AnimationLayer;
class CBasePlayerAnimState;
class CCSGOPlayerAnimState;
class C_BaseEntity;

enum CSWeaponType
{
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_C4,
	WEAPONTYPE_PLACEHOLDER,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_UNKNOWN
};

class C_BaseEntity;

class CEventInfo
{
public:
	uint16_t classID; //0x0000 0 implies not in use
	char pad_0002[2]; //0x0002 
	float fire_delay; //0x0004 If non-zero, the delay time when the event should be fired ( fixed up on the client )
	char pad_0008[4]; //0x0008
	ClientClass* pClientClass; //0x000C
	void* pData; //0x0010 Raw event data
	char pad_0014[48]; //0x0014
}; //Size: 0x0044

// Created with ReClass.NET by KN4CK3R
class CHudTexture
{
public:
	char szShortName[64];    //0x0000
	char szTextureFile[64];  //0x0040
	bool bRenderUsingFont;   //0x0080
	bool bPrecached;         //0x0081
	int8_t cCharacterInFont; //0x0082
	uint8_t pad_0083[1];     //0x0083
	uint32_t hFont;          //0x0084
	int32_t iTextureId;      //0x0088
	float afTexCoords[4];    //0x008C
	uint8_t pad_009C[16];    //0x009C
};

class C_EconItemView
{
private:
	using str_32 = char[32];
public:
	NETVAR(int32_t, m_bInitialized, "DT_BaseAttributableItem", "m_bInitialized");
	NETVAR(int16_t, m_iItemDefinitionIndex, "DT_BaseAttributableItem", "m_iItemDefinitionIndex");
	std::string weapon_name_definition() {
		if (!this)
			return ("");
		int id = this->m_iItemDefinitionIndex();
		switch (id) {
		case WEAPON_DEAGLE:
			return ("desert eagle");
		case WEAPON_AUG:
			return ("aug");
		case WEAPON_G3SG1:
			return ("g3sg1");
		case WEAPON_MAC10:
			return ("mac10");
		case WEAPON_P90:
			return ("p90");
		case WEAPON_SSG08:
			return ("ssg08");
		case WEAPON_SCAR20:
			return ("scar20");
		case WEAPON_UMP45:
			return ("ump45");
		case WEAPON_ELITE:
			return ("elites");
		case WEAPON_FAMAS:
			return ("famas");
		case WEAPON_FIVESEVEN:
			return ("five seven");
		case WEAPON_GALILAR:
			return ("galil-ar");
		case WEAPON_M4A1_SILENCER:
			return ("m4a1-s");
		case WEAPON_M4A1:
			return ("m4a1");
		case WEAPON_P250:
			return ("p250");
		case WEAPON_M249:
			return ("m249");
		case WEAPON_XM1014:
			return ("xm1014");
		case WEAPON_GLOCK:
			return ("glock 18");
		case WEAPON_USP_SILENCER:
			return ("usp-s");
		case WEAPON_HKP2000:
			return ("p2000");
		case WEAPON_AK47:
			return ("ak-47");
		case WEAPON_AWP:
			return ("awp");
		case WEAPON_BIZON:
			return ("bizon");
		case WEAPON_MAG7:
			return ("mag-7");
		case WEAPON_NEGEV:
			return ("negev");
		case WEAPON_SAWEDOFF:
			return ("sawed off");
		case WEAPON_TEC9:
			return ("tec-9");
		case WEAPON_TASER:
			return ("taser");
		case WEAPON_NOVA:
			return ("nova");
		case WEAPON_CZ75A:
			return ("cz75");
		case WEAPON_SG556:
			return ("sg556");
		case WEAPON_REVOLVER:
			return ("revolver");
		case WEAPON_MP7:
			return ("mp7");
		case WEAPON_MP9:
			return ("mp9");
		case WEAPON_MP5:
			return ("mp5sd");
		case WEAPON_C4:
			return ("c4");
		case WEAPON_HEGRENADE:
			return ("he grenade");
		case WEAPON_SMOKEGRENADE:
			return ("smoke grenade");
		case WEAPON_MOLOTOV:
			return ("molotov");
		case WEAPON_INCGRENADE:
			return ("molly");
		case WEAPON_FLASHBANG:
			return ("flashbang");
		case WEAPON_DECOY:
			return ("decoy");
		case WEAPON_KNIFE_T:
			return ("knife");
		case WEAPON_KNIFE:
			return ("knife");
		case WEAPON_TAGRENADE:
			return ("tagrenade");
		case WEAPON_FISTS:
			return ("fist");
		case WEAPON_BREACHCHARGE:
			return ("breach_charge");
		case WEAPON_TABLET:
			return ("tablet");
		case WEAPON_MELEE:
			return ("melee");
		case WEAPON_AXE:
			return ("axe");
		case WEAPON_HAMMER:
			return ("hammer");
		case WEAPON_SPANNER:
			return ("spanner");
		case WEAPON_KNIFE_GHOST:
			return ("knife");
		case WEAPON_FIREBOMB:
			return ("molotov");
		case WEAPON_DIVERSION:
			return ("diversion");
		case WEAPON_KNIFE_BAYONET:
			return ("knife");
		case WEAPON_KNIFE_FLIP:
			return ("knife");
		case WEAPON_KNIFEGG:
			return ("knife");
		case WEAPON_KNIFE_GUT:
			return ("knife");
		case WEAPON_KNIFE_KARAMBIT:
			return ("knife");
		case WEAPON_KNIFE_M9_BAYONET:
			return ("knife");
		case WEAPON_KNIFE_TACTICAL:
			return ("knife");
		case WEAPON_KNIFE_FALCHION:
			return ("knife");
		case WEAPON_KNIFE_SURVIVAL_BOWIE:
			return ("knife");
		case WEAPON_KNIFE_BUTTERFLY:
			return ("knife");
		case WEAPON_KNIFE_PUSH:
			return ("knife");
		case WEAPON_KNIFE_URSUS:
			return ("knife");
		case WEAPON_KNIFE_GYPSY_JACKKNIFE:
			return ("knife");
		case WEAPON_KNIFE_STILETTO:
			return ("knife");
		case WEAPON_KNIFE_WIDOWMAKER:
			return ("knife");
		case WEAPON_HEALTHSHOT:
			return ("healthshot");
		case GLOVE_STUDDED_BLOODHOUND:
			return ("gloves");
		case GLOVE_T_SIDE:
			return ("gloves");
		case GLOVE_CT_SIDE:
			return ("gloves");
		case GLOVE_SPORTY:
			return ("gloves");
		case GLOVE_SLICK:
			return ("gloves");
		case GLOVE_LEATHER_WRAP:
			return ("gloves");
		case GLOVE_MOTORCYCLE:
			return ("gloves");
		case GLOVE_SPECIALIST:
			return ("gloves");
		case GLOVE_HYDRA:
			return ("gloves");
		default:
			return ("");
		}
		return ("");
	}
	NETVAR(int32_t, m_iEntityLevel, "DT_BaseAttributableItem", "m_iEntityLevel");
	NETVAR(int32_t, m_iAccountID, "DT_BaseAttributableItem", "m_iAccountID");
	NETVAR(int32_t, m_iItemIDLow, "DT_BaseAttributableItem", "m_iItemIDLow");
	NETVAR(int32_t, m_iItemIDHigh, "DT_BaseAttributableItem", "m_iItemIDHigh");
	NETVAR(int32_t, m_iEntityQuality, "DT_BaseAttributableItem", "m_iEntityQuality");
	NETVAR(str_32, m_iCustomName, "DT_BaseAttributableItem", "m_szCustomName");
};
struct CPaintKit
{
	int id; // 0x0
	std::byte pad0[0x7Cu]; // 0x4
	int color1;
	int color2;
	int color3;
	int color4;
};
struct CCStrike15ItemSchema
{
	CPaintKit* GetPaintKitDefinition(int iPaintKitID)
	{
		static auto get_paint_kit_definition = Utils::PatternScan(GetModuleHandleA("client.dll"), "E8 ?? ?? ?? ?? 8B 48 40");
		// Skip the opcode, read rel32 address
		const auto item_system_offset = *reinterpret_cast<std::int32_t*>(get_paint_kit_definition + 1);
		// Add the offset to the end of the instruction
		const auto item_system_fn = reinterpret_cast<CPaintKit * (__thiscall*)(void*, int)>(get_paint_kit_definition + 5 + item_system_offset);

		return item_system_fn(this, iPaintKitID);
	}
};
struct CCStrike15ItemSystem
{
	CCStrike15ItemSchema* get_item_schema_interface()
	{
		using fn = CCStrike15ItemSchema * (__thiscall*)(void*);
		return CallVFunction<fn>(this, 0)(this);
	}
};

class C_BaseEntity : public IClientEntity
{
public:
	datamap_t * GetDataDescMap() {
		typedef datamap_t*(__thiscall *o_GetPredDescMap)(void*);
		return CallVFunction<o_GetPredDescMap>(this, 15)(this);
	}

	datamap_t *GetPredDescMap() {
		typedef datamap_t*(__thiscall *o_GetPredDescMap)(void*);
		return CallVFunction<o_GetPredDescMap>(this, 17)(this);
	}
	static __forceinline C_BaseEntity* GetEntityByIndex(int index) {
		return static_cast<C_BaseEntity*>(g_EntityList->GetClientEntity(index));
	}
	static __forceinline C_BaseEntity* get_entity_from_handle(CBaseHandle h) {
		return static_cast<C_BaseEntity*>(g_EntityList->GetClientEntityFromHandle(h));
	}

	NETVAR(int32_t, m_nModelIndex, "DT_BaseEntity", "m_nModelIndex");
	NETVAR(int32_t, m_iTeamNum, "DT_BaseEntity", "m_iTeamNum");
	NETVAR(Vector, m_vecOrigin, "DT_BaseEntity", "m_vecOrigin");
	NETVAR(Vector, m_vecAngles, "DT_BaseEntity", "m_vecAngles");
	NETVAR(int, money, "DT_CSPlayer", "m_iAccount");
	NETVAR(int, armor, "DT_CSPlayer", "m_ArmorValue");
	NETVAR(bool, has_helmet, "DT_CSPlayer", "m_bHasHelmet");
	NETVAR(bool, m_bShouldGlow, "DT_DynamicProp", "m_bShouldGlow");
	NETVAR(CHandle<C_BasePlayer>, m_hOwnerEntity, "DT_BaseEntity", "m_hOwnerEntity");
	NETVAR(bool, m_bSpotted, "DT_BaseEntity", "m_bSpotted");
	NETVAR(float_t, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow");
	NETVAR(int, m_hRagdoll, "CCSPlayer", "m_hRagdoll")
	NETVAR(CHandle<C_BasePlayer>, ground_entity, "DT_CSPlayer", "m_hGroundEntity");
	
	void getBonePos(int bone, Vector& origin) noexcept
	{
		Vector vectors[4];
		memesclass->getBonePos(this, bone, vectors);
		origin = { vectors[1].x, vectors[2].y, vectors[3].z };
	}
	const matrix3x4_t& m_rgflCoordinateFrame()
	{
		static auto _m_rgflCoordinateFrame = NetvarSys::Get().GetOffset("DT_BaseEntity", "m_CollisionGroup") - 0x30;
		return *(matrix3x4_t*)((uintptr_t)this + _m_rgflCoordinateFrame);
	}

	bool IsPlayer();
	bool IsLoot();
	bool IsWeapon();
	bool IsPlantedC4();
	bool IsDefuseKit();
	void setModelIndex(int modelIndex);
	//bool isSpotted();
};

class C_PlantedC4
{
public:
	NETVAR(bool, m_bBombTicking, "DT_PlantedC4", "m_bBombTicking");
	NETVAR(bool, m_bBombDefused, "DT_PlantedC4", "m_bBombDefused");
	NETVAR(float, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow");
	NETVAR(float, m_flTimerLength, "DT_PlantedC4", "m_flTimerLength");
	NETVAR(float, m_flDefuseLength, "DT_PlantedC4", "m_flDefuseLength");
	NETVAR(float, m_flDefuseCountDown, "DT_PlantedC4", "m_flDefuseCountDown");
	NETVAR(CHandle<C_BasePlayer>, m_hBombDefuser, "DT_PlantedC4", "m_hBombDefuser");
};

class C_BaseAttributableItem : public C_BaseEntity
{
public:
	NETVAR(uint64_t, m_OriginalOwnerXuid, "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow");
	NETVAR(int32_t, m_OriginalOwnerXuidLow, "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow");
	NETVAR(int32_t, m_OriginalOwnerXuidHigh, "DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh");
	NETVAR(int32_t, m_nFallbackStatTrak, "DT_BaseAttributableItem", "m_nFallbackStatTrak");
	NETVAR(int32_t, m_nFallbackPaintKit, "DT_BaseAttributableItem", "m_nFallbackPaintKit");
	NETVAR(int32_t, m_nFallbackSeed, "DT_BaseAttributableItem", "m_nFallbackSeed");
	NETVAR(float_t, m_flFallbackWear, "DT_BaseAttributableItem", "m_flFallbackWear");

	NETVAR(C_EconItemView, m_Item2, "DT_BaseAttributableItem", "m_Item");

	C_EconItemView& m_Item()
	{
		// Cheating. It should be this + m_Item netvar but then the netvars inside C_EconItemView wont work properly.
		// A real fix for this requires a rewrite of the netvar manager
		return *(C_EconItemView*)this;
	}
	void SetModelIndex(int modelIndex);
	void SetGloveModelIndex(int modelIndex);

};

class C_BaseWeaponWorldModel : public C_BaseEntity
{
public:
	NETVAR(int32_t, m_nModelIndex, "DT_BaseWeaponWorldModel", "m_nModelIndex");
};

class C_BaseCombatWeapon : public C_BaseAttributableItem
{
public:
	NETVAR(float_t, m_flNextPrimaryAttack, "DT_BaseCombatWeapon", "m_flNextPrimaryAttack");
	NETVAR(float_t, m_flNextSecondaryAttack, "DT_BaseCombatWeapon", "m_flNextSecondaryAttack");
	NETVAR(int32_t, m_iClip1, "DT_BaseCombatWeapon", "m_iClip1");
	NETVAR(int32_t, m_iClip2, "DT_BaseCombatWeapon", "m_iClip2");
	NETVAR(float_t, m_flRecoilIndex, "DT_WeaponCSBase", "m_flRecoilIndex");
	NETVAR(int32_t, m_iViewModelIndex, "DT_BaseCombatWeapon", "m_iViewModelIndex");
	NETVAR(int32_t, m_iWorldModelIndex, "DT_BaseCombatWeapon", "m_iWorldModelIndex");
	NETVAR(int32_t, m_iWorldDroppedModelIndex, "DT_BaseCombatWeapon", "m_iWorldDroppedModelIndex");
	NETVAR(bool, m_bPinPulled, "DT_BaseCSGrenade", "m_bPinPulled");
	NETVAR(float_t, m_fThrowTime, "DT_BaseCSGrenade", "m_fThrowTime");
	NETVAR(float_t, m_flPostponeFireReadyTime, "DT_BaseCombatWeapon", "m_flPostponeFireReadyTime");
	NETVAR(CHandle<C_BaseWeaponWorldModel>, m_hWeaponWorldModel, "DT_BaseCombatWeapon", "m_hWeaponWorldModel");
	std::string get_wpn_icon(int index = -1) {
		switch (auto idx = index != -1 ? index : this->m_Item().m_iItemDefinitionIndex(); idx) {
		case WEAPON_DEAGLE:					return u8"\uE001";
		case WEAPON_ELITE:					return u8"\uE002";
		case WEAPON_FIVESEVEN:				return u8"\uE003";
		case WEAPON_GLOCK:					return u8"\uE004";
		case WEAPON_AK47:					return u8"\uE007";
		case WEAPON_AUG:					return u8"\uE008";
		case WEAPON_AWP:					return u8"\uE009";
		case WEAPON_FAMAS:					return u8"\uE00A";
		case WEAPON_G3SG1:					return u8"\uE00B";
		case WEAPON_GALILAR:				return u8"\uE00D";
		case WEAPON_M249:					return u8"\uE00E";
		case WEAPON_M4A1:					return u8"\uE010";
		case WEAPON_MAC10:					return u8"\uE011";
		case WEAPON_P90:					return u8"\uE013";
		case WEAPON_MP5:					return u8"\uE017";
		case WEAPON_UMP45:					return u8"\uE018";
		case WEAPON_XM1014:					return u8"\uE019";
		case WEAPON_BIZON:					return u8"\uE01A";
		case WEAPON_MAG7:					return u8"\uE01B";
		case WEAPON_NEGEV:					return u8"\uE01C";
		case WEAPON_SAWEDOFF:				return u8"\uE01D";
		case WEAPON_TEC9:					return u8"\uE01E";
		case WEAPON_TASER:					return u8"\uE01F";
		case WEAPON_HKP2000:				return u8"\uE020";
		case WEAPON_MP7:					return u8"\uE021";
		case WEAPON_MP9:					return u8"\uE022";
		case WEAPON_NOVA:					return u8"\uE023";
		case WEAPON_P250:					return u8"\uE024";
		case WEAPON_SCAR20:					return u8"\uE026";
		case WEAPON_SSG08:					return u8"\uE028";
		case WEAPON_KNIFEGG:				return u8"\uE029";
		case WEAPON_KNIFE:					return u8"\uE02A";
		case WEAPON_FLASHBANG:				return u8"\uE02B";
		case WEAPON_HEGRENADE:				return u8"\uE02C";
		case WEAPON_SMOKEGRENADE:			return u8"\uE02D";
		case WEAPON_MOLOTOV:				return u8"\uE02E";
		case WEAPON_DECOY:					return u8"\uE02F";
		case WEAPON_INCGRENADE:				return u8"\uE030";
		case WEAPON_C4:						return u8"\uE031";
		case WEAPON_HEALTHSHOT:				return u8"\uE039";
		case WEAPON_KNIFE_T:				return u8"\uE03B";
		case WEAPON_M4A1_SILENCER:			return u8"\uE03C";
		case WEAPON_USP_SILENCER:			return u8"\uE03D";
		case WEAPON_CZ75A:					return u8"\uE03F";
		case WEAPON_REVOLVER:				return u8"\uE040";
		case WEAPON_TAGRENADE:				return u8"\uE044";
		case WEAPON_FISTS:					return u8"\uE045";
		case WEAPON_BREACHCHARGE:			return u8"\uE046";
		case WEAPON_TABLET:					return u8"\uE048";
		case WEAPON_MELEE:					return u8"\uE04A";
		case WEAPON_AXE:					return u8"\uE04B";
		case WEAPON_HAMMER:					return u8"\uE04C";
		case WEAPON_SPANNER:				return u8"\uE04E";
		case WEAPON_KNIFE_GHOST:			return u8"\uE050";
		case WEAPON_FIREBOMB:				return u8"\uE051";
		case WEAPON_DIVERSION:				return u8"\uE052";
		case WEAPON_FRAG_GRENADE:			return u8"\uE053";
		case WEAPON_KNIFE_FLIP:				return u8"\uE1F9";
		case WEAPON_KNIFE_GUT:				return u8"\uE1FA";
		case WEAPON_KNIFE_KARAMBIT:			return u8"\uE1FB";
		case WEAPON_KNIFE_M9_BAYONET:		return u8"\uE1FC";
		case WEAPON_KNIFE_TACTICAL:			return u8"\uE1FD";
		case WEAPON_KNIFE_FALCHION:			return u8"\uE200";
		case WEAPON_KNIFE_SURVIVAL_BOWIE:	return u8"\uE202";
		case WEAPON_KNIFE_BUTTERFLY:		return u8"\uE203";
		case WEAPON_KNIFE_PUSH:				return u8"\uE204";
		case WEAPON_KNIFE_URSUS:			return u8"\uE207";
		case WEAPON_KNIFE_GYPSY_JACKKNIFE:	return u8"\uE208";
		case WEAPON_KNIFE_STILETTO:			return u8"\uE20A";
		case WEAPON_KNIFE_WIDOWMAKER:		return u8"\uE20B";
		default:
			return u8"\u0000";
		}
	}

	bool IsMiscWeapon()
	{
		short iWeaponID = m_Item().m_iItemDefinitionIndex();

		return (iWeaponID == WEAPON_KNIFE || iWeaponID == WEAPON_C4
			|| iWeaponID == WEAPON_HEGRENADE || iWeaponID == WEAPON_SMOKEGRENADE
			|| iWeaponID == WEAPON_FLASHBANG || iWeaponID == WEAPON_DECOY
			|| iWeaponID == WEAPON_FLASHBANG || iWeaponID == WEAPON_INCGRENADE
			|| iWeaponID == WEAPON_KNIFE_T || (iWeaponID >= WEAPON_KNIFE_BAYONET && iWeaponID <= WEAPON_KNIFE_WIDOWMAKER));
	}
	CCSWeaponInfo* GetCSWeaponData();
	bool HasBullets();
	bool CanFire();
	bool IsGrenade();
	bool IsKnife();
	bool IsReloading();
	bool IsRifle();
	bool IsPistol();
	bool IsSniper();
	bool IsGun();
	float GetInaccuracy();
	float GetSpread();
	void UpdateAccuracyPenalty();
	CUtlVector<IRefCounted*>& m_CustomMaterials();
	bool* m_bCustomMaterialInitialized();

};
struct PlayerBackup {
	int m_fFlags{};
	int m_nTickBase{};
	Vector m_vecVelocity{};
	float fl_stamina{};
	float m_flMaxspeed{};
	float m_flSimulationTime{};
	float m_surfaceFriction{};
	Vector m_vecBaseVelocity{};
	Vector m_vecOrigin{};
};
class C_BasePlayer : public C_BaseEntity
{
public:
	static __forceinline C_BasePlayer* GetPlayerByUserId(int id)
	{
		return static_cast<C_BasePlayer*>(GetEntityByIndex(g_EngineClient->GetPlayerForUserID(id)));
	}
	static __forceinline C_BasePlayer* GetPlayerByIndex(int i)
	{
		return static_cast<C_BasePlayer*>(GetEntityByIndex(i));
	}
	void collect(PlayerBackup& backup) const;
	void apply(const PlayerBackup& backup);
	NETVAR(bool, m_bHasDefuser, "DT_CSPlayer", "m_bHasDefuser");
	NETVAR(bool, m_bGunGameImmunity, "DT_CSPlayer", "m_bGunGameImmunity");
	
	NETVAR(int32_t, m_iShotsFired, "DT_CSPlayer", "m_iShotsFired");
	NETVAR(QAngle, m_angEyeAngles, "DT_CSPlayer", "m_angEyeAngles[0]");
	NETVAR(int, m_ArmorValue, "DT_CSPlayer", "m_ArmorValue");
	NETVAR(bool, m_bHasHeavyArmor, "DT_CSPlayer", "m_bHasHeavyArmor");
	NETVAR(bool, m_bHasHelmet, "DT_CSPlayer", "m_bHasHelmet");
	NETVAR(bool, m_bIsScoped, "DT_CSPlayer", "m_bIsScoped");;
	NETVAR(bool, is_rescuing, "DT_CSPlayer", "m_bIsRescuing")
	NETVAR(float, m_flLowerBodyYawTarget, "DT_CSPlayer", "m_flLowerBodyYawTarget");
	NETVAR(int32_t, m_iHealth, "DT_BasePlayer", "m_iHealth");
	NETVAR(int, observer, "DT_BasePlayer", "m_iObserverMode")
	NETVAR(int32_t, m_lifeState, "DT_BasePlayer", "m_lifeState");
	NETVAR(int32_t, m_fFlags, "DT_BasePlayer", "m_fFlags");
	NETVAR(int32_t, m_nTickBase, "DT_BasePlayer", "m_nTickBase");
	NETVAR(Vector, m_vecViewOffset, "DT_BasePlayer", "m_vecViewOffset[0]");
	NETVAR(QAngle, m_viewPunchAngle, "DT_BasePlayer", "m_viewPunchAngle");
	NETVAR(QAngle, m_aimPunchAngle, "DT_BasePlayer", "m_aimPunchAngle");
	NETVAR(CHandle<C_BaseViewModel>, m_hViewModel, "DT_BasePlayer", "m_hViewModel[0]");
	NETVAR(Vector, m_vecVelocity, "DT_BasePlayer", "m_vecVelocity[0]");
	NETVAR(float, fl_stamina, "DT_CSPlayer", "m_flStamina");
	NETVAR(float, m_flMaxspeed, "DT_BasePlayer", "m_flMaxspeed");
	NETVAR(CHandle<C_BasePlayer>, m_hObserverTarget, "DT_BasePlayer", "m_hObserverTarget");
	NETVAR(float, m_flFlashMaxAlpha, "DT_CSPlayer", "m_flFlashMaxAlpha");
	NETVAR(int32_t, m_nHitboxSet, "DT_BaseAnimating", "m_nHitboxSet");
	NETVAR(CHandle<C_BaseCombatWeapon>, m_hActiveWeapon, "DT_BaseCombatCharacter", "m_hActiveWeapon");
	NETVAR(int32_t, m_iAccount, "DT_CSPlayer", "m_iAccount");
	NETVAR(float, m_flFlashDuration, "DT_CSPlayer", "m_flFlashDuration");
	NETVAR(float, m_flSimulationTime, "DT_BaseEntity", "m_flSimulationTime");
	NETVAR(float, m_flCycle, "DT_BaseAnimating", "m_flCycle");
	NETVAR(int, m_nSequence, "DT_BaseViewModel", "m_nSequence");
	NETVAR(float, m_flNextAttack, "DT_BaseCombatCharacter", "m_flNextAttack");
	NETVAR(int32_t, m_nSurvivalTeam, "DT_CSPlayer", "m_nSurvivalTeam");

	//NETVAR(int, m_iAccount, "DT_CSPlayer", "m_iAccount");

	void SetModelIndex(int index) {
		return CallVFunction<void(__thiscall*)(C_BaseEntity*, int)>(this, 75)(this, index);
	}

	NETVAR(QAngle, m_angAbsAngles, "DT_BaseEntity", "m_angAbsAngles");
	NETVAR(Vector, m_angAbsOrigin, "DT_BaseEntity", "m_angAbsOrigin");
	NETVAR(float, m_flDuckSpeed, "DT_BaseEntity", "m_flDuckSpeed");
	NETVAR(float, m_flDuckAmount, "DT_BaseEntity", "m_flDuckAmount");
	std::array<float, 24> &m_flPoseParameter() const {
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_BaseAnimating", "m_flPoseParameter");
		return *(std::array<float, 24>*)((uintptr_t)this + _m_flPoseParameter);
	}


	PNETVAR(CHandle<C_BaseCombatWeapon>, m_hMyWeapons, "DT_BaseCombatCharacter", "m_hMyWeapons");
	PNETVAR(CHandle<C_BaseAttributableItem>, m_hMyWearables, "DT_BaseCombatCharacter", "m_hMyWearables");
	PNETVAR(char, m_szLastPlaceName, "DT_BasePlayer", "m_szLastPlaceName");


	NETPROP(m_flLowerBodyYawTargetProp, "DT_CSPlayer", "m_flLowerBodyYawTarget");
	CUserCmd*& m_pCurrentCommand();

	/*gladiator v2*/
	void InvalidateBoneCache();
	int GetNumAnimOverlays();
	AnimationLayer *GetAnimOverlays();
	AnimationLayer *GetAnimOverlay(int i);
	int GetSequenceActivity(int sequence);
	CCSGOPlayerAnimState *GetPlayerAnimState();

	static void UpdateAnimationState(CCSGOPlayerAnimState *state, QAngle angle);
	static void ResetAnimationState(CCSGOPlayerAnimState *state);
	void CreateAnimationState(CCSGOPlayerAnimState *state);

	float_t &m_surfaceFriction()
	{
		static unsigned int _m_surfaceFriction = Utils::FindInDataMap(GetPredDescMap(), "m_surfaceFriction");
		return *(float_t*)((uintptr_t)this + _m_surfaceFriction);
	}
	Vector &m_vecBaseVelocity()
	{
		static unsigned int _m_vecBaseVelocity = Utils::FindInDataMap(GetPredDescMap(), "m_vecBaseVelocity");
		return *(Vector*)((uintptr_t)this + _m_vecBaseVelocity);
	}

	float_t &m_flMaxspeed()
	{
		static unsigned int _m_flMaxspeed = Utils::FindInDataMap(GetPredDescMap(), "m_flMaxspeed");
		return *(float_t*)((uintptr_t)this + _m_flMaxspeed);
	}



	Vector        GetEyePos();
	player_info_t GetPlayerInfo();
	bool          IsAlive();
	bool		  IsFlashed();
	bool		  IsEnemy();
	bool          HasC4();
	Vector        GetHitboxPos(int hitbox_id);
	mstudiobbox_t * GetHitbox(int hitbox_id);
	bool          GetHitboxPos(int hitbox, Vector &output);
	Vector        GetBonePos(int bone);
	bool          CanSeePlayer(C_BasePlayer* player, int hitbox);
	bool          CanSeePlayer(C_BasePlayer* player, const Vector& pos);
	void UpdateClientSideAnimation();
	Vector        abs_origin();

	int m_nMoveType();
	QAngle * GetVAngles();
	float_t m_flSpawnTime();

};

class C_BaseViewModel : public C_BaseEntity
{
public:
	NETVAR(int32_t, m_nModelIndex, "DT_BaseViewModel", "m_nModelIndex");
	NETVAR(int32_t, m_nViewModelIndex, "DT_BaseViewModel", "m_nViewModelIndex");
	NETVAR(CHandle<C_BaseCombatWeapon>, m_hWeapon, "DT_BaseViewModel", "m_hWeapon");
	NETVAR(CHandle<C_BasePlayer>, m_hOwner, "DT_BaseViewModel", "m_hOwner");
	NETPROP(m_nSequence, "DT_BaseViewModel", "m_nSequence");
	void SendViewModelMatchingSequence(int sequence);
};

class AnimationLayer
{
public:
	char  pad_0000[20];
	// These should also be present in the padding, don't see the use for it though
	//float	m_flLayerAnimtime;
	//float	m_flLayerFadeOuttime;
	uint32_t m_nOrder; //0x0014
	uint32_t m_nSequence; //0x0018
	float_t m_flPrevCycle; //0x001C
	float_t m_flWeight; //0x0020
	float_t m_flWeightDeltaRate; //0x0024
	float_t m_flPlaybackRate; //0x0028
	float_t m_flCycle; //0x002C
	void* m_pOwner; //0x0030 // player's thisptr
	char  pad_0038[4]; //0x0034
}; //Size: 0x0038

class CCSGOPlayerAnimState
{
public:
	void* pThis;
	char pad2[91];
	void* pBaseEntity; //0x60
	void* pActiveWeapon; //0x64
	void* pLastActiveWeapon; //0x68
	float m_flLastClientSideAnimationUpdateTime; //0x6C
	int m_iLastClientSideAnimationUpdateFramecount; //0x70
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flUnknownVelocityLean; //0x8C //changes when moving/jumping/hitting ground
	float m_flLeanAmount; //0x90
	char pad4[4]; //NaN
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
	float m_fUnknown2;
	float m_fDuckAmount; //0xA4
	float m_fLandingDuckAdditiveSomething; //0xA8
	float m_fUnknown3; //0xAC
	Vector m_vOrigin; //0xB0, 0xB4, 0xB8
	Vector m_vLastOrigin; //0xBC, 0xC0, 0xC4
	float m_vVelocityX; //0xC8
	float m_vVelocityY; //0xCC
	char pad5[4];
	float m_flUnknownFloat1; //0xD4 Affected by movement and direction
	char pad6[8];
	float m_flUnknownFloat2; //0xE0 //from -1 to 1 when moving and affected by direction
	float m_flUnknownFloat3; //0xE4 //from -1 to 1 when moving and affected by direction
	float m_unknown; //0xE8
	float speed_2d; //0xEC
	float flUpVelocity; //0xF0
	float m_flSpeedNormalized; //0xF4 //from 0 to 1
	float m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
	float m_flFeetSpeedUnknownForwardOrSideways; //0xFC //from 0 to 3. something
	float m_flTimeSinceStartedMoving; //0x100
	float m_flTimeSinceStoppedMoving; //0x104
	unsigned char m_bOnGround; //0x108
	unsigned char m_bInHitGroundAnimation; //0x109
	char pad7[10];
	float m_flLastOriginZ; //0x114
	float m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
	float m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
	char pad8[4]; //NaN
	float m_flUnknownFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
	char pad9[4]; //NaN
	float m_flUnknown3;
	char pad10[528];
}; //Size=0x344

class DT_CSPlayerResource
{
public:
	PNETVAR(int32_t, m_nActiveCoinRank, "DT_CSPlayerResource", "m_nActiveCoinRank");
	PNETVAR(int32_t, m_nMusicID, "DT_CSPlayerResource", "m_nMusicID");
	PNETVAR(int32_t, m_nPersonaDataPublicLevel, "DT_CSPlayerResource", "m_nPersonaDataPublicLevel");
	PNETVAR(int32_t, m_nPersonaDataPublicCommendsLeader, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsLeader");
	PNETVAR(int32_t, m_nPersonaDataPublicCommendsTeacher, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsTeacher");
	PNETVAR(int32_t, m_nPersonaDataPublicCommendsFriendly, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsFriendly");
	PNETVAR(int32_t, m_iCompetitiveRanking, "DT_CSPlayerResource", "m_iCompetitiveRanking");
	PNETVAR(int32_t, m_iCompetitiveWins, "DT_CSPlayerResource", "m_iCompetitiveWins");
	PNETVAR(int32_t, m_iPlayerVIP, "DT_CSPlayerResource", "m_iPlayerVIP");
	PNETVAR(int32_t, m_iMVPs, "DT_CSPlayerResource", "m_iMVPs");
	PNETVAR(int32_t, m_iScore, "DT_CSPlayerResource", "m_iScore");
};

