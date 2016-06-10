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
#include "hash.h"
#include "eval.h"
#include "ordering.h"

#include <stdio.h>

#define NO_PASS 0
#define ABORT 0x80000000

#define MPC_MIN_DEPTH 3

/***************************************************************************
*
* Global
*
****************************************************************************/
// CPU設定格納用
BOOL g_mpcFlag;
BOOL g_tableFlag;
INT32 g_empty;
INT32 g_limitDepth;
UINT64 g_casheSize;

// CPU AI情報
BOOL g_AbortFlag;
UINT64 g_countNode;

HashTable *g_hash = NULL;

MPCINFO mpcInfo[22];
double MPC_CUT_VAL;

char g_cordinates_table[64][4];
SetMessageToGUI g_set_message_funcptr;

/***************************************************************************
*
* ProtoType(private)
*
****************************************************************************/
INT32 SearchMiddle(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);
INT32 SearchWinLoss(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);
INT32 SearchExact(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);

INT32 PVS_SearchDeep(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed);
INT32 AlphaBetaSearch(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed);

INT32 PVS_SearchDeepWinLoss(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed);
INT32 AlphaBetaSearchWinLoss(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 *quad_parity, UINT32 color, INT32 alpha, INT32 beta, UINT32 passed);

INT32 PVS_SearchDeepExact(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed);
INT32 AlphaBetaSearchExact(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 *quad_parity, UINT32 color, INT32 alpha, INT32 beta, UINT32 passed);

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
	else if (flag == ON_WINLOSS)
	{
		if (cnt == -1)
		{
			if (eval == WIN)
			{
				sprintf_s(msg, msglen, "guess... move %s[WIN?]", g_cordinates_table[move]);
			}
			else if (eval == LOSS)
			{
				sprintf_s(msg, msglen, "guess... move %s[LOSS?]", g_cordinates_table[move]);
			}
			else
			{
				sprintf_s(msg, msglen, "guess... move %s[DRAW?]", g_cordinates_table[move]);
			}
		}
		else
		{
			if (eval == WIN)
			{
				sprintf_s(msg, msglen, "%s[WIN]", g_cordinates_table[move]);
			}
			else if (eval == LOSS)
			{
				sprintf_s(msg, msglen, "%s[LOSS]", g_cordinates_table[move]);
			}
			else
			{
				sprintf_s(msg, msglen, "%s[DRAW]", g_cordinates_table[move]);
			}
		}

	}
	else
	{
		if (cnt == -1)
		{
			if (eval > 0)
			{
				sprintf_s(msg, msglen, "guess... move %s[WIN:%+d?]", g_cordinates_table[move], eval);
			}
			else if (eval < 0)
			{
				sprintf_s(msg, msglen, "guess... move %s[LOSS:%d?]", g_cordinates_table[move], eval);
			}
			else
			{
				sprintf_s(msg, msglen, "guess... move %s[DRAW:+0?]", g_cordinates_table[move]);
			}
		}
		else
		{
			if (eval > 0)
			{
				sprintf_s(msg, msglen, "%s[WIN:%+d]", g_cordinates_table[move], eval);
			}
			else if (eval < 0)
			{
				sprintf_s(msg, msglen, "%s[LOSS:%d]", g_cordinates_table[move], eval);
			}
			else
			{
				sprintf_s(msg, msglen, "%s[DRAW:+0]", g_cordinates_table[move]);
			}
		}
	}
}

