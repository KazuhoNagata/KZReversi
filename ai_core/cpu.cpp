/***************************************************************************
* Name  : search.cpp
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
INT32 g_limitDepth;
UINT64 g_casheSize;

// CPU AI情報
BOOL g_AbortFlag;
UINT64 g_countNode;

HashTable *g_hash = NULL;

MPCINFO mpcInfo[22];
double MPC_CUT_VAL;


/***************************************************************************
*
* ProtoType(private)
*
****************************************************************************/
INT32 SearchMiddle(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);
INT32 SearchWinLoss(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);
INT32 SearchExact(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color);

INT32 PvSearchMiddle(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty,
	INT32 alpha, INT32 beta, UINT32 color, HashTable *hash, UINT32 pass_cnt);
INT32 AlphaBetaSearch(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty,
	INT32 alpha, INT32 beta, UINT32 color, UINT32 pass_cnt);

UINT32 *g_temp = 0;

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

	if (cpuConfig->winLossDepth <= cpuConfig->exactDepth ||
		cpuConfig->color != BLACK && cpuConfig->color != WHITE)
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

	HashClear(g_hash);
	// 今の局面の置換表を初期化しておく
	int key = KEY_HASH_MACRO(bk, wh);

	UINT32 temp;
	// CPUはパス
	if (CreateMoves(bk, wh, &temp) == 0){
		return MOVE_PASS;
	}

	// 中盤かどうかをチェック
	if (emptyNum <= cpuConfig->winLossDepth && emptyNum > cpuConfig->exactDepth)
	{
		g_limitDepth = cpuConfig->winLossDepth;
		g_evaluation = SearchWinLoss(bk, wh, emptyNum, cpuConfig->color);
	}
	else if (emptyNum <= cpuConfig->exactDepth)
	{
		g_limitDepth = cpuConfig->exactDepth;
		g_evaluation = SearchExact(bk, wh, emptyNum, cpuConfig->color);
	}
	else
	{
		g_limitDepth = cpuConfig->searchDepth;
		g_evaluation = SearchMiddle(bk, wh, emptyNum, cpuConfig->color);
	}


	// 置換表から着手を取得
	move = 1ULL << (g_hash->data[key].bestmove);
	
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

	/* 事前AI設定拡張用(今は何もない) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	// 反復深化深さ優先探索
	for (int count = 2; count <= g_limitDepth && eval != ABORT; count += 2)
	{
		eval_b = eval;
		eval = PvSearchMiddle(bk, wh, count, emptyNum, alpha, beta, color, g_hash, NO_PASS);

		// 設定した窓より評価値が低いか？
		if (eval <= alpha)
		{
			// 低いならαを下限に再設定して検索
			eval = PvSearchMiddle(bk, wh, count, emptyNum, NEGAMIN, eval, color, g_hash, NO_PASS);
		}
		// 設定した窓より評価値が高いか？
		if (eval >= beta)
		{
			// 高いならβを上限に再設定して検索
			eval = PvSearchMiddle(bk, wh, count, emptyNum, eval, NEGAMAX, color, g_hash, NO_PASS);
		}

		// 窓の幅を±8にして検索 (α,β) ---> (eval - 8, eval + 8)
		alpha = eval - (8 * EVAL_ONE_STONE);
		beta = eval + (8 * EVAL_ONE_STONE);
	}

	// 中断されたので直近の確定評価値を返却
	if (eval == ABORT){
		g_AbortFlag = FALSE;
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
	INT32 eval;

	/* 事前AI設定拡張用(今は何もない) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	eval = PvSearchMiddle(bk, wh, g_limitDepth, emptyNum, NEGAMIN, NEGAMAX, color, g_hash, NO_PASS);

	return eval;
}

/***************************************************************************
* Name  : SearchExact
* Brief : CPUの着手を石差探索によって決定する
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         empty     : 空白マスの数
*         cpuConfig : CPUの設定
* Return: 着手評価値
****************************************************************************/
INT32 SearchExact(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color)
{
	INT32 eval;

	/* 事前AI設定拡張用(今は何もない) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	eval = PvSearchMiddle(bk, wh, g_limitDepth, emptyNum, NEGAMIN, NEGAMAX, color, g_hash, NO_PASS);

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
INT32 PvSearchMiddle(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, 
	INT32 alpha, INT32 beta, UINT32 color, HashTable *hash, UINT32 pass_cnt)
{

	/* アボート処理 */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	if (depth < 5 && g_limitDepth > 4)
	{
		// 葉に近い探索
		return AlphaBetaSearch(bk, wh, depth, 60 - empty, alpha, beta, color, pass_cnt);
	}
	else if (depth == 0){
		/* 葉ノード(読みの限界値のノード)の場合は評価値を算出 */
		InitIndexBoard(bk, wh);
		return Evaluation(g_board, bk, wh, color, 60 - empty);
	}

	BOOL entry_flag;
	int ret;
	int lower, upper;
	HashInfo hash_info;

	/************************************************************
	*
	* 置換表カットオフフェーズ
	*
	*************************************************************/
	/* キーを生成 */
	UINT32 key = KEY_HASH_MACRO(bk, wh);
	if ((ret = HashGet(hash, key, bk, wh, &hash_info)) == TRUE)
	{
		if (hash_info.depth - depth >= 0)
		{
			lower = hash_info.lower;
			if (lower >= beta)
			{
				return lower;
			}
			upper = hash_info.upper;
			if (upper <= alpha || upper == lower)
			{
				return upper;
			}
			alpha = max(alpha, lower);
			beta = min(beta, upper);
		}
		else
		{
			hash_info.depth = depth;
			lower = NEGAMIN;
			upper = NEGAMAX;
		}
		entry_flag = TRUE;
	}
	else
	{
		hash_info.depth = depth;
		entry_flag = FALSE;
		lower = NEGAMIN;
		upper = NEGAMAX;
	}

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) フェーズ
	*
	*************************************************************/
