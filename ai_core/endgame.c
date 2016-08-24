#include "stdafx.h"
#include "bit64.h"
#include "board.h"
#include "move.h"
#include "rev.h"
#include "cpu.h"
#include "endgame.h"
#include "empty.h"
#include "hash.h"
#include "eval.h"
#include "ordering.h"

#include "count_last_flip_carry_64.h"

#include <stdio.h>
#include <omp.h>

/**
* @brief Stability Cutoff (TC).
*
* @param search Current position.
* @param alpha Alpha bound.
* @param score Score to return in case of a cutoff is found.
* @return 'true' if a cutoff is found, false otherwise.
*/
BOOL search_SC_NWS(UINT64 bk, UINT64 wh, INT32 empty, INT32 alpha, INT32 *score)
{
	if (alpha >= NWS_STABILITY_THRESHOLD[empty]) {
		*score = 64 - 2 * get_stability(wh, bk);
		if (*score <= alpha) {
			return TRUE;
		}
	}
	return FALSE;
}

/***************************************************************************
* Name  : PVS_SearchDeepExact
* Brief : PV Search を行い、評価値を基に最善手を取得
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         depth     : 読む深さ
*         empty     : 空きマス数
*         alpha     : このノードにおける下限値
*         beta      : このノードにおける上限値
*         color     : CPUの色
*         hash      : 置換表の先頭ポインタ
*         pass_cnt  : 今までのパスの数(２カウントで終了とみなす)
* Return: 着手可能位置のビット列
****************************************************************************/
INT32 PVS_SearchDeepExact(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color, HashTable *hash,
	INT32 alpha, INT32 beta, UINT32 passed, INT32 *p_selectivity, PVLINE *pline)
{
	/* アボート処理 */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	int score, bestscore, lower, upper, bestmove;
	UINT32 key;
	MoveList movelist[32 + 2], *iter;
	Move *move;
	HashInfo *hashInfo;

	bestmove = NOMOVE;
	lower = alpha;
	upper = beta;

	if (g_empty > 12 &&  empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
	{
		UINT64 blank = ~(bk | wh);
		UINT32 quad_parity[4];
		//create_quad_parity(quad_parity, blank);
		return AB_SearchExact(bk, wh, blank, empty,
			color, alpha, beta, passed, quad_parity, p_selectivity, pline);
	}

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &score))
	{
		// up to max threshold
		*p_selectivity = g_mpc_level;
		return score;
	}

	/************************************************************
	*
	* 置換表カットオフフェーズ
	*
	*************************************************************/
	/* キーを生成 */
	key = KEY_HASH_MACRO(bk, wh, color);
	/* transposition cutoff ? */
	if (g_tableFlag)
	{
		score = -g_infscore;
		// 排他処理開始
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->depth >= empty && hashInfo->selectivity >= g_mpc_level)
			{
				int hash_upper;
				hash_upper = hashInfo->upper;
				if (hash_upper <= lower)
				{
					// transposition table cutoff
					*p_selectivity = hashInfo->selectivity;
					pline->cmove = 0;
					return hash_upper;
				}
				int hash_lower;
				hash_lower = hashInfo->lower;
				if (hash_lower >= upper)
				{
					// transposition table cutoff
					*p_selectivity = hashInfo->selectivity;
					pline->cmove = 0;
					return hash_lower;
				}
				if (hash_lower == hash_upper)
				{
					// transposition table cutoff
					*p_selectivity = hashInfo->selectivity;
					pline->cmove = 0;
					return hash_lower;
				}

				// change window width
				lower = max(lower, hash_lower);
				upper = min(upper, hash_upper);
			}

			bestmove = hashInfo->bestmove;
		}
	}

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) フェーズ
	*
	*************************************************************/
#if 1
	if (g_mpcFlag && g_mpc_level < g_max_cut_table_size && 
		empty >= MPC_END_MIN_DEPTH  && empty <= MPC_END_MAX_DEPTH && empty <= g_empty - 1)
	{
		MPCINFO *mpcInfo_p = &mpcInfo_end[empty - MPC_END_MIN_DEPTH];
		INT32 value = (INT32)((alpha * EVAL_ONE_STONE) - (mpcInfo_p->deviation * MPC_END_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		INT32 eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed, pline);

		if (eval < value)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, alpha, empty, NOMOVE, g_mpc_level, g_infscore);
			// store cutoff level
			*p_selectivity = g_mpc_level;
			pline->cmove = 0;
			return alpha;
		}

		value = (INT32)((beta * EVAL_ONE_STONE) + (mpcInfo_p->deviation * MPC_END_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed, pline);

		if (eval > value)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, beta, empty, NOMOVE, g_mpc_level, g_infscore);
			// store cutoff level
			*p_selectivity = g_mpc_level;
			pline->cmove = 0;
			return beta;
		}
	}
