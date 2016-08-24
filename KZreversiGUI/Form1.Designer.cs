namespace KZreversi
{
    partial class Form1
    {
        /// <summary>
        /// 必要なデザイナー変数です。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        /// <param name="disposing">マネージ リソースが破棄される場合 true、破棄されない場合は false です。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows フォーム デザイナーで生成されたコード

        /// <summary>
        /// デザイナー サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディターで変更しないでください。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel2 = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel3 = new System.Windows.Forms.ToolStripStatusLabel();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.ファイルToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.置換表のメモリを解放ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.終了ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.編集ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ゲーム開始ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.新規ゲームToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.黒から始めるToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.白から始めるToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.中断ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.手戻すToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.手進むToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.手番変更ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.盤面操作ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.盤面初期化ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.盤面編集ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.盤面の回転と対照変換ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.度回転ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.度回転ToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.度回転ToolStripMenuItem2 = new System.Windows.Forms.ToolStripMenuItem();
            this.度回転ToolStripMenuItem3 = new System.Windows.Forms.ToolStripMenuItem();
            this.ｙ軸対称ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ブラックライン対称ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ホワイトライン対照変換ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.機能ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.BOOKFLAG_ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.changeBookToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.変化なしToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.変化小ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.変化中ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.変化大ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ランダムToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aI設定ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.詳細結果の表示ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.bestlineの表示ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.思考過程を表示ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Table_ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.MPC_ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.CasheToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mBToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mBToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.mBToolStripMenuItem2 = new System.Windows.Forms.ToolStripMenuItem();
            this.mBToolStripMenuItem3 = new System.Windows.Forms.ToolStripMenuItem();
            this.mBToolStripMenuItem4 = new System.Windows.Forms.ToolStripMenuItem();
            this.mBToolStripMenuItem5 = new System.Windows.Forms.ToolStripMenuItem();
            this.mBToolStripMenuItem6 = new System.Windows.Forms.ToolStripMenuItem();
            this.ツールToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.hintToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.表示なしToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Hint1ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Hint2ToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.Hint3ToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.Hint4ToolStripMenuItem3 = new System.Windows.Forms.ToolStripMenuItem();
            this.Hint5ToolStripMenuItem4 = new System.Windows.Forms.ToolStripMenuItem();
            this.Hint6ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Hint7ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Hint8ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFOテストToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO40ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO41ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO42ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO43ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO44ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO45ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO46ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO47ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO48ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO49ToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO50ToolStripMenuItem2 = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO51ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO52ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO44ToolStripMenuItem5 = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO54ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO55ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO56ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO57ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO58ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fFO59ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ヘルプToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.kZreversiについてToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.listBox2 = new System.Windows.Forms.ListBox();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.comboBox2 = new System.Windows.Forms.ComboBox();
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.button4 = new System.Windows.Forms.Button();
            this.button3 = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.statusStrip2 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel4 = new System.Windows.Forms.ToolStripStatusLabel();
            this.button5 = new System.Windows.Forms.Button();
            this.button6 = new System.Windows.Forms.Button();
            this.imageList1 = new System.Windows.Forms.ImageList(this.components);
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.button7 = new System.Windows.Forms.Button();
            this.statusStrip1.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.statusStrip2.SuspendLayout();
            this.SuspendLayout();
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.toolStripStatusLabel2,
            this.toolStripStatusLabel3});
            this.statusStrip1.Location = new System.Drawing.Point(0, 552);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(799, 22);
            this.statusStrip1.SizingGrip = false;
            this.statusStrip1.TabIndex = 2;
            this.statusStrip1.Text = "statusbar1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.AutoSize = false;
            this.toolStripStatusLabel1.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.toolStripStatusLabel1.BorderStyle = System.Windows.Forms.Border3DStyle.SunkenOuter;
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(300, 17);
            this.toolStripStatusLabel1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // toolStripStatusLabel2
            // 
            this.toolStripStatusLabel2.AutoSize = false;
            this.toolStripStatusLabel2.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.toolStripStatusLabel2.BorderStyle = System.Windows.Forms.Border3DStyle.SunkenOuter;
            this.toolStripStatusLabel2.Name = "toolStripStatusLabel2";
            this.toolStripStatusLabel2.Size = new System.Drawing.Size(150, 17);
            // 
            // toolStripStatusLabel3
            // 
            this.toolStripStatusLabel3.AutoSize = false;
            this.toolStripStatusLabel3.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.toolStripStatusLabel3.BorderStyle = System.Windows.Forms.Border3DStyle.SunkenOuter;
            this.toolStripStatusLabel3.Name = "toolStripStatusLabel3";
            this.toolStripStatusLabel3.Size = new System.Drawing.Size(350, 17);
            this.toolStripStatusLabel3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // menuStrip1
            // 
            this.menuStrip1.BackColor = System.Drawing.SystemColors.GradientInactiveCaption;
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ファイルToolStripMenuItem,
            this.編集ToolStripMenuItem,
            this.盤面操作ToolStripMenuItem,
            this.機能ToolStripMenuItem,
            this.ツールToolStripMenuItem,
            this.ヘルプToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(799, 24);
            this.menuStrip1.TabIndex = 3;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // ファイルToolStripMenuItem
            // 
            this.ファイルToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.置換表のメモリを解放ToolStripMenuItem,
            this.終了ToolStripMenuItem});
            this.ファイルToolStripMenuItem.Name = "ファイルToolStripMenuItem";
            this.ファイルToolStripMenuItem.Size = new System.Drawing.Size(52, 20);
            this.ファイルToolStripMenuItem.Text = "ファイル";
            // 
            // 置換表のメモリを解放ToolStripMenuItem
            // 
            this.置換表のメモリを解放ToolStripMenuItem.Name = "置換表のメモリを解放ToolStripMenuItem";
            this.置換表のメモリを解放ToolStripMenuItem.Size = new System.Drawing.Size(178, 22);
            this.置換表のメモリを解放ToolStripMenuItem.Text = "置換表のメモリを解放";
            this.置換表のメモリを解放ToolStripMenuItem.Click += new System.EventHandler(this.置換表のメモリを解放ToolStripMenuItem_Click);
            // 
            // 終了ToolStripMenuItem
            // 
            this.終了ToolStripMenuItem.Name = "終了ToolStripMenuItem";
            this.終了ToolStripMenuItem.Size = new System.Drawing.Size(178, 22);
            this.終了ToolStripMenuItem.Text = "終了";
            this.終了ToolStripMenuItem.Click += new System.EventHandler(this.終了ToolStripMenuItem_Click);
            // 
            // 編集ToolStripMenuItem
            // 
            this.編集ToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ゲーム開始ToolStripMenuItem,
            this.中断ToolStripMenuItem,
            this.手戻すToolStripMenuItem,
            this.手進むToolStripMenuItem,
            this.手番変更ToolStripMenuItem});
            this.編集ToolStripMenuItem.Name = "編集ToolStripMenuItem";
            this.編集ToolStripMenuItem.Size = new System.Drawing.Size(45, 20);
            this.編集ToolStripMenuItem.Text = "ゲーム";
            // 
            // ゲーム開始ToolStripMenuItem
            // 
            this.ゲーム開始ToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.新規ゲームToolStripMenuItem,
            this.黒から始めるToolStripMenuItem,
            this.白から始めるToolStripMenuItem});
            this.ゲーム開始ToolStripMenuItem.Name = "ゲーム開始ToolStripMenuItem";
            this.ゲーム開始ToolStripMenuItem.Size = new System.Drawing.Size(124, 22);
            this.ゲーム開始ToolStripMenuItem.Text = "ゲーム開始";
            // 
            // 新規ゲームToolStripMenuItem
            // 
            this.新規ゲームToolStripMenuItem.Name = "新規ゲームToolStripMenuItem";
            this.新規ゲームToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.新規ゲームToolStripMenuItem.Text = "新規ゲーム";
            this.新規ゲームToolStripMenuItem.Click += new System.EventHandler(this.新規ゲームToolStripMenuItem_Click);
            // 
            // 黒から始めるToolStripMenuItem
            // 
            this.黒から始めるToolStripMenuItem.Name = "黒から始めるToolStripMenuItem";
            this.黒から始めるToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.黒から始めるToolStripMenuItem.Text = "黒から始める";
            // 
            // 白から始めるToolStripMenuItem
            // 
            this.白から始めるToolStripMenuItem.Name = "白から始めるToolStripMenuItem";
            this.白から始めるToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.白から始めるToolStripMenuItem.Text = "白から始める";
            // 
            // 中断ToolStripMenuItem
            // 
            this.中断ToolStripMenuItem.Name = "中断ToolStripMenuItem";
            this.中断ToolStripMenuItem.Size = new System.Drawing.Size(124, 22);
            this.中断ToolStripMenuItem.Text = "中断";
            // 
            // 手戻すToolStripMenuItem
            // 
            this.手戻すToolStripMenuItem.Name = "手戻すToolStripMenuItem";
            this.手戻すToolStripMenuItem.Size = new System.Drawing.Size(124, 22);
            this.手戻すToolStripMenuItem.Text = "１手戻す";
            // 
            // 手進むToolStripMenuItem
            // 
            this.手進むToolStripMenuItem.Name = "手進むToolStripMenuItem";
            this.手進むToolStripMenuItem.Size = new System.Drawing.Size(124, 22);
            this.手進むToolStripMenuItem.Text = "１手進む";
            // 
            // 手番変更ToolStripMenuItem
            // 
            this.手番変更ToolStripMenuItem.Name = "手番変更ToolStripMenuItem";
            this.手番変更ToolStripMenuItem.Size = new System.Drawing.Size(124, 22);
            this.手番変更ToolStripMenuItem.Text = "手番変更";
            this.手番変更ToolStripMenuItem.Click += new System.EventHandler(this.手番変更ToolStripMenuItem_Click);
            // 
            // 盤面操作ToolStripMenuItem
            // 
            this.盤面操作ToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.盤面初期化ToolStripMenuItem,
            this.盤面編集ToolStripMenuItem,
            this.盤面の回転と対照変換ToolStripMenuItem});
            this.盤面操作ToolStripMenuItem.Name = "盤面操作ToolStripMenuItem";
            this.盤面操作ToolStripMenuItem.Size = new System.Drawing.Size(67, 20);
            this.盤面操作ToolStripMenuItem.Text = "盤面操作";
            // 
            // 盤面初期化ToolStripMenuItem
            // 
            this.盤面初期化ToolStripMenuItem.Name = "盤面初期化ToolStripMenuItem";
            this.盤面初期化ToolStripMenuItem.Size = new System.Drawing.Size(189, 22);
            this.盤面初期化ToolStripMenuItem.Text = "盤面初期化";
            this.盤面初期化ToolStripMenuItem.Click += new System.EventHandler(this.盤面初期化ToolStripMenuItem_Click);
            // 
            // 盤面編集ToolStripMenuItem
            // 
            this.盤面編集ToolStripMenuItem.Name = "盤面編集ToolStripMenuItem";
            this.盤面編集ToolStripMenuItem.Size = new System.Drawing.Size(189, 22);
            this.盤面編集ToolStripMenuItem.Text = "盤面編集";
            this.盤面編集ToolStripMenuItem.Click += new System.EventHandler(this.盤面編集ToolStripMenuItem_Click);
            // 
            // 盤面の回転と対照変換ToolStripMenuItem
            // 
            this.盤面の回転と対照変換ToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.度回転ToolStripMenuItem,
            this.度回転ToolStripMenuItem1,
            this.度回転ToolStripMenuItem2,
            this.度回転ToolStripMenuItem3,
            this.ｙ軸対称ToolStripMenuItem,
            this.ブラックライン対称ToolStripMenuItem,
            this.ホワイトライン対照変換ToolStripMenuItem});
            this.盤面の回転と対照変換ToolStripMenuItem.Name = "盤面の回転と対照変換ToolStripMenuItem";
            this.盤面の回転と対照変換ToolStripMenuItem.Size = new System.Drawing.Size(189, 22);
            this.盤面の回転と対照変換ToolStripMenuItem.Text = "盤面の回転と対称変換";
            // 
            // 度回転ToolStripMenuItem
            // 
            this.度回転ToolStripMenuItem.Name = "度回転ToolStripMenuItem";
            this.度回転ToolStripMenuItem.Size = new System.Drawing.Size(184, 22);
            this.度回転ToolStripMenuItem.Text = "90度回転";
            // 
            // 度回転ToolStripMenuItem1
            // 
            this.度回転ToolStripMenuItem1.Name = "度回転ToolStripMenuItem1";
            this.度回転ToolStripMenuItem1.Size = new System.Drawing.Size(184, 22);
            this.度回転ToolStripMenuItem1.Text = "180度回転";
            // 
            // 度回転ToolStripMenuItem2
            // 
            this.度回転ToolStripMenuItem2.Name = "度回転ToolStripMenuItem2";
            this.度回転ToolStripMenuItem2.Size = new System.Drawing.Size(184, 22);
            this.度回転ToolStripMenuItem2.Text = "270度回転";
            // 
            // 度回転ToolStripMenuItem3
            // 
            this.度回転ToolStripMenuItem3.Name = "度回転ToolStripMenuItem3";
            this.度回転ToolStripMenuItem3.Size = new System.Drawing.Size(184, 22);
            this.度回転ToolStripMenuItem3.Text = "X軸対称変換";
            // 
            // ｙ軸対称ToolStripMenuItem
            // 
            this.ｙ軸対称ToolStripMenuItem.Name = "ｙ軸対称ToolStripMenuItem";
            this.ｙ軸対称ToolStripMenuItem.Size = new System.Drawing.Size(184, 22);
            this.ｙ軸対称ToolStripMenuItem.Text = "Y軸対称変換";
            // 
            // ブラックライン対称ToolStripMenuItem
            // 
            this.ブラックライン対称ToolStripMenuItem.Name = "ブラックライン対称ToolStripMenuItem";
            this.ブラックライン対称ToolStripMenuItem.Size = new System.Drawing.Size(184, 22);
            this.ブラックライン対称ToolStripMenuItem.Text = "ブラックライン対称変換";
            // 
            // ホワイトライン対照変換ToolStripMenuItem
            // 
            this.ホワイトライン対照変換ToolStripMenuItem.Name = "ホワイトライン対照変換ToolStripMenuItem";
            this.ホワイトライン対照変換ToolStripMenuItem.Size = new System.Drawing.Size(184, 22);
            this.ホワイトライン対照変換ToolStripMenuItem.Text = "ホワイトライン対称変換";
            // 
            // 機能ToolStripMenuItem
            // 
            this.機能ToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.BOOKFLAG_ToolStripMenuItem,
            this.changeBookToolStripMenuItem,
            this.aI設定ToolStripMenuItem,
            this.CasheToolStripMenuItem});
            this.機能ToolStripMenuItem.Name = "機能ToolStripMenuItem";
            this.機能ToolStripMenuItem.Size = new System.Drawing.Size(62, 20);
            this.機能ToolStripMenuItem.Text = "オプション";
            // 
            // BOOKFLAG_ToolStripMenuItem
            // 
            this.BOOKFLAG_ToolStripMenuItem.Checked = true;
            this.BOOKFLAG_ToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.BOOKFLAG_ToolStripMenuItem.Name = "BOOKFLAG_ToolStripMenuItem";
            this.BOOKFLAG_ToolStripMenuItem.Size = new System.Drawing.Size(147, 22);
            this.BOOKFLAG_ToolStripMenuItem.Text = "bookを使用";
            this.BOOKFLAG_ToolStripMenuItem.Click += new System.EventHandler(this.bookを使用ToolStripMenuItem_Click);
            // 
            // changeBookToolStripMenuItem
            // 
            this.changeBookToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.変化なしToolStripMenuItem,
            this.変化小ToolStripMenuItem,
            this.変化中ToolStripMenuItem,
            this.変化大ToolStripMenuItem,
            this.ランダムToolStripMenuItem});
            this.changeBookToolStripMenuItem.Name = "changeBookToolStripMenuItem";
            this.changeBookToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.changeBookToolStripMenuItem.Text = "bookの変化度";
            // 
            // 変化なしToolStripMenuItem
            // 
            this.変化なしToolStripMenuItem.Name = "変化なしToolStripMenuItem";
            this.変化なしToolStripMenuItem.Size = new System.Drawing.Size(118, 22);
            this.変化なしToolStripMenuItem.Text = "変化なし";
            this.変化なしToolStripMenuItem.Click += new System.EventHandler(this.OnChangeBookToolStripMenuItem_Click);
            // 
            // 変化小ToolStripMenuItem
            // 
            this.変化小ToolStripMenuItem.Name = "変化小ToolStripMenuItem";
            this.変化小ToolStripMenuItem.Size = new System.Drawing.Size(118, 22);
            this.変化小ToolStripMenuItem.Text = "変化(小)";
            this.変化小ToolStripMenuItem.Click += new System.EventHandler(this.OnChangeBookToolStripMenuItem_Click);
            // 
            // 変化中ToolStripMenuItem
            // 
            this.変化中ToolStripMenuItem.Checked = true;
            this.変化中ToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.変化中ToolStripMenuItem.Name = "変化中ToolStripMenuItem";
            this.変化中ToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.変化中ToolStripMenuItem.Text = "変化(中)";
            this.変化中ToolStripMenuItem.Click += new System.EventHandler(this.OnChangeBookToolStripMenuItem_Click);
            // 
            // 変化大ToolStripMenuItem
            // 
            this.変化大ToolStripMenuItem.Name = "変化大ToolStripMenuItem";
            this.変化大ToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.変化大ToolStripMenuItem.Text = "変化(大)";
            this.変化大ToolStripMenuItem.Click += new System.EventHandler(this.OnChangeBookToolStripMenuItem_Click);
            // 
            // ランダムToolStripMenuItem
            // 
            this.ランダムToolStripMenuItem.Name = "ランダムToolStripMenuItem";
            this.ランダムToolStripMenuItem.Size = new System.Drawing.Size(118, 22);
            this.ランダムToolStripMenuItem.Text = "ランダム";
            this.ランダムToolStripMenuItem.Click += new System.EventHandler(this.OnChangeBookToolStripMenuItem_Click);
            // 
            // aI設定ToolStripMenuItem
            // 
            this.aI設定ToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.詳細結果の表示ToolStripMenuItem,
            this.bestlineの表示ToolStripMenuItem,
            this.思考過程を表示ToolStripMenuItem,
            this.Table_ToolStripMenuItem,
            this.MPC_ToolStripMenuItem});
            this.aI設定ToolStripMenuItem.Name = "aI設定ToolStripMenuItem";
            this.aI設定ToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.aI設定ToolStripMenuItem.Text = "AI設定";
            // 
            // 詳細結果の表示ToolStripMenuItem
            // 
            this.詳細結果の表示ToolStripMenuItem.Checked = true;
            this.詳細結果の表示ToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.詳細結果の表示ToolStripMenuItem.Name = "詳細結果の表示ToolStripMenuItem";
            this.詳細結果の表示ToolStripMenuItem.Size = new System.Drawing.Size(155, 22);
            this.詳細結果の表示ToolStripMenuItem.Text = "詳細結果を表示";
            // 
            // bestlineの表示ToolStripMenuItem
            // 
            this.bestlineの表示ToolStripMenuItem.Checked = true;
            this.bestlineの表示ToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.bestlineの表示ToolStripMenuItem.Name = "bestlineの表示ToolStripMenuItem";
            this.bestlineの表示ToolStripMenuItem.Size = new System.Drawing.Size(155, 22);
            this.bestlineの表示ToolStripMenuItem.Text = "PVを表示";
            // 
            // 思考過程を表示ToolStripMenuItem
            // 
            this.思考過程を表示ToolStripMenuItem.Checked = true;
            this.思考過程を表示ToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.思考過程を表示ToolStripMenuItem.Name = "思考過程を表示ToolStripMenuItem";
            this.思考過程を表示ToolStripMenuItem.Size = new System.Drawing.Size(155, 22);
            this.思考過程を表示ToolStripMenuItem.Text = "思考過程を表示";
            // 
            // Table_ToolStripMenuItem
            // 
            this.Table_ToolStripMenuItem.Checked = true;
            this.Table_ToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.Table_ToolStripMenuItem.Name = "Table_ToolStripMenuItem";
            this.Table_ToolStripMenuItem.Size = new System.Drawing.Size(155, 22);
            this.Table_ToolStripMenuItem.Text = "置換表を使用";
            this.Table_ToolStripMenuItem.Click += new System.EventHandler(this.置換表を使うToolStripMenuItem_Click);
            // 
            // MPC_ToolStripMenuItem
            // 
            this.MPC_ToolStripMenuItem.Checked = true;
            this.MPC_ToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.MPC_ToolStripMenuItem.Name = "MPC_ToolStripMenuItem";
            this.MPC_ToolStripMenuItem.Size = new System.Drawing.Size(155, 22);
            this.MPC_ToolStripMenuItem.Text = "MPCを使用";
            this.MPC_ToolStripMenuItem.Click += new System.EventHandler(this.mPC探索を行うToolStripMenuItem_Click);
            // 
            // CasheToolStripMenuItem
            // 
            this.CasheToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mBToolStripMenuItem,
            this.mBToolStripMenuItem1,
            this.mBToolStripMenuItem2,
            this.mBToolStripMenuItem3,
            this.mBToolStripMenuItem4,
            this.mBToolStripMenuItem5,
            this.mBToolStripMenuItem6});
            this.CasheToolStripMenuItem.Name = "CasheToolStripMenuItem";
            this.CasheToolStripMenuItem.Size = new System.Drawing.Size(147, 22);
            this.CasheToolStripMenuItem.Text = "キャッシュサイズ";
            // 
            // mBToolStripMenuItem
            // 
            this.mBToolStripMenuItem.Name = "mBToolStripMenuItem";
            this.mBToolStripMenuItem.Size = new System.Drawing.Size(110, 22);
            this.mBToolStripMenuItem.Text = "4MB";
            this.mBToolStripMenuItem.Click += new System.EventHandler(this.ConfigCasheToolStripMenuItem_Click);
            // 
            // mBToolStripMenuItem1
            // 
            this.mBToolStripMenuItem1.Name = "mBToolStripMenuItem1";
            this.mBToolStripMenuItem1.Size = new System.Drawing.Size(110, 22);
            this.mBToolStripMenuItem1.Text = "8MB";
            this.mBToolStripMenuItem1.Click += new System.EventHandler(this.ConfigCasheToolStripMenuItem_Click);
            // 
            // mBToolStripMenuItem2
            // 
            this.mBToolStripMenuItem2.Name = "mBToolStripMenuItem2";
            this.mBToolStripMenuItem2.Size = new System.Drawing.Size(110, 22);
            this.mBToolStripMenuItem2.Text = "16MB";
            this.mBToolStripMenuItem2.Click += new System.EventHandler(this.ConfigCasheToolStripMenuItem_Click);
            // 
            // mBToolStripMenuItem3
            // 
            this.mBToolStripMenuItem3.Name = "mBToolStripMenuItem3";
            this.mBToolStripMenuItem3.Size = new System.Drawing.Size(110, 22);
            this.mBToolStripMenuItem3.Text = "32MB";
            this.mBToolStripMenuItem3.Click += new System.EventHandler(this.ConfigCasheToolStripMenuItem_Click);
            // 
            // mBToolStripMenuItem4
            // 
            this.mBToolStripMenuItem4.Name = "mBToolStripMenuItem4";
            this.mBToolStripMenuItem4.Size = new System.Drawing.Size(110, 22);
            this.mBToolStripMenuItem4.Text = "64MB";
            this.mBToolStripMenuItem4.Click += new System.EventHandler(this.ConfigCasheToolStripMenuItem_Click);
            // 
            // mBToolStripMenuItem5
            // 
            this.mBToolStripMenuItem5.Checked = true;
            this.mBToolStripMenuItem5.CheckState = System.Windows.Forms.CheckState.Checked;
            this.mBToolStripMenuItem5.Name = "mBToolStripMenuItem5";
            this.mBToolStripMenuItem5.Size = new System.Drawing.Size(110, 22);
            this.mBToolStripMenuItem5.Text = "128MB";
            this.mBToolStripMenuItem5.Click += new System.EventHandler(this.ConfigCasheToolStripMenuItem_Click);
            // 
            // mBToolStripMenuItem6
            // 
            this.mBToolStripMenuItem6.Name = "mBToolStripMenuItem6";
            this.mBToolStripMenuItem6.Size = new System.Drawing.Size(110, 22);
            this.mBToolStripMenuItem6.Text = "256MB";
            this.mBToolStripMenuItem6.Click += new System.EventHandler(this.ConfigCasheToolStripMenuItem_Click);
            // 
            // ツールToolStripMenuItem
            // 
            this.ツールToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.hintToolStripMenuItem,
            this.fFOテストToolStripMenuItem});
            this.ツールToolStripMenuItem.Name = "ツールToolStripMenuItem";
            this.ツールToolStripMenuItem.Size = new System.Drawing.Size(46, 20);
            this.ツールToolStripMenuItem.Text = "ツール";
            // 
            // hintToolStripMenuItem
            // 
            this.hintToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.表示なしToolStripMenuItem,
            this.Hint1ToolStripMenuItem,
            this.Hint2ToolStripMenuItem1,
            this.Hint3ToolStripMenuItem1,
            this.Hint4ToolStripMenuItem3,
            this.Hint5ToolStripMenuItem4,
            this.Hint6ToolStripMenuItem,
            this.Hint7ToolStripMenuItem,
            this.Hint8ToolStripMenuItem});
            this.hintToolStripMenuItem.Name = "hintToolStripMenuItem";
            this.hintToolStripMenuItem.Size = new System.Drawing.Size(123, 22);
            this.hintToolStripMenuItem.Text = "ヒント表示";
            // 
            // 表示なしToolStripMenuItem
            // 
            this.表示なしToolStripMenuItem.Checked = true;
            this.表示なしToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.表示なしToolStripMenuItem.Name = "表示なしToolStripMenuItem";
            this.表示なしToolStripMenuItem.Size = new System.Drawing.Size(116, 22);
            this.表示なしToolStripMenuItem.Text = "表示なし";
            // 
            // Hint1ToolStripMenuItem
            // 
            this.Hint1ToolStripMenuItem.Name = "Hint1ToolStripMenuItem";
            this.Hint1ToolStripMenuItem.Size = new System.Drawing.Size(116, 22);
            this.Hint1ToolStripMenuItem.Text = "LEVEL1";
            this.Hint1ToolStripMenuItem.Click += new System.EventHandler(this.HintToolStripMenuItem_Click);
            // 
            // Hint2ToolStripMenuItem1
            // 
            this.Hint2ToolStripMenuItem1.Name = "Hint2ToolStripMenuItem1";
            this.Hint2ToolStripMenuItem1.Size = new System.Drawing.Size(116, 22);
            this.Hint2ToolStripMenuItem1.Text = "LEVEL2";
            this.Hint2ToolStripMenuItem1.Click += new System.EventHandler(this.HintToolStripMenuItem_Click);
            // 
            // Hint3ToolStripMenuItem1
            // 
            this.Hint3ToolStripMenuItem1.Name = "Hint3ToolStripMenuItem1";
            this.Hint3ToolStripMenuItem1.Size = new System.Drawing.Size(116, 22);
            this.Hint3ToolStripMenuItem1.Text = "LEVEL3";
            this.Hint3ToolStripMenuItem1.Click += new System.EventHandler(this.HintToolStripMenuItem_Click);
            // 
            // Hint4ToolStripMenuItem3
            // 
            this.Hint4ToolStripMenuItem3.Name = "Hint4ToolStripMenuItem3";
            this.Hint4ToolStripMenuItem3.Size = new System.Drawing.Size(116, 22);
            this.Hint4ToolStripMenuItem3.Text = "LEVEL4";
            this.Hint4ToolStripMenuItem3.Click += new System.EventHandler(this.HintToolStripMenuItem_Click);
            // 
            // Hint5ToolStripMenuItem4
            // 
            this.Hint5ToolStripMenuItem4.Name = "Hint5ToolStripMenuItem4";
            this.Hint5ToolStripMenuItem4.Size = new System.Drawing.Size(116, 22);
            this.Hint5ToolStripMenuItem4.Text = "LEVEL5";
            this.Hint5ToolStripMenuItem4.Click += new System.EventHandler(this.HintToolStripMenuItem_Click);
            // 
            // Hint6ToolStripMenuItem
            // 
            this.Hint6ToolStripMenuItem.Name = "Hint6ToolStripMenuItem";
            this.Hint6ToolStripMenuItem.Size = new System.Drawing.Size(116, 22);
            this.Hint6ToolStripMenuItem.Text = "LEVEL6";
            this.Hint6ToolStripMenuItem.Click += new System.EventHandler(this.HintToolStripMenuItem_Click);
            // 
            // Hint7ToolStripMenuItem
            // 
            this.Hint7ToolStripMenuItem.Name = "Hint7ToolStripMenuItem";
            this.Hint7ToolStripMenuItem.Size = new System.Drawing.Size(116, 22);
            this.Hint7ToolStripMenuItem.Text = "LEEVL7";
            this.Hint7ToolStripMenuItem.Click += new System.EventHandler(this.HintToolStripMenuItem_Click);
            // 
            // Hint8ToolStripMenuItem
            // 
            this.Hint8ToolStripMenuItem.Name = "Hint8ToolStripMenuItem";
            this.Hint8ToolStripMenuItem.Size = new System.Drawing.Size(116, 22);
            this.Hint8ToolStripMenuItem.Text = "LEVEL8";
            this.Hint8ToolStripMenuItem.Click += new System.EventHandler(this.HintToolStripMenuItem_Click);
            // 
            // fFOテストToolStripMenuItem
            // 
            this.fFOテストToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fFO40ToolStripMenuItem,
            this.fFO41ToolStripMenuItem,
            this.fFO42ToolStripMenuItem,
            this.fFO43ToolStripMenuItem,
            this.fFO44ToolStripMenuItem,
            this.fFO45ToolStripMenuItem,
            this.fFO46ToolStripMenuItem,
            this.fFO47ToolStripMenuItem,
            this.fFO48ToolStripMenuItem,
            this.fFO49ToolStripMenuItem1,
            this.fFO50ToolStripMenuItem2,
            this.fFO51ToolStripMenuItem,
            this.fFO52ToolStripMenuItem,
            this.fFO44ToolStripMenuItem5,
            this.fFO54ToolStripMenuItem,
            this.fFO55ToolStripMenuItem,
            this.fFO56ToolStripMenuItem,
            this.fFO57ToolStripMenuItem,
            this.fFO58ToolStripMenuItem,
            this.fFO59ToolStripMenuItem});
            this.fFOテストToolStripMenuItem.Name = "fFOテストToolStripMenuItem";
            this.fFOテストToolStripMenuItem.Size = new System.Drawing.Size(123, 22);
            this.fFOテストToolStripMenuItem.Text = "FFOテスト";
            // 
            // fFO40ToolStripMenuItem
            // 
            this.fFO40ToolStripMenuItem.Name = "fFO40ToolStripMenuItem";
            this.fFO40ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO40ToolStripMenuItem.Text = "#FFO40";
            this.fFO40ToolStripMenuItem.Click += new System.EventHandler(this.fFO40ToolStripMenuItem_Click);
            // 
            // fFO41ToolStripMenuItem
            // 
            this.fFO41ToolStripMenuItem.Name = "fFO41ToolStripMenuItem";
            this.fFO41ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO41ToolStripMenuItem.Text = "#FFO41";
            this.fFO41ToolStripMenuItem.Click += new System.EventHandler(this.fFO41ToolStripMenuItem_Click);
            // 
            // fFO42ToolStripMenuItem
            // 
            this.fFO42ToolStripMenuItem.Name = "fFO42ToolStripMenuItem";
            this.fFO42ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO42ToolStripMenuItem.Text = "#FFO42";
            this.fFO42ToolStripMenuItem.Click += new System.EventHandler(this.fFO42ToolStripMenuItem_Click);
            // 
            // fFO43ToolStripMenuItem
            // 
            this.fFO43ToolStripMenuItem.Name = "fFO43ToolStripMenuItem";
            this.fFO43ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO43ToolStripMenuItem.Text = "#FFO43";
            this.fFO43ToolStripMenuItem.Click += new System.EventHandler(this.fFO43ToolStripMenuItem_Click);
            // 
            // fFO44ToolStripMenuItem
            // 
            this.fFO44ToolStripMenuItem.Name = "fFO44ToolStripMenuItem";
            this.fFO44ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO44ToolStripMenuItem.Text = "#FFO44";
            this.fFO44ToolStripMenuItem.Click += new System.EventHandler(this.fFO44ToolStripMenuItem_Click);
            // 
            // fFO45ToolStripMenuItem
            // 
            this.fFO45ToolStripMenuItem.Name = "fFO45ToolStripMenuItem";
            this.fFO45ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO45ToolStripMenuItem.Text = "#FFO45";
            this.fFO45ToolStripMenuItem.Click += new System.EventHandler(this.fFO45ToolStripMenuItem_Click);
            // 
            // fFO46ToolStripMenuItem
            // 
            this.fFO46ToolStripMenuItem.Name = "fFO46ToolStripMenuItem";
            this.fFO46ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO46ToolStripMenuItem.Text = "#FFO46";
            this.fFO46ToolStripMenuItem.Click += new System.EventHandler(this.fFO46ToolStripMenuItem_Click);
            // 
            // fFO47ToolStripMenuItem
            // 
            this.fFO47ToolStripMenuItem.Name = "fFO47ToolStripMenuItem";
            this.fFO47ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO47ToolStripMenuItem.Text = "#FFO47";
            this.fFO47ToolStripMenuItem.Click += new System.EventHandler(this.fFO47ToolStripMenuItem_Click);
            // 
            // fFO48ToolStripMenuItem
            // 
            this.fFO48ToolStripMenuItem.Name = "fFO48ToolStripMenuItem";
            this.fFO48ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO48ToolStripMenuItem.Text = "#FFO48";
            this.fFO48ToolStripMenuItem.Click += new System.EventHandler(this.fFO48ToolStripMenuItem_Click);
            // 
            // fFO49ToolStripMenuItem1
            // 
            this.fFO49ToolStripMenuItem1.Name = "fFO49ToolStripMenuItem1";
            this.fFO49ToolStripMenuItem1.Size = new System.Drawing.Size(114, 22);
            this.fFO49ToolStripMenuItem1.Text = "#FFO49";
            this.fFO49ToolStripMenuItem1.Click += new System.EventHandler(this.fFO49ToolStripMenuItem_Click);
            // 
            // fFO50ToolStripMenuItem2
            // 
            this.fFO50ToolStripMenuItem2.Name = "fFO50ToolStripMenuItem2";
            this.fFO50ToolStripMenuItem2.Size = new System.Drawing.Size(114, 22);
            this.fFO50ToolStripMenuItem2.Text = "#FFO50";
            this.fFO50ToolStripMenuItem2.Click += new System.EventHandler(this.fFO50ToolStripMenuItem_Click);
            // 
            // fFO51ToolStripMenuItem
            // 
            this.fFO51ToolStripMenuItem.Name = "fFO51ToolStripMenuItem";
            this.fFO51ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO51ToolStripMenuItem.Text = "#FFO51";
            this.fFO51ToolStripMenuItem.Click += new System.EventHandler(this.fFO51ToolStripMenuItem_Click);
            // 
            // fFO52ToolStripMenuItem
            // 
            this.fFO52ToolStripMenuItem.Name = "fFO52ToolStripMenuItem";
            this.fFO52ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO52ToolStripMenuItem.Text = "#FFO52";
            this.fFO52ToolStripMenuItem.Click += new System.EventHandler(this.fFO52ToolStripMenuItem_Click);
            // 
            // fFO44ToolStripMenuItem5
            // 
            this.fFO44ToolStripMenuItem5.Name = "fFO44ToolStripMenuItem5";
            this.fFO44ToolStripMenuItem5.Size = new System.Drawing.Size(114, 22);
            this.fFO44ToolStripMenuItem5.Text = "#FFO53";
            this.fFO44ToolStripMenuItem5.Click += new System.EventHandler(this.fFO53ToolStripMenuItem_Click);
            // 
            // fFO54ToolStripMenuItem
            // 
            this.fFO54ToolStripMenuItem.Name = "fFO54ToolStripMenuItem";
            this.fFO54ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO54ToolStripMenuItem.Text = "#FFO54";
            this.fFO54ToolStripMenuItem.Click += new System.EventHandler(this.fFO54ToolStripMenuItem_Click);
            // 
            // fFO55ToolStripMenuItem
            // 
            this.fFO55ToolStripMenuItem.Name = "fFO55ToolStripMenuItem";
            this.fFO55ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO55ToolStripMenuItem.Text = "#FFO55";
            this.fFO55ToolStripMenuItem.Click += new System.EventHandler(this.fFO55ToolStripMenuItem_Click);
            // 
            // fFO56ToolStripMenuItem
            // 
            this.fFO56ToolStripMenuItem.Name = "fFO56ToolStripMenuItem";
            this.fFO56ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO56ToolStripMenuItem.Text = "#FFO56";
            this.fFO56ToolStripMenuItem.Click += new System.EventHandler(this.fFO56ToolStripMenuItem_Click);
            // 
            // fFO57ToolStripMenuItem
            // 
            this.fFO57ToolStripMenuItem.Name = "fFO57ToolStripMenuItem";
            this.fFO57ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO57ToolStripMenuItem.Text = "#FFO57";
            this.fFO57ToolStripMenuItem.Click += new System.EventHandler(this.fFO57ToolStripMenuItem_Click);
            // 
            // fFO58ToolStripMenuItem
            // 
            this.fFO58ToolStripMenuItem.Name = "fFO58ToolStripMenuItem";
            this.fFO58ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO58ToolStripMenuItem.Text = "#FFO58";
            this.fFO58ToolStripMenuItem.Click += new System.EventHandler(this.fFO58ToolStripMenuItem_Click);
            // 
            // fFO59ToolStripMenuItem
            // 
            this.fFO59ToolStripMenuItem.Name = "fFO59ToolStripMenuItem";
            this.fFO59ToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.fFO59ToolStripMenuItem.Text = "#FFO59";
            this.fFO59ToolStripMenuItem.Click += new System.EventHandler(this.fFO59ToolStripMenuItem_Click);
            // 
            // ヘルプToolStripMenuItem
            // 
            this.ヘルプToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.kZreversiについてToolStripMenuItem});
            this.ヘルプToolStripMenuItem.Name = "ヘルプToolStripMenuItem";
            this.ヘルプToolStripMenuItem.Size = new System.Drawing.Size(48, 20);
            this.ヘルプToolStripMenuItem.Text = "ヘルプ";
            // 
            // kZreversiについてToolStripMenuItem
            // 
            this.kZreversiについてToolStripMenuItem.Name = "kZreversiについてToolStripMenuItem";
            this.kZreversiについてToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.kZreversiについてToolStripMenuItem.Text = "KZreversiについて";
            // 
            // listBox2
            // 
            this.listBox2.Font = new System.Drawing.Font("MS UI Gothic", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.listBox2.FormattingEnabled = true;
            this.listBox2.Location = new System.Drawing.Point(628, 266);
            this.listBox2.Name = "listBox2";
            this.listBox2.Size = new System.Drawing.Size(157, 251);
            this.listBox2.TabIndex = 4;
            // 
            // comboBox1
            // 
            this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox1.Font = new System.Drawing.Font("メイリオ", 10F);
            this.comboBox1.Items.AddRange(new object[] {
            "Human",
            "Level 1   (2手読み)",
            "Level 2   (4手読み)",
            "Level 3   (6手読み)",
            "Level 4   (8手読み)",
            "Level 5   (10手読み)",
            "Level 6   (12手読み)",
            "Level 7   (14手読み)",
            "Level 8   (16手読み)",
            "Level 9   (18手読み)",
            "Level 10 (20手読み)",
            "Level 11 (22手読み)",
            "Level 12 (24手読み)",
            "WinLoss   (勝敗探索)",
            "Exact       (石差探索)"});
            this.comboBox1.Location = new System.Drawing.Point(590, 172);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(153, 28);
            this.comboBox1.TabIndex = 5;
            this.comboBox1.SelectedIndexChanged += new System.EventHandler(this.comboBox_SelectedIndexChanged);
            // 
            // comboBox2
            // 
            this.comboBox2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox2.Font = new System.Drawing.Font("メイリオ", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.comboBox2.Items.AddRange(new object[] {
            "Human",
            "Level 1   (2手読み)",
            "Level 2   (4手読み)",
            "Level 3   (6手読み)",
            "Level 4   (8手読み)",
            "Level 5   (10手読み)",
            "Level 6   (12手読み)",
            "Level 7   (14手読み)",
            "Level 8   (16手読み)",
            "Level 9   (18手読み)",
            "Level 10 (20手読み)",
            "Level 11 (22手読み)",
            "Level 12 (24手読み)",
            "WinLoss   (勝敗探索)",
            "Exact       (石差探索)"});
            this.comboBox2.Location = new System.Drawing.Point(590, 219);
            this.comboBox2.Name = "comboBox2";
            this.comboBox2.Size = new System.Drawing.Size(153, 28);
            this.comboBox2.TabIndex = 5;
            this.comboBox2.SelectedIndexChanged += new System.EventHandler(this.comboBox_SelectedIndexChanged);
            // 
            // listBox1
            // 
            this.listBox1.Font = new System.Drawing.Font("MS UI Gothic", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.listBox1.FormattingEnabled = true;
            this.listBox1.Location = new System.Drawing.Point(519, 266);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(103, 251);
            this.listBox1.TabIndex = 4;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("MS UI Gothic", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.label1.Location = new System.Drawing.Point(544, 178);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(40, 16);
            this.label1.TabIndex = 6;
            this.label1.Text = "黒番";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("MS UI Gothic", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.label2.Location = new System.Drawing.Point(544, 224);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(40, 16);
            this.label2.TabIndex = 6;
            this.label2.Text = "白番";
            // 
            // button4
            // 
            this.button4.Location = new System.Drawing.Point(628, 116);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(46, 23);
            this.button4.TabIndex = 7;
            this.button4.Text = ">";
            this.button4.UseVisualStyleBackColor = true;
            this.button4.Click += new System.EventHandler(this.buttonClick);
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(573, 116);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(46, 23);
            this.button3.TabIndex = 7;
            this.button3.Text = "<";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.buttonClick);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(520, 116);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(46, 23);
            this.button1.TabIndex = 7;
            this.button1.Text = "<<";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.buttonClick);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(680, 116);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(46, 23);
            this.button2.TabIndex = 7;
            this.button2.Text = ">>";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.buttonClick);
            // 
            // statusStrip2
            // 
            this.statusStrip2.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel4});
            this.statusStrip2.Location = new System.Drawing.Point(0, 530);
            this.statusStrip2.Name = "statusStrip2";
            this.statusStrip2.Size = new System.Drawing.Size(799, 22);
            this.statusStrip2.SizingGrip = false;
            this.statusStrip2.TabIndex = 8;
            this.statusStrip2.Text = "StatusStrip2";
            // 
            // toolStripStatusLabel4
            // 
            this.toolStripStatusLabel4.AutoSize = false;
            this.toolStripStatusLabel4.Name = "toolStripStatusLabel4";
            this.toolStripStatusLabel4.Size = new System.Drawing.Size(780, 17);
            // 
            // button5
            // 
            this.button5.Location = new System.Drawing.Point(519, 73);
            this.button5.Name = "button5";
            this.button5.Size = new System.Drawing.Size(100, 23);
            this.button5.TabIndex = 9;
            this.button5.Text = "ゲーム再開";
            this.button5.UseVisualStyleBackColor = true;
            this.button5.Click += new System.EventHandler(this.buttonClick);
            // 
            // button6
            // 
            this.button6.Location = new System.Drawing.Point(628, 73);
            this.button6.Name = "button6";
            this.button6.Size = new System.Drawing.Size(98, 23);
            this.button6.TabIndex = 9;
            this.button6.Text = "中断";
            this.button6.UseVisualStyleBackColor = true;
            this.button6.Click += new System.EventHandler(this.buttonClick);
            // 
            // imageList1
            // 
            this.imageList1.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList1.ImageStream")));
            this.imageList1.TransparentColor = System.Drawing.Color.Transparent;
            this.imageList1.Images.SetKeyName(0, "othello_bk.png");
            this.imageList1.Images.SetKeyName(1, "othello_wh.png");
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("MS UI Gothic", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.label3.Location = new System.Drawing.Point(517, 178);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(31, 13);
            this.label3.TabIndex = 11;
            this.label3.Text = "-->";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("MS UI Gothic", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.label4.Location = new System.Drawing.Point(517, 227);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(31, 13);
            this.label4.TabIndex = 11;
            this.label4.Text = "-->";
            // 
            // button7
            // 
            this.button7.Location = new System.Drawing.Point(519, 37);
            this.button7.Name = "button7";
            this.button7.Size = new System.Drawing.Size(207, 23);
            this.button7.TabIndex = 12;
            this.button7.Text = "新規ゲーム開始";
            this.button7.UseVisualStyleBackColor = true;
            this.button7.Click += new System.EventHandler(this.buttonClick);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(799, 574);
            this.Controls.Add(this.button7);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.button6);
            this.Controls.Add(this.button5);
            this.Controls.Add(this.statusStrip2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button4);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.comboBox2);
            this.Controls.Add(this.comboBox1);
            this.Controls.Add(this.listBox1);
            this.Controls.Add(this.listBox2);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.menuStrip1);
            this.DoubleBuffered = true;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "KZReversi";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Form1_FormClosed);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.statusStrip2.ResumeLayout(false);
            this.statusStrip2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem ファイルToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 終了ToolStripMenuItem;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel2;
        private System.Windows.Forms.ListBox listBox2;
        private System.Windows.Forms.ToolStripMenuItem 機能ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ツールToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ヘルプToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 編集ToolStripMenuItem;
        private System.Windows.Forms.ComboBox comboBox1;
        private System.Windows.Forms.ComboBox comboBox2;
        private System.Windows.Forms.ListBox listBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ToolStripMenuItem ゲーム開始ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 新規ゲームToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 黒から始めるToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 白から始めるToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 中断ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 手戻すToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 手進むToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 手番変更ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 盤面操作ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 盤面初期化ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 盤面編集ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 盤面の回転と対照変換ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem BOOKFLAG_ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem changeBookToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 変化なしToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 変化小ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 変化中ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 変化大ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ランダムToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aI設定ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 詳細結果の表示ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem bestlineの表示ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 思考過程を表示ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem hintToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 表示なしToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem Hint1ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem Hint2ToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem Hint3ToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem Hint4ToolStripMenuItem3;
        private System.Windows.Forms.ToolStripMenuItem Hint5ToolStripMenuItem4;
        private System.Windows.Forms.ToolStripMenuItem Hint6ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem Table_ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem MPC_ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem kZreversiについてToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 度回転ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 度回転ToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem 度回転ToolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem 度回転ToolStripMenuItem3;
        private System.Windows.Forms.ToolStripMenuItem ｙ軸対称ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ブラックライン対称ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ホワイトライン対照変換ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem Hint7ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem Hint8ToolStripMenuItem;
        private System.Windows.Forms.Button button4;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel3;
        private System.Windows.Forms.StatusStrip statusStrip2;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel4;
        private System.Windows.Forms.ToolStripMenuItem CasheToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem mBToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem mBToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem mBToolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem mBToolStripMenuItem3;
        private System.Windows.Forms.ToolStripMenuItem mBToolStripMenuItem4;
        private System.Windows.Forms.ToolStripMenuItem mBToolStripMenuItem5;
        private System.Windows.Forms.ToolStripMenuItem mBToolStripMenuItem6;
        private System.Windows.Forms.Button button5;
        private System.Windows.Forms.Button button6;
        private System.Windows.Forms.ImageList imageList1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ToolStripMenuItem fFOテストToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO40ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO41ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO42ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO43ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO44ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO45ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO46ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO47ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO48ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO49ToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem fFO50ToolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem fFO51ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO52ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO44ToolStripMenuItem5;
        private System.Windows.Forms.ToolStripMenuItem fFO54ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO55ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO56ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO57ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO58ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fFO59ToolStripMenuItem;
        private System.Windows.Forms.Button button7;
        private System.Windows.Forms.ToolStripMenuItem 置換表のメモリを解放ToolStripMenuItem;
    }
}

