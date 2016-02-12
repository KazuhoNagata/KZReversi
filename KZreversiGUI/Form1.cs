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
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace KZreversi
{
    public partial class Form1 : Form
    {

        public CppWrapper cppWrapper;
        public bool loadResult;

        private Image bkImg;
        private Image whImg;

        private BoardClass boardclass;
        private CpuClass cpuClass;

        private const int ON_NOTHING = 0;
        private const int ON_GAME = 1;
        private const int ON_EDIT = 2;
        private const int BOARD_SIZE = 8;

        private const int COLOR_BLACK = 0;
        private const int COLOR_WHITE = 1;

        private int m_mode = ON_NOTHING;

        private uint nowColor = COLOR_BLACK;


        private ulong playerPos;
        private uint playerColor;

        private bool m_cpuFlag = false;

        private int m_passCount;

        private const ulong MOVE_PASS = 0;

        public Form1()
        {
            InitializeComponent();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 3;

            boardclass = new BoardClass();
            cpuClass = new CpuClass();

            boardclass.SetColor(COLOR_BLACK);
            boardclass.SetBlack(0x810000000);
            boardclass.SetWhite(0x1008000000);
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

        private void buttonClick(object sender, EventArgs e)
        {
            if (sender == this.button5) 
            {
                m_mode = ON_GAME;
                playerColor = nowColor;

                this.panel1.Refresh();
            }
            else if (sender == this.button6)
            {
                m_mode = ON_NOTHING;

                this.panel1.Refresh();
            }

        }

        private void Form1_Load(object sender, EventArgs e)
        {
            bkImg = imageList1.Images[0];
            whImg = imageList1.Images[1];

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

            // CPU設定の初期化
            cpuClass.SetColor(BoardClass.WHITE);
            cpuClass.SetCasheSize(1<<21);
            cpuClass.SetSearchDepth(6);
            cpuClass.SetWinLossDepth(14);
            cpuClass.SetExactDepth(12);
            cpuClass.SetBookFlag(true);
            cpuClass.SetBookVariability(1);
            cpuClass.SetMpcFlag(true);
            cpuClass.SetTableFlag(true);
        }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {
            int pos;
            ulong temp;

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

            if (m_mode == ON_GAME && CheckGameEnd() == true) 
            {
                // ゲーム終了状態へ遷移
                m_mode = ON_NOTHING;
                PrintResult();
                return;
            }

            // ゲーム中の場合かつプレイヤーの手番の場合、着手可能場所を表示
            if (m_mode == ON_GAME && m_cpuFlag == false)
            {
                Font ft = new Font("MS UI Gothic", 14);
                Font ft2 = new Font("MS UI Gothic", 8);

                // CPUが最後に打った手を強調する
                pos = cppWrapper.ConvertMoveBit(m_cpuMoveProperty);
                if (pos >= 0) 
                {
                    e.Graphics.DrawString("●", ft2, Brushes.Red,
                    (pos / BOARD_SIZE) * 60 + 25,
                    (pos % BOARD_SIZE) * 60 + 26);
                }

                temp = cppWrapper.GetEnumMove(boardclass);
                playerPos = temp;
                if (playerPos != 0)
                {
                    m_passCount = 0;
                    while (temp > 0)
                    {
                        pos = cppWrapper.ConvertMoveBit(temp);
                        e.Graphics.DrawString("◆", ft, Brushes.Red,
                            (pos / BOARD_SIZE) * 60 + 20,
                            (pos % BOARD_SIZE) * 60 + 20);
                        temp ^= (1UL << pos);
                    }
                }
                else if (m_passCount == 2) 
                {
                    // ゲーム終了状態へ遷移
                    m_mode = ON_NOTHING;
                    // CPUがパスでプレイヤーもパス->ゲーム終了
                    PrintResult();
                }
                else
                {
                    m_passCount++;
                    m_cpuFlag = true;
                    boardclass.SetColor(playerColor ^ 1);
                    // プレイヤーが打てないのでCPUが再度打つ
                    MessageBox.Show("あなたはパスです", "情報", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    // CPU処理開始
                    m_cpuMoveProperty = cpuClass.StartCpuThread(boardclass);
                }

            }

        }

        private void panel1_Click(object sender, EventArgs e)
        {
            switch (m_mode) 
            {
                case ON_GAME:
                    Point pos = Cursor.Position;
                    // ゲーム中の場合は着手処理
                    pos = panel1.PointToClient(pos);

                    int num = ((pos.X / 60) * BOARD_SIZE) + (pos.Y / 60);

                    // 着手出来るかチェック
                    if ((playerPos & (1UL << num)) != 0) 
                    {
                        // 着手に合わせて盤面情報を更新
                        boardclass.move(num);

                        // 画面再描画
                        panel1.Refresh();

                        // 相手番の処理開始
                        if (playerColor == BoardClass.BLACK)
                        {
                            // ボードの状態を相手側の色にする
                            boardclass.SetColor(BoardClass.WHITE);
                            // 相手がCPUかをチェック
                            if (comboBox2.SelectedIndex != 0)
                            {
                                // CPU処理開始
                                m_cpuFlag = true;
                                m_cpuMoveProperty = cpuClass.StartCpuThread(boardclass);
                            }
                        }
                        else
                        {
                            // ボードの状態を相手側の色にする
                            boardclass.SetColor(BoardClass.BLACK);
                            // 相手がCPUかをチェック
                            if (comboBox1.SelectedIndex != 0)
                            {
                                // CPU処理開始
                                m_cpuFlag = true;
                                m_cpuMoveProperty = cpuClass.StartCpuThread(boardclass);
                            }
                        }
                    }

                    break;
                case ON_EDIT:
                    // エディットモードの処理
                    break;
                default:
                    // 何もしない
                    break;
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

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        public void PropertyChanged(object sender, PropertyChangedEventArgs e) 
        {
            if (m_cpuMoveProperty == MOVE_PASS)
            {
                // CPUはパス
                m_passCount++;
                MessageBox.Show("CPUはパスです", "情報", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            else
            {
                // CPUが打てたのでパスカウントをリセット
                m_passCount = 0;
                // 盤面情報更新
                boardclass.move(cppWrapper.ConvertMoveBit(m_cpuMoveProperty));
            }

            // CPU処理終了
            m_cpuFlag = false;
            // ボードの状態をプレイヤー側の色にする
            boardclass.SetColor(playerColor);
            // 画面再描画
            panel1.Refresh();
        }

        private bool CheckGameEnd()
        {
            ulong bk = boardclass.GetBlack();
            ulong wh = boardclass.GetWhite();

            if (cppWrapper.CountBit(bk | wh) == 64) 
            {
                return true;
            }

            if (cppWrapper.GetEnumMove(boardclass) == 0) 
            {
                // 色反転
                boardclass.SetColor(boardclass.GetColor() ^ 1);
                if (cppWrapper.GetEnumMove(boardclass) == 0)
                {
                    // 色反転は元に戻す
                    boardclass.SetColor(boardclass.GetColor() ^ 1);
                    return true;
                }
                // 色反転は元に戻す
                boardclass.SetColor(boardclass.GetColor() ^ 1);
            }

            return false;
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

    }
}