/**
* @brief Stability Cutoff (TC).
*
* @param search Current position.
* @param alpha Alpha bound.
* @param score Score to return in case of a cutoff is found.
* @return 'true' if a cutoff is found, false otherwise.
*/
bool search_SC_NWS(UINT64 bk, UINT64 wh, INT32 empty, INT32 alpha, INT32 *score)
{
	if (alpha >= NWS_STABILITY_THRESHOLD[empty]) {
		*score = 64 - 2 * get_stability(wh, bk);
		if (*score <= alpha) {
			return true;
		}
	}
	return false;
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

	g_mpcFlag = cpuConfig->mpcFlag;
	g_tableFlag = cpuConfig->tableFlag;

	// 今の局面の置換表を初期化しておく
	int key = KEY_HASH_MACRO(bk, wh);

	UINT32 temp;
	// CPUはパス
	if (CreateMoves(bk, wh, &temp) == 0){
		return MOVE_PASS;
	}

	g_empty = emptyNum;

	// 中盤かどうかをチェック
	if (emptyNum <= cpuConfig->exactDepth)
	{
		g_limitDepth = emptyNum;
		g_evaluation = SearchExact(bk, wh, emptyNum, cpuConfig->color);
	}
	else if (emptyNum <= cpuConfig->winLossDepth)
	{
		g_limitDepth = emptyNum;
		g_evaluation = SearchWinLoss(bk, wh, emptyNum, cpuConfig->color);
	}
	else
	{
		g_limitDepth = cpuConfig->searchDepth;
		g_evaluation = SearchMiddle(bk, wh, emptyNum, cpuConfig->color);
	}

	g_AbortFlag = FALSE;
	// 置換表から着手を取得
	move = 1ULL << (g_hash->entry[key].deepest.bestmove);

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
	UINT32 key = KEY_HASH_MACRO(bk, wh);
	INT32 move;
	char msg[64];

	/* 事前AI設定拡張用(今は何もない) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	HashClear(g_hash);
	// 反復深化深さ優先探索
	for (int count = 2; count <= limit; count += 2)
	{
		eval_b = eval;
		g_limitDepth = count;
		eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, alpha, beta, NO_PASS);

		if (eval == ABORT)
		{
			break;
		}

		// 設定した窓より評価値が低いか？
		if (eval <= alpha)
		{
			// 低いならαを下限に再設定して検索
			eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, NEGAMIN, eval, NO_PASS);
		}
		// 設定した窓より評価値が高いか？
		else if (eval >= beta)
		{
			// 高いならβを上限に再設定して検索
			eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, eval, NEGAMAX, NO_PASS);
		}

		// 窓の幅を±8にして検索 (α,β) ---> (eval - 8, eval + 8)
		alpha = eval - (8 * EVAL_ONE_STONE);
		beta = eval + (8 * EVAL_ONE_STONE);

		// UIにメッセージを送信
		move = g_hash->entry[key].deepest.bestmove;
		CreateCpuMessage(msg, sizeof(msg), eval, move, count, ON_MIDDLE);
		g_set_message_funcptr(msg);

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
	INT32 alpha = -INF_SCORE;
	INT32 beta = INF_SCORE;
	INT32 eval = 0;
	INT32 eval_b = 0;
	UINT32 key = KEY_HASH_MACRO(bk, wh);
	INT32 aVal;
	INT32 bVal;

	g_limitDepth = emptyNum;

	if (g_limitDepth % 2) g_limitDepth--;
	if (g_limitDepth > 22)
	{
		g_limitDepth = 22;
	}

	if (g_limitDepth >= 12)
	{
		// 事前勝敗探索
		eval = SearchWinLoss(bk, wh, emptyNum, color);

		if (eval == WIN)
		{
			aVal = 2;
			bVal = INF_SCORE;
		}
		else if (eval == LOSS)
		{
			aVal = -INF_SCORE;
			bVal = -2;
		}
		else
		{
			aVal = 0;
			bVal = 0;
		}

		// 置換表を石差探索用に初期化
		FixTableToExact(g_hash);
	}
	else
	{
		HashClear(g_hash);
		aVal = -INF_SCORE;
		bVal = INF_SCORE;
	}

	g_empty = emptyNum;
	// PVS石差探索
	eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, aVal, bVal, NO_PASS);
	// 中断されたので直近の確定評価値を返却
	if (eval == ABORT)
	{
		return eval_b;
	}

	if (eval == 64)
	{
		return eval;
	}
	if (eval == -64)
	{
		return -64;
	}

	eval_b = eval;

	if (eval <= aVal)
	{
		// PVS石差探索
		eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, -INF_SCORE, eval, NO_PASS);

	}
	else if (eval >= bVal)
	{
		// PVS石差探索
		eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, eval, INF_SCORE, NO_PASS);

	}

	// 中断されたので直近の確定評価値を返却
	if (eval == ABORT)
	{
		return eval_b;
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
	UINT32 key = KEY_HASH_MACRO(bk, wh);
	INT32 aVal;
	INT32 bVal;

	g_limitDepth = emptyNum;
	if (g_limitDepth % 2) g_limitDepth--;
	if (g_limitDepth > 22)
	{
		g_limitDepth = 22;
	}
	if (g_limitDepth >= 12)
	{
		// 事前探索
		eval = SearchMiddle(bk, wh, emptyNum, color);
		// 中断されたので直近の確定評価値を返却
		if (g_AbortFlag)
		{
			return eval;
		}

		eval_b = eval;
		eval /= EVAL_ONE_STONE;
		if (eval % 2)
		{
			eval++;
		}

		if (eval >= 4)
		{
			aVal = DRAW;
			bVal = WIN;
		}
		else if (eval <= -4)
		{
			aVal = LOSS;
			bVal = DRAW;
		}
		else
		{
			aVal = DRAW;
			bVal = WIN;
		}
		// 置換表を石差探索用に初期化
		FixTableToWinLoss(g_hash);
	}
	else
	{
		aVal = LOSS;
		bVal = WIN;
	}

	g_empty = emptyNum;
	// PVS石差探索
	eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, aVal, bVal, NO_PASS);
	// 中断されたので直近の確定評価値を返却
	if (eval == ABORT)
	{
		return eval_b;
	}

	eval_b = eval;

	// 幅決め打ち探索に失敗した時は幅を∞に拡大して再度探索
	if (eval <= aVal)
	{
		// PVS石差探索
		eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, LOSS, eval, NO_PASS);

	}
	else if (eval >= bVal)
	{
		// PVS石差探索
		eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, eval, WIN, NO_PASS);

	}

	// 中断されたので直近の確定評価値を返却
	if (eval == ABORT)
	{
		return eval_b;
	}

	return eval;
}

/***************************************************************************
* Name  : PvSearchMiddle
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
INT32 PVS_SearchDeepExact(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed)
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

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &score)) return score;

	/************************************************************
	*
	* 置換表カットオフフェーズ
	*
	*************************************************************/
	/* キーを生成 */
	key = KEY_HASH_MACRO(bk, wh);
	/* transposition cutoff ? */
	if (g_tableFlag)
	{
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->empty < empty)
			{
				if (upper > hashInfo->upper)
				{
					upper = hashInfo->upper;
					if (upper <= lower) return upper;
				}
				if (lower < hashInfo->lower)
				{
					lower = hashInfo->lower;
					if (lower >= upper) return lower;
				}
			}
			//hashInfo->empty = empty;
			bestmove = hashInfo->bestmove;
		}
	}

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) フェーズ
	*
	*************************************************************/

	// 今後実装予定


	/************************************************************
	*
	* Principal Variation Search(PVS) フェーズ
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;
	UINT32 quad_parity[4];

	// 着手のflip-bitを求めてmove構造体に保存
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL)
	{
		if (passed) {
			// game end...
			alpha = -(beta = INF_SCORE);
			if (bk == 0)
			{
				bestscore = -64;
			}
			else if (wh == 0)
			{
				bestscore = 64;
			}
			else
			{
				bestscore = CountBit(bk) - CountBit(wh);
				// 空きがある状態で終わったので空きマスを勝っている方に加算する
				if (bestscore > 0)
				{
					bestscore += empty;
				}
				else if (bestscore < 0)
				{
					bestscore -= empty;
				}
			}
			bestmove = NOMOVE;

		}
		else {
			if (empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
			{
				create_quad_parity(quad_parity, ~(wh | bk));
				bestscore = -AlphaBetaSearchExact(wh, bk, ~(wh | bk), empty, quad_parity, 
					color ^ 1, -upper, -lower, 1);
			}
			else
			{
				bestscore = -PVS_SearchDeepExact(wh, bk, empty, color ^ 1, hash, -upper, -lower, 1);
			}
			bestmove = PASS;
		}
	}
	else {

#if 1
		HashEntry *hash_entry;
		UINT32 hashKey;
		UINT64 bk_after, wh_after;

		/* enhanced transposition cutoff */
		if (hash != NULL) {
			if (bestmove != NOMOVE) SortMoveListTableMoveFirst(movelist, bestmove);
			for (iter = movelist->next; iter != NULL; iter = iter->next) {
				move = &(iter->move);
				bk_after = bk ^ ((1ULL << move->pos) | move->rev);
				wh_after = wh ^ move->rev;

				hashKey = KEY_HASH_MACRO(wh_after, bk_after);
				hash_entry = &(hash->entry[hashKey]);

				if (hash_entry->deepest.bk == wh_after &&
					hash_entry->deepest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->deepest.upper >= upper)
					return -hash_entry->deepest.upper;
				if (hash_entry->newest.bk == wh_after &&
					hash_entry->newest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->newest.upper >= upper)
					return -hash_entry->newest.upper;
			}
		}
