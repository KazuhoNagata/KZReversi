/***************************************************************************
* Name  : cpu.cpp
* Brief : 探索の処理全般を行う
* Date  : 2016/02/01
****************************************************************************/
#include "stdafx.h"
#include "const.h"
#include "bit64.h"
#include "board.h"
#include "move.h"
#include "rev.h"
#include "search.h"
#include "cpu.h"
#include "ybwc.h"
#include "cpu_root.h"
#include "endgame.h"
#include "hash.h"
#include "eval.h"
#include "ordering.h"

#include "count_last_flip_carry_64.h"

#include <stdio.h>


/***************************************************************************
*
* Global
*
****************************************************************************/

static INT32 inc_sort_depth[] = { 0, -2, -3 };

/***************************************************************************
*
* ProtoType(private)
*
****************************************************************************/


/**
	* @brief evaluate a midgame position with the evaluation function.
	*
	* @param search Position to evaluate.
	*/
int search_eval_0(Search *search)
{
	int score;
	INT32 stage = 60 - search->n_empties;
	INT32 color = search->player;

	//SEARCH_STATS(++statistics.n_search_eval_0);
	++search->n_nodes;

	hori_ver1 = hori_ver1_data[color][stage];
	hori_ver2 = hori_ver2_data[color][stage];
	hori_ver3 = hori_ver3_data[color][stage];
	dia_ver1 = dia_ver1_data[color][stage];
	dia_ver2 = dia_ver2_data[color][stage];
	dia_ver3 = dia_ver3_data[color][stage];
	dia_ver4 = dia_ver4_data[color][stage];
	edge = edge_data[color][stage];
	corner5_2 = corner5_2_data[color][stage];
	corner3_3 = corner3_3_data[color][stage];
	triangle = triangle_data[color][stage];
	constant = constant_data[color][stage];

	score = check_h_ver1(search->eval);
	score += check_h_ver2(search->eval);
	score += check_h_ver3(search->eval);

	score += check_dia_ver1(search->eval);
	score += check_dia_ver2(search->eval);
	score += check_dia_ver3(search->eval);
	score += check_dia_ver4(search->eval);

	score += check_edge(search->eval);
	score += check_corner5_2(search->eval);
	score += check_corner3_3(search->eval);
	score += check_triangle(search->eval);

	score += constant[0];

	if (score > 0) score += 512;
	else score -= 512;
	score /= 1024;

	if (score <= SCORE_MIN) score = SCORE_MIN + 1;
	else if (score >= SCORE_MAX) score = SCORE_MAX - 1;

	return score;
}

/**
	* @brief Evaluate a position at depth 1.
	*
	* As an optimization, the last move is used to only updates the evaluation
	* features.
	*
	* @param search Position to evaluate.
	* @param alpha Alpha bound.
	* @param beta Beta bound.
	*/
