/***************************************************************************
* Name  : cpu.cpp
* Brief : 探索の処理全般を行う
* Date  : 2016/02/01
****************************************************************************/
#include "stdafx.h"
#include "bit64.h"
#include "board.h"
#include "search.h"
#include "move.h"
#include "rev.h"
#include "cpu.h"
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
// CPU設定格納用
BOOL g_mpcFlag;
BOOL g_tableFlag;
INT32 g_solveMethod;
INT32 g_empty;
INT32 g_limitDepth;
UINT64 g_casheSize;
UINT64 g_pvCasheSize;
BOOL g_refresh_hash_flag = TRUE;
INT32 g_key;

// CPU AI情報
BOOL g_AbortFlag;
UINT64 g_countNode;
INT32 g_move;
INT32 g_color;
INT32 g_infscore;

HashTable *g_hash;
HashTable *g_pvHash;
HashTable *g_shHash;

// endgame mpc info
const double cutval_table[8] =
{
	0.25, 0.50, 0.74, 1.00, 1.48, 1.96, 2.33, 2.58
};

const int cutval_table_percent[8 + 1] =
{
	20, 38, 54, 68, 86, 95, 98, 99, 100
};
const INT32 g_max_cut_table_size = 8;

char g_cordinates_table[64][4];
char g_AiMsg[128];
char g_PVLineMsg[256];
SetMessageToGUI g_set_message_funcptr[3];

INT32 g_pvline[64];
INT32 g_pvline_len;
UINT64 g_pvline_board[2][60];


MPCINFO mpcInfo[22];
MPCINFO mpcInfo_end[26];
double MPC_CUT_VAL;
double MPC_END_CUT_VAL;
INT32 g_mpc_level;


/***************************************************************************
*
* ProtoType(private)
*
****************************************************************************/
INT32 SearchMiddle(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);


/**
 * @brief Record best move.
 *
 * @param search Search.
 * @param init_board Initial board.
 * @param bestmove Best move.
 * @param alpha Alpha Bound.
 * @param beta Beta Bound.
 * @param depth Depth.
 */
void record_best_move(Search *search, const Board *init_board, const Move *bestmove, const int alpha, const int beta, const int depth)
{
	Board board[1];
	Move move[1];
	Result *result = search->result;
	int x;
	unsigned long long hash_code;
	HashData hash_data[1];
	bool has_changed;
	Bound *bound = result->bound + bestmove->x;
	bool fail_low;
	bool guess_pv;
	int expected_depth, expected_selectivity, tmp;
	Bound expected_bound;

	*board = *init_board;

	spin_lock(result);

	has_changed = (result->move != bestmove->x || result->depth != depth || result->selectivity != search->selectivity);

	result->move = bestmove->x;
	result->score = bestmove->score;

	assert(search->stability_bound.lower <= result->score && result->score <= search->stability_bound.upper);

	if (result->score < beta && result->score < bound->upper) bound->upper = result->score;
	if (result->score > alpha && result->score > bound->lower) bound->lower = result->score;
	if (bound->lower > bound->upper) {
		if (result->score < beta) bound->upper = result->score; else bound->upper = search->stability_bound.upper;
		if (result->score > alpha) bound->lower = result->score; else bound->lower = search->stability_bound.lower;
	}

	expected_depth = result->depth = depth;
	expected_selectivity = result->selectivity = search->selectivity;
	expected_bound = *bound;

	line_init(result->pv, search->player);
	x = bestmove->x;

	guess_pv = (search->options.guess_pv && depth == search->n_empties && (bestmove->score <= alpha || bestmove->score >= beta));
	fail_low = (bestmove->score <= alpha);

	while (x != NOMOVE) {
		board_get_move(board, x, move);
		if (board_check_move(board, move)) {
			board_update(board, move);
			--expected_depth;
			tmp = expected_bound.upper; expected_bound.upper = -expected_bound.lower; expected_bound.lower = -tmp;
			fail_low = !fail_low;
			line_push(result->pv, move->x);

			hash_code = board_get_hash_code(board);
			if ((hash_get(search->pv_table, board, hash_code, hash_data) || hash_get(search->hash_table, board, hash_code, hash_data))
				&& (hash_data->depth >= expected_depth && hash_data->selectivity >= expected_selectivity)
				&& (hash_data->upper <= expected_bound.upper && hash_data->lower >= expected_bound.lower)) {
				x = hash_data->move[0];
			}
			else x = NOMOVE;
			if (guess_pv && x == NOMOVE && fail_low) x = guess_move(search, board);
		}
		else x = NOMOVE;
	}

	result->time = search_time(search);
	result->n_nodes = search_count_nodes(search);

	spin_unlock(result);

	//if (has_changed && options.noise <= depth && search->options.verbosity == 3) search->observer(search->result);
}


