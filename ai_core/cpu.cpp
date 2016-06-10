/***************************************************************************
* Name  : cpu.cpp
* Brief : �T���̏����S�ʂ��s��
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
// CPU�ݒ�i�[�p
BOOL g_mpcFlag;
BOOL g_tableFlag;
INT32 g_empty;
INT32 g_limitDepth;
UINT64 g_casheSize;

// CPU AI���
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
* Brief : CPU�̒�������t�h�ɑ��M����
* Args  : msg    : ���b�Z�[�W�i�[��
*         wh     : ���b�Z�[�W�i�[��̑傫��
*         eval   : �]���l
*         move   : ����ԍ�
*         cnt    : �[��
*         flag   : middle or winloss or exact
* Return: ����\�ʒu�̃r�b�g��
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
* Brief : CPU�̏����𒆒f����
****************************************************************************/
void SetAbortFlag(){
	g_AbortFlag = TRUE;
}

/***************************************************************************
* Name  : GetMoveFromAI
* Brief : CPU�̒����T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
UINT64 GetMoveFromAI(UINT64 bk, UINT64 wh, UINT32 emptyNum, CPUCONFIG *cpuConfig)
{
	UINT64 move;

	if (cpuConfig->color != BLACK && cpuConfig->color != WHITE)
	{
		// �ォ��n���ꂽ�p�����[�^���s��
		return ILLIGAL_ARGUMENT;
	}

	// �L���b�V����������΁A�L���b�V�����������m��(1MB�����͖�������)
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

	// ���̋ǖʂ̒u���\�����������Ă���
	int key = KEY_HASH_MACRO(bk, wh);

	UINT32 temp;
	// CPU�̓p�X
	if (CreateMoves(bk, wh, &temp) == 0){
		return MOVE_PASS;
	}

	g_empty = emptyNum;

	// ���Ղ��ǂ������`�F�b�N
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
	// �u���\���璅����擾
	move = 1ULL << (g_hash->entry[key].deepest.bestmove);

	return move;
}

/***************************************************************************
* Name  : SearchMiddle
* Brief : ���Ձ`���Ղ�CPU�̒����T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����]���l
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

	/* ���OAI�ݒ�g���p(���͉����Ȃ�) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	HashClear(g_hash);
	// �����[���[���D��T��
	for (int count = 2; count <= limit; count += 2)
	{
		eval_b = eval;
		g_limitDepth = count;
		eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, alpha, beta, NO_PASS);

		if (eval == ABORT)
		{
			break;
		}

		// �ݒ肵�������]���l���Ⴂ���H
		if (eval <= alpha)
		{
			// �Ⴂ�Ȃ烿�������ɍĐݒ肵�Č���
			eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, NEGAMIN, eval, NO_PASS);
		}
		// �ݒ肵�������]���l���������H
		else if (eval >= beta)
		{
			// �����Ȃ��������ɍĐݒ肵�Č���
			eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, eval, NEGAMAX, NO_PASS);
		}

		// ���̕����}8�ɂ��Č��� (��,��) ---> (eval - 8, eval + 8)
		alpha = eval - (8 * EVAL_ONE_STONE);
		beta = eval + (8 * EVAL_ONE_STONE);

		// UI�Ƀ��b�Z�[�W�𑗐M
		move = g_hash->entry[key].deepest.bestmove;
		CreateCpuMessage(msg, sizeof(msg), eval, move, count, ON_MIDDLE);
		g_set_message_funcptr(msg);

	}

	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT){
		return eval_b;
	}

	return eval;
}

/***************************************************************************
* Name  : SearchExact
* Brief : CPU�̒�������s�T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����]���l
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
		// ���O���s�T��
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

		// �u���\��΍��T���p�ɏ�����
		FixTableToExact(g_hash);
	}
	else
	{
		HashClear(g_hash);
		aVal = -INF_SCORE;
		bVal = INF_SCORE;
	}

	g_empty = emptyNum;
	// PVS�΍��T��
	eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, aVal, bVal, NO_PASS);
	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
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
		// PVS�΍��T��
		eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, -INF_SCORE, eval, NO_PASS);

	}
	else if (eval >= bVal)
	{
		// PVS�΍��T��
		eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, eval, INF_SCORE, NO_PASS);

	}

	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT)
	{
		return eval_b;
	}

	return eval;
}

/***************************************************************************
* Name  : SearchWinLoss
* Brief : CPU�̒�������s�T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����]���l
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
		// ���O�T��
		eval = SearchMiddle(bk, wh, emptyNum, color);
		// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
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
		// �u���\��΍��T���p�ɏ�����
		FixTableToWinLoss(g_hash);
	}
	else
	{
		aVal = LOSS;
		bVal = WIN;
	}

	g_empty = emptyNum;
	// PVS�΍��T��
	eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, aVal, bVal, NO_PASS);
	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT)
	{
		return eval_b;
	}

	eval_b = eval;

	// �����ߑł��T���Ɏ��s�������͕������Ɋg�債�čēx�T��
	if (eval <= aVal)
	{
		// PVS�΍��T��
		eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, LOSS, eval, NO_PASS);

	}
	else if (eval >= bVal)
	{
		// PVS�΍��T��
		eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, eval, WIN, NO_PASS);

	}

	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT)
	{
		return eval_b;
	}

	return eval;
}

/***************************************************************************
* Name  : PvSearchMiddle
* Brief : PV Search ���s���A�]���l����ɍőP����擾
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �ǂސ[��
*         empty     : �󂫃}�X��
*         alpha     : ���̃m�[�h�ɂ����鉺���l
*         beta      : ���̃m�[�h�ɂ��������l
*         color     : CPU�̐F
*         hash      : �u���\�̐擪�|�C���^
*         pass_cnt  : ���܂ł̃p�X�̐�(�Q�J�E���g�ŏI���Ƃ݂Ȃ�)
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 PVS_SearchDeepExact(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed)
{

	/* �A�{�[�g���� */
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
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/
	/* �L�[�𐶐� */
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
	* Multi-Prob-Cut(MPC) �t�F�[�Y
	*
	*************************************************************/

	// ��������\��


	/************************************************************
	*
	* Principal Variation Search(PVS) �t�F�[�Y
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;
	UINT32 quad_parity[4];

	// �����flip-bit�����߂�move�\���̂ɕۑ�
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
				// �󂫂������ԂŏI������̂ŋ󂫃}�X�������Ă�����ɉ��Z����
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
				/* ���՗p�̃I�[�_�����O */
				SortMoveListMiddle(movelist, bk, wh, hash, empty, empty, alpha, beta, color);
			}
			else if (empty >= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
			{
				SortMoveListEnd(movelist, bk, wh);
			}
		}

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		/* �ŏ��ɍőP��Ƃ��ڂ������ł��ĉ��ɍőP��Ƃ��� */
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

		// �őP��Ƃ���
		bestmove = move->pos;
		if (bestscore >= beta)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);
			if (empty == g_empty)
			{
				char msg[64];
				// UI�Ƀ��b�Z�[�W�𑗐M
				CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, 0, ON_EXACT);
				g_set_message_funcptr(msg);
			}
			return bestscore;   // fail-hard beta-cutoff
		}

		if (bestscore > lower) lower = bestscore;

		if (empty == g_empty)
		{
			char msg[64];
			// UI�Ƀ��b�Z�[�W�𑗐M
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
					// UI�Ƀ��b�Z�[�W�𑗐M
					CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, -1, ON_EXACT);
					g_set_message_funcptr(msg);
				}
				if (bestscore > lower) lower = bestscore;
			}
		}
	}
	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);

	if (empty == g_empty)
	{
		char msg[64];
		// UI�Ƀ��b�Z�[�W�𑗐M
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

	/* �łĂ�H */
	if (rev)
	{
		move_b = bk ^ (blank | rev);
		move_w = wh ^ rev;
		blank = 0;
	}
	else{
		rev = GetRev[pos](wh, bk);
		/* �łĂ�H */
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
	// �󂫃}�X�͏��������ɉ��Z����
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
	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (empty == 1)
	{
		return SearchEmpty_1(bk, wh, blank);
	}


	INT32 max;                    //���݂̍ō��]���l
	INT32 eval;                   //�]���l�̕ۑ�
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
			// parity�T���J�n
			while (moves)
			{
				/*
				�@�����ɗ���̂͂U�}�X�ȉ��̋󂫂Ȃ̂ŁACreateMoves���ĂԂ��
				 ���]�f�[�^�擾�ƍ��@����󂫃}�X���璼�ڃ`�F�b�N�����ق������|�I�ɑ���
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

					/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
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
			// parity�T���J�n
			while (moves)
			{
				/*
				�@�����ɗ���̂͂U�}�X�ȉ��̋󂫂Ȃ̂ŁACreateMoves���ĂԂ��
				 ���]�f�[�^�擾�ƍ��@����󂫃}�X���璼�ڃ`�F�b�N�����ق������|�I�ɑ���
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

					/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
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
		// �łĂȂ�����
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
				// �󂫃}�X�͏��������ɉ��Z����
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
* Brief : PVS���s���A�]���l����ɍőP����擾
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �ǂސ[��
*         empty     : �󂫃}�X��
*         alpha     : ���̃m�[�h�ɂ����鉺���l
*         beta      : ���̃m�[�h�ɂ��������l
*         color     : CPU�̐F
*         hash      : �u���\�̐擪�|�C���^
*         pass_cnt  : ���܂ł̃p�X�̐�(�Q�J�E���g�ŏI���Ƃ݂Ȃ�)
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 PVS_SearchDeepWinLoss(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed)
{

	/* �A�{�[�g���� */
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
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/
	/* �L�[�𐶐� */
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
	* Multi-Prob-Cut(MPC) �t�F�[�Y
	*
	*************************************************************/

	// ��������\��


	/************************************************************
	*
	* Principal Variation Search(PVS) �t�F�[�Y
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;
	UINT32 quad_parity[4];

	// �����flip-bit�����߂�move�\���̂ɕۑ�
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
				/* ���՗p�̃I�[�_�����O */
				SortMoveListMiddle(movelist, bk, wh, hash, empty, empty, alpha, beta, color);
			}
			else if (empty >= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
			{
				SortMoveListEnd(movelist, bk, wh);
			}
		}

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		/* �ŏ��ɍőP��Ƃ��ڂ������ł��ĉ��ɍőP��Ƃ��� */
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

		// �őP��Ƃ���
		bestmove = move->pos;

		if (bestscore >= beta)
		{
			HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);
			if (empty == g_empty)
			{
				char msg[64];
				// UI�Ƀ��b�Z�[�W�𑗐M
				CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, 0, ON_WINLOSS);
				g_set_message_funcptr(msg);
			}
			return bestscore;   // fail-hard beta-cutoff
		}

		if (bestscore > lower) lower = bestscore;

		if (empty == g_empty)
		{
			char msg[64];
			// UI�Ƀ��b�Z�[�W�𑗐M
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
					// UI�Ƀ��b�Z�[�W�𑗐M
					CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, -1, ON_WINLOSS);
					g_set_message_funcptr(msg);
				}
				if (bestscore > lower) lower = bestscore;
			}
		}
	}
	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);

	if (empty == g_empty)
	{
		char msg[64];
		// UI�Ƀ��b�Z�[�W�𑗐M
		CreateCpuMessage(msg, sizeof(msg), bestscore, bestmove, 0, ON_WINLOSS);
		g_set_message_funcptr(msg);
	}
	return bestscore;

}

