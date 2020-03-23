//! @file
//! C++ DLL アクセス用メソッド群
//****************************************************************************
//       (c) COPYRIGHT kazura_utb 2016-  All Rights Reserved.
//****************************************************************************
// FILE NAME     : CppWrapper.cs
// PROGRAM NAME  : KZReversi
// FUNCTION      : C++ DLL アクセス用
//
//****************************************************************************
//****************************************************************************
//
//****************************************************************************
//┌──┬─────┬──────────────────┬───────┐
//│履歴│   DATE   │              NOTES                 │     SIGN     │
//├──┼─────┼──────────────────┼───────┤
//│    │          │                                    │              │
//├──┼─────┼──────────────────┼───────┤
//│ A  │2016/02/01│新規作成                            │kazura_utb    │
//└──┴─────┴──────────────────┴───────┘
//****************************************************************************
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

        private static CppWrapper cpw = new CppWrapper();
        private volatile bool mutex;

        public static CppWrapper getInstance()
        {
            return cpw;
        }

        private CppWrapper()
        {
        }

        /// <summary>
        /// DLLや各種データの読み込みと初期化を行います
        /// </summary>
        /// <returns></returns>
        public bool LibInit()
        {
            return NativeMethods.KZ_LibInit();
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

            if (bclass.GetNowColor() == BoardClass.BLACK) 
            {
                bk = bclass.GetBlack();
                wh = bclass.GetWhite();
            }
            else 
            {
                bk = bclass.GetWhite();
                wh = bclass.GetBlack();
            }

            unsafe
            {
                moveBit = NativeMethods.KZ_GetEnumMove(bk, wh, &count);
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

            while (mutex == true);
            mutex = true;

            GCHandle gchCpuConfig = GCHandle.Alloc(cpuConfig, GCHandleType.Pinned);
            Marshal.StructureToPtr(cpuConfig, gchCpuConfig.AddrOfPinnedObject(), false);

            moveBit = NativeMethods.KZ_GetCpuMove(bk, wh, gchCpuConfig.AddrOfPinnedObject());

            gchCpuConfig.Free();
            mutex = false;
            return moveBit;
        }

        /// <summary>
        /// 着手を列挙したビット列から小さい番号順に着手番号に変換します
        /// </summary>
        /// <returns></returns>
        public int ConvertMoveBit(ulong bit)
        {
            if (bit == 0) 
            {
                return -1;
            }
            return NativeMethods.KZ_CountBit((~bit) & (bit - 1));
        }

        /// <summary>
        /// 着手した後に変化する箇所を計算します
        /// </summary>
        /// <returns></returns>
        public ulong GetBoardChangeInfo(ulong bk, ulong wh, int move)
        {
            return NativeMethods.KZ_GetBoardChangeInfo(bk, wh, move);
        }

        /// <summary>
        /// 最後にCPUが着手した時の局面評価値を取得します
        /// </summary>
        /// <returns></returns>
        public int GetLastEvaluation()
        {
            return NativeMethods.KZ_GetLastEvaluation();
        }

        /// <summary>
        /// 探索済みのノード数を取得する
        /// </summary>
        /// <returns></returns>
        public UInt64 GetCountNode()
        {
            return NativeMethods.KZ_GetCountNode();
        }

        /// <summary>
        /// 直前のCPUの着手が定石手かどうかを取得する
        /// </summary>
        /// <returns></returns>
        public bool GetIsUseBook()
        {
            return NativeMethods.KZ_GetIsUseBook();
        }

        /// <summary>
        /// CPUの処理を中断します
        /// </summary>
        /// <returns></returns>
        public int SendAbort()
        {
            return NativeMethods.KZ_SendAbort();
        }

        /// <summary>
        /// 直前の処理でアボートされたかどうかを取得します
        /// </summary>
        /// <returns></returns>
        public bool GetIsAbort()
        {
            return NativeMethods.KZ_GetIsAbort();
        }
        /// <summary>
        /// １が立っているビット数をカウントします
        /// </summary>
        /// <returns></returns>
        public int CountBit(ulong bit)
        {
            return NativeMethods.KZ_CountBit(bit);
        }

        /// <summary>
        /// ネイティブ層にC#デリゲートを登録する
        /// </summary>
        /// <returns></returns>
        public void EntryFunction(IntPtr cpuMessageDelegatePtr)
        {
            NativeMethods.KZ_EntryFunction(cpuMessageDelegatePtr);
        }

        /// <summary>
        /// AIで使用するハッシュを解放する
        /// </summary>
        /// <returns></returns>
        public void ClearHash()
        {
            NativeMethods.KZ_ClearHash();
        }

        /// <summary>
        /// AIで使用するハッシュを解放する
        /// </summary>
        /// <returns></returns>
        public void ReleaseHash()
        {
            NativeMethods.KZ_ReleaseHash();
        }

        /// <summary>
        /// AIで使用する定石データを解放する
        /// </summary>
        /// <returns></returns>
        public void ReleaseBook()
        {
            NativeMethods.KZ_ReleaseBook();
        }

    }


    /// <summary>
        /// C++ DLLImportを使ったメソッドを格納しているクラス
        /// 特に理由が無い限りは，本クラスのメンバメソッドを直接呼ばないで下さい．
        /// </summary>
    static class NativeMethods
    {
        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        //[SuppressUnmanagedCodeSecurityAttribute()]
        public extern static bool KZ_LibInit();

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static unsafe ulong KZ_GetEnumMove(ulong bk, ulong wh, uint* count);

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static ulong KZ_GetBoardChangeInfo(ulong bk, ulong wh, int move);

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static ulong KZ_GetCpuMove(ulong bk, ulong wh, [In, Out] IntPtr cpuConfig);

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static int KZ_CountBit(ulong bit);

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static int KZ_GetLastEvaluation();

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static UInt64 KZ_GetCountNode();

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static bool KZ_GetIsUseBook();

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static int KZ_SendAbort();

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static bool KZ_GetIsAbort();

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static void KZ_EntryFunction(IntPtr cpuMessageDelegatePtr);

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static void KZ_ClearHash();

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static void KZ_ReleaseHash();

        [DllImport("ai_core.dll", CallingConvention = CallingConvention.Cdecl)]
        [SuppressUnmanagedCodeSecurityAttribute()]
        public extern static void KZ_ReleaseBook();
        
    }
}
