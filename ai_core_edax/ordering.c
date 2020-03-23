/***************************************************************************
* Name  : ordering.cpp
* Brief : 手の並び替え関連の処理を行う
* Date  : 2016/02/02
****************************************************************************/

#include "stdafx.h"
#include "rev.h"
#include "bit64.h"
#include "board.h"
#include "cpu.h"
#include "eval.h"
#include "hash.h"
#include "move.h"
#include "ordering.h"
#include "endgame.h"

const int SQUARE_VALUE[] = {
	// JCW's score:
	18,  4,  16, 12, 12, 16,  4, 18,
	 4,  2,   6,  8,  8,  6,  2,  4,
	16,  6,  14, 10, 10, 14,  6, 16,
	12,  8,  10,  0,  0, 10,  8, 12,
	12,  8,  10,  0,  0, 10,  8, 12,
	16,  6,  14, 10, 10, 14,  6, 16,
	 4,  2,   6,  8,  8,  6,  2,  4,
	18,  4,  16, 12, 12, 16,  4, 18
};

/* コムソート */
void SortListUseTable(MOVELIST *pos_list, INT32 move_list[], INT32 cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int temp, swaps;
	MOVELIST move_temp;

	if (cnt <= 1){ return; }
	while (1)
	{
		swaps = 0;
		for (i = 0; i + h < cnt; i++)
		{
			if (move_list[i] > move_list[i + h])
			{
				temp = move_list[i];
				move_list[i] = move_list[i + h];
				move_list[i + h] = temp;
				move_temp = pos_list[i];
				pos_list[i] = pos_list[i + h];
				pos_list[i + h] = move_temp;
				swaps++;
			}
		}
		if (h == 1)
		{
			if (swaps == 0)
			{
				break;
			}
		}
		else
		{
			h = h * 10 / 13;
		}
	}
}



int get_corner_stability(UINT64 color){

	int n_stable = 0;

	if (color & a1){ n_stable += CountBit(color & (0x0000000000000103)); }	// a1, a2, b1
	if (color & a8){ n_stable += CountBit(color & (0x00000000000080c0)); }	// a7, a8, b8
	if (color & h1){ n_stable += CountBit(color & (0x0301000000000000)); }	// g1, h1, h2
	if (color & h8){ n_stable += CountBit(color & (0xc080000000000000)); }	// g8, h7, h8

	return n_stable;
}

/* 静的順序づけ(ビット列からの取得) */
UINT32 GetOrderPosition(UINT64 blank)
{
	/* 8 point*/
	UINT64 moves;

	if ((moves = blank & 0x8100000000000081) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 7 point */
	if ((moves = blank & 0x240000240000) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 6 point*/
	if ((moves = blank & 0x1800008181000018) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 5 point*/
	if ((moves = blank & 0x2400810000810024) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 4 point*/
	if ((moves = blank & 0x182424180000) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 3 point */
	if ((moves = blank & 0x18004242001800) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 2 point*/
	if ((moves = blank & 0x24420000422400) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}
	/* 1 point*/
	if ((moves = blank & 0x4281000000008142) != 0)
	{
		return CountBit((~moves) & (moves - 1));
	}

	/* 0 point*/
	return CountBit((~blank) & (blank - 1));

}