/***************************************************************************
* Name  : CreateCpuMessage
* Brief : CPUの着手情報をＵＩに送信する
* Args  : msg    : メッセージ格納先
*         wh     : メッセージ格納先の大きさ
*         eval   : 評価値
*         move   : 着手番号
*         cnt    : 深さ
*         flag   : middle or winloss or exact
* Return: 着手可能位置のビット列
****************************************************************************/
void CreateCpuMessage(char *msg, int msglen, int eval, int move, int cnt, int flag)
{
	if (flag == ON_MIDDLE) {
		if (eval > 1280000)
		{
			sprintf_s(msg, msglen, "%s[WIN:%+d](depth = %d)", g_cordinates_table[move],
				eval - 1280000, cnt);
		}
		else if (eval < -1280000)
		{
			sprintf_s(msg, msglen, "%s[LOSS:%d](depth = %d)", g_cordinates_table[move],
				eval + 1280000, cnt);
		}
		else if (eval == 1280000)
		{
			sprintf_s(msg, msglen, "%s[DRAW](depth = %d)", g_cordinates_table[move], cnt);
		}
		else if (eval >= 0)
		{
			sprintf_s(msg, msglen, "%s[%.3f](depth = %d)", g_cordinates_table[move],
				eval / (double)EVAL_ONE_STONE, cnt);
		}
		else
		{
			sprintf_s(msg, msglen, "%s[%.3f](depth = %d)", g_cordinates_table[move],
				eval / (double)EVAL_ONE_STONE, cnt);
		}
	}
	else if (flag == ON_WLD)
	{
		if (cnt == -1)
		{
			if (eval == WIN)
			{
				sprintf_s(msg, msglen, "guess... move %s [ WIN? ]", g_cordinates_table[move]);
			}
			else if (eval == LOSS)
			{
				sprintf_s(msg, msglen, "guess... move %s [ LOSS? ]", g_cordinates_table[move]);
			}
			else
			{
				sprintf_s(msg, msglen, "guess... move %s [ DRAW? ]", g_cordinates_table[move]);
			}
		}
		else
		{
			if (eval == WIN)
			{
				sprintf_s(msg, msglen, "%s [ WIN ]", g_cordinates_table[move]);
			}
			else if (eval == LOSS)
			{
				sprintf_s(msg, msglen, "%s [ LOSS ]", g_cordinates_table[move]);
			}
			else
			{
				sprintf_s(msg, msglen, "%s [ DRAW ]", g_cordinates_table[move]);
			}
		}

	}
	else
	{
		if (cnt == -2)
		{
			sprintf_s(msg, msglen, "guess... move %s [ %+d～%+d ]", g_cordinates_table[move], eval - 1, eval + 1);
		}
		else if (cnt == -1)
		{
			if (eval > 0)
			{
				sprintf_s(msg, msglen, "guess... move %s [ WIN:%+d? ]", g_cordinates_table[move], eval);
			}
			else if (eval < 0)
			{
				sprintf_s(msg, msglen, "guess... move %s [ LOSS:%d? ]", g_cordinates_table[move], eval);
			}
			else
			{
				sprintf_s(msg, msglen, "guess... move %s [ DRAW:+0? ]", g_cordinates_table[move]);
			}
		}
		else
		{
			if (eval > 0)
			{
				sprintf_s(msg, msglen, "%s [ WIN:%+d ]", g_cordinates_table[move], eval);
			}
			else if (eval < 0)
			{
				sprintf_s(msg, msglen, "%s [ LOSS:%d ]", g_cordinates_table[move], eval);
			}
			else
			{
				sprintf_s(msg, msglen, "%s [ DRAW:+0 ]", g_cordinates_table[move]);
			}
		}
	}
}



