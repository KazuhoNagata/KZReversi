//! @file
//! ゲーム進行の管理を行うクラス
//****************************************************************************
//       (c) COPYRIGHT Kazuho Nagata 2016-  All Rights Reserved.
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
//│ A  │2016/06/02│新規作成                            │Kazuho Nagata │
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
            BoardClass board = (BoardClass)m_recvcmdProperty[1];
            CpuClass cpuClass = (CpuClass)m_recvcmdProperty[2];
            Form1 formobj = (Form1)m_recvcmdProperty[3];

            if (message == CMD_CPU)
            {
                Thread th = new Thread(CpuThreadFunc);
                th.Start(m_recvcmdProperty);

                // ノード数表示用
                Thread th2 = new Thread(GetNodeCountFunc);
                th2.Start(m_recvcmdProperty);
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

            while (m_onAi)
            {
                // CPU処理が終了するまで更新する
                nodeCount = cp.GetCountNode();
                // Form1のプロパティにノード数を設定
                ((Form1)formobj).Invoke(((Form1)formobj).nodeCountDelegate, new object[] { nodeCount });
                Thread.Sleep(30);
            }
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
