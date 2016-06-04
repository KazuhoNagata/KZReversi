using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace KZreversi
{
    class BufferedPanel : Panel
    {

        public BufferedPanel()
        {
            this.DoubleBuffered = true;

        }

    }
}
