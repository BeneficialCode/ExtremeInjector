using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.SymbolStore;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestDemo
{
    internal class Program
    {
        static void Main(string[] args)
        {
            IntPtr hModule = GetModuleHandle("ntdll.dll");
            IntPtr pFunc = GetProcAddress(hModule, "NtQueryInformationProcess");
            Console.WriteLine("NtQueryInformationProcess: " + pFunc.ToString("X"));
            int major = Environment.Version.Major;
            Console.WriteLine("Version: " + major);
            byte[] byteArray = { 0xDE, 0xAD, 0xBE, 0xEF };
            string str = BitConverter.ToString(byteArray);
            Console.WriteLine(str);
            Console.ReadLine();
            AntiDebug();
            MethodInfo get_IsAttached = typeof(Debugger).GetProperty("IsAttached").GetGetMethod();
            IntPtr get_IsAttachedPtr = get_IsAttached.MethodHandle.GetFunctionPointer();
            // 十六进制地址
            Console.WriteLine("IsAttachedPtr: " + get_IsAttachedPtr.ToString("X"));
            Console.WriteLine("IsAttached: " + get_IsAttached.Invoke(null, null));

            Console.ReadKey();
        }

        unsafe static bool AntiDebug()
        {
            int len = 0;
            SYSTEM_KERNEL_DEBUGGER_INFORMATION info;
            
            info.DebuggerNotPresent = false;
            info.DebuggerEnabled = false;
            IntPtr pInfo = new IntPtr(&info);
            uint status = NtQuerySystemInformation(35, pInfo, (uint)Marshal.SizeOf(info), out len);
            if (info.DebuggerEnabled)
            {
                Console.WriteLine("Debugger Enabled");
            }
            bool result = false;

            IntPtr hProcess = Process.GetCurrentProcess().Handle;

            bool isDebuggerPresent = false;
            CheckRemoteDebuggerPresent(hProcess, ref isDebuggerPresent);
            if (isDebuggerPresent)
            {
                Console.WriteLine("Detect Debugger");
            }
            isDebuggerPresent = IsDebuggerPresent();
            if (isDebuggerPresent)
            {
                Console.WriteLine("Detect Debugger");
            }
            IntPtr value = new IntPtr(0);
           
            status = NtQueryInformationProcess(hProcess, 0x7, out value, Marshal.SizeOf(value), out len);
            if(status == 0)
            {
                if(value == new IntPtr(-1))
                {
                    Console.WriteLine("Detect Process Debug Port");
                    uint flags = 0;
                    IntPtr pFlags = new IntPtr(&flags);
                    status = NtQueryInformationProcess(hProcess, 0x1E, out value, Marshal.SizeOf(value), out len);
                    if (status == 0)
                    {
                        status = NtSetInformationDebugObject(value, 0x1, pFlags, Marshal.SizeOf(flags), out len);
                    }
                    status = NtRemoveProcessDebug(hProcess, value);
                    if (status == 0)
                    {
                        Console.WriteLine("Remove Process Debug Port");
                        result = true;
                    }
                    NtClose(value);
                }
            }
           
            return result;
        }

        public struct SYSTEM_KERNEL_DEBUGGER_INFORMATION
        {
            [MarshalAs(UnmanagedType.U1)] public bool DebuggerEnabled;
            [MarshalAs(UnmanagedType.U1)] public bool DebuggerNotPresent;
        }

        [DllImport("ntdll.dll", EntryPoint = "NtQueryInformationProcess", ExactSpelling = true, SetLastError = true)]
        static extern uint NtQueryInformationProcess([In] IntPtr ProcessHandle, [In] int ProcessInformationClass,
            out IntPtr ProcessInformation,[In] int ProcessInformationLength, [Optional] out int ReturnLength);

        [DllImport("ntdll.dll", EntryPoint = "NtClose", ExactSpelling = true, SetLastError = true)]
        static extern uint NtClose([In] IntPtr Handle);

        [DllImport("ntdll.dll", EntryPoint = "NtRemoveProcessDebug", ExactSpelling = true, SetLastError = true)]
        static extern uint NtRemoveProcessDebug(IntPtr ProcessHandle, IntPtr DebugHandle);

        [DllImport("ntdll.dll", EntryPoint = "NtSetInformationDebugObject", ExactSpelling = true, SetLastError = true)]
        static extern uint NtSetInformationDebugObject([In] IntPtr DebugHandle, [In] int InformationClass, [In] IntPtr Information, int InformationLength,
           [Optional] out int ReturnLength);

        [DllImport("ntdll.dll", EntryPoint = "NtQuerySystemInformation", ExactSpelling = true, SetLastError = true)]
        static extern uint NtQuerySystemInformation([In] int SystemInformationClass, IntPtr SystemInformation, uint SystemInformationLength, [Optional] out int ReturnLength);

        [DllImport("kernel32.dll", EntryPoint = "IsDebuggerPresent", ExactSpelling = true, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool IsDebuggerPresent();

        [DllImport("kernel32.dll", EntryPoint = "CheckRemoteDebuggerPresent", ExactSpelling = true, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool CheckRemoteDebuggerPresent(IntPtr hProcess, [MarshalAs(UnmanagedType.Bool)] ref bool isDebuggerPresent);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);
    }
}