/***************************************************************************
* Name  : SearchEmptyWinLoss_1
* Brief : ���s�T�����A�c��1�}�X�̎��̏������s��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �󂫃}�X�̃r�b�g��
* Return: WIN or LOSS or DRAW
****************************************************************************/
INT32 SearchEmptyWinLoss_1(UINT64 bk, UINT64 wh, UINT64 blank)
{
	INT32 pos = CountBit((~blank) & (blank - 1));
	UINT64 rev = GetRev[pos](bk, wh);
	UINT64 move_b, move_w;
	INT32 wld;

	g_countNode++;

	/* �łĂ�H */
	if (rev)
	{
		move_b = bk ^ (blank | rev);
		move_w = wh ^ rev;
	}
	else{
		rev = GetRev[pos](wh, bk);
		/* �łĂ�H */
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
* Brief : PV Search ���s���A�]���l����ɍőP����擾
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �ǂސ[��
*         empty     : �󂫃}�X��
*         alpha     : ���̃m�[�h�ɂ����鉺���l
*         beta      : ���̃m�[�h�ɂ��������l
*         color     : CPU�̐F
*         hash      : �u���\�̐擪�|�C���^
*         pass_cnt  : ���܂ł̃p�X�̐�(�Q�J�E���g�ŏI���Ƃ݂Ȃ�)
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 AlphaBetaSearchWinLoss(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty, 
	UINT32 *quad_parity, UINT32 color, INT32 alpha, INT32 beta, UINT32 passed)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (empty == 1)
	{
		return SearchEmptyWinLoss_1(bk, wh, blank);
	}

	INT32 max;                    //���݂̍ō��]���l
	INT32 eval;                   //�]���l�̕ۑ�
	INT32 pos;
	UINT64 pos_bit, rev;
	UINT64 moves = blank;

	// �p�X�p�̃t���O�����˂邽�߁Amin�l�𒲐�
	max = -INF_SCORE;


	// First, move odd parity empties
	for (int i = 0; i < 4; i++)
	{
		if (quad_parity[i])
		{
			// odd parity
			moves = blank & quad_parity_bitmask[i];
			// parity�T���J�n
			while (moves)
			{
				/*
				�@�����ɗ���̂͂U�}�X�ȉ��̋󂫂Ȃ̂ŁACreateMoves���ĂԂ��
				 ���]�f�[�^�擾�ƍ��@����󂫃}�X���璼�ڃ`�F�b�N�����ق������|�I�ɑ���
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

					/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
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
			// parity�T���J�n
			while (moves)
			{
				/*
				�@�����ɗ���̂͂U�}�X�ȉ��̋󂫂Ȃ̂ŁACreateMoves���ĂԂ��
				 ���]�f�[�^�擾�ƍ��@����󂫃}�X���璼�ڃ`�F�b�N�����ق������|�I�ɑ���
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

					/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
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
		// �łĂȂ�����
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
* Brief : PV Search ���s���A�]���l����ɍőP����擾
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         depth     : �ǂސ[��
*         empty     : �󂫃}�X��
*         alpha     : ���̃m�[�h�ɂ����鉺���l
*         beta      : ���̃m�[�h�ɂ��������l
*         color     : CPU�̐F
*         hash      : �u���\�̐擪�|�C���^
*         pass_cnt  : ���܂ł̃p�X�̐�(�Q�J�E���g�ŏI���Ƃ݂Ȃ�)
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
INT32 PVS_SearchDeep(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed)
{

	/* �A�{�[�g���� */
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
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/
	/* �L�[�𐶐� */
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
	* Multi-Prob-Cut(MPC) �t�F�[�Y
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
	// �����flip-bit�����߂�move�\���̂ɕۑ�
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
		/* ���՗p�̃I�[�_�����O */
		SortMoveListMiddle(movelist, bk, wh, hash, depth, empty, alpha, beta, color);

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		/* �ŏ��ɍőP��Ƃ��ڂ������ł��ĉ��ɍőP��Ƃ��� */
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

		// �őP��Ƃ���
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

	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, NEGAMAX);

	return bestscore;

}

INT32 AlphaBetaSearch(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (depth == 0)
	{
		/* �t�m�[�h(�ǂ݂̌��E�l�̃m�[�h)�̏ꍇ�͕]���l���Z�o */
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
	int max;                    //���݂̍ō��]���l
	int eval;                   //�]���l�̕ۑ�
	UINT64 rev;
	UINT64 moves;             //���@��̃��X�g�A�b�v

	/* ���@�萶���ƃp�X�̏��� */
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
			/* �ÓI�����Â��i���Ȃ��R�X�g�ő啝�ɍ���������݂����j */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);
			/* �^�[����i�߂čċA������ */
			eval = -AlphaBetaSearch(wh ^ rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, empty - 1, color ^ 1, -beta, -alpha, 0);

			if (beta <= eval)
			{
				return eval;
			}

			/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
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

	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (depth == 0){
		/* �t�m�[�h(�ǂ݂̌��E�l�̃m�[�h)�̏ꍇ�͕]���l���Z�o */
		InitIndexBoard(bk, wh);
		return Evaluation(g_board, bk, wh, color, 59 - empty);
	}

	UINT32 move_cnt;
	INT32 max;                    //���݂̍ō��]���l
	INT32 eval;                   //�]���l�̕ۑ�
	UINT64 rev;
	UINT64 moves;             //���@��̃��X�g�A�b�v

	/* ���@�萶���ƃp�X�̏��� */
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
			/* �ÓI�����Â��i���Ȃ��R�X�g�ő啝�ɍ���������݂����j */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);

			eval = -OrderingAlphaBeta(wh^rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, empty - 1, color ^ 1, -beta, -alpha, 0);

			if (beta <= eval){
				return eval;
			}

			/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
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