//! @file
//! ゲーム進行の管理を行うクラス
//****************************************************************************
//       (c) COPYRIGHT kazura_utb 2016-  All Rights Reserved.
//****************************************************************************
// FILE NAME     : GameThread.cs
// PROGRAM NAME  : KZReversi
// FUNCTION      : ゲーム進行管理
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
//│ A  │2016/06/02│新規作成                            │kazura_utb    │
//└──┴─────┴──────────────────┴───────┘
//****************************************************************************
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading;

namespace KZreversi
{
    public class GameThread
    {
        public const int CMD_END = 0;
        public const int CMD_CPU = 1;
        public const int CMD_CHK = 2;
        public const int CMD_HINT = 3;

        private uint[] dcTable = { 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26 };

        private object[] _m_recvcmd;

        private bool m_onAi;

        public object[] m_recvcmdProperty
        {
            get
            {
                return _m_recvcmd;
            }
            set
            {
                _m_recvcmd = value;
                OnPropertyChanged("RcvCmd");
            }
        }

        public GameThread()
        {

        }

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = null;

            if (name == "RcvCmd")
            {
                handler = PropertyChangedRcvCmd;
            }

            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }

        }

        public void PropertyChangedRcvCmd(object sender, PropertyChangedEventArgs e)
        {
            int message = (int)m_recvcmdProperty[0];
            if (message == CMD_CPU)
            {
                BoardClass board = (BoardClass)m_recvcmdProperty[1];
                CpuClass cpuClass = (CpuClass)m_recvcmdProperty[2];
                Form1 formobj = (Form1)m_recvcmdProperty[3];

                Thread th = new Thread(CpuThreadFunc);
                th.Start(m_recvcmdProperty);

                // ノード数表示用
                Thread th2 = new Thread(GetNodeCountFunc);
                th2.Start(m_recvcmdProperty);
            }
            else if (message == CMD_HINT) 
            {
                BoardClass board = (BoardClass)m_recvcmdProperty[1];
                CpuClass cpuClass = (CpuClass)m_recvcmdProperty[2];
                Form1 formobj = (Form1)m_recvcmdProperty[3];
                uint level = (uint)m_recvcmdProperty[4];

                Thread th = new Thread(HintThreadFunc);
                th.Start(m_recvcmdProperty);
            }
            else
            {
                // 予備
            }

        }

        private void CpuThreadFunc(object args)
        {
            object[] argsarray = (object[])args;
            BoardClass board = (BoardClass)argsarray[1];
            CpuClass cpuClass = (CpuClass)argsarray[2];
            Form1 formobj = (Form1)argsarray[3];

            CppWrapper cp = new CppWrapper();
            CpuConfig cpuConfig = SetCpuConfig(cpuClass);

            ulong bk = board.GetBlack();
            ulong wh = board.GetWhite();

            m_onAi = true;
            // AIで着手
            ulong moves = cp.GetCpuMove(bk, wh, cpuConfig);

            // Form1のプロパティにCPUの着手を設定
            ((Form1)formobj).Invoke(((Form1)formobj).delegateObj, new object[] { moves });
            m_onAi = false;

        }

        private void GetNodeCountFunc(object args)
        {
            object[] argsarray = (object[])args;
            CppWrapper cp = new CppWrapper();
            Form1 formobj = (Form1)argsarray[3];

            ulong nodeCount = 0;
            // Form1のプロパティにノード数を設定(初期値)
            ((Form1)formobj).Invoke(((Form1)formobj).nodeCountDelegate, new object[] { nodeCount });

            do
            {
                // CPU処理が終了するまで更新する
                nodeCount = cp.GetCountNode();
                // Form1のプロパティにノード数を設定
                ((Form1)formobj).Invoke(((Form1)formobj).nodeCountDelegate, new object[] { nodeCount });
                Thread.Sleep(30);
            } while (m_onAi);
        }

        private void HintThreadFunc(object args)
        {
            object[] argsarray = (object[])args;
            BoardClass board = (BoardClass)argsarray[1];
            CpuClass cpuClass = (CpuClass)argsarray[2];
            Form1 formobj = (Form1)argsarray[3];
            uint level = (uint)argsarray[4];

            CppWrapper cp = new CppWrapper();

            // 着手可能リスト取得
            ulong moves = cp.GetEnumMove(board);
            // 現在のＣＰＵの設定を取得
            CpuConfig cpuConfig = SetCpuConfig(cpuClass);
            // BOOK禁止
            cpuConfig.bookFlag = false;

            // スレッド処理
            int pos;
            ulong rev;
            ulong bk = board.GetBlack(), move_bk;
            ulong wh = board.GetWhite(), move_wh;

            HintClass hintData = new HintClass();


            // CPUを次の色に設定
            cpuConfig.color = board.GetColor();

            // 探索
            for (int i = 0; i < level; i++)
            {
                cpuConfig.searchDepth = (uint)i * 2 + 2;
                cpuConfig.winLossDepth = dcTable[i];
                cpuConfig.exactDepth = dcTable[i] - 2;
                // 着手可能マスに対してそれぞれ評価値を計算
                for (ulong m = moves; m != 0; m ^= 1UL << pos)
                {
                    pos = cp.CountBit((~m) & (m - 1));
                    if (cpuConfig.color == BoardClass.WHITE)
                    {
                        rev = cp.GetBoardChangeInfo(bk, wh, pos);
                        move_bk = bk ^ ((1UL << pos) | rev);
                        move_wh = wh ^ rev;
                    }
                    else
                    {
                        rev = cp.GetBoardChangeInfo(wh, bk, pos);
                        move_wh = wh ^ ((1UL << pos) | rev);
                        move_bk = bk ^ rev;

                    }
                    cp.GetCpuMove(move_bk, move_wh, cpuConfig);
                    hintData.SetPos(pos);
                    hintData.SetEval(-cp.GetLastEvaluation());

                    // UIに評価値を通知
                    ((Form1)formobj).Invoke(((Form1)formobj).hintDelegate, new object[] { hintData });
                }
                // UIに反復x回目終了を通知
                hintData.SetPos(64);
                ((Form1)formobj).Invoke(((Form1)formobj).hintDelegate, new object[] { hintData });
            }

            // UIに探索終了を通知
            ((Form1)formobj).Invoke(((Form1)formobj).hintDelegate, new object[] { null });
        }

        private CpuConfig SetCpuConfig(CpuClass cpuClass)
        {
            CpuConfig cpuConfig = new CpuConfig();

            cpuConfig.bookFlag = cpuClass.GetBookFlag();
            cpuConfig.bookVariability = cpuClass.GetBookVariability();
            cpuConfig.casheSize = cpuClass.GetCasheSize();
            cpuConfig.color = cpuClass.GetColor();
            cpuConfig.exactDepth = cpuClass.GetExactDepth();
            cpuConfig.mpcFlag = cpuClass.GetMpcFlag();
            cpuConfig.searchDepth = cpuClass.GetSearchDepth();
            cpuConfig.tableFlag = cpuClass.GetTableFlag();
            cpuConfig.winLossDepth = cpuClass.GetWinLossDepth();

            return cpuConfig;
        }

    }
}