#endif


	/************************************************************
	*
	* Principal Variation Search(PVS) フェーズ
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;
	INT32 selectivity = g_mpc_level; // init now thresould

	// 着手のflip-bitを求めてmove構造体に保存
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL)
	{
		if (passed) {
			// game end...
			bestscore = GetEndScore[g_solveMethod](bk, wh, empty);
			bestmove = NOMOVE;
			pline->cmove = 0;
		}
		else
		{
			bestscore = -PVS_SearchDeepExact(wh, bk, empty, color ^ 1, hash, -upper, -lower, 1, p_selectivity, pline);
			bestmove = NOMOVE;
		}
	}
	else {

		BOOL pv_flag = TRUE;
		if (moveCount > 1)
		{
			if (g_empty - empty <= 10)
			{
				SortMoveListMiddle(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
			else
			{
				SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
		}

		/* 置換表で参照出来た手から先に着手するためにソート */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		PVLINE line;
		bestscore = -g_infscore;
		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			selectivity = g_mpc_level;
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;
			// PV表示記憶用
			g_pvline[g_empty - empty] = move->pos;

			if (pv_flag)
			{
				score = -PVS_SearchDeepExact(move_w, move_b,
					empty - 1, color ^ 1, hash, -upper, -lower, 0, &selectivity, &line);
			}
			else
			{
				score = -PVS_SearchDeepExact(move_w, move_b,
					empty - 1, color ^ 1, hash, -lower - 1, -lower, 0, &selectivity, &line);

				if (lower < score && score < upper)
				{
					selectivity = g_mpc_level;
					score = -PVS_SearchDeepExact(move_w, move_b,
						empty - 1, color ^ 1, hash, -upper, -lower, 0, &selectivity, &line);
				}
			}

			if (score >= upper)
			{
				bestscore = score;
				bestmove = move->pos;
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (score > lower)
				{
					pv_flag = FALSE;
					lower = score;
					if (line.cmove < 0 || line.cmove > 63)
					{
						line.cmove = 0;
					}
					pline->argmove[0] = bestmove;
					memcpy(pline->argmove + 1, line.argmove, line.cmove);
					pline->cmove = line.cmove + 1;
					if (g_empty - empty <= 6 && g_mpc_level >= 4)
					{
						CreatePVLineStr(pline, empty, bestscore * (1 - (2 * ((g_empty - empty) % 2))));
						g_set_message_funcptr[1](g_PVLineMsg);
					}
				}
			}
			
		}
	}

	/* 置換表に登録 */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, g_mpc_level, g_infscore);
	*p_selectivity = selectivity;

	return bestscore;
}