#if 1
	if (depth >= MPC_MIN_DEPTH && depth <= 24)
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
		INT32 eval = AlphaBetaSearch(bk, wh, mpcInfo_p->depth, 60 - empty, value - 1, value, color, pass_cnt);
		if (eval < value) 
		{
			return alpha;
		}

		value = (INT32)(beta + (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value > NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AlphaBetaSearch(bk, wh, mpcInfo_p->depth, 60 - empty, value, value + 1, color, pass_cnt);
		if (eval > value) 
		{
			return beta;
		}
	}
#endif
	/************************************************************
	*
	* ネガアルファ探索フェーズ
	*
	*************************************************************/
	INT32 max, max_move;
	INT32 eval;
	INT32 move_cnt;
	UINT64 moves, rev;
	INT32 p;
	INT32 a_window = alpha;
	UINT64 rev_list[35];
	INT8 pos_list[35];

	bool pv_flag = false;
	max = NEGAMIN;

	if (entry_flag == TRUE)
	{
		hash_info.locked = TRUE;
		/* 置換表から前の探索における最善手を取得 */
		p = hash_info.bestmove;
		rev = GetRev[p](bk, wh);
		/* PV値を取得できると信じてやってみる(これをやると遅いのかもしれない・・・評価関数の精度による) */
		max_move = p;
		eval = -PvSearchMiddle(wh^rev, bk ^ ((1ULL << p) | rev), depth - 1, empty - 1,
			-beta, -a_window, color ^ 1, hash, 0);
		if (eval >= beta)
		{
			return beta;   // fail-soft beta-cutoff
		}
		if (eval > max)
		{
			a_window = max(a_window, eval);
			max = eval;
			if (eval > alpha) {
				pv_flag = true;
			}
		}

		// 以降，前の探索の最善手が最善ではない可能性がある場合に通る
		moves = CreateMoves(bk, wh, (UINT32 *)(&move_cnt));
		if (move_cnt == 0){
			if (pass_cnt == 1)
			{
				if (bk == 0)
				{
					return -2000064;
				}
				if (wh == 0)
				{
					return 2000064;
				}

				UINT32 bkCnt = CountBit(bk);
				UINT32 whCnt = CountBit(wh);

				if (bkCnt > whCnt)
				{
					return 2000000 + (bkCnt - whCnt);
				}
				else
				{
					return -2000000 + (bkCnt - whCnt);
				}
			}

			max = -PvSearchMiddle(wh, bk, depth - 1, empty, -beta, -alpha, color ^ 1, hash, 1);
			return max;
		}

		// 置換表の最善手を除去
		moves ^= (1ULL << p);
		move_cnt--;

		if (move_cnt != 0)
		{
			// 着手の適当な順序付け
			if (move_cnt > 1)
			{
				MoveOrderingMiddle(pos_list, bk, wh, hash, moves, rev_list,
					depth, empty, alpha, beta, color);
			}
			else
			{
				// 残り着手が1手しかない場合
				pos_list[0] = CountBit(moves - 1);
				rev_list[0] = GetRev[pos_list[0]](bk, wh);
			}

			for (int i = 0; i < move_cnt; i++)
			{
				p = pos_list[i];
				rev = rev_list[i];

				if (pv_flag == true)
				{
					// PV値を取得できているのでnull-window探索
					eval = -PvSearchMiddle(wh ^ rev, bk ^ ((1ULL << p) | rev), depth - 1, empty - 1,
						-(a_window + 1), -a_window, color ^ 1, hash, 0);
					if (eval > a_window && eval < beta)  // in fail-soft
					{
						// re-search
						eval = -PvSearchMiddle(wh ^ rev, bk ^ ((1ULL << p) | rev), depth - 1, empty - 1,
							-beta, -eval, color ^ 1, hash, 0);
					}
				}
				else
				{
					// PV値を取得できていないので通常幅での探索
					eval = -PvSearchMiddle(wh^rev, bk ^ ((1ULL << p) | rev), depth - 1, empty - 1,
						-beta, -a_window, color ^ 1, hash, 0);
				}

				if (eval >= beta)
				{
					return beta;   // fail-soft beta-cutoff
				}
				if (eval > max)
				{
					a_window = max(a_window, eval);
					max = eval;
					max_move = p;
					if (eval > alpha)
					{
						pv_flag = true;
					}
				}
			}
		}
		// 置換表更新
		HashUpdate(&hash_info, max_move, depth, max, alpha, beta, lower, upper);
		HashSet(hash, key, &hash_info);
	}
	else
	{
		/* 合法手生成とパスの処理 */
		moves = CreateMoves(bk, wh, (UINT32 *)(&move_cnt));
		if (move_cnt == 0){
			if (pass_cnt == 1)
			{
				if (bk == 0)
				{
					return -2000064;
				}
				if (wh == 0)
				{
					return 2000064;
				}

				UINT32 bkCnt = CountBit(bk);
				UINT32 whCnt = CountBit(wh);

				if (bkCnt > whCnt){
					return 2000000 + (bkCnt - whCnt);
				}
				else{
					return -2000000 + (bkCnt - whCnt);
				}
			}

			max = -PvSearchMiddle(wh, bk, depth - 1, empty, -beta, -alpha, color ^ 1, hash, 1);
			return max;
		}

		// 着手の適当な順序付け
		if (move_cnt > 1){
			MoveOrderingMiddle(pos_list, bk, wh, hash, moves, rev_list,
				depth, empty, alpha, beta, color);
		}
		else {
			// 残り着手が1手しかない場合
			pos_list[0] = CountBit(moves - 1);
			rev_list[0] = GetRev[pos_list[0]](bk, wh);
		}

		// オーダリングの先頭の手を最善として探索
		p = pos_list[0];
		rev = rev_list[0];
		max_move = p;

		eval = -PvSearchMiddle(wh^rev, bk ^ ((1ULL << p) | rev), depth - 1, empty - 1,
			-beta, -a_window, color ^ 1, hash, 0);
		if (eval >= beta)
		{
			return beta;   // fail-soft beta-cutoff
		}
		if (eval > max)
		{
			a_window = max(a_window, eval);
			max = eval;
		}

		for (int i = 1; i < move_cnt; i++){

			p = pos_list[i];
			rev = rev_list[i];

			if (pv_flag == true){
				// PV値を取得できているのでnull-window探索
				eval = -PvSearchMiddle(wh ^ rev, bk ^ ((1ULL << p) | rev), depth - 1, empty - 1,
					-(a_window + 1), -a_window, color ^ 1, hash, 0);
				if (eval > a_window && eval < beta){ // in fail-soft
					// re-search
					eval = -PvSearchMiddle(wh ^ rev, bk ^ ((1ULL << p) | rev), depth - 1, empty - 1,
						-beta, -eval, color ^ 1, hash, 0);
				}
			}
			else {
				// PV値を取得できていないので通常幅での探索
				eval = -PvSearchMiddle(wh^rev, bk ^ ((1ULL << p) | rev), depth - 1, empty - 1,
					-beta, -a_window, color ^ 1, hash, 0);
			}

			if (eval >= beta)
			{
				return beta;   // fail-soft beta-cutoff
			}
			if (eval > max)
			{
				a_window = max(a_window, eval);
				max = eval;
				max_move = p;
				if (eval > alpha) {
					pv_flag = true;
				}
			}
		}

		/* 置換表に登録 */
		if ((hash->data[key].locked == FALSE && ret != LOCKED))
		{
			HashCreate(&hash_info, bk, wh, max_move, move_cnt,
				depth, max, alpha, beta, lower, upper);
			HashSet(hash, key, &hash_info);
		}
	}

	return max;
}

