using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace entrypoint
{
   
    class Program
    {

       

        [DllImport(".\\add_basic.dll",EntryPoint = "Add@8")]
        private static extern int Add(int a, int b);

        [DllImport(".\\add_basic.dll", EntryPoint = "StrLenA@4", CharSet = CharSet.Ansi)]
        static extern int StrLenA(String s);

        [DllImport(".\\add_basic.dll", EntryPoint = "StrLenW@4",
        CharSet = CharSet.Unicode)]
        static extern int StrLenW(String s);

        [DllImport(".\\add_basic.dll", EntryPoint = "MinArray@8")]
        extern static int MinArray(int[] pData, int length);
        [DllImport(".\\add_basic.dll", EntryPoint = "MinArrayD@8")]
        extern static double MinArrayD(double[] pData, int length);
        [DllImport(".\\add_basic.dll", EntryPoint = "Average@8")]
        extern static double Average(double[] pData, int length);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]

        public struct EventData
        {

            public int I;

            public string Message;

        }


        [DllImport(".\\add_basic.dll", EntryPoint = "PutEventData@4")]
        private static extern bool PutEventData(ref EventData r);
        public delegate long CallBack(int i);
        public static long SpitConsole(int i)
        {

            Console.WriteLine("Printing from C# {0}", i * i);

            return i * i;


        }

        

        [DllImport(".\\add_basic.dll",EntryPoint = "ListOfSquares@4")]

        private static extern long ListOfSquares(CallBack a);

        [DllImport(".\\add_basic.dll",EntryPoint = "StringCopy@8")]

        private static extern bool StringCopy(StringBuilder dest, String src);

        static void Main(string[] args)
        {
            int res = Add(2, 3);
            Console.WriteLine(res);
            String s = "Hello World...";
            int first = StrLenA(s);
            int second = StrLenW(s);
            Console.WriteLine("{0} , {1}", first, second);

            int[] rs = new int[] { 10, -11, 20 };

            int rt = MinArray(rs, rs.Length);
            Console.WriteLine(rt);

            double[] rr = new double[] { 10.0, -11.0, 20.0 };

            double drt = MinArrayD(rr, rr.Length);
            Console.WriteLine(drt);


            drt = Average(rr, rr.Length);
            Console.WriteLine(drt);

            double[] srr = new double[10];
            srr[0] = 10.0;
            srr[1] = 100;
            drt = Average(srr, 2);
            Console.WriteLine(drt);

            EventData srt = new EventData();

            srt.I = 13;

            srt.Message = "I might be from MONO or Visual C# ........!";

            PutEventData(ref srt);

            /////////////////////////////
            // CallBack Demo
            //
            ListOfSquares(SpitConsole);

            ///////////////////////////////////////////
            //
            // Call StringCopy

            StringBuilder sb = new StringBuilder(256);

            StringCopy(sb, "Hello World...");

            Console.WriteLine(sb.ToString());


            Console.Read();
        }
    }
}