#if 0
/***************************************************************************
* Name  : PVS_SearchDeepExact_YBWC
* Brief : PV Search を行い、評価値を基に最善手を取得
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         depth     : 読む深さ
*         empty     : 空きマス数
*         alpha     : このノードにおける下限値
*         beta      : このノードにおける上限値
*         color     : CPUの色
*         hash      : 置換表の先頭ポインタ
*         pass_cnt  : 今までのパスの数(２カウントで終了とみなす)
* Return: 着手可能位置のビット列
****************************************************************************/
INT32 PVS_SearchDeepExact_YBWC(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
{

	/* アボート処理 */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	int score, upper, bestmove;
	int max_thread_num = omp_get_max_threads();
	int *lower = (int *)malloc(sizeof(int) * max_thread_num);

	for (int i = 0; i < max_thread_num; i++) lower[i] = alpha;

	MoveList movelist[34], movelist2[34], *iter;
	Move *move[4];

	bestmove = NOMOVE;
	upper = beta;

	/************************************************************
	*
	* Principal Variation Search(PVS) フェーズ
	*
	*************************************************************/

	UINT32 move_cnt1, move_cnt2;
	UINT64 moves = CreateMoves(bk, wh, &move_cnt1);
	UINT64 move_b[4], move_w[4];

	// 着手のflip-bitを求めてmove構造体に保存
	StoreMovelist(movelist, bk, wh, moves);

	BOOL pv_flag = TRUE;
	if (move_cnt1 > 1)
	{
		if (empty > 15)
		{
			SortMoveListMiddle(movelist, bk, wh, hash, empty, alpha, beta, color);
		}
		else if (empty > 6)
		{
			// 手の並べ替え
			SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
		}
	}

	PVLINE line[4];
	int max_lower = -INF_SCORE;

	// 一手目を最善で着手
	iter = movelist->next;
	move[0] = &(iter->move);
	move_b[0] = bk ^ ((1ULL << move[0]->pos) | move[0]->rev);
	move_w[0] = wh ^ move[0]->rev;

	score = -PVS_SearchDeepExact(move_w[0], move_b[0],
		empty - 1, color ^ 1, hash, -upper, -lower[0], 0, &line[0]);

	// PVの結果で全スレッドのαを更新
	for (int n = 0; n < max_thread_num; n++) {
		lower[n] = score;
	}

	// 1手目のYB
#pragma omp parallel for private(score, iter) schedule(dynamic, 1)
	for (int i0 = 1; i0 < (int)move_cnt1; i0++)
	{
		int tid = -1;
		iter = movelist->next;
		// iterをi0の値によって勧める(ループ初回時のみ)
		for (int j = 1; tid == -1 && j < i0; j++) iter = iter->next;

		tid = omp_get_thread_num();

		move[tid] = &(iter->move);
		move_b[tid] = bk ^ ((1ULL << move[tid]->pos) | move[tid]->rev);
		move_w[tid] = wh ^ move[tid]->rev;

		score = -PVS_SearchDeepExact(move_w[tid], move_b[tid],
			empty - 1, color ^ 1, hash, -lower[tid] - 1, -lower[tid], 0, &line[tid]);

		if (lower[tid] < score && score < upper)
		{
			score = -PVS_SearchDeepExact(move_w[tid], move_b[tid],
				empty - 1, color ^ 1, hash, -lower[tid], upper, 0, &line[tid]);
		}

		if (score > lower[tid]) lower[tid] = score;
		iter = iter->next;
	}

	max_lower = -INF_SCORE;
	for (int n = 0; n < max_thread_num; n++)
	{
		if (max_lower < -lower[n]) max_lower = -lower[n];
	}

	free(lower);

	return -max_lower;
}
#endif


INT32 AB_SearchExact(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty, UINT32 color, INT32 alpha, INT32 beta, 
	UINT32 passed, INT32 *quad_parity, INT32 *p_selectivity, PVLINE *pline)
{
	/* アボート処理 */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	INT32 max;                    //現在の最高評価値
	INT32 eval;                   //評価値の保存
	INT32 pos;
	UINT64 pos_bit, rev;

	// parity moving
	if (empty == 5)
	{
		UINT32 parity =
			((CountBit(blank & quad_parity_bitmask[3]) % 2) << 3) |
			((CountBit(blank & quad_parity_bitmask[2]) % 2) << 2) |
			((CountBit(blank & quad_parity_bitmask[1]) % 2) << 1) |
			(CountBit(blank & quad_parity_bitmask[0]) % 2);

		*p_selectivity = g_mpc_level;
		return SearchEmpty_5(bk, wh, blank, empty, parity, alpha, beta, 0, pline);
	}

	g_countNode++;

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &max))
	{
		*p_selectivity = g_mpc_level;
		return max;
	}

	max = -g_infscore;

	UINT32 move_cnt;
	MoveList movelist[24], *iter = movelist;
	UINT64 moves = CreateMoves(bk, wh, &move_cnt);
	PVLINE line;
	INT32 selectivity; // init now thresould

	if (move_cnt == 0)
	{
		// 打てなかった
		if (passed)
		{
			max = GetEndScore[g_solveMethod](bk, wh, empty);
			pline->cmove = 0;
		}
		else
		{
			max = -AB_SearchExact(wh, bk, blank, empty,
				color ^ 1, -beta, -alpha, 1, quad_parity, p_selectivity, pline);
		}
	}
	else
	{
		// 着手のflip-bitを求めてmove構造体に保存
		StoreMovelist(movelist, bk, wh, moves);
		if (move_cnt > 1)
		{
			if (empty >= 8) SortFastfirst(movelist, bk, wh);
			else SortPotentionalFastfirst(movelist, bk, wh, blank);
		}

#if 1
		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			pos_bit = 1ULL << pos;
			rev = iter->move.rev;

			eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
				empty - 1, color ^ 1, -beta, -alpha, 0, quad_parity, &selectivity, &line);

			if (beta <= eval)
			{
				max = eval;
				break;
			}

			/* 今までより良い局面が見つかれば最善手の更新 */
			if (eval > max)
			{
				max = eval;
				if (max > alpha)
				{
					alpha = max;
					pline->argmove[0] = pos;
					memcpy(pline->argmove + 1, line.argmove, line.cmove);
					pline->cmove = line.cmove + 1;
				}
			}
		}