void CreatePVLineStr(PVLINE *pline, INT32 empty, INT32 score)
{
	char *strptr = g_PVLineMsg;
	int bufsize = sizeof(g_PVLineMsg);

	// header
	int base;

	if (g_solveMethod == SOLVE_WLD)
	{
		char *wldstr[] = { "LOSS", "DRAW", "WIN" };
		base = sprintf_s(strptr, bufsize, "depth %d/%d@%s PV-Line : ", empty, g_empty, wldstr[score + 1]);
	}
	else if (g_solveMethod == SOLVE_EXACT)
	{
		base = sprintf_s(strptr, bufsize, "depth %d/%d@%+d PV-Line : ", empty, g_empty, score);
	}
	else
	{
		base = sprintf_s(strptr, bufsize, "depth %d/%d@%+.3f PV-Line : ",
			g_limitDepth - (g_empty - empty), g_limitDepth, score / (double)EVAL_ONE_STONE);
	}

	strptr += base;
	bufsize -= base;

	int i;
	// PV shallower line
	for (i = 0; i <= g_empty - empty; i++)
	{
		if (g_pvline[i] != NOMOVE)
		{
			sprintf_s(strptr, bufsize, "%s-", g_cordinates_table[g_pvline[i]]);
			strptr += 3; // 3文字ずつずらす
			bufsize -= 3;
		}
	}

	// PV deeper line
	int count = pline->cmove;
	for (; i < count - 1; i++)
	{
		if (g_pvline[i] != NOMOVE)
		{
			sprintf_s(strptr, bufsize, "%s-", g_cordinates_table[pline->argmove[i]]);
			strptr += 3; // 3文字ずつずらす
			bufsize -= 3;
		}
	}

	sprintf_s(strptr, bufsize, "%s", g_cordinates_table[pline->argmove[i]]);
}



void CreatePVLineStrAscii(INT32 *pline, INT32 empty, INT32 score)
{
	char *strptr = g_PVLineMsg;
	int bufsize = sizeof(g_PVLineMsg);

	// header
	int base;

	if (g_solveMethod == SOLVE_WLD)
	{
		char *wldstr[] = { "LOSS", "DRAW", "WIN" };
		base = sprintf_s(strptr, bufsize, "depth %d/%d@%s PV-Line : ", empty, g_empty, wldstr[score + 1]);
	}
	else if (g_solveMethod == SOLVE_EXACT)
	{
		base = sprintf_s(strptr, bufsize, "depth %d/%d@%+d PV-Line : ", empty, g_empty, score);
	}
	else
	{
		base = sprintf_s(strptr, bufsize, "depth %d/%d@%+.3f PV-Line : ",
			g_limitDepth - (g_empty - empty), g_limitDepth, score / (double)EVAL_ONE_STONE);
	}

	strptr += base;
	bufsize -= base;

	int i;
	// PV line from global
	for (i = 0; i < empty - 1; i++)
	{
		sprintf_s(strptr, bufsize, "%s-", g_cordinates_table[pline[i]]);
		strptr += 3; // 3文字ずつずらす
		bufsize -= 3;
	}

	sprintf_s(strptr, bufsize, "%s", g_cordinates_table[pline[i]]);
}


