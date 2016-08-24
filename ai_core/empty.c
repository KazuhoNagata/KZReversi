#include "stdafx.h"
#include "bit64.h"
#include "board.h"
#include "move.h"
#include "rev.h"
#include "cpu.h"
#include "endgame.h"
#include "hash.h"
#include "eval.h"
#include "ordering.h"

#include "count_last_flip_carry_64.h"


INT32 SearchEmpty_1(UINT64 bk, UINT64 wh, INT32 pos, INT32 beta, PVLINE *pline);
INT32 SearchEmptyWLD_1(UINT64 bk, UINT64 wh, INT32 pos, INT32 beta, PVLINE *pline);

/* function empty 1 */
INT32(*GetEmpty1Score[])(UINT64, UINT64, INT32, INT32, PVLINE*) = {
	SearchEmptyWLD_1,
	SearchEmpty_1
};

/**
* @brief Get the final score.
*
* Get the final score, when 1 empty squares remain.
* The following code has been adapted from Zebra by Gunnar Anderson.
*
* @param board  Board to evaluate.
* @param beta   Beta bound.
* @param x      Last empty square to play.
* @return       The final opponent score, as a disc difference.
*/
INT32 SearchEmpty_1(UINT64 bk, UINT64 wh, INT32 pos, INT32 beta, PVLINE *pline)
{

	INT32 score = 2 * CountBit(bk);
	INT32 n_flips = count_last_flip[pos](bk);

	if (n_flips)
	{
		score += n_flips + 2;
		pline->argmove[0] = pos;
		pline->cmove = 1;
	}
	else
	{
		// opponent turn...
		n_flips = count_last_flip[pos](wh);
		if (n_flips)
		{
			score -= n_flips;
			pline->argmove[0] = pos;
			pline->cmove = 1;
		}
		else
		{
			// empty = 1 end...
			// win>=(32, 31) loss<=(31,32) --> score >= 32 * 2 
			if (score >= 64) score += 2;
			pline->cmove = 0;
		}
	}

	return 	score - 64;
}



/**
* @brief Get the final score for win/loss/draw.
*
* Get the final score, when 1 empty squares remain.
* The following code has been adapted from Zebra by Gunnar Anderson.
*
* @param board  Board to evaluate.
* @param beta   Beta bound.
* @param x      Last empty square to play.
* @return       The final opponent score, as a disc difference.
*/
INT32 SearchEmptyWLD_1(UINT64 bk, UINT64 wh, INT32 pos, INT32 beta, PVLINE *pline)
{
	INT32 score = 2 * CountBit(bk);
	INT32 n_flips = count_last_flip[pos](bk);

	if (n_flips)
	{
		score += n_flips + 2;
		pline->argmove[0] = pos;
		pline->cmove = 1;
	}
	else
	{
		// opponent turn...
		n_flips = count_last_flip[pos](wh);
		if (n_flips)
		{
			score -= n_flips;
			pline->argmove[0] = pos;
			pline->cmove = 1;
		}
		else
		{
			pline->cmove = 0;
		}
	}

	if (score - 64 > 0)
	{
		return WIN;
	}
	else if (score - 64 < 0)
	{
		return LOSS;
	}
	else
	{
		return DRAW;
	}
}



INT32 SearchEmpty_2(UINT64 bk, UINT64 wh, INT32 x1, INT32 x2,
	INT32 alpha, INT32 beta, INT32 passed, PVLINE *pline)
{
	g_countNode++;

	INT32 eval, best;
	UINT64 pos_bit;
	UINT64 rev;
	PVLINE line;

	pos_bit = 1ULL << x1;
	rev = GetRev[x1](bk, wh);
	if (rev)
	{
		best = -GetEmpty1Score[g_solveMethod](wh ^ rev, bk ^ (pos_bit | rev), x2, -alpha, &line);
		if (best >= beta) return best;
		if (best > alpha)
		{
			alpha = best;
			pline->argmove[0] = x1;
			memcpy(pline->argmove + 1, line.argmove, line.cmove);
			pline->cmove = line.cmove + 1;
		}
	}
	else best = -g_infscore;

	pos_bit = 1ULL << x2;
	rev = GetRev[x2](bk, wh);
	if (rev)
	{
		eval = -GetEmpty1Score[g_solveMethod](wh ^ rev, bk ^ (pos_bit | rev), x1, -alpha, &line);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			if (best > alpha)
			{
				alpha = best;
				pline->argmove[0] = x2;
				memcpy(pline->argmove + 1, line.argmove, line.cmove);
				pline->cmove = line.cmove + 1;
			}
		}
	}

	// no move
	if (best == -g_infscore)
	{
		if (passed)
		{
			best = GetEndScore[g_solveMethod](bk, wh, 2);
			pline->cmove = 0;
		}
		else
		{
			best = -SearchEmpty_2(wh, bk, x1, x2, -beta, -alpha, 1, pline);
		}
	}

	return best;
}