#endif

		if (moveCount > 1)
		{
			if (empty >= EMPTIES_MID_ORDER_TO_END_ORDER){
				/* 中盤用のオーダリング */
				SortMoveListMiddle(movelist, bk, wh, hash, empty, empty, alpha, beta, color);
			}
			else if (empty >= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
			{
				SortMoveListEnd(movelist, bk, wh);
			}
		}

		/* 置換表で参照出来た手から先に着手するためにソート */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		/* 最初に最善手とおぼしき手を打って仮に最善手とする */
		iter = movelist->next;
		move = &(iter->move);

		move_b = bk ^ ((1ULL << move->pos) | move->rev);
		move_w = wh ^ move->rev;

		if (empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
		{
			create_quad_parity(quad_parity, ~(move_b | move_w));
			bestscore = -AlphaBetaSearchExact(move_w, move_b, ~(move_b | move_w), 
				empty - 1, quad_parity, color ^ 1, -upper, -lower, 0);
		}
		else
		{
			bestscore = -PVS_SearchDeepExact(move_w, move_b,
				empty - 1, color ^ 1, hash, -upper, -lower, 0);
		}

		// 最善手とする
		bestmove = move->pos;
		if (bestscore >= beta)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);
			if (empty == g_empty)
			{
				char msg[64];
				// UIにメッセージを送信
				CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, 0, ON_EXACT);
				g_set_message_funcptr(msg);
			}
			return bestscore;   // fail-hard beta-cutoff
		}

		if (bestscore > lower) lower = bestscore;

		if (empty == g_empty)
		{
			char msg[64];
			// UIにメッセージを送信
			CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, -1, ON_EXACT);
			g_set_message_funcptr(msg);
		}

		/* other moves : try to refute the first/best one */
		for (iter = iter->next; lower < upper && iter != NULL; iter = iter->next)
		{
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;

			if (empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
			{
				create_quad_parity(quad_parity, ~(move_b | move_w));
				score = -AlphaBetaSearchExact(move_w, move_b,
					~(move_b | move_w), empty - 1, quad_parity, color ^ 1, -lower - 1, -lower, 0);
				if (lower < score && score < upper)
				{
					create_quad_parity(quad_parity, ~(move_b | move_w));
					score = -AlphaBetaSearchExact(move_w, move_b,
						~(move_b | move_w), empty - 1, quad_parity, color ^ 1, -upper, -score, 0);
				}
			}
			else
			{
				score = -PVS_SearchDeepExact(move_w, move_b,
					empty - 1, color ^ 1, hash, -lower - 1, -lower, 0);
				if (lower < score && score < upper)
					score = -PVS_SearchDeepExact(move_w, move_b,
					empty - 1, color ^ 1, hash, -upper, -score, 0);
			}

			if (score >= beta)
			{
				bestscore = score;
				bestmove = move->pos;
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (empty == g_empty)
				{
					char msg[64];
					// UIにメッセージを送信
					CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, -1, ON_EXACT);
					g_set_message_funcptr(msg);
				}
				if (bestscore > lower) lower = bestscore;
			}
		}
	}
	/* 置換表に登録 */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);

	if (empty == g_empty)
	{
		char msg[64];
		// UIにメッセージを送信
		CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, 0, ON_EXACT);
		g_set_message_funcptr(msg);
	}

	return bestscore;
}

