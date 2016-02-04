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


        private const int ON_NOTHING = 0;
        private const int ON_GAME = 1;
        private const int ON_EDIT = 2;
        private const int BOARD_SIZE = 8;

        private const int COLOR_BLACK = 0;
        private const int COLOR_WHITE = 1;

        private int m_mode = ON_NOTHING;

        private uint nowColor = COLOR_BLACK;
        private BoardClass boardclass;

        private ulong playerPos;
        private uint playerColor;



        public Form1()
        {
            InitializeComponent();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 3;

            boardclass = new BoardClass();

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

        }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {
            uint pos;
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

            // ゲーム中の場合かつプレイヤーの手番の場合、着手可能場所を表示
            if (m_mode == ON_GAME && playerColor == boardclass.GetColor())
            {
                Font ft = new Font("MS UI Gothic", 14);
                temp = cppWrapper.GetEnumMove(boardclass);
                playerPos = temp;

                while (temp > 0)
                {
                    pos = cppWrapper.ConvertMoveBit(temp);
                    e.Graphics.DrawString("◆", ft, Brushes.Red,
                        (pos / BOARD_SIZE) * 60 + 20,
                        (pos % BOARD_SIZE) * 60 + 20);
                    temp ^= (1UL << (int)pos);
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

                    if ((playerPos & (1UL << num)) != 0) 
                    {
                        // 着手したので盤面情報を変更
                        ulong rev;
                        ulong bk = boardclass.GetBlack();
                        ulong wh = boardclass.GetWhite();

                        if (boardclass.GetColor() == BoardClass.BLACK)
                        {
                            rev = cppWrapper.GetBoardChangeInfo(bk, wh, num);

                            boardclass.SetBlack(bk ^ ((1UL << num) | rev));
                            boardclass.SetWhite(wh ^ rev);
                        }
                        else 
                        {
                            rev = cppWrapper.GetBoardChangeInfo(wh, bk, num);

                            boardclass.SetBlack(bk ^ rev);
                            boardclass.SetWhite(wh ^ ((1UL << num) | rev));
                        }
                        panel1.Refresh();

                        // CPUの起動
                        if (playerColor == BoardClass.BLACK && comboBox2.SelectedIndex != 0)
                        {
                            // To be implements...
                            //StartCpuThread(BoardClass.WHITE);
                        }
                        else if (playerColor == BoardClass.WHITE && comboBox1.SelectedIndex != 0)
                        {
                            // To be implements...
                            //StartCpuThread(BoardClass.BLACK);
                        }
                        else
                        {
                            // 両方とも人間だった場合はplayerColorを反転するだけ
                            boardclass.SetColor(BoardClass.WHITE);
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

    }
}