INT32 SearchEmpty_3(UINT64 bk, UINT64 wh, UINT64 blank, UINT32 parity,
	INT32 alpha, INT32 beta, INT32 passed, PVLINE *pline)
{
	g_countNode++;

	// 3 empties parity
	UINT64 temp_moves = blank;
	int x1 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x1);
	int x2 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x2);
	int x3 = CountBit((~temp_moves) & (temp_moves - 1));

	if (!(parity & board_parity_bit[x1])) {
		if (parity & board_parity_bit[x2]) { // case 1(x2) 2(x1 x3)
			int tmp = x1; x1 = x2; x2 = tmp;
		}
		else { // case 1(x3) 2(x1 x2)
			int tmp = x1; x1 = x3; x3 = x2; x2 = tmp;
		}
	}

	UINT64 pos_bit, rev;
	INT32 eval;
	INT32 best;
	PVLINE line;

	pos_bit = 1ULL << x1;
	rev = GetRev[x1](bk, wh);
	if (rev)
	{
		best = -SearchEmpty_2(wh ^ rev, bk ^ (pos_bit | rev), x2, x3, -beta, -alpha, 0, &line);
		if (best >= beta) return best;
		if (best > alpha)
		{
			alpha = best;
			pline->argmove[0] = x1;
			memcpy(pline->argmove + 1, line.argmove, line.cmove);
			pline->cmove = line.cmove + 1;
		}
	}
	else best = -g_infscore;

	pos_bit = 1ULL << x2;
	rev = GetRev[x2](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_2(wh ^ rev, bk ^ (pos_bit | rev), x1, x3, -beta, -alpha, 0, &line);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			if (best > alpha)
			{
				alpha = best;
				pline->argmove[0] = x2;
				memcpy(pline->argmove + 1, line.argmove, line.cmove);
				pline->cmove = line.cmove + 1;
			}
		}
	}

	pos_bit = 1ULL << x3;
	rev = GetRev[x3](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_2(wh ^ rev, bk ^ (pos_bit | rev), x1, x2, -beta, -alpha, 0, &line);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			if (best > alpha)
			{
				alpha = best;
				pline->argmove[0] = x3;
				memcpy(pline->argmove + 1, line.argmove, line.cmove);
				pline->cmove = line.cmove + 1;
			}
		}
	}

	// no move
	if (best == -g_infscore)
	{
		if (passed)
		{
			best = GetEndScore[g_solveMethod](bk, wh, 3);
			pline->cmove = 0;
		}
		else
		{
			best = -SearchEmpty_3(wh, bk, blank, parity, -beta, -alpha, 1, pline);
		}
	}

	return best;

}




