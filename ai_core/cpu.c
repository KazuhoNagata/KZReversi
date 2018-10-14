/***************************************************************************
* Name  : cpu.cpp
* Brief : 探索の処理全般を行う
* Date  : 2016/02/01
****************************************************************************/
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
BOOL g_refresh_hash_flag = TRUE;
INT32 g_key;

// CPU AI情報
BOOL g_AbortFlag;
UINT64 g_countNode;
INT32 g_move;
INT32 g_infscore;

HashTable *g_hash = NULL;

MPCINFO mpcInfo[22];
MPCINFO mpcInfo_end[26];
double MPC_END_CUT_VAL;
INT32 g_mpc_level;

// endgame mpc info
const double cutval_table[8] =
{
	0.25, 0.50, 0.75, 1.00, 1.50, 2.00, 2.40, 3.00
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

INT32 g_pvline[60];
INT32 g_pvline_len;
UINT64 g_pvline_board[2][60];

/***************************************************************************
*
* ProtoType(private)
*
****************************************************************************/
INT32 SearchMiddle(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);
INT32 SearchWinLoss(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);
INT32 SearchExact(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);

INT32 PVS_SearchDeep(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, INT32 *selectivity, PVLINE *pline);

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
	if (flag == ON_MIDDLE){
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
		sprintf_s(strptr, bufsize, "%s-", g_cordinates_table[g_pvline[i]]);
		strptr += 3; // 3文字ずつずらす
		bufsize -= 3;
	}

	// PV deeper line
	int count = pline->cmove;
	for (; i < count - 1; i++)
	{
		sprintf_s(strptr, bufsize, "%s-", g_cordinates_table[pline->argmove[i]]);
		strptr += 3; // 3文字ずつずらす
		bufsize -= 3;
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


INT32 GetMoveFromHash(UINT64 bk, UINT64 wh, INT32 key)
{
	INT32 move;
	HashInfo *hashInfo = HashGet(g_hash, key, bk, wh);

	if (hashInfo != NULL) move = hashInfo->bestmove;
	else move = g_move;

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

/**
* @brief Stability Cutoff (SC).
*
* @param search Current position.
* @param alpha Alpha bound.
* @param beta Beta bound, to adjust if necessary.
* @param score Score to return in case of a cutoff is found.
* @return 'true' if a cutoff is found, false otherwise.
*/
BOOL search_SC_PVS(UINT64 bk, UINT64 wh, INT32 empty,
	volatile INT32 *alpha, volatile INT32 *beta, INT32 *score)
{
	if (*beta >= PVS_STABILITY_THRESHOLD[empty]) {
		*score = (64 - 2 * get_stability(wh, bk)) * EVAL_ONE_STONE;
		if (*score <= *alpha) {
			return TRUE;
		}
		else if (*score < *beta) *beta = *score;
	}
	return FALSE;
}


/***************************************************************************
* Name  : SetAbortFlag
* Brief : CPUの処理を中断する
****************************************************************************/
void SetAbortFlag(){
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
			g_hash = HashNew(cpuConfig->casheSize);
			g_casheSize = cpuConfig->casheSize;
		}
		else if (g_casheSize != cpuConfig->casheSize){
			HashDelete(g_hash);
			g_hash = HashNew(cpuConfig->casheSize);
			g_casheSize = cpuConfig->casheSize;
		}
	}

	UINT32 temp;
	// CPUはパス
	if (CreateMoves(bk, wh, &temp) == 0){
		return MOVE_PASS;
	}

	g_mpcFlag = cpuConfig->mpcFlag;
	g_tableFlag = cpuConfig->tableFlag;
	g_empty = emptyNum;

	// 今の局面の置換表を初期化しておく
	int key = KEY_HASH_MACRO(bk, wh, cpuConfig->color);

	// 中盤かどうかをチェック
	if (emptyNum <= cpuConfig->exactDepth)
	{
		g_limitDepth = emptyNum;
		g_infscore = INF_SCORE;
		g_evaluation = SearchExact(bk, wh, emptyNum, cpuConfig->color);
	}
	else if (emptyNum <= cpuConfig->winLossDepth)
	{
		g_limitDepth = emptyNum;
		g_infscore = WIN + 1;
		g_evaluation = SearchWinLoss(bk, wh, emptyNum, cpuConfig->color);
	}
	else
	{
		g_limitDepth = cpuConfig->searchDepth;
		g_evaluation = SearchMiddle(bk, wh, emptyNum, cpuConfig->color) + cpuConfig->color * 17500;
	}

	g_AbortFlag = FALSE;
	// 置換表から着手を取得
	if (g_tableFlag)
	{
		move = 1ULL << GetMoveFromHash(bk, wh, key);
	}
	else
	{
		move = 1ULL << g_move;
	}

	return move;
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
INT32 SearchMiddle(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color)
{
	INT32 alpha = NEGAMIN;
	INT32 beta = NEGAMAX;
	INT32 eval = 0;
	INT32 eval_b = 0;
	INT32 limit = g_limitDepth;
	INT32 selectivity;
	UINT32 key = KEY_HASH_MACRO(bk, wh, color);
	INT32 move;
	PVLINE line;

	/* 事前AI設定拡張用(今は何もない) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	g_empty = emptyNum;
	g_solveMethod = SOLVE_MIDDLE;

	if (g_tableFlag)
	{
		//HashClear(g_hash);
		g_hash->attribute = HASH_ATTR_MIDDLE;
	}
#if 0
	else
	{
		FixTableToMiddle(g_hash);
		// 着手用データ上書き防止のため事前登録
		g_hash->entry[key].deepest.bk = bk;
		g_hash->entry[key].deepest.wh = wh;
	}
#endif
	// 着手用データ上書き防止のため事前登録
	g_key = key;

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
		eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, alpha, beta, NO_PASS, &selectivity, &line);

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
			eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, NEGAMIN, NEGAMAX, NO_PASS, &selectivity, &line);
		}
		// 設定した窓より評価値が高いか？
		else if (eval >= beta)
		{
			// PV を初期化
			memset(g_pvline, -1, sizeof(g_pvline));
			g_pvline_len = 0;
			selectivity = g_max_cut_table_size;
			// 高いならβを上限に再設定して検索
			eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, NEGAMIN, NEGAMAX, NO_PASS, &selectivity, &line);
		}

		// 窓の幅を±8にして検索 (α,β) ---> (eval - 8, eval + 8)
		alpha = eval - (8 * EVAL_ONE_STONE);
		beta = eval + (8 * EVAL_ONE_STONE);

		// UIにメッセージを送信
		move = GetMoveFromHash(bk, wh, key);
		CreateCpuMessage(g_AiMsg, sizeof(g_AiMsg), eval + (color * 17500), move, count, ON_MIDDLE);
		g_set_message_funcptr[0](g_AiMsg);
	}

	// 中断されたので直近の確定評価値を返却
	if (eval == ABORT){
		return eval_b;
	}

	return eval;
}

/***************************************************************************
* Name  : SearchExact
* Brief : CPUの着手を勝敗探索によって決定する
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         empty     : 空白マスの数
*         cpuConfig : CPUの設定
* Return: 着手評価値
****************************************************************************/
INT32 SearchExact(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color)
{
	INT32 alpha = -g_infscore;
	INT32 beta = g_infscore;
	INT32 eval = 0;
	INT32 eval_b = 0;
	UINT32 key = KEY_HASH_MACRO(bk, wh, color);
	INT32 aVal;
	INT32 bVal;
	INT32 move = NOMOVE, move_b;


#if 0
	// 相手がPV通りの手を着手した場合はPVLINEから手を参照
	INT32 pv_depth = g_pvline_len - emptyNum;
	if (pv_depth > 0 &&
		g_pvline_board[BLACK][pv_depth] == bk &&
		g_pvline_board[WHITE][pv_depth] == wh)
	{

		if (g_hash->entry[key].deepest.bk == bk && g_hash->entry[key].deepest.wh == wh)
			g_hash->entry[key].deepest.bestmove = g_pvline[pv_depth];
		else g_hash->entry[key].newest.bestmove = g_pvline[pv_depth];

		// UIにPVライン情報を通知
		CreatePVLineStrAscii(&g_pvline[pv_depth], emptyNum, g_evaluation);
		g_set_message_funcptr[1](g_PVLineMsg);

		return g_evaluation;
	}
#endif

#if 0
	g_limitDepth = (INT32)(emptyNum / 1.5);

	if (g_limitDepth % 2) g_limitDepth--;
	if (g_limitDepth > 24)
	{
		g_limitDepth = 24;
	}

	if (g_limitDepth >= 6)
	{
		// 事前勝敗探索
		eval = SearchMiddle(bk, wh, emptyNum, color);

		eval /= EVAL_ONE_STONE;

		if (eval % 2)
		{
			if (eval > 0) eval++;
			else eval--;
		}

		CreateCpuMessage(g_AiMsg, sizeof(g_AiMsg), eval, g_hash->entry[key].deepest.bestmove, -2, ON_EXACT);
		g_set_message_funcptr[0](g_AiMsg);

		// 置換表を石差探索用に初期化
		FixTableToExact(g_hash);
		//HashClear(g_hash);
	}
	else
	{
		HashClear(g_hash);
		aVal = -INF_SCORE;
		bVal = INF_SCORE;
	}

#else
	if (g_tableFlag && g_empty >= 12 && g_hash->attribute != HASH_ATTR_EXACT)
	{
		HashClear(g_hash);
		g_hash->attribute = HASH_ATTR_EXACT;
	}
	aVal = -g_infscore;
	bVal = g_infscore;
#endif

	// PV を初期化
	memset(g_pvline, -1, sizeof(g_pvline));
	g_pvline_len = 0;

	// 着手用データ上書き防止のため事前登録
	g_key = key;

	PVLINE line;
	INT32 selectivity;
	INT32 lower, upper;

	// グローバル変数初期化
	g_solveMethod = SOLVE_EXACT;
	g_limitDepth = emptyNum;
	g_empty = emptyNum;
#if 1

	if (g_empty >= 12)
	{
		// 終盤MPC探索(0.25σ～3.0σ)
		for (INT32 lv = 0; lv < g_max_cut_table_size + 1; lv++)
		{
			g_mpc_level = lv;
			// Abort時のために直前の評価結果を保存
			eval_b = eval;
			move_b = move;

			MPC_END_CUT_VAL = cutval_table[g_mpc_level];

			lower = -g_infscore;
			upper = g_infscore;

			if (lv == g_max_cut_table_size)
			{
				strcpy_s(g_AiMsg, sizeof(g_AiMsg), "Performing MTD-f proccess...");
				g_set_message_funcptr[0](g_AiMsg);
				while (lower < upper)
				{
					if (eval == lower)
					{
						bVal = eval + 1;
					}
					else
					{
						bVal = eval;
					}
					// PVS石差探索
					selectivity = lv;
					eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, bVal - 1, bVal, NO_PASS, &selectivity, &line);

					// 中断されたので直近の確定評価値を返却
					if (g_AbortFlag == TRUE)
					{
						sprintf_s(g_AiMsg, sizeof(g_AiMsg), "Aborted Solver.");
						g_set_message_funcptr[0](g_AiMsg);
						g_hash->entry[key].deepest.bestmove = move_b;
						return eval_b;
					}

					if (eval < bVal) upper = eval;
					else lower = eval;

					// 置換表から最善手を取得
					move = GetMoveFromHash(bk, wh, key);

					sprintf_s(g_AiMsg, sizeof(g_AiMsg), "[%s] : %+d @ %d%%", g_cordinates_table[move], eval, cutval_table_percent[lv - 1]);
					g_set_message_funcptr[2](g_AiMsg);

					//CreateCpuMessage(g_AiMsg, sizeof(g_AiMsg), eval, move, -2, ON_EXACT);
					//g_set_message_funcptr[0](g_AiMsg);
				}
			}
			else
			{
				strcpy_s(g_AiMsg, sizeof(g_AiMsg), "Performing MPC proccess...");
				g_set_message_funcptr[0](g_AiMsg);

				selectivity = lv;
				eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, aVal, bVal, NO_PASS, &selectivity, &line);

				// 中断されたので直近の確定評価値を返却
				if (g_AbortFlag == TRUE)
				{
					sprintf_s(g_AiMsg, sizeof(g_AiMsg), "Aborted Solver.");
					g_set_message_funcptr[0](g_AiMsg);
					g_hash->entry[key].deepest.bestmove = move_b;
					return eval_b;
				}

				if (eval <= aVal)
				{
					eval--;
				}
				else if (eval >= bVal)
				{
					eval++;
				}

				if (eval % 2)
				{
					if (eval > 0) eval++;
					else eval--;
				}

				aVal = eval - 1;
				bVal = eval + 1;
			}
			
			// 置換表から最善手を取得
			move = GetMoveFromHash(bk, wh, key);

			sprintf_s(g_AiMsg, sizeof(g_AiMsg), "[%s] : %+d @ %d%%", g_cordinates_table[move], eval, cutval_table_percent[lv]);
			g_set_message_funcptr[2](g_AiMsg);

			//CreateCpuMessage(g_AiMsg, sizeof(g_AiMsg), eval, move, -2, ON_EXACT);
			//g_set_message_funcptr[0](g_AiMsg);

		}
	}
	else
	{
		// init max threshold
		selectivity = g_max_cut_table_size;
		// PVS石差探索
		eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, -g_infscore, g_infscore, NO_PASS, &selectivity, &line);
	}
#else
	selectivity = g_max_cut_table_size;
	g_mpc_level = g_max_cut_table_size;
	eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, aVal, bVal, NO_PASS, &selectivity, &line);
	// 中断されたので直近の確定評価値を返却
	if (eval == ABORT)
	{
		return eval_b;
	}
