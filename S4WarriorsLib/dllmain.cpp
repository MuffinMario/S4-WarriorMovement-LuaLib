// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "pch.h"
#include "S4WarriorsLib.h"
#pragma comment(lib,"S4ModApi")

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (S4WarriorsLib::onAttach() != ATTACH_VALUE::SUCCESS) {
            return FALSE;
        }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if (S4WarriorsLib::onDetach() != DETACH_VALUE::SUCCESS) {
            return FALSE;
        }
        break;
    }
    return TRUE;
}