INT32 SearchEmpty_1(UINT64 bk, UINT64 wh, UINT64 blank)
{
	INT32 pos = CountBit((~blank) & (blank - 1));
	UINT64 rev = GetRev[pos](bk, wh);
	UINT64 move_b, move_w;
	INT32 n_disc;

	g_countNode++;

	/* 打てる？ */
	if (rev)
	{
		move_b = bk ^ (blank | rev);
		move_w = wh ^ rev;
		blank = 0;
	}
	else{
		rev = GetRev[pos](wh, bk);
		/* 打てる？ */
		if (rev)
		{
			move_b = bk ^ rev;
			move_w = wh ^ (blank | rev);
			blank = 0;
		}
		else
		{
			move_b = bk;
			move_w = wh;
		}
	}

	if (move_b == 0)
	{
		return -64;
	}
	if (move_w == 0)
	{
		return 64;
	}
	// game end...
	n_disc = CountBit(move_b) - CountBit(move_w);
	// 空きマスは勝った方に加算する
	if (blank)
	{
		if (n_disc > 0)
		{
			n_disc++;
		}
		else if (n_disc < 0)
		{
			n_disc--;
		}
	}

	return n_disc;
}

INT32 AlphaBetaSearchExact(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty, 
	UINT32 *quad_parity, UINT32 color, INT32 alpha, INT32 beta, UINT32 passed)
{
	/* アボート処理 */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (empty == 1)
	{
		return SearchEmpty_1(bk, wh, blank);
	}


	INT32 max;                    //現在の最高評価値
	INT32 eval;                   //評価値の保存
	INT32 pos;
	UINT64 pos_bit, rev;
	UINT64 moves = blank;

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &max)) return max;

	max = -INF_SCORE;

	// First, move odd parity empties
	for (int i = 0; i < 4; i++)
	{
		if (quad_parity[i])
		{
			// odd parity
			moves = blank & quad_parity_bitmask[i];
			// parity探索開始
			while (moves)
			{
				/*
				　ここに来るのは６マス以下の空きなので、CreateMovesを呼ぶより
				 反転データ取得と合法手を空きマスから直接チェックしたほうが圧倒的に速い
				 */
				pos = CountBit((~moves) & (moves - 1));
				pos_bit = 1ULL << pos;
				rev = GetRev[pos](bk, wh);

				if (rev)
				{
					// reverse parity
					quad_parity[i] ^= 1;
					eval = -AlphaBetaSearchExact(wh ^ rev, bk ^ (pos_bit | rev),
						blank ^ pos_bit, empty - 1, quad_parity, color ^ 1, -beta, -alpha, 0);
					// restore parity
					create_quad_parity(quad_parity, blank);
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
				}
				moves ^= pos_bit;
			}
		}
	}

	// after, even parity
	for (int i = 0; i < 4; i++)
	{
		if (!quad_parity[i])
		{
			// even parity
			moves = blank & quad_parity_bitmask[i];
			// parity探索開始
			while (moves)
			{
				/*
				　ここに来るのは６マス以下の空きなので、CreateMovesを呼ぶより
				 反転データ取得と合法手を空きマスから直接チェックしたほうが圧倒的に速い
				 */
				pos = CountBit((~moves) & (moves - 1));
				pos_bit = 1ULL << pos;
				rev = GetRev[pos](bk, wh);

				if (rev)
				{
					// reverse parity
					quad_parity[i] ^= 1;
					eval = -AlphaBetaSearchExact(wh ^ rev, bk ^ (pos_bit | rev),
						blank ^ pos_bit, empty - 1, quad_parity, color ^ 1, -beta, -alpha, 0);
					// restore parity
					create_quad_parity(quad_parity, blank);
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
				}
				moves ^= pos_bit;
			}
		}
	}
	

	if (max == -INF_SCORE)
	{
		// 打てなかった
		if (passed)
		{
			// game end...
			if (bk == 0)
			{
				max = -64;
			}
			else if (wh == 0)
			{
				max = 64;
			}
			else
			{
				max = CountBit(bk) - CountBit(wh);
				// 空きマスは勝った方に加算する
				if (max > 0)
				{
					max += empty;
				}
				else if (max < 0)
				{
					max -= empty;
				}
			}
		}
		else
		{
			max = -AlphaBetaSearchExact(wh, bk, blank, empty, quad_parity, 
				color ^ 1, -beta, -alpha, 1);
			// restore parity
			create_quad_parity(quad_parity, blank);
		}
	}

	return max;

}

