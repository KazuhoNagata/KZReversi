﻿//! @file
//! CPU メソッド群
//****************************************************************************
//       (c) COPYRIGHT kazura_utb 2016-  All Rights Reserved.
//****************************************************************************
// FILE NAME     : CpuClass.cs
// PROGRAM NAME  : KZReversi
// FUNCTION      : CPUの処理全般
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
using System.Text;
using System.Threading;

namespace KZreversi
{
    public class CpuClass
    {
        private CpuConfig cConfig;

        public CpuClass() 
        {
            cConfig = new CpuConfig();
        }


        public uint GetColor()
        {
            return cConfig.color;
        }

        public void SetColor(uint color)
        {
            cConfig.color = color;
        }

        public uint GetCasheSize()
        {
            return cConfig.casheSize;
        }

        public void SetCasheSize(uint casheSize)
        {
            cConfig.casheSize = casheSize;
        }

        public uint GetSearchDepth()
        {
            return cConfig.searchDepth;
        }

        public void SetSearchDepth(uint searchDepth)
        {
            cConfig.searchDepth = searchDepth;
        }

        public uint GetWinLossDepth()
        {
            return cConfig.winLossDepth;
        }

        public void SetWinLossDepth(uint winLossDepth)
        {
            cConfig.winLossDepth = winLossDepth;
        }

        public uint GetExactDepth()
        {
            return cConfig.exactDepth;
        }

        public void SetExactDepth(uint exactDepth)
        {
            cConfig.exactDepth = exactDepth;
        }

        public byte GetBookFlag()
        {
            return cConfig.bookFlag;
        }

        public void SetBookFlag(byte bookFlag)
        {
            cConfig.bookFlag = bookFlag;
        }

        public uint GetBookVariability()
        {
            return cConfig.bookVariability;
        }

        public void SetBookVariability(uint bookVariability)
        {
            cConfig.bookVariability = bookVariability;
        }

        public byte GetMpcFlag()
        {
            return cConfig.mpcFlag;
        }

        public void SetMpcFlag(byte mpcFlag)
        {
            cConfig.mpcFlag = mpcFlag;
        }

        public byte GetTableFlag()
        {
            return cConfig.tableFlag;
        }

        public void SetTableFlag(byte tableFlag)
        {
            cConfig.tableFlag = tableFlag;
        }


    }
}