INT32 GetMoveFromHash(Board *board)
{
	INT32 move;
	HashData hashData;

	if (hash_get(g_pvHash, board, board_get_hash_code(board), &hashData)) move = hashData.move[0];
	else if(hash_get(g_pvHash, board, board_get_hash_code(board), &hashData)) move = hashData.move[0];

	//HashInfo *hashInfo = HashGet(g_hash, key, bk, wh);

	//if (hashInfo != NULL) move = hashInfo->bestmove;
	//else move = g_move;

	//move = g_move;

	return move;
}


void StorePVLineToBoard(UINT64 bk, UINT64 wh, INT32 color, PVLINE *pline)
{
	INT32 error = 0;
	BOOL ret;
	g_pvline_len = pline->cmove;

	UINT64 bk_l = bk, wh_l = wh;

	for (int i = 0; i < g_pvline_len; i++)
	{
		if (pline->argmove[i] > 63) break;
		g_pvline[i] = pline->argmove[i];
		if (color == BLACK)
		{
			g_pvline_board[BLACK][i] = bk_l;
			g_pvline_board[WHITE][i] = wh_l;
			ret = boardMoves(&bk_l, &wh_l, 1ULL << g_pvline[i], g_pvline[i]);
			color ^= 1;
			if (ret == TRUE) error = 0;
		}
		else
		{
			g_pvline_board[BLACK][i] = wh_l;
			g_pvline_board[WHITE][i] = bk_l;
			ret = boardMoves(&wh_l, &bk_l, 1ULL << g_pvline[i], g_pvline[i]);
			color ^= 1;
			if (ret == TRUE) error = 0;
		}

		if (ret == FALSE)
		{
			i--; // やり直し
			error++;
			if (error == 2) break; // FFO#48 is entered this block...may be bug...?
		}
	}
}


/***************************************************************************
* Name  : SearchMiddle
* Brief : 序盤～中盤のCPUの着手を探索によって決定する
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         empty     : 空白マスの数
*         cpuConfig : CPUの設定
* Return: 着手評価値
****************************************************************************/
INT32 SearchMiddle(Search *search)
{
	INT32 alpha = NEGAMIN;
	INT32 beta = NEGAMAX;
	INT32 eval = 0;
	INT32 eval_b = 0;
	INT32 limit = g_limitDepth;
	INT32 selectivity;
	INT32 move;
	PVLINE line;
	Node node;

	/* 事前AI設定拡張用(今は何もない) */
	if (g_limitDepth > search->n_empties)
	{
		g_limitDepth = search->n_empties;
	}

	g_empty = search->n_empties;
	g_solveMethod = SOLVE_MIDDLE;

#if 0
	else
	{
		FixTableToMiddle(g_hash);
		// 着手用データ上書き防止のため事前登録
		g_hash->entry[key].deepest.bk = bk;
		g_hash->entry[key].deepest.wh = wh;
	}
#endif

	// 反復深化深さ優先探索
	for (int count = 2; count <= limit; count += 2)
	{
		// PV を初期化
		memset(g_pvline, -1, sizeof(g_pvline));
		g_pvline_len = 0;
		selectivity = g_max_cut_table_size; // init max threshold
		g_mpc_level = g_max_cut_table_size;

		eval_b = eval;
		g_limitDepth = count;
		eval = PVS_SearchMiddle(search, count, alpha, beta, &node);

		if (eval == ABORT)
		{
			break;
		}

		// 設定した窓より評価値が低いか？
		if (eval <= alpha)
		{
			// PV を初期化
			memset(g_pvline, -1, sizeof(g_pvline));
			g_pvline_len = 0;
			selectivity = g_max_cut_table_size;
			// 低いならαを下限に再設定して検索
			eval = PVS_SearchMiddle(search, count, NEGAMIN, eval, &node);
		}
		// 設定した窓より評価値が高いか？
		else if (eval >= beta)
		{
			// PV を初期化
			memset(g_pvline, -1, sizeof(g_pvline));
			g_pvline_len = 0;
			selectivity = g_max_cut_table_size;
			// 高いならβを上限に再設定して検索
			eval = PVS_SearchMiddle(search, count, eval, NEGAMAX, &node);
		}

		// 窓の幅を±8にして検索 (α,β) ---> (eval - 8, eval + 8)
		alpha = eval - (8 * EVAL_ONE_STONE);
		beta = eval + (8 * EVAL_ONE_STONE);

		// UIにメッセージを送信
		move = GetMoveFromHash(search->board);
		CreateCpuMessage(g_AiMsg, sizeof(g_AiMsg), eval, move, count, ON_MIDDLE);
		g_set_message_funcptr[0](g_AiMsg);
	}

	// 中断されたので直近の確定評価値を返却
	if (eval == ABORT)
	{
		return eval_b;
	}

	return eval;
}