int search_eval_1(Search *search, const int alpha, int beta)
{
	const short *w = EVAL_WEIGHT[search->eval->player ^ 1][61 - search->n_empties];
	Move move[1];
	SquareList *empty;
	register int score, bestscore;
	const Board *board = search->board;
	Eval *eval = search->eval;
	unsigned long long moves = get_moves(board->player, board->opponent);
	int *f;

	//SEARCH_STATS(++statistics.n_search_eval_1);
	++search->n_nodes;

	if (moves) {
		bestscore = -SCORE_INF;
		if (beta >= SCORE_MAX) beta = SCORE_MAX - 1;
		foreach_empty(empty, search->empties) {
			if (moves & empty->b) {
				board_get_move(board, empty->x, move);
				if (move_wipeout(move, board)) return SCORE_MAX;
				eval_update(eval, move);
				f = eval->feature;
				SEARCH_UPDATE_EVAL_NODES();
				score = -w[f[0]] - w[f[1]] - w[f[2]] - w[f[3]]
					- w[f[4]] - w[f[5]] - w[f[6]] - w[f[7]]
					- w[f[8]] - w[f[9]] - w[f[10]] - w[f[11]]
					- w[f[12]] - w[f[13]] - w[f[14]] - w[f[15]]
					- w[f[16]] - w[f[17]] - w[f[18]] - w[f[19]]
					- w[f[20]] - w[f[21]] - w[f[22]] - w[f[23]]
					- w[f[24]] - w[f[25]]
					- w[f[26]] - w[f[27]] - w[f[28]] - w[f[29]]
					- w[f[30]] - w[f[31]] - w[f[32]] - w[f[33]]
					- w[f[34]] - w[f[35]] - w[f[36]] - w[f[37]]
					- w[f[38]] - w[f[39]] - w[f[40]] - w[f[41]]
					- w[f[42]] - w[f[43]] - w[f[44]] - w[f[45]]
					- w[f[46]];
				eval_restore(eval, move);

				if (score > 0) score += 64; else score -= 64;
				score /= 128;

				if (score > bestscore)
				{
					bestscore = score;
					if (bestscore >= beta) break;
				}
			}
		}
		if (bestscore <= SCORE_MIN) bestscore = SCORE_MIN + 1;
		else if (bestscore >= SCORE_MAX) bestscore = SCORE_MAX - 1;
	}
	else {
		if (can_move(board->opponent, board->player)) {
			search_update_pass_midgame(search);
			bestscore = -search_eval_1(search, -beta, -alpha);
			search_restore_pass_midgame(search);
		}
		else { // game over
			bestscore = search_solve(search);
		}
	}

	//assert(SCORE_MIN <= bestscore && bestscore <= SCORE_MAX);
	return bestscore;
}

/**
	* @brief Evaluate a position at depth 2.
	*
	* Simple alpha-beta with no move sorting.
	*
	* @param search Position to evaluate.
	* @param alpha Lower bound
	* @param beta  Upper bound
	*/
int search_eval_2(Search *search, int alpha, const int beta)
{
	register int bestscore, score;
	SquareList *empty;
	Move move[1];
	const Board *board = search->board;
	const unsigned long long moves = get_moves(board->player, board->opponent);

	//SEARCH_STATS(++statistics.n_search_eval_2);
	++search->n_nodes;

	//assert(-SCORE_MAX <= alpha && alpha <= SCORE_MAX);
	//assert(-SCORE_MAX <= beta && beta <= SCORE_MAX);
	//assert(alpha <= beta);

	if (moves) {
		bestscore = -SCORE_INF;
		foreach_empty(empty, search->empties)
		{
			if (moves & empty->b)
			{
				board_get_move(board, empty->x, move);
				search_update_midgame(search, move);
				score = -search_eval_1(search, -beta, -alpha);
				search_restore_midgame(search, move);
				if (score > bestscore)
				{
					bestscore = score;
					if (bestscore >= beta) break;
					else if (bestscore > alpha) alpha = bestscore;
				}
			}
		}
	}
	else
	{
		if (can_move(board->opponent, board->player))
		{
			search_update_pass_midgame(search);
			bestscore = -search_eval_2(search, -beta, -alpha);
			search_restore_pass_midgame(search);
		}
		else
		{
			bestscore = search_solve(search);
		}
	}

	//assert(SCORE_MIN <= bestscore && bestscore <= SCORE_MAX);

	return bestscore;
}

static inline void search_update_probcut(Search *search, const NodeType node_type)
{
	search->node_type[search->height] = node_type;
	search->selectivity = NO_SELECTIVITY;
	++search->probcut_level;
}


static inline void search_restore_probcut(Search *search, const  NodeType node_type, const int selectivity)
{
	search->node_type[search->height] = node_type;
	search->selectivity = selectivity;
	--search->probcut_level;
}

/**
 * @brief Probcut
 *
 * Check if a position is worth to analyze further.
 *
 * @param search Position to test.
 * @param alpha Alpha lower bound.
 * @param depth Search depth.
 * @param parent Parent node.
 * @param value Returned value.
 *
 * @return true if probable cutoff has been found, false otherwise.
 */
