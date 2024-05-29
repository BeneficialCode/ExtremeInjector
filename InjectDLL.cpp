#include "pch.h"
#include "InjectDLL.h"
#include <PEParser.h>
#include <TlHelp32.h>

static PRtlCreateThread g_pRtlCreateUserThread = nullptr;
static PNtWriteVirtualMemory g_pNtWriteVirtualMemory = nullptr;
static BOOL g_bIs64Bit = FALSE;

unsigned char ShellCodeFun32[] =
{
  0xE8, 0x00, 0x00, 0x00, 0x00, 0x5B, 0x83, 0xEB, 0x05, 0x3E,
  0xFF, 0x33, 0x3E, 0xFF, 0x13, 0x33, 0xC0, 0x50, 0x6A, 0xFE,
  0x3E, 0xFF, 0x13, 0x90, 0x90, 0x90, 0x90
};


BOOL InjectDllToProcess(DWORD pid,BOOL isX64) {
	g_bIs64Bit = isX64;
	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL) {
		return FALSE;
	}

	WCHAR path[MAX_PATH];
	::GetModuleFileName(nullptr, path, MAX_PATH);
	auto bs = wcsrchr(path, L'\\');
	*bs = 0;
	
	if (g_bIs64Bit)
		wcscat_s(path, L"\\ProxyLoader64.dll");
	else
		wcscat_s(path, L"\\ProxyLoader32.dll");

	// Is the dll file exists?
	if (::GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES) {
		OutputDebugString(L"missing proxy loader dll.");
		::CloseHandle(hProcess);
		return FALSE;
	}

	ULONG size = (wcslen(path) + 1) * sizeof(WCHAR);
	LPTSTR pLibFileRemote = (LPTSTR)::VirtualAllocEx(hProcess, nullptr,size,MEM_COMMIT, PAGE_READWRITE);
	if (pLibFileRemote == NULL) {
		OutputDebugString(L"VirtualAllocEx failed");
		::CloseHandle(hProcess);
		return FALSE;
	}

	NTSTATUS status = NtWriteVirtualMemory(hProcess, pLibFileRemote, path, size, nullptr);
	if (!NT_SUCCESS(status)) {
		OutputDebugString(L"NtWriteVirtualMemory failed");
		::VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
		::CloseHandle(hProcess);
		return FALSE;
	}
	HMODULE hKernel32 = ::GetModuleHandle(L"Kernel32");
	if (hKernel32 == NULL) {
		OutputDebugString(L"GetModuleHandle failed");
		::VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
		::CloseHandle(hProcess);
		return FALSE;
	}
	
	LPTHREAD_START_ROUTINE pLoadLibraryW = nullptr;
	if (g_bIs64Bit) {
		pLoadLibraryW = (LPTHREAD_START_ROUTINE)::GetProcAddress(hKernel32, "LoadLibraryW");
	}
	else {
		std::wstring systemRoot = USER_SHARED_DATA->NtSystemRoot;
		std::wstring dllPath = systemRoot + L"\\SysWOW64\\Kernel32.dll";
		PEParser parser(dllPath.c_str());
		std::vector<ExportedSymbol> exports = parser.GetExports();
		ULONG rva = 0;
		for (auto& exp : exports) {
			if (exp.Name == "LoadLibraryW") {
				rva = exp.Address;
				break;
			}
		}
		ULONG_PTR base = GetKernel32BaseAddress(pid);
		if (base == 0) {
			OutputDebugString(L"GetKernel32BaseAddress failed");
			::VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
			::CloseHandle(hProcess);
			return FALSE;
		}
		pLoadLibraryW = (LPTHREAD_START_ROUTINE)(base + rva);
		CString str;
		str.Format(L"LoadLibraryW: %p", pLoadLibraryW);
		OutputDebugString(str);
	}
	if (pLoadLibraryW == NULL) {
		OutputDebugString(L"GetProcAddress failed");
		::VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
		::CloseHandle(hProcess);
		return FALSE;
	}

	HANDLE hThread = RtlCreateRemoteThread(hProcess, nullptr, 0, pLoadLibraryW, pLibFileRemote, 0, nullptr);
	if (hThread == NULL) {
		OutputDebugString(L"RtlCreateRemoteThread failed");
		::VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
		::CloseHandle(hProcess);
		return FALSE;
	}

	OutputDebugString(L"Inject successfullly");
	::CloseHandle(hThread);
	return TRUE;
}

NTSTATUS NtWriteVirtualMemory(
	IN HANDLE               ProcessHandle,
	IN PVOID                BaseAddress,
	IN PVOID                Buffer,
	IN ULONG                NumberOfBytesToWrite,
	OUT PULONG              NumberOfBytesWritten OPTIONAL
) {
	if (g_pNtWriteVirtualMemory == nullptr) {
		HMODULE hNtDll = ::GetModuleHandle(L"ntdll.dll");
		if (hNtDll == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}
		g_pNtWriteVirtualMemory = (PNtWriteVirtualMemory)::GetProcAddress(hNtDll, "NtWriteVirtualMemory");
		if (g_pNtWriteVirtualMemory == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}
	}
	return g_pNtWriteVirtualMemory(ProcessHandle, BaseAddress, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten);
}

HANDLE RtlCreateRemoteThread(
	IN  HANDLE hProcess,
	IN  LPSECURITY_ATTRIBUTES lpThreadAttributes,
	IN  DWORD dwStackSize,
	IN  LPTHREAD_START_ROUTINE lpStartAddress,
	IN  LPVOID lpParameter,
	IN  DWORD dwCreationFlags,
	OUT LPDWORD lpThreadId
) {
	NTSTATUS status = STATUS_SUCCESS;
	CLIENT_ID cid;
	HANDLE hThread = nullptr;

	if (hProcess == nullptr || lpStartAddress == nullptr) {
		return nullptr;
	}
	
	if (g_pRtlCreateUserThread == nullptr) {
		HMODULE hNtDll = ::GetModuleHandle(L"ntdll.dll");
		if (hNtDll == nullptr) {
			return nullptr;
		}
		g_pRtlCreateUserThread = (PRtlCreateThread)::GetProcAddress(hNtDll, "RtlCreateUserThread");
		if (g_pRtlCreateUserThread == nullptr) {
			return nullptr;
		}
	}

	status = g_pRtlCreateUserThread(hProcess, nullptr, FALSE,
		0, dwStackSize, 0,
		lpStartAddress,
		lpParameter,
		&hThread, &cid);

	if (!NT_SUCCESS(status)) {
		return nullptr;
	}

	if (lpThreadId != nullptr) {
		*lpThreadId = HandleToULong(cid.UniqueThread);
	}
	if (!(dwCreationFlags & CREATE_SUSPENDED)) {
		::ResumeThread(hThread);
	}

	return hThread;
}

ULONG_PTR GetKernel32BaseAddress(DWORD pid) {
	wil::unique_handle hSnapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid));
	if (!hSnapshot) {
		return 0;
	}

	MODULEENTRY32 me = { sizeof(me) };
	if (!::Module32First(hSnapshot.get(), &me)) {
		return 0;
	}

	do {
		if (_wcsicmp(me.szModule, L"Kernel32.dll") == 0) {
			return (ULONG_PTR)me.modBaseAddr;
		}
	} while (::Module32Next(hSnapshot.get(), &me));

	return 0;
}