/***************************************************************************
* Name  : SetAbortFlag
* Brief : CPUの処理を中断する
****************************************************************************/
void SetAbortFlag() {
	g_AbortFlag = TRUE;
}

/***************************************************************************
* Name  : GetMoveFromAI
* Brief : CPUの着手を探索によって決定する
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         empty     : 空白マスの数
*         cpuConfig : CPUの設定
* Return: 着手可能位置のビット列
****************************************************************************/
UINT64 GetMoveFromAI(UINT64 bk, UINT64 wh, UINT32 emptyNum, CPUCONFIG *cpuConfig)
{
	UINT64 move;
	Search search;

	if (cpuConfig->color != BLACK && cpuConfig->color != WHITE)
	{
		// 上から渡されたパラメータが不正
		return ILLIGAL_ARGUMENT;
	}

	// キャッシュが無ければ、キャッシュメモリを確保(1MB未満は無視する)
	if (cpuConfig->tableFlag == TRUE && cpuConfig->casheSize >= 1024)
	{
		if (g_hash == NULL)
		{
			hash_init(&g_hash, cpuConfig->casheSize);
			hash_init(&g_pvHash, cpuConfig->casheSize);
			hash_init(&g_shHash, cpuConfig->casheSize);

		}
		else if (g_casheSize != cpuConfig->casheSize)
		{
			hash_free(&g_hash);
			hash_free(&g_pvHash);
			hash_free(&g_shHash);

			hash_init(&g_hash, cpuConfig->casheSize);
			hash_init(&g_pvHash, cpuConfig->casheSize);
			hash_init(&g_shHash, cpuConfig->casheSize);
		}
	}

	UINT32 temp;
	// CPUはパス
	if (CreateMoves(bk, wh, &temp) == 0) {
		return MOVE_PASS;
	}

	g_mpcFlag = cpuConfig->mpcFlag;
	g_tableFlag = cpuConfig->tableFlag;
	g_empty = (INT32)emptyNum;
	g_color = cpuConfig->color;

	search_init(&search, cpuConfig);

	// 中盤かどうかをチェック
	if (emptyNum <= cpuConfig->exactDepth)
	{
		g_limitDepth = emptyNum;
		g_infscore = INF_SCORE;
		//g_evaluation = SearchExact(&search);
	}
	else if (emptyNum <= cpuConfig->winLossDepth)
	{
		g_limitDepth = emptyNum;
		g_infscore = WIN;
		//g_evaluation = SearchWinLoss(&search);
	}
	else
	{
		if (emptyNum == cpuConfig->exactDepth)
		{
			HashClear(g_hash);
			HashClear(g_pvHash);
		}
		g_limitDepth = cpuConfig->searchDepth;
		g_evaluation = SearchMiddle(&search);
	}

	g_AbortFlag = FALSE;
	// 置換表から着手を取得
	if (g_tableFlag)
	{
		//move = 1ULL << GetMoveFromHash(bk, wh, key);
		move = 1ULL << g_move;
	}
	else
	{
		move = 1ULL << g_move;
	}

	return move;
}