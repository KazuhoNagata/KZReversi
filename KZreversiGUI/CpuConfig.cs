using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace KZreversi
{
    [StructLayout(LayoutKind.Sequential)]
    public class CpuConfig
    {
        public uint color;				// CPUの色
        public uint casheSize;			// 置換表のサイズ
        public uint searchDepth;		// 中盤読みの深さ
        public uint winLossDepth;		// 勝敗探索を開始する深さ
        public uint exactDepth;		    // 石差探索を開始する深さ
        public bool bookFlag;			// 定石を使用するかどうか
        public uint bookVariability;	// 定石の変化度
        public bool mpcFlag;			// MPCを使用するかどうか
        public bool tableFlag;			// 置換表を使用するかどうか

        // マーシャリングここまで
    }
}
