using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace KZreversiGUI
{
    public partial class Form1 : Form
    {
        private Image bkImg;
        private Image whImg;

        public Form1()
        {
            InitializeComponent();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 3;
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void 終了ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (MessageBox.Show(
            "終了してもいいですか？", "確認",
            MessageBoxButtons.YesNo, MessageBoxIcon.Question
              ) == DialogResult.No)
            {
                e.Cancel = true;
            }
        }

        private void buttonClick(object sender, EventArgs e)
        {



        }

        private void Form1_Load(object sender, EventArgs e)
        {
            bkImg = imageList1.Images[0];
            whImg = imageList1.Images[1];

            // DLLのロード
            

        }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {
            if (bkImg != null)
            {
                //DrawImageメソッドで画像を座標(0, 0)の位置に表示する
                e.Graphics.DrawImage(bkImg,
                    181, 241, bkImg.Width, bkImg.Height);
                e.Graphics.DrawImage(bkImg,
                    241, 181, bkImg.Width, bkImg.Height);
            }
            if (whImg != null)
            {
                //DrawImageメソッドで画像を座標(0, 0)の位置に表示する
                e.Graphics.DrawImage(whImg,
                    181, 181, whImg.Width, whImg.Height);
                e.Graphics.DrawImage(whImg,
                    241, 241, whImg.Width, whImg.Height);
            }
        }

    }
}
