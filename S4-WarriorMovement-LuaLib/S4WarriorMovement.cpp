#include "pch.h"
#include "S4WarriorMovement.h"
#include "luautils.h"
#include "S4Entity.h"
#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>

using namespace std::string_literals;
S4API IS4ModInterface::m_pS4API;
S4HOOK S4WarriorMovement::m_luaOpenHook = NULL;

IEntity*** g_paSettlerPool;
WORD** g_paEntityMap;
DWORD* g_pMapSize;
//offsets, all HE because bruh imagine GE
constexpr size_t entityPoolOffsetHE = 0xECDE9;
constexpr size_t entityMapOffsetHE = 0x11630DC;
constexpr size_t mapSizeOffsetHE = 0xD6921C;

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

    g_paSettlerPool = (IEntity***)(g_s4Base + entityPoolOffsetHE); // all types are 32-bit on most common compilers at least
    g_paEntityMap = (WORD**)(g_s4Base + entityMapOffsetHE); 
    g_pMapSize = (DWORD*)(g_s4Base + mapSizeOffsetHE);

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

ATTACH_VALUE S4WarriorMovement::onAttach()
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

DETACH_VALUE S4WarriorMovement::onDetach()
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
constexpr size_t libfunccount = 2;

static std::array<struct luaL_reg,
    libfunccount
#if _DEBUG 
    + 1
#endif
> aWarriorsLibArr{ {
    {const_cast<char*>("Send"),S4WarriorMovement::Send},
    {const_cast<char*>("SelectWarriors"),S4WarriorMovement::SelectWarriors}
#ifdef _DEBUG
    ,{const_cast<char*>("warriorDebug"),S4WarriorMovement::warriorDebug}
#endif
} };
std::map<const char*, S4_MOVEMENT_ENUM> aWarriorsLibMovementVars{
    {"MOVE_FORWARDS",S4_MOVEMENT_ENUM::S4_MOVEMENT_FORWARD},
    {"MOVE_PATROL",S4_MOVEMENT_ENUM::S4_MOVEMENT_PATROL},
    {"MOVE_ACCUMULATE",S4_MOVEMENT_ENUM::S4_MOVEMENT_ACCUMULATE},
    {"MOVE_WATCH",S4_MOVEMENT_ENUM::S4_MOVEMENT_WATCH},
    {"MOVE_STOP",S4_MOVEMENT_ENUM::S4_MOVEMENT_STOP}
};

