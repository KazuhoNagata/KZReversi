using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace KZreversi
{
    class BufferedPanel : Panel
    {
        private bool m_allowRedraw;

        public BufferedPanel(bool allowRedraw)
        {
            m_allowRedraw = allowRedraw;
            this.DoubleBuffered = true;
            SetStyle(
                ControlStyles.DoubleBuffer |
                ControlStyles.UserPaint |
                ControlStyles.OptimizedDoubleBuffer |
                ControlStyles.AllPaintingInWmPaint |
                ControlStyles.SupportsTransparentBackColor, true);

        }
    }
}
