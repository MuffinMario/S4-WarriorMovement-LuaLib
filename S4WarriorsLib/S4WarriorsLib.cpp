#include "pch.h"
#include "S4WarriorsLib.h"

#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>

using namespace std::string_literals;
S4API IS4ModInterface::m_pS4API;
S4HOOK S4WarriorsLib::m_luaOpenHook = NULL;

struct CPlayerMetadata {
    void* baseObj;
    DWORD race;
    DWORD startPosX;
    DWORD startPosY;
    DWORD unknown2;
    DWORD unknown4; // 0xFFFFFFFF or 0x0
    DWORD playerColor;
    wchar_t* playerName;
    DWORD unknown6;
    DWORD unknown7;
    DWORD unknown8;
    DWORD playerNameLength; 
    DWORD unknown9; // something determining the player name length?
    DWORD unknown10;
    DWORD unknown11;
};
static_assert(sizeof(CPlayerMetadata) == 0x3C, "CPlayerMetadata is not 0x3C bytes");

IEntity*** g_paSettlerPool;
WORD** g_paEntityMap;
DWORD* g_pMapSize;
DWORD* g_pAIPartiesBitflags;
CPlayerMetadata* g_aPlayerMetadata;
//offsets, all HE because bruh imagine GE
constexpr size_t entityPoolOffsetHE = 0xECDE9;
constexpr size_t entityMapOffsetHE = 0x11630DC;
constexpr size_t mapSizeOffsetHE = 0xD6921C;
// thx 2 jhnp
constexpr size_t aiPartiesBitflagsOffsetHE = 0x106B150;
// thx 2 kdsystem
constexpr size_t aPlayerMetadataOffset = 0x109B628-0x1c-0x3C; // player 0 is empty dummy with random adress in the middle (hopefully not vtable and im mixing stuff up)


class CSelection {
public:
    std::vector<WORD> m_aIds;
};
std::vector<CSelection> g_aSettlerSelections;

namespace S4ModApi {
    DWORD GetMapSize() {
        return g_pMapSize != NULL ? *g_pMapSize : 0;
    }
    WORD GetEntityIdAt(WORD x, WORD y) {
        if (g_paEntityMap && *g_paEntityMap) {
            auto size = GetMapSize();
            if (size == 0 || y >= size || x >= size) return NULL;
            return (*g_paEntityMap)[y * size + x];
        }
        return 0;
    }
    IEntity* GetEntityAt(WORD x, WORD y) {
        if (g_paSettlerPool && *g_paSettlerPool) {
            auto eid = GetEntityIdAt(x, y);
            if (eid) {
                return (*g_paSettlerPool)[eid];
            }
        }
        return NULL;
    }
}
DWORD g_s4Base = NULL;
bool initOffsets() {
    g_s4Base = (DWORD)GetModuleHandle(nullptr);
    if (!g_s4Base) return false;

    g_paSettlerPool = reinterpret_cast<IEntity***>(g_s4Base + entityPoolOffsetHE); // all types are 32-bit on most common compilers at least
    g_paEntityMap = reinterpret_cast<WORD**>(g_s4Base + entityMapOffsetHE);
    g_pMapSize = reinterpret_cast<DWORD*>(g_s4Base + mapSizeOffsetHE);
    g_pAIPartiesBitflags = reinterpret_cast<DWORD*>(g_s4Base + aiPartiesBitflagsOffsetHE);
    g_aPlayerMetadata = reinterpret_cast<CPlayerMetadata*>(g_s4Base + aPlayerMetadataOffset);

    return true;
}
bool IS4ModInterface::createAPI()
{
    m_pS4API = S4ApiCreate();
    if (m_pS4API == nullptr) {
        return false;
    }
    return true;
}

bool IS4ModInterface::releaseAPI()
{
    if (m_pS4API) {
        m_pS4API->Release();
        m_pS4API = nullptr;
        return true;
    }
    return false;
}

