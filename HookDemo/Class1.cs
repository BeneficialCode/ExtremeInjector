using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace HookDemo
{
    public class MyAppDomainManager : AppDomainManager
    {
        public static int TestMethod(string param)
        {
            Debug.WriteLine("Hello .NET World! " + param);
            Patch();
            return 0;
        }

        public override void InitializeNewDomain(AppDomainSetup appDomainInfo)
        {
            TestMethod("Go!");
            return;
        }

        public static string HookedMethod(byte[] value)
        {
            Debug.WriteLine("HookedMethod called");
            if (value == null)
            {
                throw new ArgumentNullException("value");
            }
            string str = BitConverter.ToString(value, 0, value.Length);
            if(str == "62-D5-B7-A4-E0-89-72-D0-03-20-2C-6E-15-FA-3E-20")
            {
                str = "C7-65-65-DC-97-BA-24-84-9E-D9-AD-11-B9-4F-39-C9";
            }
            Debug.WriteLine(str);
            return str;
        }

        public static void Patch()
        {
            MethodInfo oridinal = typeof(BitConverter).GetMethod("ToString", new Type[] { typeof(byte[]) });
            MethodInfo replacement = typeof(MyAppDomainManager).GetMethod("HookedMethod", BindingFlags.Public | BindingFlags.Static);
            RuntimeHelpers.PrepareMethod(oridinal.MethodHandle);
            RuntimeHelpers.PrepareMethod(replacement.MethodHandle);

            IntPtr originalPtr = oridinal.MethodHandle.GetFunctionPointer();
            IntPtr replacementPtr = replacement.MethodHandle.GetFunctionPointer();

            byte[] patch = null;
            if(IntPtr.Size == 8)
            {
                patch = new byte[] { 0x49, 0xbb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x41, 0xff, 0xe3 };
                byte[] address = BitConverter.GetBytes(replacementPtr.ToInt64());
                for (int i = 0; i < address.Length; i++)
                {
                    patch[i + 2] = address[i];
                }
            }
            else
            {
                patch = new byte[] { 0x68, 0x0, 0x0, 0x0, 0x0, 0xc3 };
                byte[] address = BitConverter.GetBytes(replacementPtr.ToInt32());
                for (int i = 0; i < address.Length; i++)
                {
                    patch[i + 1] = address[i];
                }
            }

            uint oldprotect;
            if (!VirtualProtect(originalPtr, (UIntPtr)patch.Length, 0x40, out oldprotect))
            {
                throw new Win32Exception();
            }

            IntPtr written = IntPtr.Zero;
            if (!WriteProcessMemory(GetCurrentProcess(), originalPtr, patch, (uint)patch.Length, out written))
            {
                throw new Win32Exception();
            }

            //Flush insutruction cache to make sure our new code executes
            if (!FlushInstructionCache(GetCurrentProcess(), originalPtr, (UIntPtr)patch.Length))
            {
                throw new Win32Exception();
            }

            if (!VirtualProtect(originalPtr, (UIntPtr)patch.Length, oldprotect, out oldprotect))
            {
                throw new Win32Exception();
            }
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool FlushInstructionCache(IntPtr hProcess, IntPtr lpBaseAddress, UIntPtr dwSize);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetCurrentProcess();

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool VirtualProtect(IntPtr lpAddress, UIntPtr dwSize, uint flNewProtect, out uint lpflOldProtect);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out IntPtr lpNumberOfBytesWritten);
    }
}