/***************************************************************************
* Name  : PVS_SearchDeepWinLoss
* Brief : PVSを行い、評価値を基に最善手を取得
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
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed)
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

	/************************************************************
	*
	* 置換表カットオフフェーズ
	*
	*************************************************************/
	/* キーを生成 */
	key = KEY_HASH_MACRO(bk, wh);
	/* transposition cutoff ? */
	if (g_tableFlag)
	{
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->empty < empty)
			{
				if (upper > hashInfo->upper)
				{
					upper = hashInfo->upper;
					if (upper <= lower) return upper;
				}
				if (lower < hashInfo->lower)
				{
					lower = hashInfo->lower;
					if (lower >= upper) return lower;
				}
			}
			hashInfo->empty = empty;
			bestmove = hashInfo->bestmove;
		}
	}

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) フェーズ
	*
	*************************************************************/

	// 今後実装予定


	/************************************************************
	*
	* Principal Variation Search(PVS) フェーズ
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;
	UINT32 quad_parity[4];

	// 着手のflip-bitを求めてmove構造体に保存
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL)
	{
		if (passed) {
			// game end...
			bestscore = CountBit(bk) - CountBit(wh);
			if (bestscore > 0)
			{
				bestscore = WIN;
			}
			else if (bestscore < 0)
			{
				bestscore = LOSS;
			}
			else
			{
				bestscore = DRAW;
			}

			bestmove = NOMOVE;
		}
		else {
			if (empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
			{
				create_quad_parity(quad_parity, ~(wh | bk));
				bestscore = -AlphaBetaSearchWinLoss(wh, bk, ~(wh | bk), empty, quad_parity, color ^ 1, -upper, -lower, 1);
			}
			else
			{
				bestscore = -PVS_SearchDeepWinLoss(wh, bk, empty, color ^ 1, hash, -upper, -lower, 1);
			}
			bestmove = PASS;
		}
	}
	else
	{

#if 1
		HashEntry *hash_entry;
		UINT32 hashKey;
		UINT64 bk_after, wh_after;
		/* enhanced transposition cutoff */
		if (hash != NULL) {
			if (bestmove != NOMOVE) SortMoveListTableMoveFirst(movelist, bestmove);
			for (iter = movelist->next; iter != NULL; iter = iter->next) {
				move = &(iter->move);
				bk_after = bk ^ ((1ULL << move->pos) | move->rev);
				wh_after = wh ^ move->rev;

				hashKey = KEY_HASH_MACRO(wh_after, bk_after);
				hash_entry = &(hash->entry[hashKey]);

				if (hash_entry->deepest.bk == wh_after &&
					hash_entry->deepest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->deepest.upper >= upper)
					return -hash_entry->deepest.upper;
				if (hash_entry->newest.bk == wh_after &&
					hash_entry->newest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->newest.upper >= upper)
					return -hash_entry->newest.upper;
			}
		}
