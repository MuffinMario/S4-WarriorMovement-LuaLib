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

typedef struct wstr_struct {
    union {
        wchar_t value[8];
        struct {
            wchar_t* pValue;
            BYTE gap[12];
        };
    };
    DWORD capacity;
    DWORD length;

    [[nodiscard]] const wchar_t* c_str() const {
        if (length < 8u) {
            return this->value;
        }

        return this->pValue;
    }
} wstr_struct;

struct SPlayerMetadata {
    void* baseObj;
    DWORD race;
    DWORD startPosX;
    DWORD startPosY;
    DWORD unknown2;
    DWORD unknown4; // 0xFFFFFFFF or 0x0
    DWORD playerColor;
    wstr_struct playerName;
    DWORD unknown10;
    DWORD unknown11;
};
enum class EResourceType :uint8_t {
    FISH,
    COAL_ORE,
    IRON_ORE,
    GOLD_ORE,
    SULFUR_ORE,
    STONE_ORE,
    //this will overwrite the resource ore "underneath it". Also explains why stone cutter/wood cutter don't work on mountains I guess
    STONE,
    WOOD
};
struct SResource {
    uint8_t m_resourceData;

    EResourceType getResourceType() {
        return static_cast<EResourceType>((m_resourceData & 0xF0) >> 4);
    }
    uint8_t getResourceCount() {
        return m_resourceData & 0xF;
    }
    void setResourceType(EResourceType type) {
        m_resourceData = (m_resourceData & 0x0F) | static_cast<uint8_t>(type) & 0xF0;
    }
    void setResourceCount(uint8_t count) {
        m_resourceData = (m_resourceData & 0xF0) | count & 0x0F;
    }
};
static_assert(sizeof(SPlayerMetadata) == 0x3C, "SPlayerMetadata is not 0x3C bytes");
typedef bool(__fastcall* IS_AI_PROC)(int player);

IEntity*** g_paEntityPool;
SResource** g_paResourceMap;
size_t* g_pEntityPoolMaxOccupation;
WORD** g_paEntityMap;
DWORD* g_pMapSize;
DWORD* g_pAIPartiesBitflags;
SPlayerMetadata* g_aPlayerMetadata;
IS_AI_PROC g_pIsAI;
DWORD g_s4Base = NULL;

//offsets, all HE because bruh imagine GE
constexpr size_t entityPoolOffsetHE = 0xECDE9;
constexpr size_t entityPoolMaxOccupationHE = 0xE9B0BC;
constexpr size_t entityMapOffsetHE = 0x11630DC;
constexpr size_t mapSizeOffsetHE = 0xD6921C;
constexpr size_t paResourceMapOffsetHE = 0x11630E4;
// thx 2 jhnp
constexpr size_t aiPartiesBitflagsOffsetHE = 0x106B150;
constexpr size_t IsAi = 0x4F7A40 - 0x400000;
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
        if (g_paEntityPool && *g_paEntityPool) {
            auto eid = GetEntityIdAt(x, y);
            if (eid) {
                return (*g_paEntityPool)[eid];
            }
        }
        return NULL;
    }
}

template<typename Func>
void foreachEntities(Func f) {
    for (int i = 0; i < *g_pEntityPoolMaxOccupation; i++)
    {
        IEntity* pEnt = (*g_paEntityPool)[i];
        if (pEnt)
        {
            if (f(pEnt))
                break;
        }
    }
}

