/***************************************************************************
* Name  : board.cpp
* Brief : 盤面関連の処理を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"
#include "board.h"
#include "eval.h"

UCHAR g_board[64];

/***************************************************************************
* Name  : Swap
* Brief : 盤面情報（ビット列）の白黒反転
* Args  : *bk        : 黒のビット列
*         *wh        : 白のビット列
****************************************************************************/
void Swap(UINT64 *bk, UINT64 *wh)
{
	UINT64 temp = *bk;
	*bk = *wh;
	*wh = temp;
}

/***************************************************************************
* Name  : InitIndexBoard
* Brief : 盤面情報（ビット列）から配列に変換
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
****************************************************************************/
void InitIndexBoard(UINT64 bk, UINT64 wh)
{
	g_board[0] = (int)(bk & a1) + (int)((wh & a1) * 2);
	g_board[1] = (int)((bk & a2) >> 1) + (int)((wh & a2));
	g_board[2] = (int)((bk & a3) >> 2) + (int)((wh & a3) >> 1);
	g_board[3] = (int)((bk & a4) >> 3) + (int)((wh & a4) >> 2);
	g_board[4] = (int)((bk & a5) >> 4) + (int)((wh & a5) >> 3);
	g_board[5] = (int)((bk & a6) >> 5) + (int)((wh & a6) >> 4);
	g_board[6] = (int)((bk & a7) >> 6) + (int)((wh & a7) >> 5);
	g_board[7] = (int)((bk & a8) >> 7) + (int)((wh & a8) >> 6);
	g_board[8] = (int)((bk & b1) >> 8) + (int)((wh & b1) >> 7);
	g_board[9] = (int)((bk & b2) >> 9) + (int)((wh & b2) >> 8);
	g_board[10] = (int)((bk & b3) >> 10) + (int)((wh & b3) >> 9);
	g_board[11] = (int)((bk & b4) >> 11) + (int)((wh & b4) >> 10);
	g_board[12] = (int)((bk & b5) >> 12) + (int)((wh & b5) >> 11);
	g_board[13] = (int)((bk & b6) >> 13) + (int)((wh & b6) >> 12);
	g_board[14] = (int)((bk & b7) >> 14) + (int)((wh & b7) >> 13);
	g_board[15] = (int)((bk & b8) >> 15) + (int)((wh & b8) >> 14);
	g_board[16] = (int)((bk & c1) >> 16) + (int)((wh & c1) >> 15);
	g_board[17] = (int)((bk & c2) >> 17) + (int)((wh & c2) >> 16);
	g_board[18] = (int)((bk & c3) >> 18) + (int)((wh & c3) >> 17);
	g_board[19] = (int)((bk & c4) >> 19) + (int)((wh & c4) >> 18);
	g_board[20] = (int)((bk & c5) >> 20) + (int)((wh & c5) >> 19);
	g_board[21] = (int)((bk & c6) >> 21) + (int)((wh & c6) >> 20);
	g_board[22] = (int)((bk & c7) >> 22) + (int)((wh & c7) >> 21);
	g_board[23] = (int)((bk & c8) >> 23) + (int)((wh & c8) >> 22);
	g_board[24] = (int)((bk & d1) >> 24) + (int)((wh & d1) >> 23);
	g_board[25] = (int)((bk & d2) >> 25) + (int)((wh & d2) >> 24);
	g_board[26] = (int)((bk & d3) >> 26) + (int)((wh & d3) >> 25);
	g_board[27] = (int)((bk & d4) >> 27) + (int)((wh & d4) >> 26);
	g_board[28] = (int)((bk & d5) >> 28) + (int)((wh & d5) >> 27);
	g_board[29] = (int)((bk & d6) >> 29) + (int)((wh & d6) >> 28);
	g_board[30] = (int)((bk & d7) >> 30) + (int)((wh & d7) >> 29);
	g_board[31] = (int)((bk & d8) >> 31) + (int)((wh & d8) >> 30);
	g_board[32] = (int)((bk & e1) >> 32) + (int)((wh & e1) >> 31);
	g_board[33] = (int)((bk & e2) >> 33) + (int)((wh & e2) >> 32);
	g_board[34] = (int)((bk & e3) >> 34) + (int)((wh & e3) >> 33);
	g_board[35] = (int)((bk & e4) >> 35) + (int)((wh & e4) >> 34);
	g_board[36] = (int)((bk & e5) >> 36) + (int)((wh & e5) >> 35);
	g_board[37] = (int)((bk & e6) >> 37) + (int)((wh & e6) >> 36);
	g_board[38] = (int)((bk & e7) >> 38) + (int)((wh & e7) >> 37);
	g_board[39] = (int)((bk & e8) >> 39) + (int)((wh & e8) >> 38);
	g_board[40] = (int)((bk & f1) >> 40) + (int)((wh & f1) >> 39);
	g_board[41] = (int)((bk & f2) >> 41) + (int)((wh & f2) >> 40);
	g_board[42] = (int)((bk & f3) >> 42) + (int)((wh & f3) >> 41);
	g_board[43] = (int)((bk & f4) >> 43) + (int)((wh & f4) >> 42);
	g_board[44] = (int)((bk & f5) >> 44) + (int)((wh & f5) >> 43);
	g_board[45] = (int)((bk & f6) >> 45) + (int)((wh & f6) >> 44);
	g_board[46] = (int)((bk & f7) >> 46) + (int)((wh & f7) >> 45);
	g_board[47] = (int)((bk & f8) >> 47) + (int)((wh & f8) >> 46);
	g_board[48] = (int)((bk & g1) >> 48) + (int)((wh & g1) >> 47);
	g_board[49] = (int)((bk & g2) >> 49) + (int)((wh & g2) >> 48);
	g_board[50] = (int)((bk & g3) >> 50) + (int)((wh & g3) >> 49);
	g_board[51] = (int)((bk & g4) >> 51) + (int)((wh & g4) >> 50);
	g_board[52] = (int)((bk & g5) >> 52) + (int)((wh & g5) >> 51);
	g_board[53] = (int)((bk & g6) >> 53) + (int)((wh & g6) >> 52);
	g_board[54] = (int)((bk & g7) >> 54) + (int)((wh & g7) >> 53);
	g_board[55] = (int)((bk & g8) >> 55) + (int)((wh & g8) >> 54);
	g_board[56] = (int)((bk & h1) >> 56) + (int)((wh & h1) >> 55);
	g_board[57] = (int)((bk & h2) >> 57) + (int)((wh & h2) >> 56);
	g_board[58] = (int)((bk & h3) >> 58) + (int)((wh & h3) >> 57);
	g_board[59] = (int)((bk & h4) >> 59) + (int)((wh & h4) >> 58);
	g_board[60] = (int)((bk & h5) >> 60) + (int)((wh & h5) >> 59);
	g_board[61] = (int)((bk & h6) >> 61) + (int)((wh & h6) >> 60);
	g_board[62] = (int)((bk & h7) >> 62) + (int)((wh & h7) >> 61);
	g_board[63] = (int)((bk & h8) >> 63) + (int)((wh & h8) >> 62);
}