INT32 SearchEmpty_4(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 parity, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
{
	g_countNode++;

	INT32 best;
	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &best))
	{
		if (g_solveMethod == SOLVE_WLD)
		{
			if (best > DRAW) best = WIN;
			else if (best < DRAW) best = LOSS;
		}
		return best;
	}
	// 4 empties parity
	UINT64 temp_moves = blank;
	int x1 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x1);
	int x2 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x2);
	int x3 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x3);
	int x4 = CountBit((~temp_moves) & (temp_moves - 1));

	// parity...move sort
	//    4 - 1 3 - 2 2 - 1 1 2 - 1 1 1 1
	// Only the 1 1 2 case needs move sorting.
	if (!(parity & board_parity_bit[x1])) {
		if (parity & board_parity_bit[x2]) {
			if (parity & board_parity_bit[x3]) { // case 1(x2) 1(x3) 2(x1 x4)
				int tmp = x1; x1 = x2; x2 = x3; x3 = tmp;
			}
			else { // case 1(x2) 1(x4) 2(x1 x3)
				int tmp = x1; x1 = x2; x2 = x4; x4 = x3; x3 = tmp;
			}
		}
		else if (parity & board_parity_bit[x3]) { // case 1(x3) 1(x4) 2(x1 x2)
			int tmp = x1; x1 = x3; x3 = tmp; tmp = x2; x2 = x4; x4 = tmp;
		}
	}
	else {
		if (!(parity & board_parity_bit[x2])) {
			if (parity & board_parity_bit[x3]) { // case 1(x1) 1(x3) 2(x2 x4)
				int tmp = x2; x2 = x3; x3 = tmp;
			}
			else { // case 1(x1) 1(x4) 2(x2 x3)
				int tmp = x2; x2 = x4; x4 = x3; x3 = tmp;
			}
		}
	}

	UINT64 pos_bit, rev;
	INT32 eval;
	PVLINE line;

	pos_bit = 1ULL << x1;
	rev = GetRev[x1](bk, wh);
	if (rev)
	{
		best = -SearchEmpty_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, parity ^ board_parity_bit[x1], -beta, -alpha, 0, &line);
		if (best >= beta) return best;
		if (best > alpha)
		{
			alpha = best;
			pline->argmove[0] = x1;
			memcpy(pline->argmove + 1, line.argmove, line.cmove);
			pline->cmove = line.cmove + 1;
		}
	}
	else best = -g_infscore;

	pos_bit = 1ULL << x2;
	rev = GetRev[x2](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, parity ^ board_parity_bit[x2], -beta, -alpha, 0, &line);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			if (best > alpha)
			{
				alpha = best;
				pline->argmove[0] = x2;
				memcpy(pline->argmove + 1, line.argmove, line.cmove);
				pline->cmove = line.cmove + 1;
			}
		}
	}

	pos_bit = 1ULL << x3;
	rev = GetRev[x3](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_3(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
			parity ^ board_parity_bit[x3], -beta, -alpha, 0, &line);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			if (best > alpha)
			{
				alpha = best;
				pline->argmove[0] = x3;
				memcpy(pline->argmove + 1, line.argmove, line.cmove);
				pline->cmove = line.cmove + 1;
			}
		}
	}

	pos_bit = 1ULL << x4;
	rev = GetRev[x4](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_3(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, parity ^ board_parity_bit[x4], -beta, -alpha, 0, &line);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			if (best > alpha)
			{
				alpha = best;
				pline->argmove[0] = x4;
				memcpy(pline->argmove + 1, line.argmove, line.cmove);
				pline->cmove = line.cmove + 1;
			}
		}
	}

	// no move
	if (best == -g_infscore)
	{
		if (passed)
		{
			best = GetEndScore[g_solveMethod](bk, wh, 4);
			pline->cmove = 0;
		}
		else
		{
			best = -SearchEmpty_4(wh, bk, blank, empty, parity, -beta, -alpha, 1, pline);
		}
	}

	return best;
}