#else
		// first move odd parity
		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			if (quad_parity[board_parity[pos]])
			{
				pos_bit = 1ULL << pos;
				rev = iter->move.rev;
				// reverse parity
				quad_parity[board_parity[pos]] ^= 1;
				eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
					empty - 1, color ^ 1, -beta, -alpha, 0, quad_parity, &selectivity, &line);
				// restore parity
				create_quad_parity(quad_parity, blank);
				if (beta <= eval)
				{
					max = eval;
					*p_selectivity = selectivity; // store level
					return max;
				}

				/* 今までより良い局面が見つかれば最善手の更新 */
				if (eval > max)
				{
					max = eval;
					if (max > alpha)
					{
						alpha = max;
						pline->argmove[0] = pos;
						memcpy(pline->argmove + 1, line.argmove, line.cmove);
						pline->cmove = line.cmove + 1;
					}
				}
			}
		}

		// after, even parity
		iter = movelist;
		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			if (!quad_parity[board_parity[pos]])
			{
				pos_bit = 1ULL << pos;
				rev = iter->move.rev;
				// reverse parity
				quad_parity[board_parity[pos]] ^= 1;
				eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
					empty - 1, color ^ 1, -beta, -alpha, 0, quad_parity, &selectivity, &line);
				// restore parity
				create_quad_parity(quad_parity, blank);
				if (beta <= eval)
				{
					max = eval;
					*p_selectivity = selectivity; // store level
					return max;
				}

				/* 今までより良い局面が見つかれば最善手の更新 */
				if (eval > max)
				{
					max = eval;
					if (max > alpha)
					{
						alpha = max;
						pline->argmove[0] = pos;
						memcpy(pline->argmove + 1, line.argmove, line.cmove);
						pline->cmove = line.cmove + 1;
					}
				}
			}
		}

#endif
	}

	*p_selectivity = selectivity; // store level

	return max;

}



/***************************************************************************
* Name  : PVS_SearchDeepWinLoss
* Brief : PV Search を行い、評価値を基に最善手を取得
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         depth     : 読む深さ
*         empty     : 空きマス数
*         alpha     : このノードにおける下限値
*         beta      : このノードにおける上限値
*         color     : CPUの色
*         hash      : 置換表の先頭ポインタ
*         pass_cnt  : 今までのパスの数(２カウントで終了とみなす)
* Return: 着手可能位置のビット列
****************************************************************************/
INT32 PVS_SearchDeepWinLoss(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, INT32* p_selectivity, PVLINE *pline)
{
	/* アボート処理 */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	int score, bestscore, lower, upper, bestmove;
	UINT32 key;
	MoveList movelist[32 + 2], *iter;
	Move *move;
	HashInfo *hashInfo;

	bestmove = NOMOVE;
	lower = alpha;
	upper = beta;

	if (g_empty > 12 && empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
	{
		UINT64 blank = ~(bk | wh);
		//UINT32 quad_parity[4];
		//create_quad_parity(quad_parity, blank);
		return AB_SearchWinLoss(bk, wh, blank, empty,
			color, alpha, beta, passed, p_selectivity, pline);
	}

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &score))
	{
		// up to max threshold
		*p_selectivity = g_mpc_level;

		if (score > DRAW) score = WIN;
		else if (score < DRAW) score = LOSS;

		return score;
	}

	/************************************************************
	*
	* 置換表カットオフフェーズ
	*
	*************************************************************/
	/* キーを生成 */
	key = KEY_HASH_MACRO(bk, wh, color);
	/* transposition cutoff ? */
	if (g_tableFlag)
	{
		score = -g_infscore;
		// 排他処理開始
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->depth >= empty && hashInfo->selectivity >= g_mpc_level)
			{
				int hash_upper;
				hash_upper = hashInfo->upper;
				if (hash_upper <= lower)
				{
					// transposition table cutoff
					*p_selectivity = hashInfo->selectivity;
					pline->cmove = 0;
					return hash_upper;
				}
				int hash_lower;
				hash_lower = hashInfo->lower;
				if (hash_lower >= upper)
				{
					// transposition table cutoff
					*p_selectivity = hashInfo->selectivity;
					pline->cmove = 0;
					return hash_lower;
				}
				if (hash_lower == hash_upper)
				{
					// transposition table cutoff
					*p_selectivity = hashInfo->selectivity;
					pline->cmove = 0;
					return hash_lower;
				}

				// change window width
				lower = max(lower, hash_lower);
				upper = min(upper, hash_upper);
			}

			bestmove = hashInfo->bestmove;
		}
	}

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) フェーズ
	*
	*************************************************************/
