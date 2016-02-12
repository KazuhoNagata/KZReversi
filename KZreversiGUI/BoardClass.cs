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


        /**
         * 盤面の着手処理を行います
         */
        public bool move(int pos) 
        {
            UInt64 rev;
            CppWrapper cppWrapper = new CppWrapper();

            if (this.color == BoardClass.BLACK)
            {
                rev = cppWrapper.GetBoardChangeInfo(black, white, pos);

                if (rev == 0) 
                {
                    return false;
                }

                black ^= ((1UL << pos) | rev);
                white ^= rev;
            }
            else
            {
                rev = cppWrapper.GetBoardChangeInfo(white, black, pos);

                if (rev == 0)
                {
                    return false;
                }

                white ^= ((1UL << pos) | rev);
                black ^= rev;
            }

            return true;

        }

    }
}
