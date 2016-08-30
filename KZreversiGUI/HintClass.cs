using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace KZreversi
{
    public class HintClass
    {
        private int eval;
        private int pos;

        public HintClass() 
        {
            eval = 0;
            pos = -1;
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
