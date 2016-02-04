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
        uint color;				// CPUの色
        uint casheSize;			// 置換表のサイズ
        uint searchDepth;		// 中盤読みの深さ
        uint winLossDepth;		// 勝敗探索を開始する深さ
        uint exactDepth;		// 石差探索を開始する深さ
        bool bookFlag;			// 定石を使用するかどうか
        bool bookVariability;	// 定石の変化度
        bool mpcFlag;			// MPCを使用するかどうか
        bool tableFlag;			// 置換表を使用するかどうか

    }
}
