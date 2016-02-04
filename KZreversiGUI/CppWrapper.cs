using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;

namespace KZreversi
{
    /// <summary>
    /// 
    /// aI_coreアクセス用ラッパークラス
    /// 
    /// </summary>
    public class CppWrapper
    {
        /// <summary>
        /// DLLや各種データの読み込みと初期化を行います
        /// </summary>
        /// <returns></returns>
        public bool LibInit()
        {
            return CoreAccesor.KZ_LibInit();
        }

        /// <summary>
        /// 盤面の情報を基に着手可能手を列挙します
        /// </summary>
        /// <returns></returns>
        public ulong GetEnumMove(BoardClass bclass)
        {
            ulong moveBit;
            ulong bk, wh;
            uint count = 0;

            if (bclass.GetColor() == BoardClass.BLACK) 
            {
                bk = bclass.GetBlack();
                wh = bclass.GetWhite();
            }
            else 
            {
                wh = bclass.GetBlack();
                bk = bclass.GetWhite();
            }

            unsafe
            {
                moveBit = CoreAccesor.KZ_GetEnumMove(bk, wh, &count);
            }

            return moveBit;
        }

        /// <summary>
        /// 盤面の情報を基にCPUの着手を取得します
        /// </summary>
        /// <returns></returns>
        public ulong GetCpuMove(ulong bk, ulong wh, CpuConfig cpuConfig)
        {
            ulong moveBit;

            moveBit = CoreAccesor.KZ_GetCpuMove(bk, wh, cpuConfig);

            return moveBit;
        }

        /// <summary>
        /// 着手を列挙したビット列から小さい番号順に着手番号に変換します
        /// </summary>
        /// <returns></returns>
        public uint ConvertMoveBit(ulong bit)
        {
            return CoreAccesor.KZ_CountBit((~bit) & (bit - 1));
        }

        /// <summary>
        /// 着手した後に変化する箇所を計算します
        /// </summary>
        /// <returns></returns>
        public ulong GetBoardChangeInfo(ulong bk, ulong wh, int move)
        {
            return CoreAccesor.KZ_GetBoardChangeInfo(bk, wh, move);
        }

        /// <summary>
        /// 最後にCPUが着手した時の局面評価値を取得します
        /// </summary>
        /// <returns></returns>
        public int GetLastEvaluation(ulong bk, ulong wh, int move)
        {
            return CoreAccesor.KZ_GetLastEvaluation();
        }


    }


    /// <summary>
        /// C++ DLLImportを使ったメソッドを格納しているクラス
        /// 特に理由が無い限りは，本クラスのメンバメソッドを直接呼ばないで下さい．
        /// </summary>
    static class CoreAccesor
    {
        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static bool KZ_LibInit();

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static unsafe ulong KZ_GetEnumMove(ulong bk, ulong wh, uint* count);

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static ulong KZ_GetBoardChangeInfo(ulong bk, ulong wh, int move);

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static ulong KZ_GetCpuMove(ulong bk, ulong wh, [Out] CpuConfig cpuConfig);

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static uint KZ_CountBit(ulong bit);

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static int KZ_GetLastEvaluation();
        
    }
}