#endif

		if (moveCount > 1)
		{
			if (empty > EMPTIES_MID_ORDER_TO_END_ORDER){
				/* 中盤用のオーダリング */
				SortMoveListMiddle(movelist, bk, wh, hash, empty, empty, alpha, beta, color);
			}
			else if (empty >= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
			{
				SortMoveListEnd(movelist, bk, wh);
			}
		}

		/* 置換表で参照出来た手から先に着手するためにソート */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		/* 最初に最善手とおぼしき手を打って仮に最善手とする */
		iter = movelist->next;
		move = &(iter->move);

		move_b = bk ^ ((1ULL << move->pos) | move->rev);
		move_w = wh ^ move->rev;

		if (empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
		{
			create_quad_parity(quad_parity, ~(move_w | move_b));
			bestscore = -AlphaBetaSearchWinLoss(move_w, move_b, ~(move_b | move_w), 
				empty - 1, quad_parity, color ^ 1, -upper, -lower, 0);
		}
		else
		{
			bestscore = -PVS_SearchDeepWinLoss(move_w, move_b,
				empty - 1, color ^ 1, hash, -upper, -lower, 0);
		}

		// 最善手とする
		bestmove = move->pos;

		if (bestscore >= beta)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);
			if (empty == g_empty)
			{
				char msg[64];
				// UIにメッセージを送信
				CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, 0, ON_WINLOSS);
				g_set_message_funcptr(msg);
			}
			return bestscore;   // fail-hard beta-cutoff
		}

		if (bestscore > lower) lower = bestscore;

		if (empty == g_empty)
		{
			char msg[64];
			// UIにメッセージを送信
			CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, -1, ON_WINLOSS);
			g_set_message_funcptr(msg);
		}

		/* other moves : try to refute the first/best one */
		for (iter = iter->next; lower < upper && iter != NULL; iter = iter->next)
		{
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;

			if (empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH + (g_empty / 10))
			{
				create_quad_parity(quad_parity, ~(move_w | move_b));
				score = -AlphaBetaSearchWinLoss(move_w, move_b,
					~(move_b | move_w), empty - 1, quad_parity, color ^ 1, -lower - 1, -lower, 0);
				if (lower < score && score < upper)
				{
					create_quad_parity(quad_parity, ~(move_w | move_b));
					score = -AlphaBetaSearchWinLoss(move_w, move_b,
						~(move_b | move_w), empty - 1, quad_parity, color ^ 1, -upper, -score, 0);
				}
			}
			else
			{
				score = -PVS_SearchDeepWinLoss(move_w, move_b,
					empty - 1, color ^ 1, hash, -lower - 1, -lower, 0);
				if (lower < score && score < upper)
					score = -PVS_SearchDeepWinLoss(move_w, move_b,
					empty - 1, color ^ 1, hash, -upper, -score, 0);
				if (score == 64)
				{
					int a = score;
				}
			}

			if (score >= beta)
			{
				bestscore = score;
				bestmove = move->pos;
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (empty == g_empty)
				{
					char msg[64];
					// UIにメッセージを送信
					CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, -1, ON_WINLOSS);
					g_set_message_funcptr(msg);
				}
				if (bestscore > lower) lower = bestscore;
			}
		}
	}
	/* 置換表に登録 */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);

	if (empty == g_empty)
	{
		char msg[64];
		// UIにメッセージを送信
		CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, 0, ON_WINLOSS);
		g_set_message_funcptr(msg);
	}
	return bestscore;

}