/*
############################################################################
#
#  局面の対称変換関数
#  (ひとつの局面には、回転・対称の関係にある7つの同一局面が含まれている)
#
############################################################################
*/
UINT64 rotate_90(UINT64 board)
{
	/* 反時計回り90度の回転 */
	board = board << 1 & 0xaa00aa00aa00aa00 |
		board >> 1 & 0x0055005500550055 |
		board >> 8 & 0x00aa00aa00aa00aa |
		board << 8 & 0x5500550055005500;

	board = board << 2 & 0xcccc0000cccc0000 |
		board >> 2 & 0x0000333300003333 |
		board >> 16 & 0x0000cccc0000cccc |
		board << 16 & 0x3333000033330000;

	board = board << 4 & 0xf0f0f0f000000000 |
		board >> 4 & 0x000000000f0f0f0f |
		board >> 32 & 0x00000000f0f0f0f0 |
		board << 32 & 0x0f0f0f0f00000000;

	return board;
}

UINT64 rotate_180(UINT64 board)
{
	/* 反時計回り180度の回転 */
	board = board << 9 & 0xaa00aa00aa00aa00 |
		board >> 9 & 0x0055005500550055 |
		board >> 7 & 0x00aa00aa00aa00aa |
		board << 7 & 0x5500550055005500;

	board = board << 18 & 0xcccc0000cccc0000 |
		board >> 18 & 0x0000333300003333 |
		board >> 14 & 0x0000cccc0000cccc |
		board << 14 & 0x3333000033330000;

	board = board << 36 & 0xf0f0f0f000000000 |
		board >> 36 & 0x000000000f0f0f0f |
		board >> 28 & 0x00000000f0f0f0f0 |
		board << 28 & 0x0f0f0f0f00000000;

	return board;
}