#if 1
	if (g_mpcFlag && g_mpc_level < g_max_cut_table_size &&
		empty >= MPC_END_MIN_DEPTH  && empty <= MPC_END_MAX_DEPTH && empty <= g_empty - 1)
	{
		MPCINFO *mpcInfo_p = &mpcInfo_end[empty - MPC_END_MIN_DEPTH];
		INT32 value = (INT32)((alpha * EVAL_ONE_STONE) - (mpcInfo_p->deviation * MPC_END_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		INT32 eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed, pline);

		if (eval < value)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, alpha, empty, NOMOVE, g_mpc_level, WIN + 1);
			// store cutoff level
			*p_selectivity = g_mpc_level;
			pline->cmove = 0;
			return alpha;
		}

		value = (INT32)((beta * EVAL_ONE_STONE) + (mpcInfo_p->deviation * MPC_END_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed, pline);

		if (eval > value)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, beta, empty, NOMOVE, g_mpc_level, WIN + 1);
			// store cutoff level
			*p_selectivity = g_mpc_level;
			pline->cmove = 0;
			return beta;
		}
	}
#endif


	/************************************************************
	*
	* Principal Variation Search(PVS) フェーズ
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;
	INT32 selectivity = g_mpc_level; // init now thresould

	// 着手のflip-bitを求めてmove構造体に保存
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL)
	{
		if (passed) {
			// game end...
			bestscore = GetEndScore[g_solveMethod](bk, wh, empty);
			bestmove = NOMOVE;
			pline->cmove = 0;
		}
		else
		{
			bestscore = -PVS_SearchDeepWinLoss(wh, bk, empty, color ^ 1, hash, -upper, -lower, 1, p_selectivity, pline);
			bestmove = NOMOVE;
		}
	}
	else {

		BOOL pv_flag = TRUE;
		if (moveCount > 1)
		{
			if (g_empty - empty <= 10)
			{
				SortMoveListMiddle(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
			else
			{
				SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
		}

		/* 置換表で参照出来た手から先に着手するためにソート */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		PVLINE line;
		bestscore = -g_infscore;
		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			selectivity = g_mpc_level;
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;
			// PV表示記憶用
			g_pvline[g_empty - empty] = move->pos;

			if (pv_flag)
			{
				score = -PVS_SearchDeepWinLoss(move_w, move_b,
					empty - 1, color ^ 1, hash, -upper, -lower, 0, &selectivity, &line);
			}
			else
			{
				score = -PVS_SearchDeepWinLoss(move_w, move_b,
					empty - 1, color ^ 1, hash, -lower - 1, -lower, 0, &selectivity, &line);

				if (lower < score && score < upper)
				{
					selectivity = g_mpc_level;
					score = -PVS_SearchDeepWinLoss(move_w, move_b,
						empty - 1, color ^ 1, hash, -upper, -lower, 0, &selectivity, &line);
				}
			}

			if (score >= upper)
			{
				bestscore = score;
				bestmove = move->pos;
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (score > lower)
				{
					pv_flag = FALSE;
					lower = score;
				}
			}

		}
	}

	/* 置換表に登録 */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, g_mpc_level, WIN + 1);
	*p_selectivity = selectivity;

	return bestscore;
}