/***************************************************************************
* Name  : SearchEmptyWinLoss_1
* Brief : 勝敗探索中、残り1マスの時の処理を行う
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         depth     : 空きマスのビット列
* Return: WIN or LOSS or DRAW
****************************************************************************/
INT32 SearchEmptyWinLoss_1(UINT64 bk, UINT64 wh, UINT64 blank)
{
	INT32 pos = CountBit((~blank) & (blank - 1));
	UINT64 rev = GetRev[pos](bk, wh);
	UINT64 move_b, move_w;
	INT32 wld;

	g_countNode++;

	/* 打てる？ */
	if (rev)
	{
		move_b = bk ^ (blank | rev);
		move_w = wh ^ rev;
	}
	else{
		rev = GetRev[pos](wh, bk);
		/* 打てる？ */
		if (rev)
		{
			move_b = bk ^ rev;
			move_w = wh ^ (blank | rev);
		}
		else
		{
			move_b = bk;
			move_w = wh;
		}
	}

	// game end...
	wld = CountBit(move_b) - CountBit(move_w);
	if (wld > 0)
	{
		wld = WIN;
	}
	else if (wld < 0)
	{
		wld = LOSS;
	}
	else
	{
		wld = DRAW;
	}

	return wld;
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
INT32 AlphaBetaSearchWinLoss(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty, 
	UINT32 *quad_parity, UINT32 color, INT32 alpha, INT32 beta, UINT32 passed)
{
	/* アボート処理 */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (empty == 1)
	{
		return SearchEmptyWinLoss_1(bk, wh, blank);
	}

	INT32 max;                    //現在の最高評価値
	INT32 eval;                   //評価値の保存
	INT32 pos;
	UINT64 pos_bit, rev;
	UINT64 moves = blank;

	// パス用のフラグをかねるため、min値を調整
	max = -INF_SCORE;


	// First, move odd parity empties
	for (int i = 0; i < 4; i++)
	{
		if (quad_parity[i])
		{
			// odd parity
			moves = blank & quad_parity_bitmask[i];
			// parity探索開始
			while (moves)
			{
				/*
				　ここに来るのは６マス以下の空きなので、CreateMovesを呼ぶより
				 反転データ取得と合法手を空きマスから直接チェックしたほうが圧倒的に速い
				 */
				pos = CountBit((~moves) & (moves - 1));
				pos_bit = 1ULL << pos;
				rev = GetRev[pos](bk, wh);

				if (rev)
				{
					// reverse parity
					quad_parity[i] ^= 1;
					eval = -AlphaBetaSearchWinLoss(wh ^ rev, bk ^ (pos_bit | rev),
						blank ^ pos_bit, empty - 1, quad_parity, color ^ 1, -beta, -alpha, 0);
					// restore parity
					create_quad_parity(quad_parity, blank);
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
				}
				moves ^= pos_bit;
			}
		}
	}

	// after, even parity
	for (int i = 0; i < 4; i++)
	{
		if (!quad_parity[i])
		{
			// even parity
			moves = blank & quad_parity_bitmask[i];
			// parity探索開始
			while (moves)
			{
				/*
				　ここに来るのは６マス以下の空きなので、CreateMovesを呼ぶより
				 反転データ取得と合法手を空きマスから直接チェックしたほうが圧倒的に速い
				 */
				pos = CountBit((~moves) & (moves - 1));
				pos_bit = 1ULL << pos;
				rev = GetRev[pos](bk, wh);

				if (rev)
				{
					// reverse parity
					quad_parity[i] ^= 1;
					eval = -AlphaBetaSearchWinLoss(wh ^ rev, bk ^ (pos_bit | rev),
						blank ^ pos_bit, empty - 1, quad_parity, color ^ 1, -beta, -alpha, 0);
					// restore parity
					create_quad_parity(quad_parity, blank);
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
				}
				moves ^= pos_bit;
			}
		}
	}

	if (max == -INF_SCORE)
	{
		// 打てなかった
		if (passed)
		{
			// game end...
			max = CountBit(bk) - CountBit(wh);
			if (max > 0)
			{
				max = WIN;
			}
			else if (max < 0)
			{
				max = LOSS;
			}
			else
			{
				max = DRAW;
			}
		}
		else
		{
			max = -AlphaBetaSearchWinLoss(wh, bk, blank, empty, quad_parity, 
				color ^ 1, -beta, -alpha, 1);
			// restore parity
			create_quad_parity(quad_parity, blank);
		}
	}

	return max;

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
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed)
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

	/************************************************************
	*
	* 置換表カットオフフェーズ
	*
	*************************************************************/
	/* キーを生成 */
	key = KEY_HASH_MACRO(bk, wh);
	/* transposition cutoff ? */
	if (g_tableFlag){
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->empty < empty)
			{
				if (upper > hashInfo->upper) {
					upper = hashInfo->upper;
					if (upper <= lower) return upper;
				}
				if (lower < hashInfo->lower) {
					lower = hashInfo->lower;
					if (lower >= upper) return lower;
				}
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
		if ((60 - empty) >= 36)
		{
			MPC_CUT_VAL = 1.4;
		}
		else
		{
			MPC_CUT_VAL = 1.0;
		}

		MPCINFO *mpcInfo_p = &mpcInfo[depth - MPC_MIN_DEPTH];
		INT32 value = (INT32)(alpha - (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		INT32 eval = AlphaBetaSearch(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed);
		if (eval < value)
		{
			return alpha;
		}

		value = (INT32)(beta + (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AlphaBetaSearch(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed);
		if (eval > value)
		{
			return beta;
		}
	}
#endif

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	// 着手のflip-bitを求めてmove構造体に保存
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL) {
		if (passed) {
			bestscore = CountBit(bk) - CountBit(wh);

			if (bestscore > 0)
			{
				bestscore += 640000;
			}
			else if (bestscore < 0)
			{
				bestscore -= 640000;
			}
			else
			{
				bestscore = 640000;
			}

			bestmove = NOMOVE;
		}
		else {
			bestscore = -PVS_SearchDeep(wh, bk, depth, empty, color ^ 1, hash, -upper, -lower, 1);
			bestmove = PASS;
		}
	}
	else {

#if 1
		HashEntry *hash_entry;
		UINT32 hashKey;
		UINT64 bk_after, wh_after;
		/* enhanced transposition cutoff */
		if (hash != NULL) {
			if (bestmove != NOMOVE) SortMoveListTableMoveFirst(movelist, bestmove);
			for (iter = movelist->next; iter != NULL; iter = iter->next) {
				move = &(iter->move);
				bk_after = bk ^ ((1ULL << move->pos) | move->rev);
				wh_after = wh ^ move->rev;

				hashKey = KEY_HASH_MACRO(wh_after, bk_after);
				hash_entry = &(hash->entry[hashKey]);

				if (hash_entry->deepest.bk == wh_after &&
					hash_entry->deepest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->deepest.upper >= upper &&
					depth != g_limitDepth)
					return -hash_entry->deepest.upper;
				if (hash_entry->newest.bk == wh_after &&
					hash_entry->newest.wh == bk_after &&
					hash_entry->deepest.empty < empty - 1 &&
					-hash_entry->newest.upper >= upper &&
					depth != g_limitDepth)
					return -hash_entry->newest.upper;
			}
		}
#endif
		/* 中盤用のオーダリング */
		SortMoveListMiddle(movelist, bk, wh, hash, depth, empty, alpha, beta, color);

		/* 置換表で参照出来た手から先に着手するためにソート */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		/* 最初に最善手とおぼしき手を打って仮に最善手とする */
		iter = movelist->next;
		move = &(iter->move);
		if (depth < DEPTH_DEEP_TO_SHALLOW_SEARCH)
		{
			bestscore = -AlphaBetaSearch(wh^move->rev, bk ^ ((1ULL << move->pos) | move->rev),
				depth - 1, empty - 1, color ^ 1, -upper, -lower, 0);
		}
		else
		{
			bestscore = -PVS_SearchDeep(wh ^ move->rev, bk ^ ((1ULL << move->pos) | move->rev),
				depth - 1, empty - 1, color ^ 1, hash, -upper, -lower, 0);
		}

		// 最善手とする
		bestmove = move->pos;
		if (bestscore >= beta)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, NEGAMAX);
			return bestscore;   // fail-hard beta-cutoff
		}
		if (bestscore > lower) lower = bestscore;


		/* other moves : try to refute the first/best one */
		for (iter = iter->next; lower < upper && iter != NULL; iter = iter->next)
		{
			move = &(iter->move);
			if (depth < DEPTH_DEEP_TO_SHALLOW_SEARCH)
			{
				score = -AlphaBetaSearch(wh ^ move->rev, bk ^ ((1ULL << move->pos) | move->rev),
					depth - 1, empty - 1, color ^ 1, -lower - 1, -lower, 0);
				if (lower < score && score < upper)
				{
					score = -AlphaBetaSearch(wh^move->rev, bk ^ ((1ULL << move->pos) | move->rev),
						depth - 1, empty - 1, color ^ 1, -upper, -score, 0);
				}
			}
			else
			{
				score = -PVS_SearchDeep(wh^move->rev, bk ^ ((1ULL << move->pos) | move->rev),
					depth - 1, empty - 1, color ^ 1, hash, -lower - 1, -lower, 0);
				if (lower < score && score < upper)
					score = -PVS_SearchDeep(wh ^ move->rev, bk ^ ((1ULL << move->pos) | move->rev),
					depth - 1, empty - 1, color ^ 1, hash, -upper, -score, 0);
			}

			if (score >= beta)
			{
				bestscore = score;
				bestmove = move->pos;
				HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, NEGAMAX);
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (bestscore > lower) lower = bestscore;
			}
		}
	}

	/* 置換表に登録 */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, NEGAMAX);

	return bestscore;

}