ATTACH_VALUE S4WarriorsLib::onAttach()
{

    if (!createAPI())
        return ATTACH_VALUE::FAILED_COULD_NOT_CREATE_API_PTR;
    if (!m_pS4API->IsEdition(S4_EDITION_HISTORY))
        return ATTACH_VALUE::FAILED_NOT_HISTORY_EDITION;
    if (!initOffsets())
        return ATTACH_VALUE::FAILED_S4_MODULE_NOT_FOUND;

    //m_pS4API->AddSettlerSendListener(callbacksettlersend);
    m_luaOpenHook = m_pS4API->AddLuaOpenListener(onLuaOpen);

    if (m_luaOpenHook == NULL)
        return ATTACH_VALUE::FAILED_COULD_NOT_HOOK_LISTENERS;

    return ATTACH_VALUE::SUCCESS;
}

DETACH_VALUE S4WarriorsLib::onDetach()
{

    m_pS4API->RemoveListener(m_luaOpenHook);

    if (!releaseAPI()) {
        return DETACH_VALUE::FAILED_COULD_NOT_RELEASE_API_PTR;
    }
    return DETACH_VALUE::SUCCESS;
}

// lib name
static const char* libName = "WarriorsLib";
// lib functions
constexpr size_t libfunccount = 7;

static std::array<struct luaL_reg,
    libfunccount> aWarriorsLibArr{ {
    {const_cast<char*>("Send"), S4WarriorsLib::Send},
    {const_cast<char*>("SelectWarriors"), S4WarriorsLib::SelectWarriors},
    {const_cast<char*>("isHuman"), S4WarriorsLib::isHuman},
    {const_cast<char*>("getPlayerName"), S4WarriorsLib::getPlayerName},
    //{const_cast<char*>("UnGarrisonWarriors"), S4WarriorsLib::UnGarrisonWarriors},
    //{const_cast<char*>("GarrisonWarriors"), S4WarriorsLib::GarrisonWarriors},
    {const_cast<char*>("AiUnGarrisonWarriors"), S4WarriorsLib::AiUnGarrisonWarriors},
    {const_cast<char*>("AiGarrisonWarriors"), S4WarriorsLib::AiGarrisonWarriors},
    //{const_cast<char*>("RecruitWarriors"), S4WarriorsLib::RecruitWarriors},
    {const_cast<char*>("AiRecruitWarriors"), S4WarriorsLib::AiRecruitWarriors}
} };
std::map<const char*, S4_MOVEMENT_ENUM> aWarriorsLibMovementVars{
    {"MOVE_FORWARDS",S4_MOVEMENT_ENUM::S4_MOVEMENT_FORWARD},
    {"MOVE_PATROL",S4_MOVEMENT_ENUM::S4_MOVEMENT_PATROL},
    {"MOVE_ACCUMULATE",S4_MOVEMENT_ENUM::S4_MOVEMENT_ACCUMULATE},
    {"MOVE_WATCH",S4_MOVEMENT_ENUM::S4_MOVEMENT_WATCH},
    {"MOVE_STOP",S4_MOVEMENT_ENUM::S4_MOVEMENT_STOP}
};

// WarriorsLib.Send(group,to_x,to_y,movementtype);
void S4WarriorsLib::Send()
{
    // param 1
    auto grouptbl = lua_lua2C(1);
    int x = luaL_check_int(2);
    int y = luaL_check_int(3);
    DWORD movementType = S4_MOVEMENT_FORWARD;
    auto param4 = lua_lua2C(4);

    if (lua_isnumber(param4))
        movementType = lua_getnumber(param4);
    if (lua_isnumber(grouptbl)) {
        CSelection& selection = g_aSettlerSelections.at(lua_getnumber(grouptbl));
        m_pS4API->SendWarriors(x, y, static_cast<S4_MOVEMENT_ENUM>(movementType), selection.m_aIds.data(), selection.m_aIds.size(), 0);
    }
    // do nothing?
    //else
    //    lua_error((char*)"Send called with wrong type (not number/table)");
}

template <typename F, typename... P>
void circle(WORD x, WORD y, WORD radius, WORD mapsize, F func) {
    unsigned int r = radius;
    unsigned int xorig = x, yorig = y;

    unsigned int its = 2 * r - 1;
    //    basically 
    //
    //      LLX
    //     LLXRR
    //      XRR
    unsigned int left = r - 1, right = 0;
    for (unsigned int i = its; i > 0; --i) {
        unsigned int cx = xorig - left;
        unsigned int cy = yorig + r - i;

        // this and the following lines will be out of map
        if (cy >= mapsize - 1) break;

        // don't care aobut the ones left from this.
        if (cx < 1) cx = 1;

        while (cx <= xorig + right && cx < mapsize - 1) {
            func(cx, cy);
            ++cx;
        }

        if (right == r - 1) --left;
        else ++right;
    }
}

