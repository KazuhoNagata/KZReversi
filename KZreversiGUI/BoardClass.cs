using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace KZreversi
{
    [StructLayout(LayoutKind.Sequential)]
    public class BoardClass
    {
        // マーシャル対象
        private uint color;
        private ulong black;
        private ulong white;
        // ここまでマーシャル対象


        public static uint BLACK = 0;
        public static uint WHITE = 1;
        public static uint EMPTY = 2;


        public void SetColor(uint c)
        {
            color = c;
        }

        public void SetBlack(ulong b)
        {
            black = b;
        }

        public void SetWhite(ulong w)
        {
            white = w;
        }

        public uint GetColor()
        {
            return color;
        }

        public ulong GetBlack()
        {
            return black;
        }

        public ulong GetWhite()
        {
            return white;
        }
    }
}
