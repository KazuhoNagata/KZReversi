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

void sort_movelist_score_descending(MoveList *movelist)
{
	MoveList *iter, *best, *previous_best, *previous;

	for (iter = movelist; iter->next != NULL; iter = iter->next)
	{
		previous_best = iter;
		for (previous = previous_best->next; previous->next != NULL; previous = previous->next)
		{
			if (previous_best->next->move.score < previous->next->move.score)
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
			if (previous_best->next->move.score > previous->next->move.score)
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
		iter->move.score = ((UINT64)n_moves_wh << 4) - get_corner_stability(bk);
		//iter->move.score += CountBit(GetPotentialMoves(move_w, move_b, (~(bk | wh)) ^ (1ULL << iter->move.pos)));
	}

	sort_movelist_score_ascending(movelist);
}

void SortPotentionalFastfirst(MoveList *movelist, UINT64 bk, UINT64 wh, UINT64 blank)
{
	MoveList *iter;
	INT32 score;
	UINT64 move_b, move_w;

	for (iter = movelist->next; iter != NULL; iter = iter->next)
	{
		score = 0;
		move_b = bk ^ ((1ULL << (iter->move.pos)) | iter->move.rev);
		move_w = wh ^ (iter->move.rev);
		/* 敵の着手可能数を取得 */
		score += CountBit(GetPotentialMoves(move_w, move_b));
		iter->move.score = score;
	}
	/* 敵の得点の少ない順にソート */
	sort_movelist_score_ascending(movelist);
}

#if 1
/* 序盤中盤用 move ordering */
void SortMoveListMiddle(
	MoveList  *movelist,
	UINT64     bk, 
	UINT64     wh,
	HashTable *hash,
	HashTable *pvHash,
	UINT32     empty,
	INT32      depth,
	INT32      alpha, 
	INT32      beta,
	UINT32     color
)
{
#if 1
	MoveList *iter;
	INT32 score;
	UINT32 move_cnt, cnt = 0;
	UINT64 n_moves_wh, move_b, move_w;
	UINT32 key;
	INT32 searched = depth;

	if (searched < 6 && g_limitDepth >= 16)
	{
		for (iter = movelist->next; iter != NULL; iter = iter->next)
		{
			move_b = bk ^ ((1ULL << (iter->move.pos)) | iter->move.rev);
			move_w = wh ^ (iter->move.rev);
			
			score = -AB_SearchNoPV(move_w, move_b, 6 - searched, empty - 1, color ^ 1,
				-NEGAMAX, -NEGAMIN, 0);
			iter->move.score = score;
		}
		/* 自分の得点の多い順にソート */
		sort_movelist_score_descending(movelist);
	}
	else if (searched == 6 && g_limitDepth >= 16)
	{
		for (iter = movelist->next; iter != NULL; iter = iter->next)
		{
			move_b = bk ^ ((1ULL << (iter->move.pos)) | iter->move.rev);
			move_w = wh ^ (iter->move.rev);

			InitIndexBoard(move_w, move_b);

			score = -Evaluation(g_board, move_w, move_b, color ^ 1, 59 - (empty - 1));
			iter->move.score = score;
		}
		/* 自分の得点の多い順にソート */
		sort_movelist_score_descending(movelist);
	}
	else
	{
		for (iter = movelist->next; iter != NULL; iter = iter->next)
		{
			move_b = bk ^ ((1ULL << (iter->move.pos)) | iter->move.rev);
			move_w = wh ^ (iter->move.rev);

			// 相手を全滅させる手か
			if (move_w == 0)
			{
				iter->move.score -= (1 << 30);
			}
			else
			{
				// Enhanced cut-off
				if (CheckTableCutOff(hash, &key, move_w, move_b, color ^ 1, empty - 1, -beta, -alpha, &score) ||
					CheckTableCutOff_PV(pvHash, &key, move_w, move_b, color ^ 1, empty - 1, -beta, -alpha, &score))
				{
					iter->move.score -= (1ULL << 29) - score;
				}
				else
				{
					score = 0;
					n_moves_wh = CreateMoves(move_w, move_b, &move_cnt);

					/* 位置得点 */
					//score -= posEval[iter->move.pos] << 2;
					//score += CountBit(iter->move.rev) << 4;
					/* 敵の着手可能数を取得 */
					score += ((move_cnt + CountBit(n_moves_wh & 0x8100000000000081)) << 2)
						- get_corner_stability(move_b);
					score += CountBit(GetPotentialMoves(move_w, move_b)) << 1;
					iter->move.score = score;
				}
			}
		}
		/* 敵の得点の少ない順にソート */
		sort_movelist_score_ascending(movelist);
	}

#else
	SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
#endif

}

#endif

#if 0
static void move_evaluate(
	Move *move,
	UINT64 bk, 
	UINT64 wh,
	HashTable *hash,
	HashTable *pvHash,
	HashInfo *hashInfo,
	INT32 empty,
	UINT32 parity,
	INT32 alpha, 
	INT32 beta,
	UINT32 color,
	INT32 *selectivity,
	INT32 sortDepth
)
{
	const int w_hash = 1 << 15;
	const int w_eval = 1 << 15;
	const int w_mobility = 1 << 15;
	const int w_corner_stability = 1 << 11;
	const int w_edge_stability = 1 << 11;
	const int w_potential_mobility = 1 << 5;
	const int w_low_parity = 1 << 3;
	const int w_mid_parity = 1 << 2;
	const int w_high_parity = 1 << 1;

	UINT64 move_b, move_w;
	INT32 key, pv_key;

	if (move->rev == wh) move->score = (1 << 30);
	else if (hashInfo && move->pos == hashInfo->bestmove) move->score = (1 << 29);
	// else if (move->pos == hash_dat) move->score = (1 << 28);
	else
	{
		move->score = SQUARE_VALUE[move->pos]; // square type
		if (empty < 12 && (parity & board_parity_bit[move->pos])) move->score += w_low_parity; // parity
		else if (empty < 21 && (parity & board_parity_bit[move->pos])) move->score += w_mid_parity; // parity
		else if (empty < 30 && (parity & board_parity_bit[move->pos])) move->score += w_high_parity; // parity

		if (sortDepth < 0)
		{
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;
			move->score += (36 - get_potential_mobility(move_w, move_b)) * w_potential_mobility; // potential mobility
			move->score += get_corner_stability(move_b) * w_corner_stability; // corner stability
			move->score += (36 - get_weighted_mobility(move_w, move_b)) * w_mobility; // real mobility
		}
		else
		{
			//INT32 l_selectivity = *selectivity;
			//*selectivity = g_max_cut_table_size;

			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;

			move->score += (36 - get_potential_mobility(move_w, move_b)) * w_potential_mobility; // potential mobility
			move->score += get_edge_stability(move_b, move_w) * w_edge_stability; // edge stability
			move->score += (36 - get_weighted_mobility(move_w, move_b)) *  w_mobility; // real mobility
			switch (sortDepth)
			{
			case 0:
				InitIndexBoard(move_w, move_b);
				move->score += ((64 - (Evaluation(g_board, move_w, move_b, color ^ 1, 59 - (empty - 1)) / EVAL_ONE_STONE)) >> 2) * w_eval; // 1 level score bonus
				break;
#if 0
			case 1:
				move->score += ((NEGAMAX - search_eval_1(search, NEGAMIN, -sort_alpha)) >> 1) * w_eval;  // 2 level score bonus
				break;
			case 2:
				move->score += ((NEGAMAX - search_eval_2(search, NEGAMIN, -sort_alpha)) >> 1) * w_eval;  // 3 level score bonus
				break;
#endif
			default:
				key = KEY_HASH_MACRO(move_w, move_b, color ^ 1);
				pv_key = KEY_HASH_MACRO_PV(move_w, move_b, color ^ 1);
				if (HashGet(hash, key, move_w, move_b) ||
					HashGet(pvHash, pv_key, move_w, move_b))
				{
					move->score += w_hash; // bonus if the position leads to a position stored in the hash-table
				}
				move->score += (64 - (AB_SearchNoPV(move_w, move_b, sortDepth, empty - 1, color ^ 1,
					-NEGAMAX, -NEGAMIN, 0) / EVAL_ONE_STONE)) * w_eval; // > 3 level bonus
				break;
			}
			//*selectivity = l_selectivity;
		}
	}
}

/**
* @brief スコア順に手を並び替える
*
* 重要な順に以下の要素を考慮
*   - 相手を全滅させる手か                              : 1 << 30
*   - 第一ハッシュテーブル(deepest)に登録されている手か : 1 << 29
*   - 第二ハッシュテーブル(newest)に登録されている手か  : 1 << 28
*   - 浅い探索による評価値                              : 1 << 14
*   - 相手の着手可能数                                  : 1 << 15
*   - 相手の４隅における安定度                          : 1 << 11
*   - 相手の潜在的着手可能数(開放度理論)                : 1 << 5
*/
void SortMoveList(
	MoveList *movelist,
	UINT64 bk,
	UINT64 wh,
	HashTable *hash,
	HashTable *pvHash,
	HashInfo *hashInfo,
	INT32 empty,
	UINT32 parity,
	INT32 alpha,
	INT32 beta,
	UINT32 color,
	INT32 *selectivity
)
{
	int depth;
	int sort_depth, min_depth;
	MoveList *iter;
	//int sort_alpha;

#if 0
	depth = g_empty - empty;
	min_depth = 9;
	if (empty <= 27) min_depth += (30 - empty) / 3;
	if (depth >= min_depth)
	{
		sort_depth = (depth - 15) / 3;
		if (hashInfo && hashInfo->upper < alpha) sort_depth -= 2;
		if (empty >= 27) ++sort_depth;
		if (sort_depth < 0) sort_depth = 0; else if (sort_depth > 6) sort_depth = 6;
	}
	else
	{
		sort_depth = -1;
	}
#else
	sort_depth = 6 - (g_empty - empty);
	if (sort_depth < 0) sort_depth = -1;
#endif
	//sort_alpha = MAX(NEGAMIN, alpha - SORT_ALPHA_DELTA);
	for(iter = movelist->next; iter->next != NULL; iter = iter->next)
	{
		move_evaluate(&iter->move, bk, wh, hash, pvHash, hashInfo, empty, parity, alpha, beta, color, selectivity, sort_depth);
	}

	/* 得点の高い順にソート */
	sort_movelist_score_descending(movelist);
}


#else

/**
* @brief スコア順に手を並び替える
*
* 重要な順に以下の要素を考慮
*   - 相手を全滅させる手か                              : 1 << 30
*   - 第一ハッシュテーブル(deepest)に登録されている手か : 1 << 29
*   - 第二ハッシュテーブル(newest)に登録されている手か  : 1 << 28
*   - 浅い探索による評価値                              : 1 << 14
*   - 相手の着手可能数                                  : 1 << 15
*   - 相手の４隅における安定度                          : 1 << 11
*   - 相手の潜在的着手可能数(開放度理論)                : 1 << 5
*/
void SortMoveListEnd(
	MoveList *movelist,
	UINT64 bk, UINT64 wh,
	HashTable *hash,
	UINT32 empty,
	INT32 alpha, INT32 beta,
	UINT32 color
)
{
	MoveList *iter;
	UINT64 blank, n_moves_wh;
	UINT64 move_b, move_w;
	UINT32 key, move_cnt;

	for (iter = movelist->next; iter != NULL; iter = iter->next)
	{
		iter->move.score = 0;
		move_b = bk ^ ((1ULL << iter->move.pos) | iter->move.rev);
		move_w = wh ^ iter->move.rev;
		key = KEY_HASH_MACRO(move_w, move_b, color ^ 1);

		// 相手を全滅させる手か
		if (move_w == 0) {
			iter->move.score += (1 << 30);
			continue;
		}
		// 第一ハッシュテーブル(deepest)に登録されている手か 
		else if (hash->entry[key].deepest.bk == move_w && 
			     hash->entry[key].deepest.wh == move_b)
		{
			iter->move.score += (1 << 29);
		}
		// 第二ハッシュテーブル(newest)に登録されている手か 
		else if (hash->entry[key].newest.bk == move_w &&
			     hash->entry[key].newest.wh == move_b)
		{
			iter->move.score += (1 << 28);
		}
		else
		{
			blank = ~(move_w | move_b);

			// 着手可能数を計算
			n_moves_wh = CreateMoves(move_w, move_b, &move_cnt);
			// 相手の着手可能数
			iter->move.score -= ((UINT64)move_cnt + CountBit(n_moves_wh & 0x8100000000000081ULL)) * (1ULL << 4);
			// 自分の４隅における安定度
			iter->move.score -= (get_edge_stability(move_w, move_b)) * (1ULL << 2);
			// 相手の潜在的着手可能数(開放度理論)
			iter->move.score -= (CountBit(GetPotentialMoves(move_w, move_b))) * (1ULL << 2);
		}
		
	}

	/* 得点の高い順にソート */
	sort_movelist_score_descending(movelist);
}

#endif

/*  終盤用 move ordering */
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
		/* 反転データ取得 */
		rev = GetRev[pos](b_board, w_board);

		move_b = b_board ^ ((1ULL << pos) | rev);
		move_w = w_board^rev;

		ligal_move_w = CreateMoves(move_w, move_b, &move_cnt);

		/* 敵に隅を明け渡さない */
		score = CountBit(ligal_move_w & 0x8100000000000081) << 1;
		/* 適切な星Ｃ打ちの可能性大の場合 */
		if ((1ULL << pos) & 0x42C300000000C342){
			if (score == 0)
			{
				score--;
			}
		}
		/*if(0x8100000000000081 & (one << pos)){
		score -= 4;
		}*/

		/* 敵の着手可能数を取得 */
		score += (move_cnt << 5);
		score += get_corner_stability(move_w) - get_corner_stability(move_b);

		score_list[cnt] = score;
		pos_list[cnt].move = (char)pos;
		pos_list[cnt].rev = rev;
		cnt++;
		moves = moves & (moves - 1);
	} while (moves);

	/* 敵の得点の少ない順にソート */
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