#endif

	// 中断されたので直近の確定評価値を返却
	if (g_AbortFlag == TRUE)
	{
		sprintf_s(g_AiMsg, sizeof(g_AiMsg), "Aborted Solver.");
		g_set_message_funcptr[0](g_AiMsg);
		g_hash->entry[key].deepest.bestmove = move_b;
		return eval_b;
	}
	else
	{
		sprintf_s(g_AiMsg, sizeof(g_AiMsg), "Solved.");
		g_set_message_funcptr[0](g_AiMsg);
		// PVLINE局面を保存
		StorePVLineToBoard(bk, wh, color, &line);
	}


	return eval;
}

/***************************************************************************
* Name  : SearchWinLoss
* Brief : CPUの着手を勝敗探索によって決定する
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         empty     : 空白マスの数
*         cpuConfig : CPUの設定
* Return: 着手評価値
****************************************************************************/
INT32 SearchWinLoss(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color)
{
	INT32 eval = 0;
	INT32 eval_b = 0;
	UINT32 key = KEY_HASH_MACRO(bk, wh, color);
	INT32 move = NOMOVE, move_b;

#if 1
	g_limitDepth = emptyNum - 2;

	if (g_limitDepth % 2) g_limitDepth--;
	if (g_limitDepth > 24)
	{
		g_limitDepth = 24;
	}

	if (g_limitDepth >= 12)
	{
		// 事前勝敗探索
		eval = SearchMiddle(bk, wh, emptyNum, color);

		eval /= EVAL_ONE_STONE;
		eval -= eval % 2;

		CreateCpuMessage(g_AiMsg, sizeof(g_AiMsg), eval, g_hash->entry[key].deepest.bestmove, -2, ON_WLD);
		g_set_message_funcptr[0](g_AiMsg);

		// 置換表を石差探索用に初期化
		if (g_tableFlag) FixTableToWinLoss(g_hash);
	}
	else
	{
		if (g_hash->attribute != HASH_ATTR_WLD)
		{
			HashClear(g_hash);
			g_hash->attribute = HASH_ATTR_WLD;
		}
		// 着手用データ上書き防止のため事前登録
		g_key = key;
	}

#else
	HashClear(g_hash);
#endif

	PVLINE line;
	INT32 selectivity;
	char *wld_str[] = {"LOSS", "DRAW", "WIN"};

	// グローバル変数初期化
	g_solveMethod = SOLVE_WLD;
	g_limitDepth = emptyNum;
	g_empty = emptyNum;

	// PVライン表示無効
	g_set_message_funcptr[1]("");

	// 終盤MPC探索(0.25σ～3.0σ)
	for (INT32 lv = 0; lv < g_max_cut_table_size + 1; lv++)
	{
		selectivity = lv;
		g_mpc_level = lv;
		// Abort時のために直前の評価結果を保存
		eval_b = eval;
		move_b = move;

		MPC_END_CUT_VAL = cutval_table[g_mpc_level];

		// PVS石差探索
		eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, LOSS, WIN, NO_PASS, &selectivity, &line);

		// 中断されたので直近の確定評価値を返却
		if (g_AbortFlag == TRUE)
		{
			sprintf_s(g_AiMsg, sizeof(g_AiMsg), "Aborted Solver.");
			g_set_message_funcptr[0](g_AiMsg);
			g_hash->entry[key].deepest.bestmove = move_b;
			return eval_b;
		}

		// 置換表から最善手を取得
		move = GetMoveFromHash(bk, wh, key);

		sprintf_s(g_AiMsg, sizeof(g_AiMsg), "[%s] : %s @ %d%%", g_cordinates_table[move], wld_str[eval + 1], cutval_table_percent[lv]);
		g_set_message_funcptr[2](g_AiMsg);

		CreateCpuMessage(g_AiMsg, sizeof(g_AiMsg), eval, move, -1, ON_WLD);
		g_set_message_funcptr[0](g_AiMsg);
	}

	// 中断されたので直近の確定評価値を返却
	if (g_AbortFlag == TRUE)
	{
		sprintf_s(g_AiMsg, sizeof(g_AiMsg), "Aborted Solver.");
		g_set_message_funcptr[0](g_AiMsg);
		return eval_b;
	}
	else
	{
		sprintf_s(g_AiMsg, sizeof(g_AiMsg), "Solved.");
		g_set_message_funcptr[0](g_AiMsg);
	}

	return eval;
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
INT32 PVS_SearchDeep(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, INT32 *p_selectivity, PVLINE *pline)
{

	/* アボート処理 */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	if (depth == 0)
	{
		pline->cmove = 0;
		/* 葉ノード(読みの限界値のノード)の場合は評価値を算出 */
		InitIndexBoard(bk, wh);
		return Evaluation(g_board, bk, wh, color, 59 - empty);
	}

	if (g_limitDepth >= DEPTH_DEEP_TO_SHALLOW_SEARCH && depth < DEPTH_DEEP_TO_SHALLOW_SEARCH)
	{
		return AB_Search(bk, wh, depth, empty, color, alpha, beta, passed, pline);
	}

	g_countNode++;

	INT32 score, bestscore, lower, upper, bestmove;
	UINT32 key;
	MoveList movelist[36], *iter;
	Move *move;
	HashInfo *hashInfo;

	bestmove = NOMOVE;
	lower = alpha;
	upper = beta;

	// stability cutoff
	if (depth != g_limitDepth && search_SC_PVS(bk, wh, empty, &alpha, &beta, &score)) return score;

	/************************************************************
	*
	* 置換表カットオフフェーズ
	*
	*************************************************************/

	/* transposition cutoff ? */
	key = KEY_HASH_MACRO(bk, wh, color);
	if (g_tableFlag){
		/* キーを生成 */
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->depth >= depth &&
				hashInfo->selectivity >= *p_selectivity)
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
	if (g_mpcFlag && depth >= MPC_MIN_DEPTH && depth <= 24)
	{
		INT32 mpc_level;
		double MPC_CUT_VAL;
		if (empty <= 24)
		{
			MPC_CUT_VAL = cutval_table[5];
			mpc_level = 5;
		}
		else if (empty <= 36)
		{
			MPC_CUT_VAL = cutval_table[4];
			mpc_level = 4;
		}
		else
		{
			MPC_CUT_VAL = cutval_table[3];
			mpc_level = 3;
		}

		MPCINFO *mpcInfo_p = &mpcInfo[depth - MPC_MIN_DEPTH];
		INT32 value = (INT32)(alpha - (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		score = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed,  pline);
		if (score < value)
		{
			//HashUpdate(hash, key, bk, wh, alpha, beta, alpha, mpcInfo_p->depth, NOMOVE, mpc_level, NEGAMAX);
			*p_selectivity = g_max_cut_table_size; // store selectivity
			return alpha;
		}

		value = (INT32)(beta + (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		score = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed, pline);
		if (score > value)
		{
			//HashUpdate(hash, key, bk, wh, alpha, beta, beta, mpcInfo_p->depth, NOMOVE, mpc_level, NEGAMAX);
			*p_selectivity = g_max_cut_table_size; // store selectivity
			return beta;
		}
	}
#endif

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	BOOL pv_flag = TRUE;
	INT32 selectivity;
	INT32 max_selectivity = 0;

	// 着手のflip-bitを求めてmove構造体に保存
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL) {
		if (passed) {
			bestscore = CountBit(bk) - CountBit(wh);

			if (bestscore > 0)
			{
				bestscore += 1280000;
			}
			else if (bestscore < 0)
			{
				bestscore -= 1280000;
			}

			bestmove = NOMOVE;
			pline->cmove = 0;
		}
		else {
			bestscore = -PVS_SearchDeep(wh, bk, depth, empty, color ^ 1, hash, -upper, -lower, 1, p_selectivity, pline);
			max_selectivity = *p_selectivity;
			bestmove = PASS;
		}

		return bestscore;
	}
	else
	{
		PVLINE line;
		if (moveCount > 1)
		{
			if (empty > 15)
			{
				SortMoveListMiddle(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
			else
			{
				// 手の並べ替え
				SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
		}

		/* 置換表で参照出来た手から先に着手するためにソート */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		bestscore = NEGAMIN;
		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			selectivity = g_max_cut_table_size; // init max thresould
			move = &(iter->move);
			// PV表示記憶用
			g_pvline[g_empty - empty] = move->pos;

			if (pv_flag)
			{
				score = -PVS_SearchDeep(wh ^ move->rev, bk ^ ((1ULL << move->pos) | move->rev),
					depth - 1, empty - 1, color ^ 1, hash, -upper, -lower, 0, &selectivity, &line);
				pv_flag = FALSE;
			}
			else
			{
				score = -PVS_SearchDeep(wh^move->rev, bk ^ ((1ULL << move->pos) | move->rev),
					depth - 1, empty - 1, color ^ 1, hash, -lower - 1, -lower, 0, &selectivity, &line);
				if (score > lower && score < upper)
				{
					selectivity = g_max_cut_table_size; // init max thresould
					lower = score;
					score = -PVS_SearchDeep(wh ^ move->rev, bk ^ ((1ULL << move->pos) | move->rev),
						depth - 1, empty - 1, color ^ 1, hash, -upper, -lower, 0, &selectivity, &line);
				}
			}

			if (score >= upper)
			{
				bestscore = score;
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (bestscore > lower)
				{
					lower = bestscore;
					if (line.cmove < 0 || line.cmove > 59)
					{
						line.cmove = 0;
					}
					pline->argmove[0] = bestmove;
					memcpy(pline->argmove + 1, line.argmove, line.cmove);
					pline->cmove = line.cmove + 1;
					if (g_empty - empty == 0)
					{
						CreatePVLineStr(pline, empty, (bestscore + color * 17500) * (1 - (2 * ((g_empty - empty) % 2))));
						g_set_message_funcptr[1](g_PVLineMsg);
					}
				}
			}
		}
	}

	/* 置換表に登録 */
	if (g_empty == empty)
	{
		g_move = bestmove;
	}

	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, depth, bestmove, g_max_cut_table_size, NEGAMAX);
	*p_selectivity = g_max_cut_table_size; // store selectivity

	return bestscore;

}

INT32 AB_Search(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
{

	/* アボート処理 */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (depth == 0)
	{
		pline->cmove = 0;
		/* 葉ノード(読みの限界値のノード)の場合は評価値を算出 */
		InitIndexBoard(bk, wh);
		return Evaluation(g_board, bk, wh, color, 59 - empty);
	}

	int eval;
	// stability cutoff
	if (search_SC_PVS(bk, wh, empty, &alpha, &beta, &eval)) return eval;

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) フェーズ
	*
	*************************************************************/
#if 1
	if (g_mpcFlag && depth >= MPC_MIN_DEPTH && depth <= 24)
	{
		double MPC_CUT_VAL;
		if (empty <= 24)
		{
			MPC_CUT_VAL = cutval_table[5];
		}
		else if (empty <= 36)
		{
			MPC_CUT_VAL = cutval_table[4];
		}
		else
		{
			MPC_CUT_VAL = cutval_table[3];
		}


		MPCINFO *mpcInfo_p = &mpcInfo[depth - MPC_MIN_DEPTH];
		INT32 value = (INT32)(alpha - (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		INT32 eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed, pline);
		if (eval < value)
		{
			return alpha;
		}

		value = (INT32)(beta + (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AB_Search(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed, pline);
		if (eval > value)
		{
			return beta;
		}
	}
#endif

	int move_cnt;
	int max;                    //現在の最高評価値
	UINT64 rev;
	UINT64 moves;             //合法手のリストアップ

	/* 合法手生成とパスの処理 */
	if ((moves = CreateMoves(bk, wh, (UINT32 *)(&move_cnt))) == 0)
	{
		if (passed)
		{
			max = CountBit(bk) - CountBit(wh);
			if (max > 0)
			{
				max += 1280000;
			}
			else if (max < 0)
			{
				max -= 1280000;
			}

			pline->cmove = 0;
			return max;
		}
		max = -AB_Search(wh, bk, depth, empty, color ^ 1, -beta, -alpha, 1, pline);
	}
	else
	{
		int pos;
		PVLINE line;

		max = NEGAMIN;
		do
		{
			/* 静的順序づけ（少ないコストで大幅に高速化するみたい） */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);
			// PV表示記憶用
			g_pvline[g_empty - empty] = pos;

			/* ターンを進めて再帰処理へ */
			eval = -AB_Search(wh ^ rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, empty - 1, color ^ 1, -beta, -alpha, 0, &line);

			if (beta <= eval)
			{
				return eval;
			}

			/* 今までより良い局面が見つかれば最善手の更新 */
			if (eval > max)
			{
				max = eval;
				if (max > alpha)
				{
					alpha = max;
					if (line.cmove < 0 || line.cmove > 59)
					{
						line.cmove = 0;
					}
					pline->argmove[0] = pos;
					memcpy(pline->argmove + 1, line.argmove, line.cmove);
					pline->cmove = line.cmove + 1;
				}
			}

			moves ^= 1ULL << pos;

		} while (moves);
	}

	return max;

}

INT32 AB_SearchNoPV(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed)
{

	/* アボート処理 */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (depth == 0)
	{
		/* 葉ノード(読みの限界値のノード)の場合は評価値を算出 */
		InitIndexBoard(bk, wh);
		return Evaluation(g_board, bk, wh, color, 59 - empty);
	}

	int eval;
	// stability cutoff
	if (search_SC_PVS(bk, wh, empty, &alpha, &beta, &eval)) return eval;

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) フェーズ
	*
	*************************************************************/
#if 1
	if (g_mpcFlag && depth >= MPC_MIN_DEPTH && depth <= 24)
	{
		double MPC_CUT_VAL;
		if (empty <= 24)
		{
			MPC_CUT_VAL = cutval_table[5];
		}
		else if (empty <= 36)
		{
			MPC_CUT_VAL = cutval_table[4];
		}
		else
		{
			MPC_CUT_VAL = cutval_table[3];
		}

		MPCINFO *mpcInfo_p = &mpcInfo[depth - MPC_MIN_DEPTH];
		INT32 value = (INT32)(alpha - (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		INT32 eval = AB_SearchNoPV(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed);
		if (eval < value)
		{
			return alpha;
		}

		value = (INT32)(beta + (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AB_SearchNoPV(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed);
		if (eval > value)
		{
			return beta;
		}
	}
#endif


	int move_cnt;
	int max;                    //現在の最高評価値
	UINT64 rev;
	UINT64 moves;             //合法手のリストアップ

	/* 合法手生成とパスの処理 */
	if ((moves = CreateMoves(bk, wh, (UINT32 *)(&move_cnt))) == 0)
	{
		if (passed)
		{
			max = CountBit(bk) - CountBit(wh);
			if (max > 0)
			{
				max += 1280000;
			}
			else if (max < 0)
			{
				max -= 1280000;
			}
			else
			{
				max = 1280000;
			}

			return max;
		}
		max = -AB_SearchNoPV(wh, bk, depth, empty, color ^ 1, -beta, -alpha, 1);
	}
	else
	{
		int pos;
		max = NEGAMIN;
		do
		{
			/* 静的順序づけ（少ないコストで大幅に高速化するみたい） */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);
			/* ターンを進めて再帰処理へ */
			eval = -AB_SearchNoPV(wh ^ rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, empty - 1, color ^ 1, -beta, -alpha, 0);

			if (beta <= eval)
			{
				return eval;
			}

			/* 今までより良い局面が見つかれば最善手の更新 */
			if (eval > max)
			{
				max = eval;
				alpha = max(alpha, eval);
			}

			moves ^= 1ULL << pos;

		} while (moves);
	}

	return max;

}