// selectwarriors(number x, number y, number radius, number party, number warriortype)
void S4WarriorsLib::SelectWarriors() {
    auto x = luaL_check_int(1);
    auto y = luaL_check_int(2);
    auto radius = luaL_check_int(3);
    auto party = luaL_check_int(4);
    auto warriortype = luaL_check_int(5);

    CSelection& newSelection = g_aSettlerSelections.emplace_back();

    auto TableAppendInt = [](CSelection& const sel, int v) {
        sel.m_aIds.push_back(v);
    };
    auto CircleSelectWarriors = [&warriortype, &party, TableAppendInt](WORD x, WORD y) -> void {
        auto pEntity = S4ModApi::GetEntityAt(x, y);

        if (pEntity) {
            // only move settlers, carts, warmachines, boats, manacopter
            if (pEntity->GetClass() != S4_ENTITY_ENUM::S4_ENTITY_SETTLER && !(pEntity->GetClass() >= S4_ENTITY_ENUM::S4_ENTITY_CART && pEntity->GetClass() <= S4_ENTITY_ENUM::S4_ENTITY_MANAKOPTER))
                return;
            auto settlertype = pEntity->objectId;
            auto settlerID = pEntity->id2;
            auto player = pEntity->tribe; // nyfrk mistake, tribe => player, player => tribe
            if (settlertype == warriortype && player == party) {
                TableAppendInt(g_aSettlerSelections.back(), settlerID);
            }
        }
    };
    circle(x, y, radius, S4ModApi::GetMapSize(), CircleSelectWarriors);
    if (newSelection.m_aIds.size() == 0)
    {
        g_aSettlerSelections.pop_back();
    }
    else {
        lua_pushnumber(g_aSettlerSelections.size() - 1);
    }
}

bool checkPartyIsHuman(int party)
{
    if (*g_pAIPartiesBitflags & (1 <<party)) {
        return false;
    }
    return true;
}
 
// isHuman(number party)
// WarriorsLib.isHuman(1)
void S4WarriorsLib::isHuman() {
    auto party = luaL_check_int(1);

    if (checkPartyIsHuman(party)) {
        lua_pushnumber(1);
    }
    else {
        lua_pushnumber(0);
    }
}

// https://stackoverflow.com/questions/21456926/how-do-i-convert-a-string-in-utf-16-to-utf-8-in-c
std::string WstrToUtf8Str(const std::wstring& wstr)
{
    std::string retStr;
    if (!wstr.empty())
    {
        int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);

        if (sizeRequired > 0)
        {
            std::vector<char> utf8String(sizeRequired);
            int bytesConverted = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(),
                -1, &utf8String[0], utf8String.size(), NULL,
                NULL);
            if (bytesConverted != 0)
            {
                retStr = &utf8String[0];
            }
            else
            {
                std::stringstream err;
                err << __FUNCTION__
                    << " std::string WstrToUtf8Str failed to convert wstring '"
                    << wstr.c_str() << L"'";
                throw std::runtime_error(err.str());
            }
        }
    }
    return retStr;
}
// getPlayerName(party)
// WarriorsLib.getPlayerName(1)
void S4WarriorsLib::getPlayerName()
{
    auto party = luaL_check_int(1);
    if (party >= 1 && party <= m_pS4API->GetNumberOfPlayers())
    {
        // we have no reliable way to instanciate UTF8 names at start (onluaopen untested), so.. yeah
        static std::string playerUTF8Username[9];
        std::wstring s = g_aPlayerMetadata[party].playerName;
        playerUTF8Username[party] = WstrToUtf8Str(s);

        lua_pushstring(const_cast<char*>(playerUTF8Username[party].c_str()));
    }
}
// RecruitWarriors(number buildingid, number warriortype, number amount, number party)
// WarriorsLib.RecruitWarriors(Buildings.GetFirstBuilding(1, Buildings.BARRACKS), Settlers.SWORDSMAN_03, 5, 1)
void S4WarriorsLib::RecruitWarriors() {  
    auto buildingid = luaL_check_int(1);
    auto warriortype = luaL_check_int(2);
    auto amount = luaL_check_int(3);
    auto party = luaL_check_int(4);
    if (m_pS4API->GetLocalPlayer() == party) {
        m_pS4API->RecruitWarriors(buildingid, static_cast<S4_SETTLER_ENUM>(warriortype), amount, party);
    }
}

