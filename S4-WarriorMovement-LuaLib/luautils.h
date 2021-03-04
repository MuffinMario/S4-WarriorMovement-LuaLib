#pragma once
#include "S4ModApi.h"
#include <variant>
class CLuaUtils {
public:
	__declspec(noinline) static void push(double);
	__declspec(noinline) static void push(const char*);
	__declspec(noinline) static void push(lua_Object);
	__declspec(noinline) static void push();
	__declspec(noinline) static void push(lua_CFunction);

	__declspec(noinline) static void addtableval(std::variant<lua_Object, const char*> t, std::variant<double, const char*> i, std::variant<double, const char*, lua_Object, nullptr_t, lua_CFunction> v);

};

