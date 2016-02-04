using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace KZreversi
{
    public partial class LoadForm : Form
    {

        public LoadForm()
        {
            InitializeComponent();
        }

        private void LoadForm_Shown(object sender, EventArgs e)
        {
            Thread loaderThread = new Thread(LoadData);
            loaderThread.Start();
        }

        private void LoadData(object obj)
        {
            // ラッパークラスの初期化とDLLのロード
            ((Form1)this.Owner).cppWrapper = new CppWrapper();
            ((Form1)this.Owner).loadResult = ((Form1)this.Owner).cppWrapper.LibInit();

            this.Close();
        }

    }
}