// AiRecruitWarriors(number buildingid, number warriortype, number amount, number party)
// WarriorsLib.AiRecruitWarriors(Buildings.GetFirstBuilding(1, Buildings.BARRACKS), Settlers.SWORDSMAN_03, 5, 1)
void S4WarriorsLib::AiRecruitWarriors() {
    auto buildingid = luaL_check_int(1);
    auto warriortype = luaL_check_int(2);
    auto amount = luaL_check_int(3);
    auto party = luaL_check_int(4);

    m_pS4API->RecruitWarriors(buildingid, static_cast<S4_SETTLER_ENUM>(warriortype), amount, party);
}

// GarrisonWarriors(number buildingid, number party)
// WarriorsLib.GarrisonWarriors(Buildings.GetFirstBuilding(1, Buildings.GUARDTOWERSMALL), 1)
void S4WarriorsLib::GarrisonWarriors() {
    auto buildingid = luaL_check_int(1);
    auto party = luaL_check_int(2);
    if (m_pS4API->GetLocalPlayer() == party) {
        m_pS4API->GarrisonWarriors(buildingid, party);
    }
}

// UnGarrisonWarriors(number buildingid, number column, boolean bowman, number party)
// WarriorsLib.UnGarrisonWarriors(Buildings.GetFirstBuilding(1, Buildings.GUARDTOWERSMALL),-1,1,1) see: https://github.com/nyfrk/S4ModApi/wiki/UnGarrisonWarriors
void S4WarriorsLib::UnGarrisonWarriors() {
    auto buildingid = luaL_check_int(1);
    auto column = luaL_check_int(2);
    auto bowman = luaL_check_int(3);
    auto party = luaL_check_int(4);
    if (m_pS4API->GetLocalPlayer() == party) {
        m_pS4API->UnGarrisonWarriors(buildingid, column, bowman, party);
    }
}
// AiGarrisonWarriors(number buildingid, number party)
// WarriorsLib.AiGarrisonWarriors(Buildings.GetFirstBuilding(1, Buildings.GUARDTOWERSMALL), 1)
void S4WarriorsLib::AiGarrisonWarriors() {
    auto buildingid = luaL_check_int(1);
    auto party = luaL_check_int(2);
    m_pS4API->GarrisonWarriors(buildingid, party);
}

// AiUnGarrisonWarriors(number buildingid, number column, boolean bowman, number party)
// WarriorsLib.AiUnGarrisonWarriors(Buildings.GetFirstBuilding(1, Buildings.GUARDTOWERSMALL),-1,1,1) see: https://github.com/nyfrk/S4ModApi/wiki/UnGarrisonWarriors
void S4WarriorsLib::AiUnGarrisonWarriors() {
    auto buildingid = luaL_check_int(1);
    auto column = luaL_check_int(2);
    auto bowman = luaL_check_int(3);
    auto party = luaL_check_int(4);

    m_pS4API->UnGarrisonWarriors(buildingid, column, bowman, party);

}


HRESULT __stdcall S4WarriorsLib::onLuaOpen()
{
    g_aSettlerSelections.clear();
    lua_wmlibopen();
    return 0;
}

void S4WarriorsLib::lua_wmlibopen()
{
    lua_Object t = lua_createtable();
    CLuaUtils::push(t);
    lua_setglobal(const_cast<char*>(libName));

    //insert functions in table
    for (auto& red : aWarriorsLibArr)
        CLuaUtils::addtableval(lua_getglobal(const_cast<char*>(libName)), red.name, red.func);

    //insert move constants in table
    for (auto& red : aWarriorsLibMovementVars)
        CLuaUtils::addtableval(lua_getglobal(const_cast<char*>(libName)), red.first, static_cast<double>(red.second));

}