// WarriorsLib.Send(group,to_x,to_y,movementtype);
void S4WarriorMovement::Send()
{
    // param 1
    auto grouptbl = lua_lua2C(1);
    int x = luaL_check_int(2);
    int y = luaL_check_int(3);
    DWORD movementType = S4_MOVEMENT_FORWARD;
    auto param4 = lua_lua2C(4);
    if (lua_isnumber(param4))
        movementType = lua_getnumber(param4);
    if (lua_istable(grouptbl)) {
        std::vector<WORD> warriorIDs;
        CLuaUtils::push(grouptbl);
        CLuaUtils::push(); // push nil
        lua_callfunction(lua_getglobal((char*)"next"));
        lua_Object i = lua_getresult(1);
        lua_Object v = lua_getresult(2);
        while (!lua_isnil(i)) {

            if (lua_isnumber(v)) {
                int warriorID = lua_getnumber(v);
                warriorIDs.push_back(warriorID);
            }
            CLuaUtils::push(grouptbl);
            CLuaUtils::push(i);
            lua_callfunction(lua_getglobal((char*)"next"));
            i = lua_getresult(1);
            v = lua_getresult(2);
        }
        m_pS4API->SendWarriors(x, y, static_cast<S4_MOVEMENT_ENUM>(movementType), warriorIDs.data(), warriorIDs.size(), 1);
    }
    else if (lua_isnumber(grouptbl)) { // one id for some reason
        WORD warrior = (WORD)lua_getnumber(grouptbl);
        m_pS4API->SendWarriors(x, y, S4_MOVEMENT_ENUM::S4_MOVEMENT_FORWARD, &warrior,1, 1);
    }
    // do nothing?
    //else
    //    lua_error((char*)"Send called with wrong type (not number/table)");
}
template <typename F,typename... P>
void circle(WORD x, WORD y, WORD radius,WORD mapsize,F func) {
    unsigned int r = radius;
    unsigned int xorig =  x, yorig = y;

    unsigned int its = 2 * r - 1;
    //    basically 
    //
    //      LLX
    //     LLXRR
    //      XRR
    unsigned int left = r-1, right = 0;
    for (unsigned int i = its; i > 0; --i) {
        unsigned int cx = xorig - left;
        unsigned int cy = yorig + r - i;

        // this and the following lines will be out of map
        if (cy >= mapsize-1) break;

        // don't care aobut the ones left from this.
        if (cx < 1) cx = 1;

        while (cx <= xorig + right && cx < mapsize-1) {
            func(cx,cy);
            ++cx;
        }

        if (right == r - 1) --left;
        else ++right;
    }
}
// selectwarriors(number x, number y, number radius, number party, number warriortype)
void S4WarriorMovement::SelectWarriors() {
    auto x = luaL_check_int(1);
    auto y = luaL_check_int(2);
    auto radius = luaL_check_int(3);
    auto party = luaL_check_int(4);
    auto warriortype = luaL_check_int(5);

    auto t = lua_createtable();
    auto TableAppendInt = [](lua_Object& table, int v) {
        CLuaUtils::push(table);
        lua_callfunction(lua_getglobal((char*)"getn"));
        auto val = lua_getresult(1);
        int n = lua_getnumber(val);
        CLuaUtils::addtableval(table, n + 1, (double)v);
    };
    auto CircleSelectWarriors = [&t,&warriortype,&party,TableAppendInt](WORD x, WORD y) -> void {
        auto pEntity = S4ModApi::GetEntityAt(x, y);
        if (pEntity) {
             // only move settlers, carts, warmachines, boats, manacopter
             if (pEntity->GetClass() != S4_ENTITY_ENUM::S4_ENTITY_SETTLER && !(pEntity->GetClass() >= S4_ENTITY_ENUM::S4_ENTITY_CART && pEntity->GetClass() <= S4_ENTITY_ENUM::S4_ENTITY_MANAKOPTER))
                return;
            auto settlertype = pEntity->objectId;
            auto settlerID = pEntity->id2;
            auto player = pEntity->tribe; // nyfrk mistake, tribe => player, player => tribe
            if (settlertype == warriortype && player == party) {
                TableAppendInt(t, settlerID);
            }
        }
    };
    circle(x, y, radius, S4ModApi::GetMapSize(), CircleSelectWarriors);
    CLuaUtils::push(t);
}

// just a debug function
void S4WarriorMovement::warriorDebug() {
    auto mapsize = S4ModApi::GetMapSize();
    auto entityID = S4ModApi::GetEntityIdAt(mapsize / 2, mapsize / 2);
    m_pS4API->ShowTextMessage(("Entity ID at middle of map: "s + std::to_string(entityID)).c_str(), 0, 0);
    if (entityID) {
        auto entity = (*g_paSettlerPool)[entityID];
        std::stringstream entityData;
        entityData << "Entity data: " << entity << " / " << entity->x << "|" << entity->y;
        m_pS4API->ShowTextMessage(entityData.str().c_str(), 0, 0);
    }
}


HRESULT __stdcall S4WarriorMovement::onLuaOpen()
{
    lua_wmlibopen();
    return 0;
}

void S4WarriorMovement::lua_wmlibopen()
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

    CLuaUtils::addtableval(lua_getglobal(const_cast<char*>(libName)), "VERSION", "1.0");

}
