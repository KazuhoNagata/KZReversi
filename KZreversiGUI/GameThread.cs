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

        CppWrapper cpw;

        private bool m_abort;

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
            cpw = new CppWrapper();
            m_abort = false;
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
            CpuConfig cpuConfig = SetCpuConfig(cpuClass);

            ulong bk = board.GetBlack();
            ulong wh = board.GetWhite();

            m_onAi = true;
            // AIで着手
            ulong moves = cpw.GetCpuMove(bk, wh, cpuConfig);

            // Form1のプロパティにCPUの着手を設定
            ((Form1)formobj).Invoke(((Form1)formobj).delegateObj, new object[] { moves });
            m_onAi = false;

        }

        private void GetNodeCountFunc(object args)
        {
            object[] argsarray = (object[])args;
            Form1 formobj = (Form1)argsarray[3];

            ulong nodeCount = 0;
            // Form1のプロパティにノード数を設定(初期値)
            ((Form1)formobj).Invoke(((Form1)formobj).nodeCountDelegate, new object[] { nodeCount });

            do
            {
                // CPU処理が終了するまで更新する
                nodeCount = cpw.GetCountNode();
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
            int ret = 0;

            CppWrapper cpw = new CppWrapper();

            // 着手可能リスト取得
            ulong moves = cpw.GetEnumMove(board);
            // 現在のＣＰＵの設定を取得
            CpuConfig cpuConfig = SetCpuConfig(cpuClass);
            // BOOK禁止
            cpuConfig.bookFlag = false;

            ulong bk = board.GetBlack();
            ulong wh = board.GetWhite();

            HintClass hintData = new HintClass();

            // 空きマス数
            int empty = cpw.CountBit(~(bk | wh));

            // CPU設定
            cpuConfig.color = board.GetColor();
            cpuConfig.winLossDepth = dcTable[level - 1];
            cpuConfig.exactDepth = dcTable[level - 1] - 2;

            if (cpuConfig.exactDepth >= empty)
            {
                // WLDとEXACTの前にある程度の探索を行うため0で初期化
                cpuConfig.exactDepth = 0;
                cpuConfig.winLossDepth = 0;
                hintData.SetAttr(HintClass.SOLVE_MIDDLE);
                // 反復深化探索(level - 8)
                for (int i = 0; i < level / 2; i++)
                {
                    cpuConfig.searchDepth = (uint)i * 2 + 2;
                    // 着手可能マスに対してそれぞれ評価値を計算
                    ret = doSearch(bk, wh, cpuConfig, moves, formobj, hintData);
                    if (ret == -1) break;
                    // UIに反復x回目終了を通知
                    hintData.SetPos(64);
                    ((Form1)formobj).Invoke(((Form1)formobj).hintDelegate, new object[] { hintData });
                }

                if (ret == 0)
                {
                    hintData.SetAttr(HintClass.SOLVE_EXCAT);
                    cpuConfig.exactDepth = dcTable[level - 1] - 2;
                    // 着手可能マスに対してそれぞれ評価値を計算
                    ret = doSearch(bk, wh, cpuConfig, moves, formobj, hintData);
                }
            }
            else if (cpuConfig.winLossDepth >= empty)
            {
                cpuConfig.exactDepth = 0;
                cpuConfig.winLossDepth = 0;
                hintData.SetAttr(HintClass.SOLVE_MIDDLE);
                // 反復深化探索(level - 8)
                for (int i = 0; i < level / 2; i++)
                {
                    cpuConfig.searchDepth = (uint)i * 2 + 2;
                    // 着手可能マスに対してそれぞれ評価値を計算
                    ret = doSearch(bk, wh, cpuConfig, moves, formobj, hintData);
                    if (ret == -1) break;
                    // UIに反復x回目終了を通知
                    hintData.SetPos(64);
                    ((Form1)formobj).Invoke(((Form1)formobj).hintDelegate, new object[] { hintData });
                }
                if (ret == 0)
                {
                    hintData.SetAttr(HintClass.SOLVE_WLD);
                    cpuConfig.winLossDepth = dcTable[level - 1];
                    // 着手可能マスに対してそれぞれ評価値を計算
                    ret = doSearch(bk, wh, cpuConfig, moves, formobj, hintData);
                }
            }
            else
            {
                hintData.SetAttr(HintClass.SOLVE_MIDDLE);
                // 反復深化探索
                for (int i = 0; i < level && i <= empty; i++)
                {
                    cpuConfig.searchDepth = (uint)i * 2 + 2;
                    // 着手可能マスに対してそれぞれ評価値を計算
                    ret = doSearch(bk, wh, cpuConfig, moves, formobj, hintData);
                    if (ret == -1) break;
                    // UIに反復x回目終了を通知
                    hintData.SetPos(64);
                    ((Form1)formobj).Invoke(((Form1)formobj).hintDelegate, new object[] { hintData });
                }
            }

            // UIに探索終了を通知
            ((Form1)formobj).Invoke(((Form1)formobj).hintDelegate, new object[] { null });
        }



        private int doSearch(ulong bk, ulong wh, CpuConfig cpuConfig, ulong moves, Form1 formobj, HintClass hintData)
        {
            int pos;
            int ret = 0;
            ulong move_bk, move_wh, rev;

            for (ulong m = moves; m != 0; m ^= 1UL << pos)
            {
                pos = cpw.CountBit((~m) & (m - 1));
                if (cpuConfig.color == BoardClass.WHITE)
                {
                    rev = cpw.GetBoardChangeInfo(bk, wh, pos);
                    move_bk = bk ^ ((1UL << pos) | rev);
                    move_wh = wh ^ rev;
                }
                else
                {
                    rev = cpw.GetBoardChangeInfo(wh, bk, pos);
                    move_wh = wh ^ ((1UL << pos) | rev);
                    move_bk = bk ^ rev;

                }
                cpw.GetCpuMove(move_bk, move_wh, cpuConfig);

                hintData.SetPos(pos);
                hintData.SetEval(-cpw.GetLastEvaluation());

                // UIに評価値を通知
                ((Form1)formobj).Invoke(((Form1)formobj).hintDelegate, new object[] { hintData });

                // 中断処理
                if (m_abort == true)
                {
                    m_abort = false;
                    ret = -1; 
                    break;
                }
            }

            return ret;
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


        public void AbortAll()
        {
            m_abort = true;
        }
    }
}