/***************************************************************************
* Name  : AlphaBetaSearchWinLoss
* Brief : PV Search を行い、評価値を基に最善手を取得
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         depth     : 読む深さ
*         empty     : 空きマス数
*         alpha     : このノードにおける下限値
*         beta      : このノードにおける上限値
*         color     : CPUの色
*         hash      : 置換表の先頭ポインタ
*         pass_cnt  : 今までのパスの数(２カウントで終了とみなす)
* Return: 着手可能位置のビット列
****************************************************************************/
INT32 AB_SearchWinLoss(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 color, INT32 alpha, INT32 beta, UINT32 passed, INT32* p_selectivity, PVLINE *pline)
{
	/* アボート処理 */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	INT32 max;                    //現在の最高評価値
	INT32 eval;                   //評価値の保存
	INT32 pos;
	UINT64 pos_bit, rev;

	// parity moving
	if (empty == 5)
	{
		UINT32 parity =
			((CountBit(blank & quad_parity_bitmask[3]) % 2) << 3) |
			((CountBit(blank & quad_parity_bitmask[2]) % 2) << 2) |
			((CountBit(blank & quad_parity_bitmask[1]) % 2) << 1) |
			(CountBit(blank & quad_parity_bitmask[0]) % 2);

		*p_selectivity = g_mpc_level;
		return SearchEmpty_5(bk, wh, blank, empty, parity, alpha, beta, 0, pline);
	}

	g_countNode++;

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &max))
	{
		*p_selectivity = g_mpc_level;

		if (max > DRAW) max = WIN;
		else if (max < DRAW) max = LOSS;

		return max;
	}

	max = LOSS - 1;

	UINT32 move_cnt;
	MoveList movelist[24], *iter = movelist;
	UINT64 moves = CreateMoves(bk, wh, &move_cnt);
	PVLINE line;
	INT32 selectivity; // init now thresould

	if (move_cnt == 0)
	{
		// 打てなかった
		if (passed)
		{
			max = GetEndScore[g_solveMethod](bk, wh, empty);
			pline->cmove = 0;
		}
		else
		{
			max = -AB_SearchWinLoss(wh, bk, blank, empty,
				color ^ 1, -beta, -alpha, 1, p_selectivity, pline);
		}
	}
	else
	{
		// 着手のflip-bitを求めてmove構造体に保存
		StoreMovelist(movelist, bk, wh, moves);
		if (move_cnt > 1)
		{
			if (empty >= 8) SortFastfirst(movelist, bk, wh);
			else SortPotentionalFastfirst(movelist, bk, wh, blank);
		}

#if 1
		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			pos_bit = 1ULL << pos;
			rev = iter->move.rev;

			eval = -AB_SearchWinLoss(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
				empty - 1, color ^ 1, -beta, -alpha, 0, &selectivity, &line);

			if (beta <= eval)
			{
				max = eval;
				break;
			}

			/* 今までより良い局面が見つかれば最善手の更新 */
			if (eval > max)
			{
				max = eval;
				if (max > alpha) alpha = max;
			}
		}
#else
		// first move odd parity
		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			if (quad_parity[board_parity[pos]])
			{
				pos_bit = 1ULL << pos;
				rev = iter->move.rev;
				// reverse parity
				quad_parity[board_parity[pos]] ^= 1;
				eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
					empty - 1, color ^ 1, -beta, -alpha, 0, quad_parity, &selectivity, &line);
				// restore parity
				create_quad_parity(quad_parity, blank);
				if (beta <= eval)
				{
					max = eval;
					*p_selectivity = selectivity; // store level
					return max;
				}

				/* 今までより良い局面が見つかれば最善手の更新 */
				if (eval > max)
				{
					max = eval;
					if (max > alpha)
					{
						alpha = max;
						pline->argmove[0] = pos;
						memcpy(pline->argmove + 1, line.argmove, line.cmove);
						pline->cmove = line.cmove + 1;
					}
				}
			}
		}

		// after, even parity
		iter = movelist;
		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			if (!quad_parity[board_parity[pos]])
			{
				pos_bit = 1ULL << pos;
				rev = iter->move.rev;
				// reverse parity
				quad_parity[board_parity[pos]] ^= 1;
				eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev), blank ^ pos_bit,
					empty - 1, color ^ 1, -beta, -alpha, 0, quad_parity, &selectivity, &line);
				// restore parity
				create_quad_parity(quad_parity, blank);
				if (beta <= eval)
				{
					max = eval;
					*p_selectivity = selectivity; // store level
					return max;
				}

				/* 今までより良い局面が見つかれば最善手の更新 */
				if (eval > max)
				{
					max = eval;
					if (max > alpha)
					{
						alpha = max;
						pline->argmove[0] = pos;
						memcpy(pline->argmove + 1, line.argmove, line.cmove);
						pline->cmove = line.cmove + 1;
					}
				}
			}
		}

#endif
	}

	*p_selectivity = selectivity; // store level

	return max;
}