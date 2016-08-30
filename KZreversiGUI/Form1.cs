//! @file
//! アプリケーションフォーム
//****************************************************************************
//       (c) COPYRIGHT kazura_utb 2016-  All Rights Reserved.
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
//│ A  │2016/02/01│新規作成                            │kazura_utb    │
//└──┴─────┴──────────────────┴───────┘
//****************************************************************************

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace KZreversi
{
    public partial class Form1 : Form
    {

        private BufferedPanel panel1;
        private BufferedPanel panel_back;

        public CppWrapper cppWrapper;
        public bool loadResult;

        private Bitmap bkImg;
        private Bitmap whImg;

        private BoardClass boardclass;
        private CpuClass[] cpuClass;

        private int EVAL_THRESHOLD = 10000;

        private const int ON_NOTHING = 0;
        private const int ON_GAME = 1;
        private const int ON_EDIT = 3;
        private const int ON_HINT = 4;

        private const int TURN_HUMAN = 0;
        private const int TURN_CPU = 1;

        private const int BOARD_SIZE = 8;

        private const int COLOR_BLACK = 0;
        private const int COLOR_WHITE = 1;

        private const int INFINITY_SCORE = 2500000;

        private uint[] dcTable = { 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26 };

        private int m_mode = ON_NOTHING;
        private bool m_abort = false;

        private uint nowColor = COLOR_BLACK;
        private Player nowPlayer;
        private Player[] playerArray;

        private int m_passCount;

        private const ulong MOVE_PASS = 0;

        // ヒント表示用
        private uint m_hintLevel;
        private List<int[]> m_hintList;
        private int m_hintEvalMax;

        private Stopwatch m_sw;

        public delegate void SetMoveProperty(ulong moves);
        public delegate void SetNodeCountProperty(ulong nodeCount);
        public delegate void DoHintProperty(HintClass evalList);
        public delegate void SetCpuMessageProperty(string cpuMsg);
        public delegate void SetMPCInfoProperty(string mpcMsg);

        public SetMoveProperty delegateObj;
        public SetNodeCountProperty nodeCountDelegate;
        public DoHintProperty hintDelegate;
        public SetCpuMessageProperty cpuMessageDelegate;
        public SetCpuMessageProperty setPVLineDelegate;
        public SetCpuMessageProperty setMPCInfoDelegate;

        Font m_ft = new Font("MS UI Gothic", 9);
        Font m_ft2 = new Font("MS UI Gothic", 8);
        Font m_ft3 = new Font("Arial", 18, FontStyle.Bold | FontStyle.Italic);

        public int m_event;

        private IntPtr cpuMessageDelegatePtr;
        private IntPtr setPVLineDelegatePtr;
        private IntPtr setMPCInfoDelegatePtr;

        private float m_scale;
        private float m_mass_size;
        private float m_fix_x = 0, m_fix_y = 0;
        private float m_board_width, m_board_height;
        private const float border_rate = (float)(290.0 / 2450.0);


        public Form1()
        {
            boardclass = new BoardClass();

            cpuClass = new CpuClass[2];
            cpuClass[0] = new CpuClass(); // BLACK
            cpuClass[1] = new CpuClass(); // WHITE

            delegateObj = new SetMoveProperty(setMove);
            nodeCountDelegate = new SetNodeCountProperty(setNodeCount);
            cpuMessageDelegate = new SetCpuMessageProperty(setCpuMessage);
            setPVLineDelegate = new SetCpuMessageProperty(setPVLine);
            setMPCInfoDelegate = new SetCpuMessageProperty(setMPCInfo);
            hintDelegate = new DoHintProperty(doHintProcess);

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

            // 探索時間表示用
            m_sw = new Stopwatch();

            // ヒント表示用
            m_hintList = new List<int[]>();
            
        }

        private void 終了ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            // デバッグ作業で煩わしいため一旦コメントアウト
            //if (MessageBox.Show(
            //"終了してもいいですか？", "確認",
            //MessageBoxButtons.YesNo, MessageBoxIcon.Question
            //  ) == DialogResult.No)
            //{
            //    e.Cancel = true;
            //}
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            // リソースの解放
            m_ft.Dispose();
            m_ft2.Dispose();
            m_ft3.Dispose();
            cppWrapper.ReleaseHash();
            cppWrapper.ReleaseBook();
        }

        private void buttonClick(object sender, EventArgs e)
        {
            if (sender == this.button7)  // ゲーム開始ボタン
            {
                boardclass.InitBoard(COLOR_BLACK);
                m_hintList.Clear();
                cppWrapper.ReleaseHash();
                nowColor = boardclass.GetNowColor();
                SetPlayerInfo();

                m_mode = ON_GAME;
                this.panel1.Refresh();

                ChangePlayer();
                if (nowPlayer.playerInfo == Player.PLAYER_CPU)
                {
                    // CPUモードに移行(ハンドラコール)
                    m_cpuFlagProperty = true;
                }
            }
            else if (sender == this.button5) // ゲーム再開ボタン
            {
                m_mode = ON_GAME;
                toolStripStatusLabel1.Text = "";
                toolStripStatusLabel2.Text = "";
                toolStripStatusLabel3.Text = "";
                //toolStripStatusLabel4.Text = "";

                m_hintList.Clear();
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
            bkImg = new Bitmap(KZreversi.Properties.Resources.othello_bk);
            whImg = new Bitmap(KZreversi.Properties.Resources.othello_wh);

            Bitmap panel1_bitmap = new Bitmap(KZreversi.Properties.Resources.othello_board);

            // 盤面背景のセット
            panel_back = new BufferedPanel();
            panel_back.Width = 534;
            panel_back.Height = 534;
            panel_back.Location = new Point(0, 0);
            panel_back.BackgroundImage = new Bitmap(KZreversi.Properties.Resources.wood_pattern);
            panel_back.BackgroundImageLayout = ImageLayout.Stretch;
            panel_back.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top;
            this.Controls.Add(panel_back);

            // 盤面のセット
            panel1 = new BufferedPanel();
            panel1.Width = 480;
            panel1.Height = 480;
            panel1.Location = new Point(26, 30);
            panel1.BackColor = Color.Transparent;
            panel1.BackgroundImage = panel1_bitmap;
            panel1.BackgroundImageLayout = ImageLayout.Zoom;
            panel1.Paint += panel1_Paint;
            panel1.Click += panel1_Click;
            panel1.DoubleClick += panel1_Click;
            panel1.Resize += panel1_Resize;
            panel1.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top;
            // 盤面背景の子として登録
            panel_back.Controls.Add(panel1);

            m_board_width = panel1.Width - (panel1.Width * border_rate);
            m_board_height = panel1.Height - (panel1.Height * border_rate);

            // 石の画像のスケーリング
            resize_stone(panel1);

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
            gcHandle.Free();

            // CPU情報表示用
            gcHandle = GCHandle.Alloc(setPVLineDelegate);
            setPVLineDelegatePtr = Marshal.GetFunctionPointerForDelegate(setPVLineDelegate);
            cppWrapper.EntryFunction(setPVLineDelegatePtr);
            gcHandle.Free();

            // MPC進捗状況表示用
            gcHandle = GCHandle.Alloc(setMPCInfoDelegate);
            setMPCInfoDelegatePtr = Marshal.GetFunctionPointerForDelegate(setMPCInfoDelegate);
            cppWrapper.EntryFunction(setMPCInfoDelegatePtr);
            gcHandle.Free();

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

                // size(MB)--> size * 1024 * 1024 / sizeof(hash_entry) = size * 1024 * 16
                cpuClass[i].SetCasheSize(128 * 1024 * 16); // 128MB default
                cpuClass[i].SetSearchDepth(6);
                cpuClass[i].SetWinLossDepth(14);
                cpuClass[i].SetExactDepth(12);
                cpuClass[i].SetBookFlag(true);
                cpuClass[i].SetBookVariability(2);
                cpuClass[i].SetMpcFlag(true);
                cpuClass[i].SetTableFlag(true);
            }
        }

        void resize_stone(Panel panel)
        {
            //label5.Text = "x=" + panel.Width + "y=" + panel.Height;
            float board_x, board_y;
            // 縦横の小さい方に合わせる
            if (panel.Width < panel.Height)
            {
                // margin考慮
                board_x = panel.Width - (panel1.Width * border_rate);
                board_y = panel.Height - (panel1.Width * border_rate);
                m_scale = board_x / (m_board_width + 16);
                m_mass_size = board_x / BOARD_SIZE;
                m_fix_x = (float)30.5 * m_scale;
                m_fix_y = (board_y - board_x) / 2 + ((float)30.5 * m_scale);
            }
            else
            {
                board_x = panel.Width - (panel1.Height * border_rate);
                board_y = panel.Height - (panel1.Height * border_rate);
                m_scale = board_y / (m_board_height + 16);
                m_mass_size = board_y / BOARD_SIZE;
                m_fix_x = (board_x - board_y) / 2 + ((float)30.5 * m_scale);
                m_fix_y = (float)30.5 * m_scale;
            }
        }

        // 盤面のリサイズ処理
        void panel1_Resize(object sender, EventArgs e)
        {
            resize_stone((Panel)sender);

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
                float stone_width = m_board_width / BOARD_SIZE;
                float stone_height = m_board_height / BOARD_SIZE;
                pos = cppWrapper.ConvertMoveBit(temp);
                e.Graphics.DrawImage(bkImg,
                    (pos / BOARD_SIZE) * m_mass_size + m_fix_x,
                    (pos % BOARD_SIZE) * m_mass_size + m_fix_y,
                    stone_width * m_scale, stone_height * m_scale);
                temp ^= (1UL << (int)pos);
            }

            temp = boardclass.GetWhite();
            while (temp > 0)
            {
                float stone_width = m_board_width / BOARD_SIZE;
                float stone_height = m_board_height / BOARD_SIZE;
                pos = cppWrapper.ConvertMoveBit(temp);
                e.Graphics.DrawImage(whImg,
                    (pos / BOARD_SIZE) * m_mass_size + m_fix_x,
                    (pos % BOARD_SIZE) * m_mass_size + m_fix_y,
                    stone_width * m_scale, stone_height * m_scale);
                temp ^= (1UL << (int)pos);
            }

            // 最後に打った手を強調する
            pos = boardclass.GetRecentMove();
            if (pos >= 0)
            {
                m_ft2 = new Font("MS UI Gothic", 8 * m_scale);
                e.Graphics.DrawString("●", m_ft2, Brushes.OrangeRed,
                (pos / BOARD_SIZE) * m_mass_size + (21 * m_scale) + m_fix_x,
                (pos % BOARD_SIZE) * m_mass_size + (21 * m_scale) + m_fix_y);
            }

            // ゲーム中の場合かつプレイヤーの手番の場合、着手可能場所を表示
            if (m_mode == ON_GAME && m_cpuFlagProperty == false)
            {
                temp = cppWrapper.GetEnumMove(boardclass);
                nowPlayer.moves = temp;
                if (temp != 0)
                {
                    m_ft = new Font("MS UI Gothic", 9 * m_scale);
                    m_passCount = 0;
                    while (temp > 0)
                    {
                        pos = cppWrapper.ConvertMoveBit(temp);
                        e.Graphics.DrawString("×", m_ft, Brushes.DarkOrange,
                            (pos / BOARD_SIZE) * m_mass_size + (19 * m_scale) + m_fix_x,
                            (pos % BOARD_SIZE) * m_mass_size + (21 * m_scale) + m_fix_y);
                        temp ^= (1UL << pos);
                    }
                }
            }
            else if (m_mode == ON_HINT && m_hintList.Count > 0 && m_cpuFlagProperty == false)
            {
                // 記憶したヒントを表示
                int eval;
                string sign;
                float font_fix_x;
                Brush brs = null;
                m_ft3 = new Font("Arial", 18 * m_scale, FontStyle.Bold | FontStyle.Italic);

                foreach (var data in m_hintList)
                {
                    pos = data[0];
                    eval = data[1];
                    if (eval >= 0) // +0 ～ +64
                    {
                        sign = "+";
                        font_fix_x = 3;
                        if (eval >= 100000) // +10 ～ +64
                        {
                            font_fix_x = 0;
                        }
                    }
                    else
                    {
                        if (eval > -10000) // -0
                        {
                            sign = "-";
                            font_fix_x = 5;
                        }
                        else if (eval <= -100000) // -10 ～ -64
                        {
                            sign = "";
                            font_fix_x = 2;
                        }
                        else // -1 ～ -9
                        {
                            sign = "";
                            font_fix_x = 5;
                        }
                    }
                    font_fix_x *= m_scale;
                    // ループの初回が最善手なので目立つよう表示
                    if (brs == null || eval >= m_hintEvalMax)
                    {
                        m_hintEvalMax = eval;
                        brs = Brushes.LightGreen;
                    }
                    else brs = Brushes.DarkOrange;
                    eval /= 10000;
                    e.Graphics.DrawString(sign + eval, m_ft3, brs,
                           (pos / BOARD_SIZE) * m_mass_size + (2 * m_scale) + m_fix_x + font_fix_x,
                           (pos % BOARD_SIZE) * m_mass_size + (14 * m_scale) + m_fix_y);
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
                    num = (int)(((double)pos.X - m_fix_x) / m_mass_size) * BOARD_SIZE;
                    num += (int)(((double)pos.Y - m_fix_y) / m_mass_size);
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
            StringBuilder sb = new StringBuilder(256);
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

        // lock用オブジェクト
        private static Object lockObj = new Object();
        private void setCpuMessage(string cpuMessage)
        {
            lock (lockObj)
            {
                toolStripStatusLabel3.Text = cpuMessage;
            }
        }

        private void setPVLine(string cpuMessage)
        {
            lock (lockObj)
            {
                toolStripStatusLabel4.Text = cpuMessage;
            }
        }

        private void setMPCInfo(string mpcMessage)
        {
            lock (lockObj)
            {
                toolStripStatusLabel2.Text = mpcMessage;
            }
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

        private bool _m_hintFlag;
        public bool m_hintFlagProperty
        {
            get
            {
                return _m_hintFlag;
            }
            set
            {
                _m_hintFlag = value;
                if (value == true)
                {
                    OnPropertyChanged("Hint");
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
            else if (name == "Hint")
            {
                handler = PropertyChangedHint;
            }

            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        public void PropertyChangedCpuMove(object sender, PropertyChangedEventArgs e)
        {
            bool bootRet;
            // CPUの算出した評価値を取得
            int eval = cppWrapper.GetLastEvaluation();
            // 評価値の表示
            toolStripStatusLabel2.Text = ConvertEvaltoString(eval, cpuClass[nowColor]);

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
                    SetControlEnable(true);
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
                bootRet = boardclass.move(cppWrapper.ConvertMoveBit(m_cpuMoveProperty));
                if (bootRet == false)
                {
                    MessageBox.Show("内部エラーが発生しました", "エラー",
                                MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                // CPUが打ったのでプレイヤー変更
                ChangePlayer();
                SetControlEnable(true);
            }

            GameThread gmt;
            // プレイヤー変更後もCPUなら再度ゲームスレッドにリクエスト送信(双方がCPUの場合)
            if (m_mode == ON_GAME && nowPlayer.playerInfo == Player.PLAYER_CPU)
            {
                // 画面再描画(前のCPUの手を画面に反映しておく)
                panel1.Refresh();

                gmt = new GameThread();
                object[] args = new object[] { GameThread.CMD_CPU, boardclass, cpuClass[nowColor], this };
                gmt.m_recvcmdProperty = args;
                m_sw.Restart();

                return;
            }

            // 人間がパスだった場合は通知して再度ゲームスレッドにリクエスト送信
            if (nowPlayer.playerInfo == Player.PLAYER_HUMAN && cppWrapper.GetEnumMove(boardclass) == 0)
            {
                // 画面再描画(前のCPUの手を画面に反映しておく)
                panel1.Refresh();

                m_sw.Stop();
                MessageBox.Show("プレイヤー" + (nowColor + 1) + "はパスです", "情報",
                                MessageBoxButtons.OK, MessageBoxIcon.Information);
                m_sw.Start();

                m_passCount++;
                if (m_passCount == 2)
                {
                    // ゲーム終了
                    m_mode = ON_NOTHING;
                    m_cpuFlagProperty = false;
                    m_passCount = 0;
                    SetControlEnable(true);
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
            SetControlEnable(true);
            // 画面再描画
            panel1.Refresh();

        }

        private string ConvertEvaltoString(int eval, CpuClass cpu)
        {
            StringBuilder evalSb = new StringBuilder();

            int empty = cppWrapper.CountBit(~(boardclass.GetBlack() | boardclass.GetWhite()));

            if (empty <= cpu.GetExactDepth())
            {
                if (eval >= 0)
                {
                    evalSb.Append("+");
                }

                evalSb.Append(eval);
            }
            else if (empty <= cpu.GetWinLossDepth())
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
            // UIを中断ボタン以外無効化
            SetControlEnable(false);
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


        public void PropertyChangedHint(object sender, PropertyChangedEventArgs e)
        {
            // 画面再描画
            panel1.Refresh();
            // UIを中断ボタン以外無効化
            SetControlEnable(false);
            // ゲームスレッドにヒント処理リクエストを送信
            GameThread gmt = new GameThread();
            object[] args = new object[] { GameThread.CMD_HINT, boardclass, cpuClass[nowColor], this, m_hintLevel };
            gmt.m_recvcmdProperty = args;
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



        private void doHintProcess(HintClass hintData)
        {
            if (hintData != null)
            {
                if (hintData.GetPos() == 64)
                {

                }
                else
                {
                    // ヒントデータ更新
                    int position = hintData.GetPos();
                    int index = findIndexFromPosition(position);
                    if (index == -1)
                    {
                        // 新規データ
                        m_hintList.Add(new int[] { position, hintData.GetEval() });
                    }
                    else
                    {
                        // 更新
                        m_hintList[index] = new int[] { position, hintData.GetEval() };
                    }
                    // ソート処理
                    m_hintList.Sort(CompareEval);
                }
            }
            else
            {
                // 終了通知
                m_hintFlagProperty = false;
                SetControlEnable(true);
            }
            // 画面再描画
            panel1.Refresh();
        }

        private int CompareEval(int[] x, int[] y)
        {
            return y[1] - x[1];
        }




        int findIndexFromPosition(int pos) 
        {
            int i = 0, index = -1;
            foreach (var data in m_hintList) 
            {
                if (data[0] == pos)
                {
                    index = i;
                    break;
                }
                i++;
            }

            return index;
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
            else if (index == 13)
            {
                // 強制勝敗探索モード
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

                cpuClass[color].SetSearchDepth(0);
                cpuClass[color].SetWinLossDepth(60);
                cpuClass[color].SetExactDepth(0);
            }
            else if (index == 14)
            {
                // 強制石差探索モード
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

                cpuClass[color].SetSearchDepth(0);
                cpuClass[color].SetWinLossDepth(0);
                cpuClass[color].SetExactDepth(60);
            }
        }

        private void 新規ゲームToolStripMenuItem_Click(object sender, EventArgs e)
        {
            boardclass.InitBoard(COLOR_BLACK);
            cppWrapper.ReleaseHash();
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

        private void 盤面初期化ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            boardclass.InitBoard(COLOR_BLACK);
            nowColor = boardclass.GetNowColor();
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void mPC探索を行うToolStripMenuItem_Click(object sender, EventArgs e)
        {

            if (MPC_ToolStripMenuItem.Checked)
            {
                cpuClass[0].SetMpcFlag(false);
                cpuClass[1].SetMpcFlag(false);
                MPC_ToolStripMenuItem.Checked = false;
            }
            else
            {
                cpuClass[0].SetMpcFlag(true);
                cpuClass[1].SetMpcFlag(true);
                MPC_ToolStripMenuItem.Checked = true;
            }
        }


        private void 置換表を使うToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (Table_ToolStripMenuItem.Checked)
            {
                cpuClass[0].SetTableFlag(false);
                cpuClass[1].SetTableFlag(false);
                Table_ToolStripMenuItem.Checked = false;
            }
            else
            {
                cpuClass[0].SetTableFlag(true);
                cpuClass[1].SetTableFlag(true);
                Table_ToolStripMenuItem.Checked = true;
            }
        }

        private void bookを使用ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (BOOKFLAG_ToolStripMenuItem.Checked)
            {
                cpuClass[0].SetBookFlag(false);
                cpuClass[1].SetBookFlag(false);
                BOOKFLAG_ToolStripMenuItem.Checked = false;
            }
            else
            {
                cpuClass[0].SetBookFlag(true);
                cpuClass[1].SetBookFlag(true);
                BOOKFLAG_ToolStripMenuItem.Checked = true;
            }
        }

        private void fFO40ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#40(black to move) (WinLoss:[a2:WIN] Exact:[a2:+38])
            //boardclass.InitBoard(COLOR_BLACK, 0x6042795c404000, 0xff9fbc8080000000);
            boardclass.InitBoard(COLOR_BLACK, 9158069842325798912, 11047339776155165);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO41ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#41(black to move) (WinLoss:(h4:DRAW) 3.46sec Exact:(h4:+0) 3.28sec)
            boardclass.InitBoard(COLOR_BLACK, 616174399789064, 39493460025648416);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO42ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#42(black to move) (WinLoss:(g2:WIN) 2.26sec Exact:(g2:+6) 3.88sec)
            boardclass.InitBoard(COLOR_BLACK, 22586176447709200, 9091853944868375556);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO43ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#43(white to move) (WinLoss:(c7:LOSS) 0.501sec Exact:(c7:-12) 8.06sec)
            boardclass.InitBoard(COLOR_WHITE, 38808086923902976, 13546258740034592);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 14;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO44ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#44(white to move) (WinLoss:(d2:LOSS) 0.729sec  Exact:(d2:-14) 2.09ec)
            boardclass.InitBoard(COLOR_WHITE, 2494790880993312, 1010251075753548824);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 14;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO45ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#45(black to move) (WinLoss:(b2:WIN) 2.37sec Exact:(b2:+6) 38.21sec)
            boardclass.InitBoard(COLOR_BLACK, 282828816915486, 9287318235258944);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO46ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#46(black to move) (WinLoss:(b7:LOSS) 8.09sec Exact:(b3:-8) 15.63sec)
            boardclass.InitBoard(COLOR_BLACK, 4052165999611379712, 36117299622447104);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO47ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#47(white to move) (WinLoss:(g2:WIN) 2.87sec: Exact:(g2:+4) 4.70sec)
            boardclass.InitBoard(COLOR_WHITE, 277938752194568, 3536224466208);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 14;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO48ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#48(white to move) (WinLoss:(f6:WIN) 0.98sec: Exact:(f6:+28) 42.42sec)
            boardclass.InitBoard(COLOR_WHITE, 38519958422848574, 4725679339520);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 14;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO49ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#49(black to move) (WinLoss:(e1:WIN) 4.52sec: Exact:(e1:+16) 95.10sec)
            boardclass.InitBoard(COLOR_BLACK, 5765976742297600, 4253833575484);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO50ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#50(black to move) (WinLoss:(d8:WIN) 2.861sec: Exact:(d8:+10) 141.83sec)
            boardclass.InitBoard(COLOR_BLACK, 4504145659822080, 4336117619740130304);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO51ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#51(white to move) (WinLoss:(e2:WIN) 2.92sec: Exact:(e2:+6) 137.96sec)
            boardclass.InitBoard(COLOR_WHITE, 349834415978528, 8664011788383158280);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 14;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO52ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#52(white to move) (WinLoss:(a3:DRAW) 140.0sec: Exact:(a3:+0) 167.26sec)
            boardclass.InitBoard(COLOR_WHITE, 9096176176681728056, 35409824317440);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 14;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO53ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#53(black to move) (WinLoss:(WIN) 8.79sec: Exact:(g2:+4) 22.5sec)
            boardclass.InitBoard(COLOR_BLACK, 2515768979493888, 8949795312300457984);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO54ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#54(black to move) (WinLoss:(WIN) 8.79sec: Exact:(g2:+4) 22.5sec)
            boardclass.InitBoard(COLOR_BLACK, 26457201720894, 289431515079835648);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO55ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#55(white to move) (WinLoss:(WIN) 8.79sec: Exact:(g2:+4) 22.5sec)
            boardclass.InitBoard(COLOR_WHITE, 4635799596172290, 289361502099486840);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 14;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO56ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#56(white to move) (WinLoss:(WIN) 8.79sec: Exact:(g2:+4) 22.5sec)
            boardclass.InitBoard(COLOR_WHITE, 4925086697193472, 9007372734053408);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 14;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO57ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#57(black to move) (WinLoss:(WIN) 8.79sec: Exact:(g2:+4) 22.5sec)
            boardclass.InitBoard(COLOR_BLACK, 9060166336512000, 8943248156475301888);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO58ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#58(black to move) (WinLoss:(WIN) 8.79sec: Exact:(g2:+4) 22.5sec)
            boardclass.InitBoard(COLOR_BLACK, 4636039783186432, 3383245044333600);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void fFO59ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //FFO#59(black to move) (WinLoss:(g8:WIN) 0.32sec: Exact:(g8:+64) 0.34sec)
            boardclass.InitBoard(COLOR_BLACK, 17320879491911778304, 295223649004691488);
            nowColor = boardclass.GetNowColor();
            comboBox1.SelectedIndex = 14;
            comboBox2.SelectedIndex = 0;
            SetPlayerInfo();
            panel1.Refresh();
        }

        private void ConfigCasheToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int size = CasheToolStripMenuItem.DropDownItems.Count;
            ToolStripMenuItem stripItem = ((ToolStripMenuItem)sender);

            // チェック全解除
            for (int i = 0; i < size; i++)
            {
                ((ToolStripMenuItem)CasheToolStripMenuItem.DropDownItems[i]).Checked = false;
            }

            uint casheSize = Convert.ToUInt32(stripItem.Text.Replace("MB", ""));

            // size(MB)--> size * 1024 * 1024 / sizeof(hash_entry) = size * 1024 * 16
            cpuClass[0].SetCasheSize(casheSize * 1024 * 16);
            cpuClass[1].SetCasheSize(casheSize * 1024 * 16);
            stripItem.Checked = true;
        }


        void SetControlEnable(bool flag)
        {
            button1.Enabled = flag;
            button2.Enabled = flag;
            button3.Enabled = flag;
            button4.Enabled = flag;
            button5.Enabled = flag;
            button7.Enabled = flag;
            comboBox1.Enabled = flag;
            comboBox2.Enabled = flag;

            menuStrip1.Enabled = flag;

        }

        private void 手番変更ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            boardclass.ChangeColor();
            SetPlayerInfo();
            ChangePlayer();

            if (boardclass.GetRecentTurn() == 0)
            {
                boardclass.InitBoard(nowColor, boardclass.GetBlack(), boardclass.GetWhite());
            }
        }

        private void 置換表のメモリを解放ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            cppWrapper.ReleaseHash();
        }

        private void OnChangeBookToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int size = changeBookToolStripMenuItem.DropDownItems.Count;
            ToolStripMenuItem stripItem = ((ToolStripMenuItem)sender);

            // チェック全解除
            for (int i = 0; i < size; i++)
            {
                ((ToolStripMenuItem)changeBookToolStripMenuItem.DropDownItems[i]).Checked = false;
            }
            stripItem.Checked = true;

            uint idx = (uint)changeBookToolStripMenuItem.DropDownItems.IndexOf(stripItem);

            cpuClass[0].SetBookVariability(idx);
            cpuClass[1].SetBookVariability(idx);

        }

        private void HintToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int size = hintToolStripMenuItem.DropDownItems.Count;
            ToolStripMenuItem stripItem = ((ToolStripMenuItem)sender);

            // チェック全解除
            for (int i = 0; i < size; i++)
            {
                ((ToolStripMenuItem)hintToolStripMenuItem.DropDownItems[i]).Checked = false;
            }
            stripItem.Checked = true;

            // レベル取得
            m_hintLevel = (uint)hintToolStripMenuItem.DropDownItems.IndexOf(stripItem);

            m_mode = ON_HINT;
            m_hintEvalMax = -INFINITY_SCORE;
            m_hintList.Clear();
            // ヒント処理ハンドラをコール
            m_hintFlagProperty = true;
        }


    }
}