INT32 AlphaBetaSearch(UINT64 bk, UINT64 wh, INT32 depth, INT32 turn,
	INT32 alpha, INT32 beta, UINT32 color, UINT32 pass_cnt)
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
		return Evaluation(g_board, bk, wh, color, turn - 1);
	}

	if (depth >= MPC_MIN_DEPTH) 
	{
		if (turn >= 36)
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
		INT32 eval = AlphaBetaSearch(bk, wh, mpcInfo_p->depth, turn, value - 1, value, color, pass_cnt);
		if (eval < value) 
		{
			return alpha;
		}
		value = (INT32)(beta + (mpcInfo_p->deviation * MPC_CUT_VAL) - mpcInfo_p->offset);
		if (value < NEGAMAX - 1) value = NEGAMAX - 1;
		eval = AlphaBetaSearch(bk, wh, mpcInfo_p->depth, turn, value, value + 1, color, pass_cnt);
		if (eval > value) 
		{
			return beta;
		}
	}

	int move_cnt;
	int max;                    //現在の最高評価値
	int eval;                   //評価値の保存
	UINT64 rev;
	UINT64 moves;             //合法手のリストアップ

	/* 合法手生成とパスの処理 */
	if ((moves = CreateMoves(bk, wh, (UINT32 *)(&move_cnt))) == 0)
	{
		if (pass_cnt == 1)
		{
			/* 勝ち(1)と負け(-1)および引き分け(0)であれば、それ相応の評価値を返す */
			if (bk == 0)
			{
				return -2000064;
			}
			else if (wh == 0)
			{
				return 2000064;
			}
			else 
			{
				INT32 bkCnt = CountBit(bk);
				INT32 whCnt = CountBit(wh);

				if (bkCnt >= whCnt)
				{
					return 2000000 + (bkCnt - whCnt);
				}
				else
				{
					return -2000000 + (bkCnt - whCnt);
				}
			}
		}
		max = -AlphaBetaSearch(wh, bk, depth - 1, turn, -beta, -alpha, color ^ 1, 1);

		return max;
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
			eval = -AlphaBetaSearch(wh^rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, turn + 1, -beta, -alpha, color ^ 1, 0);
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

INT32 OrderingAlphaBeta(UINT64 bk, UINT64 wh, UINT32 depth,
	INT32 alpha, INT32 beta, UINT32 color, UINT32 turn, UINT32 pass_cnt)
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
		return Evaluation(g_board, bk, wh, color, turn - 1);
	}

	UINT32 move_cnt;
	INT32 max;                    //現在の最高評価値
	INT32 eval;                   //評価値の保存
	UINT64 rev;
	UINT64 moves;             //合法手のリストアップ

	/* 合法手生成とパスの処理 */
	if ((moves = CreateMoves(bk, wh, &move_cnt)) == 0){
		if (pass_cnt == 1)
		{
			/* 勝ち(1)と負け(-1)および引き分け(0)であれば、それ相応の評価値を返す */
			if (bk == 0)
			{
				return -2000064;
			}
			else if (wh == 0)
			{
				return 2000064;
			}
			else
			{
				INT32 bkCnt = CountBit(bk);
				INT32 whCnt = CountBit(wh);

				if (bkCnt >= whCnt)
				{
					return 2000000 + (bkCnt - whCnt);
				}
				else
				{
					return -2000000 + (bkCnt - whCnt);
				}
			}
		}
		max = -OrderingAlphaBeta(wh, bk, depth, -beta, -alpha, color ^ 1, turn, 1);

		return max;
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
				depth - 1, -beta, -alpha, color ^ 1, turn + 1, 0);

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

