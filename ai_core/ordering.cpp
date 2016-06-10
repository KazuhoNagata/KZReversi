/***************************************************************************
* Name  : ordering.cpp
* Brief : ��̕��ёւ��֘A�̏������s��
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

void sort_move_list(char *move_list, int eval_list[], int cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int temp, swaps;
	char ctemp;

	if (cnt == 1){ return; }
	while (1)
	{
		swaps = 0;
		for (i = 0; i + h < cnt; i++)
		{
			if (eval_list[i] < eval_list[i + h])
			{
				ctemp = move_list[i];
				move_list[i] = move_list[i + h];
				move_list[i + h] = ctemp;
				temp = eval_list[i];
				eval_list[i] = eval_list[i + h];
				eval_list[i + h] = temp;
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

void sort_move_list(MOVELIST *move_list, int eval_list[], int cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int temp, swaps;
	MOVELIST ctemp;

	if (cnt == 1){ return; }
	while (1)
	{
		swaps = 0;
		for (i = 0; i + h < cnt; i++)
		{
			if (eval_list[i] < eval_list[i + h])
			{
				ctemp = move_list[i];
				move_list[i] = move_list[i + h];
				move_list[i + h] = ctemp;
				temp = eval_list[i];
				eval_list[i] = eval_list[i + h];
				eval_list[i + h] = temp;
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

/* �R���\�[�g */
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