static bool search_probcut(Search *search, const int alpha, const int depth, Node *parent, int *value)
{
	MPCINFO *mpcInfo_p;
	INT32 value;
	INT32 eval;
	const INT32 beta = alpha + 1;
	const int saved_selectivity = search->selectivity;
	const NodeType node_type = search->node_type[search->height];

	// do probcut ?
	if (g_mpcFlag && depth >= MPC_MIN_DEPTH && search->selectivity < NO_SELECTIVITY && search->probcut_level < 2)
	{
		double mpc_value;
		if (search->n_empties <= 24)
		{
			mpc_value = cutval_table[4];
		}
		else if (search->n_empties <= 36)
		{
			mpc_value = cutval_table[3];
		}
		else
		{
			mpc_value = cutval_table[3];
		}

		mpcInfo_p = &mpcInfo[depth - MPC_MIN_DEPTH];
		value = (INT32)(alpha - (mpcInfo_p->deviation * mpc_value) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		search_update_probcut(search, CUT_NODE);
		eval = NWS_SearchMid(search, value, mpcInfo_p->depth, parent);
		search_restore_probcut(search, node_type, saved_selectivity);
		if (eval <= value)
		{
			*value = alpha;
			return true;
		}

		value = (INT32)(beta + (mpcInfo_p->deviation * mpc_value) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		search_update_probcut(search, CUT_NODE);
		eval = NWS_SearchMid(search, value, mpcInfo_p->depth, parent);
		search_restore_probcut(search, node_type, saved_selectivity);
		if (eval >= value)
		{
			*value = alpha;
			return true;
		}

	}

	return false;
}

/**
 * @brief Evaluate a midgame position with a Null Window Search algorithm.
 *
 * This function is used when there are still many empty squares on the board. Move
 * ordering, hash table cutoff, enhanced transposition cutoff, etc. are used in
 * order to diminish the size of the tree to analyse, but at the expense of a
 * slower speed.
 *
 * @param search     search.
 * @param alpha      lower bound.
 * @param depth Search remaining depth.
 * @param hash_table Hash Table to use.
 * @return An evaluated score, as a disc difference.
 */
int NWS_SearchShallow(Search *search, const int alpha, int depth, HashTable *hash_table)
{
	int score;
	unsigned long long hash_code;
	const int beta = alpha + 1;
	HashData hash_data[1];
	Board *board = search->board;
	MoveList movelist[1];
	Move *move;
	int bestscore, bestmove;
	long long cost = -search->n_nodes;

	if (depth == 2) return search_eval_2(search, alpha, beta);

	//SEARCH_STATS(++statistics.n_NWS_midgame);
	++search->n_nodes;

	//assert(search->n_empties == bit_count(~(search->board->player | search->board->opponent)));
	//assert(SCORE_MIN <= alpha && alpha <= SCORE_MAX);
	//assert(depth > 2);
	//assert(hash_table != NULL);

	// stability cutoff
	if (search_SC_NWS(search, alpha, &score)) return score;

	// transposition cutoff
	hash_code = board_get_hash_code(board);
	if (hash_get(hash_table, board, hash_code, hash_data) && search_TC_NWS(hash_data, depth, search->selectivity, alpha, &score)) return score;
	search_get_movelist(search, movelist);

	if (movelist_is_empty(movelist)) { // no moves ?
		if (can_move(board->opponent, board->player)) { // pass ?
			search_update_pass_midgame(search);
			bestscore = -NWS_SearchShallow(search, -beta, depth, hash_table);
			bestmove = PASS;
			search_restore_pass_midgame(search);
		}
		else { // game-over !
			bestscore = search_solve(search);
			bestmove = NOMOVE;
		}
	}
	else {
		// sort the list of moves
		movelist_evaluate(movelist, search, hash_data, alpha, depth);
		movelist_sort(movelist);

		// loop over all moves
		bestscore = -SCORE_INF; bestmove = NOMOVE;
		foreach_move(move, movelist) {
			search_update_midgame(search, move);
			score = -NWS_SearchShallow(search, -beta, depth - 1, hash_table);
			search_restore_midgame(search, move);
			if (score > bestscore) {
				bestscore = score;
				bestmove = move->x;
				if (bestscore >= beta) break;
			}
		}
	}

	// save the best result in hash tables
	cost += search->n_nodes;
	hash_store(hash_table, board, hash_code, depth, search->selectivity, last_bit(cost), alpha, beta, bestscore, bestmove);
	//assert(SCORE_MIN <= bestscore && bestscore <= SCORE_MAX);

	return bestscore;
}

/**
 * @brief Evaluate a midgame position at shallow depth.
 *
 * This function is used when there are still many empty squares on the board. Move
 * ordering, hash table cutoff, enhanced transposition cutoff, etc. are used in
 * order to diminish the size of the tree to analyse, but at the expense of a
 * slower speed.
 *
 * @param search Search.
 * @param alpha Alpha bound.
 * @param beta Beta bound.
 * @param depth Search depth.
 * @return An evaluated score, as a disc difference.
 */
int PVS_SearchShallow(Search *search, int alpha, int beta, int depth)
{
	int score;
	HashTable *hash_table = search->shallow_table;
	unsigned long long hash_code;
	HashData hash_data[1];
	Board *board = search->board;
	MoveList movelist[1];
	Move *move;
	int bestscore, bestmove;
	long long cost = -search->n_nodes;
	int lower;

	if (depth == 2) return search_eval_2(search, alpha, beta);

	//SEARCH_STATS(++statistics.n_PVS_shallow);
	++search->n_nodes;

	//assert(search->n_empties == bit_count(~(search->board->player | search->board->opponent)));
	//assert(SCORE_MIN <= alpha && alpha <= SCORE_MAX);

	// stability cutoff
	if (search_SC_PVS(search, &alpha, &beta, &score)) return score;

	// transposition cutoff
	hash_code = board_get_hash_code(board);
	//	if (hash_get(hash_table, board, hash_code, hash_data) && search_TC_PVS(hash_data, depth, search->selectivity, &alpha, &beta, &score)) return score;
	hash_get(hash_table, board, hash_code, hash_data);

	search_get_movelist(search, movelist);

	if (movelist_is_empty(movelist)) { // no moves ?
		if (can_move(board->opponent, board->player)) { // pass ?
			search_update_pass_midgame(search);
			bestscore = -PVS_shallow(search, -beta, -alpha, depth);
			bestmove = PASS;
			search_restore_pass_midgame(search);
		}
		else { // game-over !
			bestscore = search_solve(search);
			bestmove = NOMOVE;
		}
	}
	else {
		// sort the list of moves
		movelist_evaluate(movelist, search, hash_data, alpha, depth);
		movelist_sort(movelist);

		// loop over all moves
		bestscore = -SCORE_INF; bestmove = NOMOVE;
		lower = alpha;
		foreach_move(move, movelist) {
			search_update_midgame(search, move);
			if (bestscore == -SCORE_INF) {
				score = -PVS_shallow(search, -beta, -lower, depth - 1);
			}
			else {
				score = -NWS_shallow(search, -lower - 1, depth - 1, hash_table);
				if (alpha < score && score < beta) {
					score = -PVS_shallow(search, -beta, -lower, depth - 1);
				}
			}
			search_restore_midgame(search, move);
			if (score > bestscore) {
				bestscore = score;
				bestmove = move->x;
				if (bestscore >= beta) break;
				else if (bestscore > lower) lower = bestscore;
			}
		}
	}

	// save the best result in hash tables
	cost += search->n_nodes;
	hash_store(hash_table, board, hash_code, depth, search->selectivity, last_bit(cost), alpha, beta, bestscore, bestmove);
	//assert(SCORE_MIN <= bestscore && bestscore <= SCORE_MAX);

	return bestscore;
}


/**
 * @brief Evaluate a midgame position with a Null Window Search algorithm.
 *
 * This function is used when there are still many empty squares on the board. Move
 * ordering, hash table cutoff, enhanced transposition cutoff, etc. are used in
 * order to diminish the size of the tree to analyse, but at the expense of a
 * slower speed.
 *
 * @param search Search.
 * @param alpha Alpha bound.
 * @param depth Depth.
 * @param parent Parent node.
 * @return A score, as a disc difference.
 */
INT32 NWS_SearchMid(Search *search, const INT32 alpha, INT32 depth, Node *parent)
{
	INT32 score;
	HashTable *hash_table = search->hash_table;
	HashTable *pv_table = search->pv_table;
	UINT64 hash_code;
	const INT32 beta = alpha + 1;
	HashData hash_data[1];
	Board *board = search->board;
	MoveList movelist[1];
	Move *move;
	Node node[1];
	long long cost = -search->n_nodes - search->child_nodes;
	int hash_selectivity;

	//search_check_timeout(search);

	if (search->stop) return alpha;
	else if (search->n_empties == 0) return search_solve_0(search);
	else if (depth <= 3 && depth < search->n_empties) return NWS_SearchShallow(search, alpha, depth, hash_table);
	else if (search->n_empties <= depth && depth < DEPTH_MIDGAME_TO_ENDGAME) return NWS_SearchEnd(search, alpha);

	++search->n_nodes;

	// stability cutoff
	if (search_SC_NWS(search, alpha, &score)) return score;

	// transposition cutoff
	hash_code = board_get_hash_code(board);
	if ((hash_get(hash_table, board, hash_code, hash_data) || 
		 hash_get(pv_table, board, hash_code, hash_data)) && 
		 search_TC_NWS(hash_data, depth, search->selectivity, alpha, &score)) return score;

	search_get_movelist(search, movelist);

	if (movelist->n_moves == 0) { // no moves ?
		node_init(node, search, alpha, beta, depth, movelist->n_moves, parent);
		if (can_move(board->opponent, board->player)) { // pass ?
			search_update_pass_midgame(search);
			node->bestscore = -NWS_midgame(search, -node->beta, depth, node);
			search_restore_pass_midgame(search);
		}
		else { // game-over !
			node->bestscore = search_solve(search);
		}
	}
	else {
		// probcut
		if (search_probcut(search, alpha, depth, parent, &score)) return score;

		// sort the list of moves
		if (movelist->n_moves > 1)
		{
			if (hash_data->move[0] == NOMOVE) hash_get(hash_table, board, hash_code, hash_data);
			movelist_evaluate(movelist, search, hash_data, alpha, depth + inc_sort_depth[search->node_type[search->height]]);
			movelist_sort(movelist);
		}

		// ETC
		if (search_ETC_NWS(search, movelist, hash_code, depth, search->selectivity, alpha, &score)) return score;

		node_init(node, search, alpha, beta, depth, movelist->n_moves, parent);

		// loop over all moves
		for (move = node_first_move(node, movelist); move; move = node_next_move(node))
		{
			if (!node_split(node, move))
			{
				search_update_midgame(search, move);
				move->score = -NWS_SearchMid(search, -beta, depth - 1, node);
				search_restore_midgame(search, move);
				node_update(node, move);
			}
		}
		node_wait_slaves(node);
	}

	// save the best result in hash tables
	if (!search->stop)
	{
		cost += search->n_nodes + search->child_nodes;
		if (search->n_empties < depth && depth <= DEPTH_MIDGAME_TO_ENDGAME) hash_selectivity = NO_SELECTIVITY; // hack
		else hash_selectivity = search->selectivity;
		if (search->height <= PV_HASH_HEIGHT) hash_store(pv_table, board, hash_code, depth, hash_selectivity, last_bit(cost), alpha, beta, node->bestscore, node->bestmove);
		hash_store(hash_table, board, hash_code, depth, hash_selectivity, last_bit(cost), alpha, beta, node->bestscore, node->bestmove);
	}
	else {
		node->bestscore = alpha;
	}

	node_free(node);

	return node->bestscore;
}


/***************************************************************************
* Name  : PVS_SearchDeep
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
INT32 PVS_SearchMid(Search *search, INT32 depth, INT32 alpha, INT32 beta, Node *parent)
{
	HashTable *hash_table = search->hash_table;
	HashTable *pv_table = search->pv_table;
	UINT64     hash_code;
	HashData   hash_data[1];
	Board     *board = search->board;
	MoveList   movelist[1];
	Move      *move;
	Node       node[1];
	long long  cost;
	INT32      reduced_depth, depth_pv_extension, saved_selectivity;
	INT32      hash_selectivity;

	/* アボート処理 */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	if (search->stop) return alpha;
	else if (search->n_empties == 0) return search_solve_0(search);
	else if (depth < search->n_empties && search->n_empties <= search->depth_pv_extension)
	{
		return PVS_SearchMid(search, alpha, beta, search->n_empties, parent);
	}
	else if (depth == 2 && search->n_empties > 2)
	{
		return search_eval_2(search, alpha, beta);
	}

	cost = -search_count_nodes(search);
	++search->n_nodes;

	search_get_movelist(search, movelist);
	node_init(node, search, alpha, beta, depth, movelist->n_moves, parent);
	node->pv_node = true;
	hash_code = board_get_hash_code(board);

	if (movelist->n_moves == 0)
	{
		if (can_move(board->opponent, board->player))
		{
			search_update_pass_midgame(search); search->node_type[search->height] = PV_NODE;
			node->bestscore = -PVS_SearchMid(search, -beta, -alpha, depth, node);
			search_restore_pass_midgame(search);
			node->bestmove = PASS;
		}
		else
		{
			node->alpha = -(node->beta = +SCORE_INF);
			node->bestscore = search_solve(search);
			node->bestmove = NOMOVE;
		}
	}
	else 
	{ // normal PVS
		if (movelist->n_moves > 1)
		{
			//IID
			if (!hash_get(pv_table, board, hash_code, hash_data)) hash_get(hash_table, board, hash_code, hash_data);
			if (hash_data->move[0] == NOMOVE)
			{
				if (depth == search->n_empties) reduced_depth = depth - ITERATIVE_MIN_EMPTIES;
				else reduced_depth = depth - 2;
				if (reduced_depth >= 3)
				{
					saved_selectivity = search->selectivity; search->selectivity = 0;
					depth_pv_extension = search->depth_pv_extension;
					search->depth_pv_extension = 0;
					PVS_SearchMid(search, SCORE_MIN, SCORE_MAX, reduced_depth, parent);
					hash_get(pv_table, board, hash_code, hash_data);
					search->depth_pv_extension = depth_pv_extension;
					search->selectivity = saved_selectivity;
				}
			}

			// Evaluate moves for sorting. For a better ordering, the depth is artificially increased
			movelist_evaluate(movelist, search, hash_data, node->alpha, depth + inc_sort_depth[PV_NODE]);
			movelist_sort(movelist);
		}

		// first move
		if ((move = node_first_move(node, movelist)))
		{
			search_update_midgame(search, move); search->node_type[search->height] = PV_NODE;
			move->score = -PVS_SearchMid(search, -beta, -alpha, depth - 1, node);
			search_restore_midgame(search, move);
			node_update(node, move);

			// other moves : try to refute the first/best one
			while ((move = node_next_move(node)))
			{
				if (!node_split(node, move))
				{
					const int alpha = node->alpha;
					search_update_midgame(search, move);
					move->score = -NWS_SearchMid(search, -alpha - 1, depth - 1, node);
					if (!search->stop && alpha < move->score && move->score < beta)
					{
						search->node_type[search->height] = PV_NODE;
						move->score = -PVS_SearchMid(search, -beta, -alpha, depth - 1, node);
					}
					search_restore_midgame(search, move);
					node_update(node, move);
				}
			}
			node_wait_slaves(node);
		}
	}

	// save the best result in hash tables
	if (!search->stop) {
		cost += search_count_nodes(search);
		if (search->n_empties < depth && depth <= DEPTH_MIDGAME_TO_ENDGAME) hash_selectivity = NO_SELECTIVITY;
		else hash_selectivity = search->selectivity;
		hash_store(hash_table, board, hash_code, depth, hash_selectivity, last_bit(cost), alpha, beta, node->bestscore, node->bestmove);
		hash_store(pv_table, board, hash_code, depth, hash_selectivity, last_bit(cost), alpha, beta, node->bestscore, node->bestmove);

		//SQUARE_STATS(foreach_move(move, movelist))
			//SQUARE_STATS(++statistics.n_played_square[search->n_empties][SQUARE_TYPE[move->x]];)
			//SQUARE_STATS(if (node->bestscore > alpha) ++statistics.n_good_square[search->n_empties][SQUARE_TYPE[node->bestmove]];)

			//assert(SCORE_MIN <= node->bestscore && node->bestscore <= SCORE_MAX);

	}
	else {
		node->bestscore = alpha;
	}

	node_free(node);

	return node->bestscore;
}

