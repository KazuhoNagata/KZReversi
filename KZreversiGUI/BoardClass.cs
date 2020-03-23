using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace KZreversi
{
    [StructLayout(LayoutKind.Sequential)]
    public class BoardClass
    {
        // マーシャル対象
        private uint color;
        private ulong black;
        private ulong white;
        // ここまでマーシャル対象


        public static uint BLACK = 0;
        public static uint WHITE = 1;
        public static uint EMPTY = 2;

        private List<int[]> moveList;
        private List<ulong[]> boardList;

        private uint nowColor;
        private int nowTurn;

        public BoardClass()
        {
            moveList = new List<int[]>();
            boardList = new List<ulong[]>();
        }

        public void SetBoard(ulong bw, ulong wh)
        {
            boardList.Add(new ulong[] { bw, wh });
        }

        public uint GetNowColor()
        {
            return nowColor;
        }

        public int GetNowTurn()
        {
            return nowTurn;
        }

        public uint GetColor()
        {
            return (uint)moveList[nowTurn][0];
        }
        public uint GetMove()
        {
            return (uint)moveList[nowTurn][1];
        }

        public ulong GetBlack()
        {
            return boardList[nowTurn][0];
        }

        public ulong GetWhite()
        {
            return boardList[nowTurn][1];
        }


        /**
         * 盤面の着手処理を行います
         */
        public bool move(int pos)
        {
            UInt64 rev;
            CppWrapper cppWrapper = CppWrapper.getInstance();

            black = GetBlack();
            white = GetWhite();

            if (nowColor == BoardClass.BLACK)
            {
                rev = cppWrapper.GetBoardChangeInfo(black, white, pos);

                if (rev == 0)
                {
                    return false;
                }

                black ^= ((1UL << pos) | rev);
                white ^= rev;
            }
            else
            {
                rev = cppWrapper.GetBoardChangeInfo(white, black, pos);

                if (rev == 0)
                {
                    return false;
                }

                white ^= ((1UL << pos) | rev);
                black ^= rev;
            }

            // 着手リスト更新
            boardList.Add(new ulong[] { black, white });
            moveList.Add(new int[] { (int)nowColor, pos });

            nowTurn++;
            nowColor ^= 1;

            return true;

        }

        public int GetRecentMove()
        {

            int[] recent = moveList[nowTurn];
            return recent[1];
        }

        public void InitBoard(uint color)
        {
            nowTurn = 0;
            nowColor = color;
            boardList.Clear();
            this.SetBoard(0x810000000, 0x1008000000);
            this.moveList.Clear();
            this.moveList.Add(new int[] { (int)nowColor ^ 1, -1 });
        }

        public void InitBoard(uint color, ulong bk, ulong wh)
        {
            nowTurn = 0;
            nowColor = color;
            boardList.Clear();
            this.SetBoard(bk, wh);
            this.moveList.Clear();
            this.moveList.Add(new int[] { (int)nowColor ^ 1, -1 });
        }

        public bool SetHistory(int index)
        {
            if (index < 0 || index > moveList.Count - 1)
            {
                return false;
            }

            nowColor = (uint)moveList[index][0] ^ 1;
            nowTurn = index;

            return true;
        }

        public int GetRecentTurn()
        {
            return moveList.Count - 1;
        }

        public void DeleteHistory(int turn)
        {
            moveList.RemoveRange(turn + 1, moveList.Count - turn - 1);
            boardList.RemoveRange(turn + 1, boardList.Count - turn - 1);
        }

        // プレイヤーパス時に呼ぶ
        public uint ChangeColor()
        {
            nowColor ^= 1;

            return nowColor;
        }

        public void EditBoard(ulong bk, ulong wh)
        {
            boardList[nowTurn][0] = bk;
            boardList[nowTurn][1] = wh;
        }
    }
}
