using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace KZreversi
{
    class Player
    {
        public const int PLAYER_HUMAN = 0;
        public const int PLAYER_CPU = 1;

        public int color;
        public int playerInfo;
        public ulong moves;

        public Player(int pInfo, int color)
        {
            this.playerInfo = pInfo;
            this.color = color;
        }
    
    }
}
