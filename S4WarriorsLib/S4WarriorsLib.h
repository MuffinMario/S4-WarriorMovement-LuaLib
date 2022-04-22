#pragma once
#include <string>

enum class ATTACH_VALUE {
	SUCCESS = 0,
	FAILED_COULD_NOT_CREATE_API_PTR,
	FAILED_COULD_NOT_HOOK_LISTENERS,
	FAILED_NOT_HISTORY_EDITION,
	FAILED_S4_MODULE_NOT_FOUND,
	FAILED_UNKNOWN
};
enum class DETACH_VALUE {
	SUCCESS = 0,
	FAILED_COULD_NOT_RELEASE_API_PTR,
	FAILED_UNKNOWN
};

struct IS4ModInterface {
	virtual ATTACH_VALUE onAttach() = 0;
	virtual DETACH_VALUE onDetach() = 0;

	static const std::string getLastErrorStr() {
		return m_lastErrorStr;
	}
protected:
	static std::string m_lastErrorStr;
	static S4API m_pS4API;

	static bool createAPI();
	static bool releaseAPI();

};
struct S4WarriorsLib :public IS4ModInterface {
	static ATTACH_VALUE onAttach();
	static DETACH_VALUE onDetach();

	/* Lua functions */
	static void Send();
	static void SelectWarriors();
	static void isHuman();
	static void getPlayerName();
	static void RecruitWarriors();
	static void GarrisonWarriors();
	static void UnGarrisonWarriors();
	static void SetTradingRoute();
	static void TradeGood();
	static void StoreGood();
	static void GetBuildingIdAt();
	static void SetBuildingWorkarea();
	static void GetResourceCountInArea();
	static void GetResourceCountAt();



private:
	static HRESULT __stdcall onLuaOpen();
	static void lua_wmlibopen();

	static S4HOOK m_luaOpenHook;
};


