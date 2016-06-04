//! @file
//! アプリケーションフォーム
//****************************************************************************
//       (c) COPYRIGHT Kazuho Nagata 2016-  All Rights Reserved.
//****************************************************************************
// FILE NAME     : Form1.cs
// PROGRAM NAME  : KZReversi
// FUNCTION      : フォーム
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
//│ A  │2016/02/01│新規作成                            │Kazuho Nagata │
//└──┴─────┴──────────────────┴───────┘
//****************************************************************************

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace KZreversi
{
    public partial class Form1 : Form
    {

        private BufferedPanel panel1;

        public CppWrapper cppWrapper;
        public bool loadResult;

        private Image bkImg;
        private Image whImg;

        private BoardClass boardclass;
        private CpuClass[] cpuClass;

        private int EVAL_THRESHOLD = 10000;

        private const int ON_NOTHING = 0;
        private const int ON_GAME = 1;
        private const int ON_EDIT = 3;

        private const int TURN_HUMAN = 0;
        private const int TURN_CPU = 1;

        private const int BOARD_SIZE = 8;

        private const int COLOR_BLACK = 0;
        private const int COLOR_WHITE = 1;

        private int m_mode = ON_NOTHING;
        private bool m_abort = false;

        private uint nowColor = COLOR_BLACK;
        private Player nowPlayer;
        //private int nowTurn;

        private Player[] playerArray;

        private int m_passCount;

        private const ulong MOVE_PASS = 0;

        private uint[] dcTable = { 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26 };

        private Stopwatch m_sw;

        public delegate void SetMoveProperty(ulong moves);
        public delegate void SetNodeCountProperty(ulong nodeCount);
        public delegate void SetCpuMessageProperty(string cpuMsg);

        public SetMoveProperty delegateObj;
        public SetNodeCountProperty nodeCountDelegate;
        public SetCpuMessageProperty cpuMessageDelegate;

        Font m_ft = new Font("MS UI Gothic", 14);
        Font m_ft2 = new Font("MS UI Gothic", 8);

        public int m_event;

        public IntPtr cpuMessageDelegatePtr;

        public Form1()
        {
            boardclass = new BoardClass();

            cpuClass = new CpuClass[2];
            cpuClass[0] = new CpuClass(); // BLACK
            cpuClass[1] = new CpuClass(); // WHITE

            delegateObj = new SetMoveProperty(setMove);
            nodeCountDelegate = new SetNodeCountProperty(setNodeCount);
            cpuMessageDelegate = new SetCpuMessageProperty(setCpuMessage);

            boardclass.InitBoard(COLOR_BLACK);

            InitializeComponent();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 3;

            // プレイヤー情報を初期化
            playerArray = new Player[2];
            playerArray[COLOR_BLACK] = new Player(Player.PLAYER_HUMAN, COLOR_BLACK);
            playerArray[COLOR_WHITE] = new Player(Player.PLAYER_CPU, COLOR_WHITE);

            // デフォルトプレイヤー
            nowPlayer = playerArray[COLOR_BLACK];
            label3.Visible = true;
            label4.Visible = false;

            m_sw = new Stopwatch();
        }

        private void 終了ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            m_ft.Dispose();
            m_ft2.Dispose();
            // デバッグ作業で煩わしいため一旦コメントアウト
            //if (MessageBox.Show(
            //"終了してもいいですか？", "確認",
            //MessageBoxButtons.YesNo, MessageBoxIcon.Question
            //  ) == DialogResult.No)
            //{
            //    e.Cancel = true;
            //}
        }

        private void buttonClick(object sender, EventArgs e)
        {
            if (sender == this.button5) // ゲーム開始ボタン
            {
                m_mode = ON_GAME;
                boardclass.DeleteHistory(boardclass.GetNowTurn());
                nowColor = boardclass.GetNowColor();

                SetPlayerInfo();
                this.panel1.Refresh();

                ChangePlayer();

                if (nowPlayer.playerInfo == Player.PLAYER_CPU)
                {
                    // CPUモードに移行(ハンドラコール)
                    m_cpuFlagProperty = true;
                }

            }
            else if (sender == this.button6) // 中断ボタン
            {
                if (m_cpuFlagProperty == false)
                {
                    m_mode = ON_NOTHING;
                    this.panel1.Refresh();
                }
                else
                {
                    // CPUスレッドに停止命令送信
                    m_abort = true;
                    m_mode = ON_NOTHING;
                    cppWrapper.SendAbort();
                    MessageBox.Show(
                        "AIの処理を中断しました。再開はゲーム開始ボタンを押してください。",
                        "中断",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Information);
                }

               // this.panel1.Refresh();
            }
            else if (sender == this.button1) // 最初に戻るボタン
            {
                bool ret = boardclass.SetHistory(0);
                if (ret == true)
                {
                    m_mode = ON_NOTHING;
                    ChangePlayer();
                    this.panel1.Refresh();
                }
            }
            else if (sender == this.button2) // 最新に進むボタン
            {
                bool ret = boardclass.SetHistory(boardclass.GetRecentTurn());
                if (ret == true)
                {
                    m_mode = ON_NOTHING;
                    ChangePlayer();
                    this.panel1.Refresh();
                }
            }
            else if (sender == this.button3) // 一手戻るボタン
            {
                bool ret = boardclass.SetHistory(boardclass.GetNowTurn() - 1);
                if (ret == true)
                {
                    m_mode = ON_NOTHING;
                    ChangePlayer();
                    this.panel1.Refresh();
                }
            }
            else if (sender == this.button4) // 一手進むボタン
            {
                bool ret = boardclass.SetHistory(boardclass.GetNowTurn() + 1);
                if (ret == true)
                {
                    m_mode = ON_NOTHING;
                    ChangePlayer();
                    this.panel1.Refresh();
                }
            }
        }

        private void ChangePlayer()
        {
            nowColor = boardclass.GetNowColor();
            nowPlayer = playerArray[nowColor];
            if (nowColor == COLOR_BLACK)
            {
                label3.Visible = true;
                label4.Visible = false;
            }
            else
            {
                label3.Visible = false;
                label4.Visible = true;
            }
        }

        private void SetPlayerInfo()
        {
            if (comboBox1.SelectedIndex == 0)
            {
                playerArray[0] = new Player(Player.PLAYER_HUMAN, COLOR_BLACK);
            }
            else
            {
                playerArray[0] = new Player(Player.PLAYER_CPU, COLOR_BLACK);
            }
            if (comboBox2.SelectedIndex == 0)
            {
                playerArray[1] = new Player(Player.PLAYER_HUMAN, COLOR_WHITE);
            }
            else
            {
                playerArray[1] = new Player(Player.PLAYER_CPU, COLOR_WHITE);
            }

        }

        private void Form1_Load(object sender, EventArgs e)
        {
            bkImg = imageList1.Images[0];
            whImg = imageList1.Images[1];

            // 盤面のセット
            panel1 = new BufferedPanel();
            panel1.Width = 480;
            panel1.Height = 480;
            panel1.Location = new Point(18, 37);
            panel1.BackgroundImage = new Bitmap(KZreversi.Properties.Resources.boardPicture);
            panel1.Paint += panel1_Paint;
            panel1.Click += panel1_Click;
            panel1.DoubleClick += panel1_Click;
            this.Controls.Add(panel1);

            LoadForm lf = new LoadForm();
            lf.ShowDialog(this);

            if (loadResult == false)
            {
                MessageBox.Show(
                    "DLLかAIデータ読み込みに失敗しました。アプリケーションを終了します。",
                    "読み込みエラー",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);

                this.Close();
            }

            // このdelegateをGC対象外にする
            GCHandle gcHandle = GCHandle.Alloc(cpuMessageDelegate);
            // C++側から呼び出せるようにする
            cpuMessageDelegatePtr = Marshal.GetFunctionPointerForDelegate(cpuMessageDelegate);
            // C側に関数ポインタを登録
            cppWrapper.EntryFunction(cpuMessageDelegatePtr);

            // デフォルトのCPU設定
            for (int i = 0; i < cpuClass.Length; i++)
            {
                if (i == 0)
                {
                    cpuClass[i].SetColor(BoardClass.BLACK);
                }
                else
                {
                    cpuClass[i].SetColor(BoardClass.WHITE);
                }

                cpuClass[i].SetCasheSize(1 << 21);
                cpuClass[i].SetSearchDepth(6);
                cpuClass[i].SetWinLossDepth(14);
                cpuClass[i].SetExactDepth(12);
                cpuClass[i].SetBookFlag(true);
                cpuClass[i].SetBookVariability(1);
                cpuClass[i].SetMpcFlag(true);
                cpuClass[i].SetTableFlag(true);
            }
        }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {
            int pos;
            ulong temp;

            nowPlayer = playerArray[nowColor];
            // 盤面情報から描画
            temp = boardclass.GetBlack();
            while (temp > 0)
            {
                pos = cppWrapper.ConvertMoveBit(temp);
                e.Graphics.DrawImage(bkImg,
                    (pos / BOARD_SIZE) * 60 + 1,
                    (pos % BOARD_SIZE) * 60 + 1,
                    bkImg.Width, bkImg.Height);
                temp ^= (1UL << (int)pos);
            }

            temp = boardclass.GetWhite();
            while (temp > 0)
            {
                pos = cppWrapper.ConvertMoveBit(temp);
                e.Graphics.DrawImage(whImg,
                    (pos / BOARD_SIZE) * 60 + 1,
                    (pos % BOARD_SIZE) * 60 + 1,
                    whImg.Width, whImg.Height);
                temp ^= (1UL << (int)pos);
            }

            // 最後に打った手を強調する
            pos = boardclass.GetRecentMove();
            if (pos >= 0)
            {
                e.Graphics.DrawString("●", m_ft2, Brushes.OrangeRed,
                (pos / BOARD_SIZE) * 60 + 25,
                (pos % BOARD_SIZE) * 60 + 26);
            }

            // ゲーム中の場合かつプレイヤーの手番の場合、着手可能場所を表示
            if (m_mode == ON_GAME && m_cpuFlagProperty == false)
            {
                temp = cppWrapper.GetEnumMove(boardclass);
                nowPlayer.moves = temp;
                if (temp != 0)
                {
                    m_passCount = 0;
                    while (temp > 0)
                    {
                        pos = cppWrapper.ConvertMoveBit(temp);
                        e.Graphics.DrawString("◆", m_ft, Brushes.Red,
                            (pos / BOARD_SIZE) * 60 + 20,
                            (pos % BOARD_SIZE) * 60 + 20);
                        temp ^= (1UL << pos);
                    }
                }
            }
        }

        private void panel1_Click(object sender, EventArgs e)
        {
            Point pos;
            int num;
            MouseEventArgs mouseEvent = (MouseEventArgs)e;
            MouseButtons buttons = mouseEvent.Button;

            switch (m_mode)
            {
                case ON_GAME:

                    if (m_cpuFlagProperty == true)
                    {
                        // CPU処理中のため操作無効
                        break;
                    }

                    // 押された瞬間の座標を取得
                    pos = mouseEvent.Location;
                    num = ((pos.X / 60) * BOARD_SIZE) + (pos.Y / 60);

                    // 着手出来るかチェック
                    if ((nowPlayer.moves & (1UL << num)) != 0)
                    {
                        // 着手に合わせて盤面情報を更新
                        boardclass.move(num);
                        // プレイヤー変更
                        ChangePlayer();
                        // 画面再描画
                        panel1.Refresh();

                        if (nowPlayer.playerInfo == Player.PLAYER_CPU)
                        {
                            // CPUモードに移行(ハンドラコール)
                            m_cpuFlagProperty = true;

                            return;
                        }

                        // 相手が打てない
                        if (cppWrapper.GetEnumMove(boardclass) == 0)
                        {
                            m_passCount++;

                            if (m_passCount == 2)
                            {
                                // ゲーム終了
                                m_mode = ON_NOTHING;
                                m_cpuFlagProperty = false;
                                m_passCount = 0;
                                // 結果表示
                                PrintResult();
                                // 画面描画
                                panel1.Refresh();

                                return;
                            }

                            MessageBox.Show("プレイヤー" + (nowColor + 1) + "はパスです", "情報",
                                            MessageBoxButtons.OK, MessageBoxIcon.Information);
                            // プレイヤー変更
                            ChangePlayerReasonPass();

                            if (cppWrapper.GetEnumMove(boardclass) == 0) 
                            {
                                // 双方が人間でお互いがパスだった場合にここに来る
                                m_mode = ON_NOTHING;
                                m_cpuFlagProperty = false;
                                m_passCount = 0;
                                // 結果表示
                                PrintResult();
                                // 画面描画
                                panel1.Refresh();
                            }
                        }

                    }

                    break;
                case ON_EDIT:
                    // エディットモードの処理

                    // 押された瞬間の座標を取得
                    pos = mouseEvent.Location;
                    num = ((pos.X / 60) * BOARD_SIZE) + (pos.Y / 60);
                    ulong posBit = (1UL << num);
                    ulong bk = boardclass.GetBlack();
                    ulong wh = boardclass.GetWhite();

                    if (buttons == MouseButtons.Left)
                    {
                        // 黒を置くor白を置く
                        if ((bk & posBit) != 0)
                        {
                            // 黒がすでに置いてある場合は白にする
                            boardclass.EditBoard(bk & ~posBit, wh | posBit);
                        }
                        else if ((wh & posBit) != 0)
                        {
                            // 白がすでに置いてある場合は黒にする
                            boardclass.EditBoard(bk | posBit, wh & ~posBit);
                        }
                        else
                        {
                            // 置いてない場合は黒を置く
                            boardclass.EditBoard(bk | posBit, wh);
                        }
                        
                    }
                    else
                    {
                        // 消す
                        boardclass.EditBoard(bk & ~posBit, wh & ~posBit);
                    }

                    // 画面再描画
                    panel1.Refresh();

                    break;
                default:
                    // 何もしない
                    break;
            }
        }

        private void ChangePlayerReasonPass()
        {
            boardclass.ChangeColor();
            ChangePlayer();
        }

        private void setMove(ulong move)
        {
            m_cpuMoveProperty = move;
        }

        private void setNodeCount(ulong nodeCount)
        {
            StringBuilder sb = new StringBuilder(128);
            string temp;

            // 探索済みノード数
            sb.Append("node:");

            if (nodeCount >= 1000000000)  // Gn
            {
                sb.Append((nodeCount / (double)1000000000).ToString("f2"));
                sb.Append("[Gn]");
            }
            else if (nodeCount >= 1000000) // Mn
            {
                sb.Append((nodeCount / (double)1000000).ToString("f2"));
                sb.Append("[Mn]");
            }
            else if (nodeCount >= 1000) // Kn
            {
                sb.Append((nodeCount / (double)1000).ToString("f2"));
                sb.Append("[Kn]");
            }
            else
            {
                sb.Append(nodeCount);
                sb.Append("[n]");
            }
            
            // 経過時間
            sb.Append(" time:");
            sb.Append((m_sw.ElapsedMilliseconds / (double)1000).ToString("f2"));

            // NPS(node per second)
            sb.Append(" nps:");
            temp = ((nodeCount / (m_sw.ElapsedMilliseconds / (double)1000)) / 1000).ToString("f0");
            sb.Append(temp);
            sb.Append("[Knps]");

            toolStripStatusLabel1.Text = sb.ToString();
        }

        
        private void setCpuMessage(string cpuMessage)
        {
            toolStripStatusLabel3.Text = cpuMessage;
        }

        private ulong _m_cpuMove;
        public ulong m_cpuMoveProperty
        {
            get
            {
                return _m_cpuMove;
            }
            set
            {
                _m_cpuMove = value;
                OnPropertyChanged("CpuMove");
            }
        }

        private bool _m_cpuFlag;
        public bool m_cpuFlagProperty
        {
            get
            {
                return _m_cpuFlag;
            }
            set
            {
                _m_cpuFlag = value;
                if (value == true)
                {
                    OnPropertyChanged("CpuMode");
                }
            }
        }

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = null;

            if (name == "CpuMove")
            {
                handler = PropertyChangedCpuMove;
            }
            else if (name == "CpuMode")
            {
                handler = PropertyChangedCpuMode;
            }

            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        public void PropertyChangedCpuMove(object sender, PropertyChangedEventArgs e)
        {
            // CPUの算出した評価値を取得
            int eval = cppWrapper.GetLastEvaluation();

            if (m_cpuMoveProperty == MOVE_PASS)
            {
                // CPUはパス
                m_passCount++;

                // ゲーム終了？
                if (m_passCount == 2)
                {
                    // ゲーム終了
                    m_mode = ON_NOTHING;
                    m_cpuFlagProperty = false;
                    m_passCount = 0;
                    m_sw.Stop();
                    // 結果表示
                    PrintResult();
                    // 画面描画
                    panel1.Refresh();

                    return;
                }

                //MessageBox.Show("プレイヤー" + (nowColor + 1) + "はパスです", "情報", 
                //    MessageBoxButtons.OK, MessageBoxIcon.Information);

                // CPUがパスなのでプレイヤー変更
                ChangePlayerReasonPass();
            }
            else
            {
                // CPUが打てたのでパスカウントをリセット
                m_passCount = 0;
                // 盤面情報更新
                boardclass.move(cppWrapper.ConvertMoveBit(m_cpuMoveProperty));
                // CPUが打ったのでプレイヤー変更
                ChangePlayer();
                // 画面再描画
                panel1.Refresh();
            }

            // 評価値の表示
            toolStripStatusLabel2.Text = ConvertEvaltoString(eval, cpuClass[nowColor]);

            GameThread gmt;
            // プレイヤー変更後もCPUなら再度ゲームスレッドにリクエスト送信(双方がCPUの場合)
            if (nowPlayer.playerInfo == Player.PLAYER_CPU)
            {
                gmt = new GameThread();
                object[] args = new object[] { GameThread.CMD_CPU, boardclass, cpuClass[nowColor], this };
                gmt.m_recvcmdProperty = args;
                m_sw.Restart();

                return;
            }

            // 人間がパスだった場合は通知して再度ゲームスレッドにリクエスト送信
            if (nowPlayer.playerInfo == Player.PLAYER_HUMAN && cppWrapper.GetEnumMove(boardclass) == 0)
            {
                MessageBox.Show("プレイヤー" + (nowColor + 1) + "はパスです", "情報",
                                MessageBoxButtons.OK, MessageBoxIcon.Information);

                m_passCount++;
                if (m_passCount == 2)
                {
                    // ゲーム終了
                    m_mode = ON_NOTHING;
                    m_cpuFlagProperty = false;
                    m_passCount = 0;
                    // 結果表示
                    PrintResult();
                    // 画面描画
                    panel1.Refresh();

                    return;
                }

                // プレイヤー変更
                ChangePlayerReasonPass();
                // ゲームスレッドにCPU処理リクエスト送信
                gmt = new GameThread();
                object[] args = new object[] { GameThread.CMD_CPU, boardclass, cpuClass[nowColor], this };
                gmt.m_recvcmdProperty = args;
                m_sw.Restart();
                // 画面再描画
                panel1.Refresh();
                return;

            }

            // ここに来るのは次のプレイヤーが人間でかつ手を打てる状態
            m_cpuFlagProperty = false;
            m_passCount = 0;
            // 画面再描画
            panel1.Refresh();

        }

        private string ConvertEvaltoString(int eval, CpuClass cpu)
        {
            StringBuilder evalSb = new StringBuilder();

            int empty = cppWrapper.CountBit(~(boardclass.GetBlack() | boardclass.GetWhite()));

            if (empty < cpu.GetExactDepth())
            {
                if (eval >= 0)
                {
                    evalSb.Append("+");
                }

                evalSb.Append(eval);
            }
            else if (empty < cpu.GetWinLossDepth())
            {
                if (eval > 0)
                {
                    evalSb.Append("WIN");
                }
                else if (eval < 0)
                {
                    evalSb.Append("LOSS");
                }
                else
                {
                    evalSb.Append("DRAW");
                }
            }
            else
            {
                if (eval >= 0)
                {
                    evalSb.Append("+");
                }

                evalSb.Append((eval / (double)EVAL_THRESHOLD).ToString("f3"));
            }

            // CPUが定石から手を算出した場合
            if (cppWrapper.GetIsUseBook())
            {
                evalSb.Append("(book)");
            }

            // 中断ボタンが押された場合
            if (m_abort)
            {
                evalSb.Append("?(abort)");
                m_cpuFlagProperty = false;
                m_abort = false;
            }

            return evalSb.ToString();

        }

        public void PropertyChangedCpuMode(object sender, PropertyChangedEventArgs e)
        {
            // 画面再描画
            panel1.Refresh();

            // ゲームスレッドにCPU処理リクエスト送信
            GameThread gmt = new GameThread();
            object[] args = new object[] { GameThread.CMD_CPU, boardclass, cpuClass[nowColor], this };
            //MessageBox.Show("ゲームスレッドにCPU処理リクエスト送信", "情報", MessageBoxButtons.OK, MessageBoxIcon.Information);
            gmt.m_recvcmdProperty = args;

            toolStripStatusLabel1.Text = "";
            m_sw.Restart();

            // 画面再描画
            panel1.Refresh();
        }

        private void PrintResult()
        {
            String msg;
            String winStr;
            ulong bk = boardclass.GetBlack();
            ulong wh = boardclass.GetWhite();
            int bkCnt = cppWrapper.CountBit(bk);
            int whCnt = cppWrapper.CountBit(wh);

            if (bkCnt - whCnt > 0)
            {
                winStr = "黒の勝ち";
            }
            else if (bkCnt - whCnt < 0)
            {
                winStr = "白の勝ち";
            }
            else
            {
                winStr = "引き分け";
            }

            msg = String.Format("黒{0:D}-白{1:D}で{2}です。", bkCnt, whCnt, winStr);
            MessageBox.Show(msg, "情報", MessageBoxButtons.OK, MessageBoxIcon.Information);

        }

        private void comboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            ComboBox cbox = (ComboBox)sender;
            int index = cbox.SelectedIndex;
            uint color;

            if (index != 0 && index < 13)
            {
                if (cbox == this.comboBox1)
                {
                    cpuClass[BoardClass.BLACK].SetColor(BoardClass.BLACK);
                    color = BoardClass.BLACK;
                }
                else
                {
                    cpuClass[BoardClass.WHITE].SetColor(BoardClass.WHITE);
                    color = BoardClass.WHITE;
                }

                cpuClass[color].SetSearchDepth((uint)index * 2);
                cpuClass[color].SetWinLossDepth(dcTable[index - 1]);
                cpuClass[color].SetExactDepth(dcTable[index - 1] - 2);
            }
        }

        private void 新規ゲームToolStripMenuItem_Click(object sender, EventArgs e)
        {
            boardclass.InitBoard(COLOR_BLACK);
            nowColor = boardclass.GetNowColor();
            SetPlayerInfo();

            m_mode = ON_GAME;

            this.panel1.Refresh();
        }

        private void 盤面編集ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_mode = ON_EDIT;
        }

        private void panel1_DoubleClick(object sender, EventArgs e)
        {
            Point pos;
            int num;
            MouseEventArgs mouseEvent = (MouseEventArgs)e;
            MouseButtons buttons = mouseEvent.Button;

            switch (m_mode)
            {
                case ON_EDIT:
                    // エディットモードの処理

                    // 押された瞬間の座標を取得
                    pos = mouseEvent.Location;
                    num = ((pos.X / 60) * BOARD_SIZE) + (pos.Y / 60);
                    ulong posBit = (1UL << num);
                    ulong bk = boardclass.GetBlack();
                    ulong wh = boardclass.GetWhite();

                    if (buttons == MouseButtons.Left)
                    {
                        // 黒を置くor白を置く
                        if ((bk & posBit) != 0)
                        {
                            // 黒がすでに置いてある場合は白にする
                            boardclass.EditBoard(bk & ~posBit, wh | posBit);
                        }
                        else if ((wh & posBit) != 0)
                        {
                            // 白がすでに置いてある場合は黒にする
                            boardclass.EditBoard(bk | posBit, wh & ~posBit);
                        }
                        else
                        {
                            // 置いてない場合は黒を置く
                            boardclass.EditBoard(bk | posBit, wh);
                        }

                    }
                    else
                    {
                        // 消す
                        boardclass.EditBoard(bk & ~posBit, wh & ~posBit);
                    }

                    // 画面再描画
                    panel1.Refresh();

                    break;
                default:
                    // 何もしない
                    break;
            }
        }


    }
}
