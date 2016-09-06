using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace KZreversi
{
    public class HintClass
    {
        public const int SOLVE_MIDDLE = 0;
        public const int SOLVE_WLD = 1;
        public const int SOLVE_EXCAT = 2;

        private int attr;
        private int eval;
        private int pos;

        public HintClass() 
        {
            attr = SOLVE_MIDDLE;
            eval = 0;
            pos = -1;
        }

        public int GetAttr()
        {
            return attr;
        }
        public void SetAttr(int attr)
        {
            this.attr = attr;
        }
        public int GetEval() 
        {
            return eval;
        }
        public void SetEval(int eval) 
        {
            this.eval = eval;
        }
        public int GetPos() 
        {
            return pos;
        }
        public void SetPos(int pos) 
        {
            this.pos = pos;
        }
    }
}