bool initOffsets() {
    g_s4Base = (DWORD)GetModuleHandle(nullptr);
    if (!g_s4Base) return false;

    g_paEntityPool = reinterpret_cast<IEntity***>(g_s4Base + entityPoolOffsetHE); // all types are 32-bit on most common compilers at least
    g_pEntityPoolMaxOccupation = reinterpret_cast<size_t*>(g_s4Base + entityPoolMaxOccupationHE); // all types are 32-bit on most common compilers at least
    g_paEntityMap = reinterpret_cast<WORD**>(g_s4Base + entityMapOffsetHE);
    g_pMapSize = reinterpret_cast<DWORD*>(g_s4Base + mapSizeOffsetHE);
    g_pAIPartiesBitflags = reinterpret_cast<DWORD*>(g_s4Base + aiPartiesBitflagsOffsetHE);
    g_aPlayerMetadata = reinterpret_cast<SPlayerMetadata*>(g_s4Base + aPlayerMetadataOffset);
    g_pIsAI = reinterpret_cast<IS_AI_PROC>(g_s4Base + IsAi);
    g_paResourceMap = reinterpret_cast<SResource**>(g_s4Base + paResourceMapOffsetHE);

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

ATTACH_VALUE S4WarriorsLib::onAttach() {
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

DETACH_VALUE S4WarriorsLib::onDetach() {
    m_pS4API->RemoveListener(m_luaOpenHook);

    if (!releaseAPI()) {
        return DETACH_VALUE::FAILED_COULD_NOT_RELEASE_API_PTR;
    }
    return DETACH_VALUE::SUCCESS;
}

// lib/table names
static const char* libName = "WarriorsLib";
static const char* resourceTableName = "Resources";
// lib functions
constexpr size_t libfunccount = 14;
static const std::array<struct luaL_reg,
    libfunccount> aWarriorsLibArr{ {
    {const_cast<char*>("Send"), S4WarriorsLib::Send},
    {const_cast<char*>("SelectWarriors"), S4WarriorsLib::SelectWarriors},
    {const_cast<char*>("isHuman"), S4WarriorsLib::isHuman},
    {const_cast<char*>("getPlayerName"), S4WarriorsLib::getPlayerName},
    {const_cast<char*>("RecruitWarriors"), S4WarriorsLib::RecruitWarriors},
    {const_cast<char*>("GarrisonWarriors"), S4WarriorsLib::GarrisonWarriors},
    {const_cast<char*>("UnGarrisonWarriors"), S4WarriorsLib::UnGarrisonWarriors},
    {const_cast<char*>("SetTradingRoute"), S4WarriorsLib::SetTradingRoute},
    {const_cast<char*>("TradeGood"), S4WarriorsLib::TradeGood},
    {const_cast<char*>("StoreGood"), S4WarriorsLib::StoreGood},
    {const_cast<char*>("GetBuildingIdAt"), S4WarriorsLib::GetBuildingIdAt},
    {const_cast<char*>("SetBuildingWorkarea"), S4WarriorsLib::SetBuildingWorkarea},
    {const_cast<char*>("GetResourceCountAt"), S4WarriorsLib::GetResourceCountAt},
    {const_cast<char*>("GetResourceCountInArea"), S4WarriorsLib::GetResourceCountInArea}
} };
const std::map<const char*, S4_MOVEMENT_ENUM> aWarriorsLibMovementVars{
    {"MOVE_FORWARDS",S4_MOVEMENT_ENUM::S4_MOVEMENT_FORWARD},
    {"MOVE_PATROL",S4_MOVEMENT_ENUM::S4_MOVEMENT_PATROL},
    {"MOVE_ACCUMULATE",S4_MOVEMENT_ENUM::S4_MOVEMENT_ACCUMULATE},
    {"MOVE_WATCH",S4_MOVEMENT_ENUM::S4_MOVEMENT_WATCH},
    {"MOVE_STOP",S4_MOVEMENT_ENUM::S4_MOVEMENT_STOP}
};
const std::map<const char*, EResourceType> aResourceTypeValues{
    {"FISH",        EResourceType::FISH},
    {"COAL_ORE",    EResourceType::COAL_ORE},
    {"IRON_ORE",    EResourceType::IRON_ORE},
    {"GOLD_ORE",    EResourceType::GOLD_ORE},
    {"SULFUR_ORE",  EResourceType::SULFUR_ORE},
    {"STONE_ORE",   EResourceType::STONE_ORE},
    {"STONE",       EResourceType::STONE},
    {"WOOD",        EResourceType::WOOD}
};

const std::map<const char*, const char*> aWarriorsLibVars{
    {"VERSION", "1.4.0"},
    {"MAJOR_VERSION", "1"},
    {"MINOR_VERSION", "4"},
    {"AUTHOR", "MuffinMario & Gemil"}
};
// WarriorsLib.Send(group,to_x,to_y,movementtype);
void S4WarriorsLib::Send() {
    // param 1
    auto grouptbl = lua_lua2C(1);
    unsigned int x = luaL_check_int(2);
    unsigned int y = luaL_check_int(3);
    DWORD movementType = S4_MOVEMENT_FORWARD;
    auto param4 = lua_lua2C(4);

    if (lua_isnumber(param4))
        movementType = (DWORD)lua_getnumber(param4);
    if (lua_isnumber(grouptbl)) {
        CSelection& selection = g_aSettlerSelections.at((unsigned int)lua_getnumber(grouptbl));
        m_pS4API->SendWarriors(x, y, static_cast<S4_MOVEMENT_ENUM>(movementType), selection.m_aIds.data(), selection.m_aIds.size(), 0);
    }
    // do nothing?
    //else
    //    lua_error((char*)"Send called with wrong type (not number/table)");
}


/*
    This will call func(x,y) at every pos in a circle, will ignore the outmost outer edges (rectangle (0,0) (mapsize-1,mapsize-1))
*/
template <typename F, typename... P>
void circle(WORD x, WORD y, WORD radius, WORD mapsize, F func) {
    // INT to prevent arithmetic underflow

    int r = radius;
    int xorig = x, yorig = y;

    int its = 2 * r - 1;
    //    basically 
    //
    //      LLX
    //     LLXRR
    //      XRR
    int left = r - 1, right = 0;
    for (int i = its; i > 0; --i) {
#undef max
        int cx = xorig - left;
        int cy = yorig + r - i;

        // this and the following lines will be out of map
        if (cy >= unsigned(mapsize - 1)) break;

        // don't care aobut the ones left from this.
        if (cx < 1) cx = 1;

        while (cx <= xorig + right && cx < unsigned(mapsize - 1)) {
            func(cx, cy);
            ++cx;
        }

        if (right == r - 1) --left;
        else ++right;
    }
}

// selectwarriors(number x, number y, number radius, number party, number warriortype)
void S4WarriorsLib::SelectWarriors() {
    unsigned int x = luaL_check_int(1);
    unsigned int y = luaL_check_int(2);
    unsigned int radius = luaL_check_int(3);
    unsigned int party = luaL_check_int(4);
    DWORD warriortype = luaL_check_int(5);

    CSelection& newSelection = g_aSettlerSelections.emplace_back();

    auto TableAppendInt = [](CSelection& sel, int v) {
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
    circle(x, y, radius, (WORD)S4ModApi::GetMapSize(), CircleSelectWarriors);
    if (newSelection.m_aIds.size() == 0)
    {
        g_aSettlerSelections.pop_back();
    }
    else {
        lua_pushnumber(g_aSettlerSelections.size() - 1);
    }
}

bool checkPartyIsHuman(int party) {
    if (g_pIsAI(party)) 
        return false;
    if (*g_pAIPartiesBitflags & (1 << party))
        return false;
    return true;

}
 
// isHuman(number party)
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
void S4WarriorsLib::getPlayerName()
{
    auto party = luaL_check_int(1);
    if (party >= 1 && unsigned(party) <= (m_pS4API->GetNumberOfPlayers()))
    {
        // we have no reliable way to instanciate UTF8 names at start (onluaopen untested), so.. yeah
        static std::string playerUTF8Username[9];
        std::wstring s = g_aPlayerMetadata[party].playerName.c_str();
        playerUTF8Username[party] = WstrToUtf8Str(s);

        lua_pushstring(const_cast<char*>(playerUTF8Username[party].c_str()));
    }
}

// RecruitWarriors(number buildingid, number warriortype, number amount, number party)
void S4WarriorsLib::RecruitWarriors() {  
    auto buildingid = luaL_check_int(1);
    auto warriortype = luaL_check_int(2);
    auto amount = luaL_check_int(3);
    auto party = luaL_check_int(4);

    if (checkPartyIsHuman(party)) {
        if (m_pS4API->GetLocalPlayer() == party) {
            m_pS4API->RecruitWarriors(buildingid, static_cast<S4_SETTLER_ENUM>(warriortype), amount, party);
        }
    }
    else {
        m_pS4API->RecruitWarriors(buildingid, static_cast<S4_SETTLER_ENUM>(warriortype), amount, party);
    }

}


// GarrisonWarriors(number buildingid, number party)
void S4WarriorsLib::GarrisonWarriors() {
    auto buildingid = luaL_check_int(1);
    auto party = luaL_check_int(2);

    if (checkPartyIsHuman(party)) {
        if (m_pS4API->GetLocalPlayer() == party) {
            m_pS4API->GarrisonWarriors(buildingid, party);
        }
    }
    else {
        m_pS4API->GarrisonWarriors(buildingid, party);
    }
}

// UnGarrisonWarriors(number buildingid, number column, boolean bowman, number party)
void S4WarriorsLib::UnGarrisonWarriors() {
    auto buildingid = luaL_check_int(1);
    auto column = luaL_check_int(2);
    auto bowman = luaL_check_int(3);
    auto party = luaL_check_int(4);

    if (checkPartyIsHuman(party)) {
        if (m_pS4API->GetLocalPlayer() == party) {
            m_pS4API->UnGarrisonWarriors(buildingid, column, bowman, party);
        }
    }
    else {
        m_pS4API->UnGarrisonWarriors(buildingid, column, bowman, party);
    }
}

// SetTradingRoute(number buildingid_source, number buildingid_dest, number party)
void S4WarriorsLib::SetTradingRoute() {
    auto buildingid_source = luaL_check_int(1);
    auto buildingid_dest = luaL_check_int(2);
    auto party = luaL_check_int(3);

    if (checkPartyIsHuman(party)) {
        if (m_pS4API->GetLocalPlayer() == party) {
            m_pS4API->SetTradingRoute(buildingid_source, buildingid_dest, party);
        }
    }
    else {
        m_pS4API->SetTradingRoute(buildingid_source, buildingid_dest, party);
    }
}


// TradeGood(number buildingid, number goodtype, number amount, number party)
void S4WarriorsLib::TradeGood() {
    auto buildingid = luaL_check_int(1);
    auto goodtype = luaL_check_int(2);
    auto amount = luaL_check_int(3);
    auto party = luaL_check_int(4);

    if (checkPartyIsHuman(party)) {
        if (m_pS4API->GetLocalPlayer() == party) {
            m_pS4API->TradeGood(buildingid, static_cast<S4_GOOD_ENUM>(goodtype), amount, party);
        }
    }
    else {
        m_pS4API->TradeGood(buildingid, static_cast<S4_GOOD_ENUM>(goodtype), amount, party);
    }
}
void S4WarriorsLib::GetResourceCountInArea()
{
    if (g_paResourceMap && *g_paResourceMap && g_pMapSize)
    {
        auto x = luaL_check_int(1);
        auto y = luaL_check_int(2);
        auto r = luaL_check_int(3);
        auto type = lua_lua2C(4);
        bool typeisNum = lua_isnumber(type);
        EResourceType typeParam;
        if(typeisNum)
            typeParam = static_cast<EResourceType>(lua_getnumber(type));
        uint32_t count = 0U;
        circle(x, y, r, *g_pMapSize,
            [&typeisNum, &count, &typeParam](const int& cx, const int& cy) {
                if (typeisNum)
                {
                    if (static_cast<EResourceType>(typeParam) == (*g_paResourceMap)[cx + cy * (*g_pMapSize)].getResourceType())
                        count += static_cast<uint8_t>((*g_paResourceMap)[cx + cy * (*g_pMapSize)].getResourceCount());
                }
                else {
                    count += static_cast<uint8_t>((*g_paResourceMap)[cx + cy * (*g_pMapSize)].getResourceCount());
                }
            }
        );
        lua_pushnumber(count);
    }
}
void S4WarriorsLib::GetResourceCountAt()
{
    if (g_paResourceMap && *g_paResourceMap && g_pMapSize)
    {
        auto x = luaL_check_int(1);
        auto y = luaL_check_int(2);
        auto type = lua_lua2C(3);
        if (x >= 0 && x < *g_pMapSize &&
            y >= 0 && y < *g_pMapSize)
        {
            if (lua_isnumber(type))
            {
                auto typeNum = lua_getnumber(type);
                if (static_cast<EResourceType>(typeNum) == (*g_paResourceMap)[x + y * (*g_pMapSize)].getResourceType())
                    lua_pushnumber(static_cast<uint8_t>((*g_paResourceMap)[x + y * (*g_pMapSize)].getResourceCount()));
                else
                    lua_pushnumber(0.0);
            }
            else {
                lua_pushnumber(static_cast<uint8_t>((*g_paResourceMap)[x + y * (*g_pMapSize)].getResourceCount()));
            }
        }
    }
}
// StoreGood(number buildingid, number goodtype, boolean enable, number party)
void S4WarriorsLib::StoreGood() {
    auto buildingid = luaL_check_int(1);
    auto goodtype = luaL_check_int(2);
    auto enable = luaL_check_int(3);
    auto party = luaL_check_int(4);

    if (checkPartyIsHuman(party)) {
        if (m_pS4API->GetLocalPlayer() == party) {
            m_pS4API->StoreGood(buildingid, static_cast<S4_GOOD_ENUM>(goodtype), enable, party);
        }
    }
    else {
        m_pS4API->StoreGood(buildingid, static_cast<S4_GOOD_ENUM>(goodtype), enable, party);
    }
}

// SetBuildingWorkarea(number buildingid, number x, number y, number party)
void S4WarriorsLib::SetBuildingWorkarea() {
    int dummy = luaL_check_int(1); // remove me
    auto buildingid = luaL_check_int(1);
    auto x = luaL_check_int(2);
    auto y = luaL_check_int(3);
    auto party = luaL_check_int(4);

    if (checkPartyIsHuman(party)) {
        if (m_pS4API->GetLocalPlayer() == party) {
            m_pS4API->SetBuildingWorkarea(buildingid, x, y, party);
        }
    }
    else {
        m_pS4API->SetBuildingWorkarea(buildingid, x, y, party);
    }
}

auto GetBuildingID(WORD x, WORD y) {
    auto pEntity = S4ModApi::GetEntityAt(x, y);
    if (pEntity && pEntity->GetClass() == S4_ENTITY_ENUM::S4_ENTITY_BUILDING)
        return pEntity->id;
    else
        return 0UL;
}

// GetBuildingIdAt(number x, number y)
void S4WarriorsLib::GetBuildingIdAt() {
    auto x = luaL_check_int(1);
    auto y = luaL_check_int(2);

    lua_pushnumber(GetBuildingID(x,y));
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

    //insert other constants in table
    for (auto& red : aWarriorsLibVars)
        CLuaUtils::addtableval(lua_getglobal(const_cast<char*>(libName)), red.first, const_cast<char*>(red.second));
    
    //insert table WarriorsLib.Resources
    t = lua_createtable();
    CLuaUtils::push(t);
    CLuaUtils::addtableval(lua_getglobal(const_cast<char*>(libName)),resourceTableName, t);
    //insert WariorsLib.Resources elements
    for (auto& red : aResourceTypeValues)
        CLuaUtils::addtableval(t, red.first, static_cast<double>(red.second));
}