UINT64 rotate_270(UINT64 board)
{
	/* 反時計回り270度の回転=時計回り90度の回転 */
	board = board << 8 & 0xaa00aa00aa00aa00 |
		board >> 8 & 0x0055005500550055 |
		board << 1 & 0x00aa00aa00aa00aa |
		board >> 1 & 0x5500550055005500;

	board = board << 16 & 0xcccc0000cccc0000 |
		board >> 16 & 0x0000333300003333 |
		board << 2 & 0x0000cccc0000cccc |
		board >> 2 & 0x3333000033330000;

	board = board << 32 & 0xf0f0f0f000000000 |
		board >> 32 & 0x000000000f0f0f0f |
		board << 4 & 0x00000000f0f0f0f0 |
		board >> 4 & 0x0f0f0f0f00000000;

	return board;
}

UINT64 symmetry_x(UINT64 board)
{
	/* X軸に対し対称変換 */
	board = board << 1 & 0xaaaaaaaaaaaaaaaa |
		board >> 1 & 0x5555555555555555;

	board = board << 2 & 0xcccccccccccccccc |
		board >> 2 & 0x3333333333333333;

	board = board << 4 & 0xf0f0f0f0f0f0f0f0 |
		board >> 4 & 0x0f0f0f0f0f0f0f0f;

	return board;
}

UINT64 symmetry_y(UINT64 board)
{
	/* Y軸に対し対称変換 */
	board = board << 8 & 0xff00ff00ff00ff00 |
		board >> 8 & 0x00ff00ff00ff00ff;

	board = board << 16 & 0xffff0000ffff0000 |
		board >> 16 & 0x0000ffff0000ffff;

	board = board << 32 & 0xffffffff00000000 |
		board >> 32 & 0x00000000ffffffff;

	return board;
}

UINT64 symmetry_b(UINT64 board)
{
	/* ブラックラインに対し対象変換 */
	board = board >> 9 & 0x0055005500550055 |
		board << 9 & 0xaa00aa00aa00aa00 |
		board & 0x55aa55aa55aa55aa;

	board = board >> 18 & 0x0000333300003333 |
		board << 18 & 0xcccc0000cccc0000 |
		board & 0x3333cccc3333cccc;

	board = board >> 36 & 0x000000000f0f0f0f |
		board << 36 & 0xf0f0f0f000000000 |
		board & 0x0f0f0f0ff0f0f0f0;

	return board;
}

UINT64 symmetry_w(UINT64 board)
{
	/* ホワイトラインに対し対象変換 */
	board = board >> 7 & 0x00aa00aa00aa00aa |
		board << 7 & 0x5500550055005500 |
		board & 0xaa55aa55aa55aa55;

	board = board >> 14 & 0x0000cccc0000cccc |
		board << 14 & 0x3333000033330000 |
		board & 0xcccc3333cccc3333;

	board = board >> 28 & 0x00000000f0f0f0f0 |
		board << 28 & 0x0f0f0f0f00000000 |
		board & 0xf0f0f0f00f0f0f0f;

	return board;
}