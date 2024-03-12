using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HookDemo
{
    public class Class1
    {
        public static int TestMethod(string param)
        {
            Debug.WriteLine("Hello .NET World! " + param);
            return 0;
        }
    }
}
