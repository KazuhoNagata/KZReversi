/***************************************************************************
* Name  : board.cpp
* Brief : î’ñ ä÷òAÇÃèàóùÇçsÇ§
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#define BLACK 0
#define WHITE 1

extern UCHAR g_board[64];

void InitIndexBoard(UINT64 bk, UINT64 wh);

UINT64 rotate_90(UINT64 board);
UINT64 rotate_180(UINT64 board);
UINT64 rotate_270(UINT64 board);
UINT64 symmetry_x(UINT64 board);
UINT64 symmetry_y(UINT64 board);
UINT64 symmetry_b(UINT64 board);
UINT64 symmetry_w(UINT64 board);