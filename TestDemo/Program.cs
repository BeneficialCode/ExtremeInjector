using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TestDemo
{
    internal class Program
    {
        static void Main(string[] args)
        {
            byte[] byteArray = { 0xDE, 0xAD, 0xBE, 0xEF };
            string str = BitConverter.ToString(byteArray);
            Console.WriteLine(str);
            Console.ReadKey();
        }
    }
}
