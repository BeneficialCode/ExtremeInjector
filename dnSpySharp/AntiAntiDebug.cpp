#include "pch.h"
#include "AntiAntiDebug.h"
#include <ntstatus.h>

PNtQueryInformationProcess pNtQueryInformationProcess = nullptr;

NTSTATUS NTAPI HookedNtQueryInformationProcess(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
) {
    NTSTATUS status = pNtQueryInformationProcess(
        ProcessHandle,
        ProcessInformationClass,
        ProcessInformation,
        ProcessInformationLength,
        ReturnLength
    );
    if (ProcessInformationClass == ProcessDebugObjectHandle) {
        status = STATUS_UNSUCCESSFUL;
    }
    else if (ProcessInformationClass == ProcessDebugFlags) {
        status = STATUS_UNSUCCESSFUL;
	}
    else if (ProcessInformationClass == ProcessDebugPort) {
        status = STATUS_UNSUCCESSFUL;
    }
    // Call the original function
    return status;
}