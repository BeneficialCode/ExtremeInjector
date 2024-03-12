#pragma once

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
#define STATUS_UNSUCCESSFUL					((NTSTATUS) 0xC0000001L)
#define STATUS_INFO_LENGTH_MISMATCH			((NTSTATUS) 0xC0000004L) 
#define STATUS_IO_DEVICE_ERROR              ((NTSTATUS) 0xC0000185L) 
#define STATUS_ACCESS_DENIED             ((NTSTATUS)0xC0000022L)

typedef PVOID PUSER_THREAD_START_ROUTINE;

typedef struct _CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID* PCLIENT_ID;

typedef NTSTATUS(__stdcall* PRtlCreateThread)(
	IN HANDLE Process,
	IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
	IN BOOLEAN CreateSuspended,
	IN ULONG ZeroBits OPTIONAL,
	IN SIZE_T MaximumStackSize OPTIONAL,
	IN SIZE_T CommittedStackSize OPTIONAL,
	IN PUSER_THREAD_START_ROUTINE StartAddress,
	IN PVOID Parameter OPTIONAL,
	OUT PHANDLE Thread OPTIONAL,
	OUT PCLIENT_ID ClientId OPTIONAL
	);

typedef NTSTATUS(NTAPI* PNtWriteVirtualMemory)(
	IN HANDLE               ProcessHandle,
	IN PVOID                BaseAddress,
	IN PVOID                Buffer,
	IN ULONG                NumberOfBytesToWrite,
	OUT PULONG              NumberOfBytesWritten OPTIONAL
	);

#include <pshpack4.h>

typedef struct _KSYSTEM_TIME
{
    ULONG LowPart;
    LONG High1Time;
    LONG High2Time;
} KSYSTEM_TIME, * PKSYSTEM_TIME;

#define PROCESSOR_FEATURE_MAX 64

typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE
{
    StandardDesign,
    NEC98x86,
    EndAlternatives
} ALTERNATIVE_ARCHITECTURE_TYPE;

typedef struct _KUSER_SHARED_DATA
{
    ULONG TickCountLowDeprecated;
    ULONG TickCountMultiplier;

    volatile KSYSTEM_TIME InterruptTime;
    volatile KSYSTEM_TIME SystemTime;
    volatile KSYSTEM_TIME TimeZoneBias;

    USHORT ImageNumberLow;
    USHORT ImageNumberHigh;

    WCHAR NtSystemRoot[260];

    ULONG MaxStackTraceDepth;

    ULONG CryptoExponent;

    ULONG TimeZoneId;
    ULONG LargePageMinimum;
    ULONG AitSamplingValue;
    ULONG AppCompatFlag;
    ULONGLONG RNGSeedVersion;
    ULONG GlobalValidationRunlevel;
    LONG TimeZoneBiasStamp;
    ULONG Reserved2;

    ULONG NtProductType;
    BOOLEAN ProductTypeIsValid;
    UCHAR Reserved0[1];
    USHORT NativeProcessorArchitecture;

    ULONG NtMajorVersion;
    ULONG NtMinorVersion;

    BOOLEAN ProcessorFeatures[PROCESSOR_FEATURE_MAX];

    ULONG Reserved1;
    ULONG Reserved3;

    volatile ULONG TimeSlip;

    ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;
    ULONG AltArchitecturePad[1];

    LARGE_INTEGER SystemExpirationDate;

    ULONG SuiteMask;

    BOOLEAN KdDebuggerEnabled;
    union
    {
        UCHAR MitigationPolicies;
        struct
        {
            UCHAR NXSupportPolicy : 2;
            UCHAR SEHValidationPolicy : 2;
            UCHAR CurDirDevicesSkippedForDlls : 2;
            UCHAR Reserved : 2;
        };
    };
    UCHAR Reserved6[2];

    volatile ULONG ActiveConsoleId;

    volatile ULONG DismountCount;

    ULONG ComPlusPackage;

    ULONG LastSystemRITEventTickCount;

    ULONG NumberOfPhysicalPages;

    BOOLEAN SafeBootMode;
    UCHAR Reserved12[3];

    union
    {
        ULONG SharedDataFlags;
        struct
        {
            ULONG DbgErrorPortPresent : 1;
            ULONG DbgElevationEnabled : 1;
            ULONG DbgVirtEnabled : 1;
            ULONG DbgInstallerDetectEnabled : 1;
            ULONG DbgLkgEnabled : 1;
            ULONG DbgDynProcessorEnabled : 1;
            ULONG DbgConsoleBrokerEnabled : 1;
            ULONG DbgSecureBootEnabled : 1;
            ULONG SpareBits : 24;
        };
    };
    ULONG DataFlagsPad[1];

    ULONGLONG TestRetInstruction;
    ULONGLONG QpcFrequency;
    ULONGLONG SystemCallPad[3];

    union
    {
        volatile KSYSTEM_TIME TickCount;
        volatile ULONG64 TickCountQuad;
        ULONG ReservedTickCountOverlay[3];
    };
    ULONG TickCountPad[1];

    ULONG Cookie;
    ULONG CookiePad[1];

    LONGLONG ConsoleSessionForegroundProcessId;
    ULONGLONG TimeUpdateLock;
    ULONGLONG BaselineSystemTimeQpc;
    ULONGLONG BaselineInterruptTimeQpc;
    ULONGLONG QpcSystemTimeIncrement;
    ULONGLONG QpcInterruptTimeIncrement;
    ULONG QpcSystemTimeIncrement32;
    ULONG QpcInterruptTimeIncrement32;
    UCHAR QpcSystemTimeIncrementShift;
    UCHAR QpcInterruptTimeIncrementShift;
    UCHAR Reserved8[14];

    USHORT UserModeGlobalLogger[16];
    ULONG ImageFileExecutionOptions;

    ULONG LangGenerationCount;
    ULONGLONG Reserved4;
    volatile ULONG64 InterruptTimeBias;
    volatile ULONG64 QpcBias;

    volatile ULONG ActiveProcessorCount;
    volatile UCHAR ActiveGroupCount;
    UCHAR Reserved9;
    union
    {
        USHORT QpcData;
        struct
        {
            UCHAR QpcBypassEnabled : 1;
            UCHAR QpcShift : 1;
        };
    };

    LARGE_INTEGER TimeZoneBiasEffectiveStart;
    LARGE_INTEGER TimeZoneBiasEffectiveEnd;
    XSTATE_CONFIGURATION XState;
} KUSER_SHARED_DATA, * PKUSER_SHARED_DATA;
#include <poppack.h>

#define USER_SHARED_DATA ((KUSER_SHARED_DATA * const)0x7ffe0000)

NTSTATUS NtWriteVirtualMemory(
	IN HANDLE               ProcessHandle,
	IN PVOID                BaseAddress,
	IN PVOID                Buffer,
	IN ULONG                NumberOfBytesToWrite,
	OUT PULONG              NumberOfBytesWritten OPTIONAL
);

HANDLE RtlCreateRemoteThread(
	IN  HANDLE hProcess,
	IN  LPSECURITY_ATTRIBUTES lpThreadAttributes,
	IN  DWORD dwStackSize,
	IN  LPTHREAD_START_ROUTINE lpStartAddress,
	IN  LPVOID lpParameter,
	IN  DWORD dwCreationFlags,
	OUT LPDWORD lpThreadId
);

BOOL InjectDllToProcess(DWORD pid,BOOL isX64);
EXTERN_C VOID ShellcodeFun64(VOID);
ULONG_PTR GetKernel32BaseAddress(DWORD pid);



