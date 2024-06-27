// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "AntiAntiDebug.h"
#include <detours/detours.h>

extern PNtQueryInformationProcess pNtQueryInformationProcess;

DWORD WINAPI SetHook(void* params) {
    HMODULE hModule = ::GetModuleHandle(L"ntdll.dll");
    if (hModule == nullptr) {
		return -1;
	}
    pNtQueryInformationProcess = (PNtQueryInformationProcess)::GetProcAddress(hModule, "NtQueryInformationProcess");
    if (pNtQueryInformationProcess == nullptr) {
        return -1;
    }
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pNtQueryInformationProcess, HookedNtQueryInformationProcess);
    DetourTransactionCommit();

    return 0;
}

void RemoveHook() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pNtQueryInformationProcess, HookedNtQueryInformationProcess);
    DetourTransactionCommit();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE hThread = ::CreateThread(nullptr, 0, SetHook, nullptr, 0, nullptr);
        ::CloseHandle(hThread);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        RemoveHook();
        break;
    }
    return TRUE;
}