INT32 AlphaBetaSearch(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
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

#if 1
	if (depth >= MPC_MIN_DEPTH)
	{
		if ((60 - empty) >= 36)
		{
			MPC_CUT_VAL = 1.4;
		}
		else
		{
			MPC_CUT_VAL = 1.0;
		}

		MPCINFO *mpcInfo_p = &mpcInfo[depth - MPC_MIN_DEPTH];
		INT32 value = (INT32)(alpha - (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMIN + 1) value = NEGAMIN + 1;
		INT32 eval = AlphaBetaSearch(bk, wh, mpcInfo_p->depth, empty, color, value - 1, value, passed);
		if (eval < value)
		{
			return alpha;
		}
		value = (INT32)(beta + (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AlphaBetaSearch(bk, wh, mpcInfo_p->depth, empty, color, value, value + 1, passed);
		if (eval > value)
		{
			return beta;
		}
	}
#endif
	int move_cnt;
	int max;                    //現在の最高評価値
	int eval;                   //評価値の保存
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
				max += 640000;
			}
			else if (max < 0)
			{
				max -= 640000;
			}

			return max;
		}
		max = -AlphaBetaSearch(wh, bk, depth, empty, color ^ 1, -beta, -alpha, 1);
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
			eval = -AlphaBetaSearch(wh ^ rev, bk ^ ((1ULL << pos) | rev),
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

INT32 OrderingAlphaBeta(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed)
{

	/* アボート処理 */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (depth == 0){
		/* 葉ノード(読みの限界値のノード)の場合は評価値を算出 */
		InitIndexBoard(bk, wh);
		return Evaluation(g_board, bk, wh, color, 59 - empty);
	}

	UINT32 move_cnt;
	INT32 max;                    //現在の最高評価値
	INT32 eval;                   //評価値の保存
	UINT64 rev;
	UINT64 moves;             //合法手のリストアップ

	/* 合法手生成とパスの処理 */
	if ((moves = CreateMoves(bk, wh, &move_cnt)) == 0){
		if (passed)
		{
			max = CountBit(bk) - CountBit(wh);
			if (max > 0)
			{
				max += 640000;
			}
			else if (max < 0)
			{
				max -= 640000;
			}
			else
			{
				max = 640000;
			}
		}
		else
		{
			max = -OrderingAlphaBeta(wh, bk, depth, empty, color ^ 1, -beta, -alpha, 1);
		}
	}
	else
	{
		int pos;
		max = NEGAMIN;
		do{
			/* 静的順序づけ（少ないコストで大幅に高速化するみたい） */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);

			eval = -OrderingAlphaBeta(wh^rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, empty - 1, color ^ 1, -beta, -alpha, 0);

			if (beta <= eval){
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