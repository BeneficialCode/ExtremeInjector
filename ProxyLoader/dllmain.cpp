// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <metahost.h>
#include <string>
#include <fstream>
#include "IniFile.h"

#pragma comment(lib,"mscoree.lib")


DWORD WINAPI Loader(void* params) {
    ICLRMetaHost* pMetaHost = nullptr;
    ICLRRuntimeInfo* pRuntimeInfo = nullptr;
    ICLRRuntimeHost* pRuntimeHost = nullptr;

    WCHAR path[MAX_PATH];
    ::GetModuleFileName(nullptr, path, MAX_PATH);
    auto bs = wcsrchr(path, L'\\');
    *bs = 0;
    wcscat_s(path, L"\\Hook.ini");

    OutputDebugString(path);

    IniFile ini(path);

    CString dll = ini.ReadString(L"HookInfo", L"Dll");
    OutputDebugString(dll);

    CString typeName = ini.ReadString(L"HookInfo", L"TypeName");
    OutputDebugString(typeName);

    CString method = ini.ReadString(L"HookInfo", L"Method");
    OutputDebugString(method);

    CString arg = ini.ReadString(L"HookInfo", L"Argument");
    OutputDebugString(arg);

    // Tring to get runtime meta host
    HRESULT hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&pMetaHost);
    if (FAILED(hr))
    {
        OutputDebugString(L"CLRCreateInstance failed");
        return -1;
    }

    // Get runtime version
    WCHAR version[MAX_PATH];
    DWORD size = MAX_PATH;
    hr = pMetaHost->GetVersionFromFile(dll, version, &size);
    if (FAILED(hr))
    {
        OutputDebugString(L"GetVersionFromFile failed");
        return -1;
    }

    // Get runtime info
    hr = pMetaHost->GetRuntime(version, IID_ICLRRuntimeInfo, (LPVOID*)&pRuntimeInfo);
    if (FAILED(hr))
    {
        OutputDebugString(L"GetRuntime failed");
        return -1;
    }

    // Trying to get runtime host
    pRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (LPVOID*)&pRuntimeHost);
    if (FAILED(hr))
    {
        OutputDebugString(L"GetInterface failed");
        return -1;
    }

    DWORD value = 0;
    hr = pRuntimeHost->ExecuteInDefaultAppDomain(dll, typeName, method, arg, &value);
    if (FAILED(hr))
    {
        OutputDebugString(L"ExecuteInDefaultAppDomain failed");
        return -1;
    }

    pRuntimeHost->Release();
    pRuntimeInfo->Release();
    pMetaHost->Release();

    return 0;
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
        HANDLE hThread = ::CreateThread(nullptr, 0, Loader, nullptr, 0, nullptr);
        ::CloseHandle(hThread);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