INT32 SearchEmpty_5(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 parity, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
{
	g_countNode++;

	INT32 best;
	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &best))
	{
		if (g_solveMethod == SOLVE_WLD)
		{
			if (best > DRAW) best = WIN;
			else if (best < DRAW) best = LOSS;
		}
		return best;
	}

	// 4 empties parity
	UINT64 temp_moves = blank;
	int x1 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x1);
	int x2 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x2);
	int x3 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x3);
	int x4 = CountBit((~temp_moves) & (temp_moves - 1));
	temp_moves ^= (1ULL << x4);
	int x5 = CountBit((~temp_moves) & (temp_moves - 1));

	// parity...move sort
	if (!(parity & board_parity_bit[x1])) {             // x1 = even?
		if (!(parity & board_parity_bit[x2])) {         // x2 = even?
			if (!(parity & board_parity_bit[x3])) {     // x3 = even?
				if (!(parity & board_parity_bit[x4])) { // x4 = even?
					if (parity & board_parity_bit[x5]) {  // x5 = odd?
						// pattern ( x1, x2, x3, x4 ) (x5)
						int tmp = x5; x5 = x1; x1 = tmp;
					}
				}
				else{
					if (parity & board_parity_bit[x5]) {  // x5 = odd?
						// pattern ( x1, x2, x3 ) (x4) (x5)
						int tmp = x4; x4 = x1; x1 = tmp; tmp = x5; x5 = x2; x2 = tmp;
					}
					else
					{
						// pattern ( x1, x2, x3, x5 ) (x4)
						int tmp = x4; x4 = x1; x1 = tmp;
					}
				}
			}
			else {
				if (!(parity & board_parity_bit[x4])) { // x4 = even?
					if (!(parity & board_parity_bit[x5])) { // x5 = even?
						// pattern ( x1, x2, x4, x5 ) (x3)
						int tmp = x3; x3 = x1; x1 = tmp;
					}
					else {
						// pattern ( x1, x2, x4 ) (x3) (x5)
						int tmp = x3; x3 = x1; x1 = tmp; tmp = x5; x5 = x2; x2 = tmp;
					}
				}
				else {
					if (!(parity & board_parity_bit[x5])) { // x5 = even?
						// pattern ( x1, x2, x5 ) (x3) (x4)
						int tmp = x3; x3 = x1; x1 = tmp;
					}
					else {
						// pattern ( x1, x2 ) (x3) (x4) (x5)
						int tmp = x3; x3 = x1; x1 = tmp; tmp = x4; x4 = x2; x2 = tmp; tmp = x5; x5 = x3; x3 = tmp;
					}
				}
			}
		}
		else{
			// x1:even, x2:odd
			if (!(parity & board_parity_bit[x3])) {     // x3 = even?
				if (!(parity & board_parity_bit[x4])) { // x4 = even?
					if (parity & board_parity_bit[x5]) {  // x5 = odd?
						// pattern ( x1, x3, x4) (x2) (x5)
						int tmp = x2; x2 = x1; x1 = tmp; tmp = x5; x5 = x2; x2 = tmp;
					}
					else {
						// pattern ( x1, x3, x4, x5) (x2)
						int tmp = x2; x2 = x1; x1 = tmp;
					}
				}
				else{
					if (parity & board_parity_bit[x5]) {  // x5 = odd?
						// pattern ( x1, x3 ) (x2) (x4) (x5)
						int tmp = x2; x2 = x1; x1 = tmp; tmp = x4; x4 = x2; x2 = tmp; tmp = x5; x5 = x3; x3 = tmp;
					}
					else
					{
						// pattern ( x1, x3, x5 ) (x2) (x4)
						int tmp = x2; x2 = x1; x1 = tmp; tmp = x4; x4 = x2; x2 = tmp;
					}
				}
			}
			else {
				// x1:even, x2:odd, x3:odd
				if (!(parity & board_parity_bit[x4])) { // x4 = even?
					if (!(parity & board_parity_bit[x5])) { // x5 = even?
						// pattern ( x1, x4, x5 ) (x2) (x3)
						int tmp = x2; x2 = x1; x1 = tmp; tmp = x3; x3 = x2; x2 = tmp;
					}
					else {
						// pattern ( x1, x4 ) (x2) (x3) (x5)
						int tmp = x2; x2 = x1; x1 = tmp; tmp = x3; x3 = x2; x2 = tmp; tmp = x5; x5 = x3; x3 = tmp;
					}
				}
				else {
					// x1:even, x2:odd, x3:odd, x4:odd
					if (!(parity & board_parity_bit[x5])) { // x5 = even?
						// pattern ( x1, x5 ) (x2) (x3) (x4)
						int tmp = x2; x2 = x1; x1 = tmp; tmp = x3; x3 = x2; x2 = tmp; tmp = x4; x4 = x3; x3 = tmp;
					}
					else {
						// pattern (x1) (x2) (x3) (x4) (x5)
						// do nothing
					}
				}
			}


		}
	}
	else if (!(parity & board_parity_bit[x2])) {      // x2 = even?
		if (!(parity & board_parity_bit[x3])) {       // x3 = even?
			if (!(parity & board_parity_bit[x4])) {   // x4 = even?
				if (parity & board_parity_bit[x5]) {  // x5 = odd?
					// pattern ( x2, x3, x4 ) (x1) (x5)
					int tmp = x5; x5 = x2; x2 = tmp;
				}
				else {
					// pattern ( x2, x3, x4, X5 ) (x1)
					// do nothing
				}
			}
			else{
				// x1:odd, x2:even, x3:even, x4:odd
				if (parity & board_parity_bit[x5]) {  // x5 = odd?
					// pattern ( x2, x3 ) (x1) (x4) (x5)
					int tmp = x4; x4 = x2; x2 = tmp; tmp = x5; x5 = x3; x3 = tmp;
				}
				else {
					// pattern ( x2, x3, x5 ) (x1) (x4)
					int tmp = x4; x4 = x2; x2 = tmp;
				}
			}
		}
		else {
			if (!(parity & board_parity_bit[x4])) {     // x4 = even?
				if (!(parity & board_parity_bit[x5])) { // x5 = even?
					// pattern ( x2, x4, x5 ) (x1) (x3)
					int tmp = x3; x3 = x2; x2 = tmp;
				}
				else {
					// pattern ( x2, x4 ) (x1) (x3) (X5)
					int tmp = x3; x3 = x2; x2 = tmp; tmp = x5; x5 = x3; x3 = tmp;
				}
			}
			else {
				if (!(parity & board_parity_bit[x5])) { // x5 = even?
					// pattern (x2, x5) (x1) (x3) (x4)
					int tmp = x3; x3 = x2; x2 = tmp; tmp = x4; x4 = x3; x3 = tmp;
				}
				else {
					// pattern (x2) (x1) (x3) (x4) (X5)
					// do nothing
				}
			}
		}
	}
	else
	{
		if (!(parity & board_parity_bit[x3])) {       // x3 = even?
			if (!(parity & board_parity_bit[x4])) {   // x4 = even?
				if (parity & board_parity_bit[x5]) {  // x5 = odd?
					// pattern ( x3, x4 ) (x1) (x2) (x5)
					int tmp = x5; x5 = x3; x3 = tmp;
				}
				else {
					// pattern ( x3, x4, x5 ) (x1) (x2)
					// do nothing
				}
			}
			else{
				// x1:odd, x2:even, x3:even, x4:odd
				if (!(parity & board_parity_bit[x5])) {  // x5 = even?
					// pattern ( x3, x5 ) (x1) (x2) (x4)
					int tmp = x4; x4 = x3; x3 = tmp;
				}
				else {
					// pattern (x3) (x1) (x2) (x4) (x5)
					// do nothing
				}
			}
		}
	}

	UINT64 pos_bit, rev;
	INT32 eval;
	PVLINE line;

	pos_bit = 1ULL << x1;
	rev = GetRev[x1](bk, wh);
	if (rev)
	{
		best = -SearchEmpty_4(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, empty, parity ^ board_parity_bit[x1], -beta, -alpha, 0, &line);
		if (best >= beta) return best;
		if (best > alpha)
		{
			alpha = best;
			pline->argmove[0] = x1;
			memcpy(pline->argmove + 1, line.argmove, line.cmove);
			pline->cmove = line.cmove + 1;
		}
	}
	else best = -g_infscore;

	pos_bit = 1ULL << x2;
	rev = GetRev[x2](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_4(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, empty, parity ^ board_parity_bit[x2], -beta, -alpha, 0, &line);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			if (best > alpha)
			{
				alpha = best;
				pline->argmove[0] = x2;
				memcpy(pline->argmove + 1, line.argmove, line.cmove);
				pline->cmove = line.cmove + 1;
			}
		}
	}

	pos_bit = 1ULL << x3;
	rev = GetRev[x3](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_4(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, empty, parity ^ board_parity_bit[x3], -beta, -alpha, 0, &line);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			if (best > alpha)
			{
				alpha = best;
				pline->argmove[0] = x3;
				memcpy(pline->argmove + 1, line.argmove, line.cmove);
				pline->cmove = line.cmove + 1;
			}
		}
	}

	pos_bit = 1ULL << x4;
	rev = GetRev[x4](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_4(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, empty, parity ^ board_parity_bit[x4], -beta, -alpha, 0, &line);
		if (eval >= beta) return eval;
		else if (eval > best)
		{
			best = eval;
			if (best > alpha)
			{
				alpha = best;
				pline->argmove[0] = x4;
				memcpy(pline->argmove + 1, line.argmove, line.cmove);
				pline->cmove = line.cmove + 1;
			}
		}
	}

	pos_bit = 1ULL << x5;
	rev = GetRev[x5](bk, wh);
	if (rev)
	{
		eval = -SearchEmpty_4(wh ^ rev, bk ^ (pos_bit | rev),
			blank ^ pos_bit, empty, parity ^ board_parity_bit[x5], -beta, -alpha, 0, &line);
		if (eval >= beta) return eval;
		if (eval > best) {
			best = eval;
			if (best > alpha)
			{
				alpha = best;
				pline->argmove[0] = x5;
				memcpy(pline->argmove + 1, line.argmove, line.cmove);
				pline->cmove = line.cmove + 1;
			}
		}

	}

	// no move
	if (best == -g_infscore)
	{
		if (passed)
		{
			best = GetEndScore[g_solveMethod](bk, wh, 5);
			pline->cmove = 0;
		}
		else
		{
			best = -SearchEmpty_5(wh, bk, blank, empty, parity, -beta, -alpha, 1, pline);
		}
	}


	return best;
}