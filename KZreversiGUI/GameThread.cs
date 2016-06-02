﻿using System;
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

            // AIで着手
            ulong moves = cp.GetCpuMove(bk, wh, cpuConfig);

            // Form1のプロパティにCPUの着手を設定
            ((Form1)formobj).Invoke(((Form1)formobj).delegateObj, new object[] { moves });

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