/* �R���\�[�g */
void SortListUseTable(INT8 *pos_list, INT32 move_list[], UINT64 rev_list[], INT32 cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int temp, swaps;
	char move_temp;
	UINT64 rev_temp;

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

				rev_temp = rev_list[i];
				rev_list[i] = rev_list[i + h];
				rev_list[i + h] = rev_temp;

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

/* �ÓI�����Â�(�r�b�g�񂩂�̎擾) */
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

void sort_movelist_score_descending(MoveList *movelist)
{
	MoveList *iter, *best, *previous_best, *previous;

	for (iter = movelist; iter->next != NULL; iter = iter->next) 
	{
		previous_best = iter;
		for (previous = previous_best->next; previous->next != NULL; previous = previous->next) 
		{
			if (previous_best->next->move.score <  previous->next->move.score)
			{
				previous_best = previous;
			}
		}
		if (previous_best != iter) {
			best = previous_best->next;
			previous_best->next = best->next;
			best->next = iter->next;
			iter->next = best;
		}
	}
}

void sort_movelist_score_ascending(MoveList *movelist)
{
	MoveList *iter, *best, *previous_best, *previous;

	for (iter = movelist; iter->next != NULL; iter = iter->next) {
		previous_best = iter;
		for (previous = previous_best->next; previous->next != NULL; previous = previous->next) 
		{
			if (previous_best->next->move.score >  previous->next->move.score) 
			{
				previous_best = previous;
			}
		}
		if (previous_best != iter) {
			best = previous_best->next;
			previous_best->next = best->next;
			best->next = iter->next;
			iter->next = best;
		}
	}
}

/*!
* \brief Sort a list of move, fastest-first.
*
* Sort a list to accelerate the alphabeta research. The moves that minimize
* opponent mobility and increase player's relative stability will be played
* first.
* \param movelist   : list of moves to sort.
* \param board      : board on which the moves applied.
*/
void SortFastfirst(MoveList *movelist, UINT64 bk, UINT64 wh)
{
	MoveList *iter;
	UINT32 n_moves_wh;
	UINT64 move_b, move_w;

	for (iter = movelist->next; iter != NULL; iter = iter->next) 
	{
		iter->move.rev = GetRev[iter->move.pos](bk, wh);
		move_b = bk ^ ((1ULL << (iter->move.pos)) | iter->move.rev);
		move_w = wh ^ (iter->move.rev);
		CreateMoves(move_w, move_b, &n_moves_wh);
		iter->move.score = (n_moves_wh << 4) - get_corner_stability(bk);
	}

	sort_movelist_score_ascending(movelist);
}

#if 1
/* ���Ւ��՗p move ordering */
void SortMoveListMiddle(
	MoveList *movelist, 
	UINT64 bk, UINT64 wh,
	HashTable *hash, 
	INT32 depth, 
	UINT32 empty,
	INT32 alpha, INT32 beta, 
	UINT32 color)
{
	MoveList *iter;
	INT32 score;
	UINT32 cnt = 0;
	UINT32 move_cnt;
	UINT64 move_b, move_w;
	UINT64 n_moves_wh;

	INT32 searched = g_empty - empty;
	if (searched < 6 && g_limitDepth >= 14)
	{
		for (iter = movelist->next; iter != NULL; iter = iter->next)
		{
			move_b = bk ^ ((1ULL << (iter->move.pos)) | iter->move.rev);
			move_w = wh ^ (iter->move.rev);

			score = -OrderingAlphaBeta(move_w, move_b, 6 - searched, empty - 1, color ^ 1,
				-NEGAMAX, -NEGAMIN, 0);
			iter->move.score = score;
		}
		/* �����̓��_�̑������Ƀ\�[�g */
		sort_movelist_score_descending(movelist);
	}
	else if (g_empty - empty == 6){
		for (iter = movelist->next; iter != NULL; iter = iter->next)
		{
			move_b = bk ^ ((1ULL << (iter->move.pos)) | iter->move.rev);
			move_w = wh ^ (iter->move.rev);
			InitIndexBoard(move_w, move_b);

			score = -Evaluation(g_board, move_w, move_b, color ^ 1, 59 - (empty - 1));
			iter->move.score = score;
		}
		/* �����̓��_�̑������Ƀ\�[�g */
		sort_movelist_score_descending(movelist);
	}
	else
	{
		for (iter = movelist->next; iter != NULL; iter = iter->next)
		{
			score = 0;
			move_b = bk ^ ((1ULL << (iter->move.pos)) | iter->move.rev);
			move_w = wh ^ (iter->move.rev);
			n_moves_wh = CreateMoves(move_w, move_b, &move_cnt);

			/* �ʒu���_ */
			score -= posEval[iter->move.pos] << 2;
			//score += CountBit(iter->move.rev) << 4;
			/* �G�̒���\�����擾 */
			score += ((move_cnt + CountBit(n_moves_wh & 0x8100000000000081)) << 4)
				- get_corner_stability(move_b);
			score += CountBit(GetPotentialMoves(move_w, move_b, ~(move_b | move_w))) << 3;
			iter->move.score = score;
		}
		/* �G�̓��_�̏��Ȃ����Ƀ\�[�g */
		sort_movelist_score_ascending(movelist);
	}
}

#endif

/*  �I�՗p move ordering */
void SortMoveListEnd(MoveList *movelist, UINT64 bk, UINT64 wh)
{
	MoveList *iter;
	INT32 score;
	UINT64 ligal_move_w;
	UINT64 move_b, move_w;
	UINT32 move_cnt;

	for (iter = movelist->next; iter != NULL; iter = iter->next)
	{
		score = 0;
		move_b = bk ^ ((1ULL << iter->move.pos) | iter->move.rev);
		move_w = wh ^ iter->move.rev;

		ligal_move_w = CreateMoves(move_w, move_b, &move_cnt);

		/* �G�̒���\�����擾 */
		score += (move_cnt + CountBit(ligal_move_w & 0x8100000000000081)) << 4;
		score -= get_corner_stability(move_b);

		iter->move.score = score;
	}

	/* �G�̓��_�̏��Ȃ����Ƀ\�[�g */
	sort_movelist_score_ascending(movelist);
}


/*  �I�՗p move ordering */
char MoveOrderingEnd(MOVELIST *pos_list, UINT64 b_board, UINT64 w_board, UINT64 moves)
{
	char cnt = 0;
	int pos = 0;
	UINT64 rev, ligal_move_w;
	UINT64 move_b, move_w;
	UINT32 move_cnt;
	int score = 0;
	int score_list[36];

	do{
		score;
		pos = CountBit((moves & (-(INT64)moves)) - 1);
		/* ���]�f�[�^�擾 */
		rev = GetRev[pos](b_board, w_board);

		move_b = b_board ^ ((1ULL << pos) | rev);
		move_w = w_board^rev;

		ligal_move_w = CreateMoves(move_w, move_b, &move_cnt);

		/* �G�ɋ��𖾂��n���Ȃ� */
		score = CountBit(ligal_move_w & 0x8100000000000081) << 1;
		/* �K�؂Ȑ��b�ł��̉\����̏ꍇ */
		if ((1ULL << pos) & 0x42C300000000C342){
			if (score == 0)
			{
				score--;
			}
		}
		/*if(0x8100000000000081 & (one << pos)){
		score -= 4;
		}*/

		/* �G�̒���\�����擾 */
		score += (move_cnt << 5);
		score += get_corner_stability(move_w) - get_corner_stability(move_b);

		score_list[cnt] = score;
		pos_list[cnt].move = (char)pos;
		pos_list[cnt].rev = rev;
		cnt++;
		moves = moves & (moves - 1);
	} while (moves);

	/* �G�̓��_�̏��Ȃ����Ƀ\�[�g */
	SortListUseTable(pos_list, score_list, cnt);

	return cnt;
}

void SortMoveListTableMoveFirst(MoveList *movelist, int move){

	MoveList *iter, *previous;

	for (iter = (previous = movelist)->next; iter != NULL; iter = (previous = iter)->next){
		if (iter->move.pos == move){
			previous->next = iter->next;
			iter->next = movelist->next;
			movelist->next = iter;
			break;
		}
	}
}