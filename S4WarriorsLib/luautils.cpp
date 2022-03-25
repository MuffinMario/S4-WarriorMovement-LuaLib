#include "pch.h"
#include "luautils.h"
#include "S4ModApi.h"

void CLuaUtils::push(double v)
{
	lua_pushnumber(v);
}

void CLuaUtils::push(const char* s)
{
	lua_pushstring(const_cast<char*>(s));
}

void CLuaUtils::push(lua_Object o)
{
	lua_pushobject(o);
}

void CLuaUtils::push()
{
	lua_pushnil();
}

void CLuaUtils::push(lua_CFunction f)
{
	lua_pushcfunction(f);
}
void CLuaUtils::addtableval(std::variant<lua_Object, const char*> t, std::variant<double, const char*> i, std::variant<double, const char*, lua_Object, nullptr_t, lua_CFunction> v)
{
	// table or global table name
	if (t.index() == 0)
		push(std::get<lua_Object>(t));
	else if (t.index() == 1)
		push(std::get<const char*>(t));

	// index by number or string

	if (i.index() == 0)
		push(std::get<double>(i));
	else if (i.index() == 1)
		push(std::get<const char*>(i));

	// value
	switch (v.index()) {
	case 0:
		push(std::get<double>(v));
		break;
	case 1:
		push(std::get<const char*>(v));
		break;
	case 2:
		push(std::get<lua_Object>(v));
		break;
	case 3:
		push();
		break;
	case 4:
		push(std::get<lua_CFunction>(v));
		break;
	}
	// finally, t[i] = v
	lua_settable();
}
