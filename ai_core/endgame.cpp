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
* Name  : PVS_SearchDeepExact
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
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
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

	if (g_empty > 12 && empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
	{
		UINT64 blank = ~(bk | wh);
		UINT32 quad_parity[4];
		create_quad_parity(quad_parity, blank);
		return AB_SearchExact(bk, wh, blank, empty,
			color, alpha, beta, passed, pline);
	}

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &score))
	{
		return score;
	}

	/************************************************************
	*
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/
	/* �L�[�𐶐� */
	key = KEY_HASH_MACRO(bk, wh, color);
	/* transposition cutoff ? */
	if (g_tableFlag)
	{
		score = -INF_SCORE;
		// �r�������J�n
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->empty < empty)
			{
				if (upper > hashInfo->upper)
				{
					upper = hashInfo->upper;
					if (upper <= lower)
					{
						// transposition table cutoff
						return upper;
					}
				}
				if (lower < hashInfo->lower)
				{
					lower = hashInfo->lower;
					if (lower >= upper)
					{
						// transposition table cutoff
						return lower;
					}
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

	// �����flip-bit�����߂�move�\���̂ɕۑ�
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL)
	{
		if (passed) {
			// game end...
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
			pline->cmove = 0;
		}
		else
		{
			bestscore = -PVS_SearchDeepExact(wh, bk, empty, color ^ 1, hash, -upper, -lower, 1, pline);
			bestmove = NOMOVE;
		}
	}
	else {

		bool pv_flag = true;
		if (moveCount > 1)
		{
			if (empty > 15)
			{
				SortMoveListMiddle(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
			else if (empty > 6)
			{
				// ��̕��בւ�
				SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
		}

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		PVLINE line;
		bestscore = -INF_SCORE;
		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;
			// PV�\���L���p
			g_pvline[g_empty - empty] = move->pos;

			if (pv_flag)
			{
				score = -PVS_SearchDeepExact(move_w, move_b,
					empty - 1, color ^ 1, hash, -upper, -lower, 0, &line);
			}
			else
			{

				score = -PVS_SearchDeepExact(move_w, move_b,
					empty - 1, color ^ 1, hash, -lower - 1, -lower, 0, &line);

				if (lower < score && score < upper)
				{
					score = -PVS_SearchDeepExact(move_w, move_b,
						empty - 1, color ^ 1, hash, -upper, -lower, 0, &line);
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
					pv_flag = false;
					lower = score;
					pline->argmove[0] = bestmove;
					memcpy(pline->argmove + 1, line.argmove, line.cmove);
					pline->cmove = line.cmove + 1;
					if (g_empty - empty <= 6)
					{
						CreatePVLineStr(pline, empty, bestscore * (1 - (2 * ((g_empty - empty) % 2))));
						g_set_message_funcptr[1](g_PVLineMsg);
					}
				}
			}
		}
	}

	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);

	return bestscore;
}



/***************************************************************************
* Name  : PVS_SearchDeepExact_YBWC
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
INT32 PVS_SearchDeepExact_YBWC(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
{

	/* �A�{�[�g���� */
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
	* Principal Variation Search(PVS) �t�F�[�Y
	*
	*************************************************************/

	UINT32 move_cnt1, move_cnt2;
	UINT64 moves = CreateMoves(bk, wh, &move_cnt1);
	UINT64 move_b[4], move_w[4];

	// �����flip-bit�����߂�move�\���̂ɕۑ�
	StoreMovelist(movelist, bk, wh, moves);

	bool pv_flag = true;
	if (move_cnt1 > 1)
	{
		if (empty > 15)
		{
			SortMoveListMiddle(movelist, bk, wh, hash, empty, alpha, beta, color);
		}
		else if (empty > 6)
		{
			// ��̕��בւ�
			SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
		}
	}

	PVLINE line[4];
	int max_lower = -INF_SCORE;

	// ���ڂ��őP�Œ���
	iter = movelist->next;
	move[0] = &(iter->move);
	move_b[0] = bk ^ ((1ULL << move[0]->pos) | move[0]->rev);
	move_w[0] = wh ^ move[0]->rev;

	score = -PVS_SearchDeepExact(move_w[0], move_b[0],
		empty - 1, color ^ 1, hash, -upper, -lower[0], 0, &line[0]);

	// PV�̌��ʂőS�X���b�h�̃����X�V
	for (int n = 0; n < max_thread_num; n++) {
		lower[n] = score;
	}

	// 1��ڂ�YB
#pragma omp parallel for private(score, iter) schedule(dynamic, 1)
	for (int i0 = 1; i0 < (int)move_cnt1; i0++)
	{
		int tid = -1;
		iter = movelist->next;
		// iter��i0�̒l�ɂ���Ċ��߂�(���[�v���񎞂̂�)
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



INT32 AB_SearchExact(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 color, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	INT32 max;                    //���݂̍ō��]���l
	INT32 eval;                   //�]���l�̕ۑ�
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
		return SearchEmpty_5(bk, wh, blank, empty, parity, alpha, beta, 0, pline);
	}

	g_countNode++;

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &max)) return max;

	max = -INF_SCORE;

	UINT32 move_cnt;
	MoveList movelist[24], *iter = movelist;
	UINT64 moves = CreateMoves(bk, wh, &move_cnt);
	PVLINE line;

	if (move_cnt == 0)
	{
		// �łĂȂ�����
		if (passed)
		{
			max = GetEndScore[g_solveMethod](bk, wh, empty);
			pline->cmove = 0;
		}
		else
		{
			max = -AB_SearchExact(wh, bk, blank, empty,
				color ^ 1, -beta, -alpha, 1, pline);
		}
	}
	else
	{
		// �����flip-bit�����߂�move�\���̂ɕۑ�
		StoreMovelist(movelist, bk, wh, moves);
		if (move_cnt > 1) SortPotentionalFastfirst(movelist, bk, wh, blank);

		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			pos_bit = 1ULL << pos;
			rev = iter->move.rev;

			eval = -AB_SearchExact(wh ^ rev, bk ^ (pos_bit | rev),
				blank ^ pos_bit, empty - 1, color ^ 1, -beta, -alpha, 0, &line);

			if (beta <= eval)
			{
				return eval;
			}

			/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
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

	return max;

			}



/***************************************************************************
* Name  : PVS_SearchDeepWinLoss
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
INT32 PVS_SearchDeepWinLoss(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
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

	if (g_empty > 12 && empty <= EMPTIES_DEEP_TO_SHALLOW_SEARCH)
	{
		UINT64 blank = ~(bk | wh);
		UINT32 quad_parity[4];
		create_quad_parity(quad_parity, blank);
		return AB_SearchWinLoss(bk, wh, blank, empty, color, alpha, beta, passed, pline);
	}

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &score))
	{
		return score;
	}

	/************************************************************
	*
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/
	/* �L�[�𐶐� */
	key = KEY_HASH_MACRO(bk, wh, color);
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
					if (upper <= lower)
					{
						return upper;
					}
				}
				if (lower < hashInfo->lower)
				{
					lower = hashInfo->lower;
					if (lower >= upper)
					{
						return lower;
					}
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

	// ��������\��


	/************************************************************
	*
	* Principal Variation Search(PVS) �t�F�[�Y
	*
	*************************************************************/

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	UINT64 move_b, move_w;

	// �����flip-bit�����߂�move�\���̂ɕۑ�
	StoreMovelist(movelist, bk, wh, moves);

	if (movelist->next == NULL)
	{
		if (passed) {
			// game end...
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
			pline->cmove = 0;
		}
		else
		{
			bestscore = -PVS_SearchDeepWinLoss(wh, bk, empty, color ^ 1, hash, -upper, -lower, 1, pline);
			bestmove = NOMOVE;
		}
	}
	else {

		bool pv_flag = true;
		if (moveCount > 1)
		{
			if (empty > 15)
			{
				SortMoveListMiddle(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
			else if (empty > 6)
			{
				// ��̕��בւ�
				SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
		}

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		PVLINE line;
		bestscore = -INF_SCORE;
		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			move = &(iter->move);
			move_b = bk ^ ((1ULL << move->pos) | move->rev);
			move_w = wh ^ move->rev;
			// PV�\���L���p
			g_pvline[g_empty - empty] = move->pos;

			if (pv_flag)
			{
				score = -PVS_SearchDeepWinLoss(move_w, move_b,
					empty - 1, color ^ 1, hash, -upper, -lower, 0, &line);
			}
			else
			{
				score = -PVS_SearchDeepWinLoss(move_w, move_b,
					empty - 1, color ^ 1, hash, -lower - 1, -lower, 0, &line);

				if (lower < score && score < upper)
				{
					score = -PVS_SearchDeepWinLoss(move_w, move_b,
						empty - 1, color ^ 1, hash, -upper, -lower, 0, &line);
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
					pv_flag = false;
					lower = score;
					pline->argmove[0] = bestmove;
					memcpy(pline->argmove + 1, line.argmove, line.cmove);
					pline->cmove = line.cmove + 1;
					if (g_empty - empty <= 6)
					{
						CreatePVLineStr(pline, empty, bestscore * (1 - (2 * ((g_empty - empty) % 2))));
						g_set_message_funcptr[1](g_PVLineMsg);
					}
				}
			}
		}
	}
	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, INF_SCORE);


	return bestscore;
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
INT32 AB_SearchWinLoss(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 color, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
{
	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	INT32 max;                    //���݂̍ō��]���l
	INT32 eval;                   //�]���l�̕ۑ�
	INT32 pos;
	UINT64 pos_bit, rev;

	// parity moving
	if (empty == 4)
	{
		UINT32 parity =
			((CountBit(blank & quad_parity_bitmask[3]) % 2) << 3) |
			((CountBit(blank & quad_parity_bitmask[2]) % 2) << 2) |
			((CountBit(blank & quad_parity_bitmask[1]) % 2) << 1) |
			(CountBit(blank & quad_parity_bitmask[0]) % 2);
		return SearchEmpty_4(bk, wh, blank, empty, parity, alpha, beta, 0, pline);
	}

	g_countNode++;

	// stability cutoff
	if (search_SC_NWS(bk, wh, empty, alpha, &max)) return max;

	max = -INF_SCORE;

	UINT32 move_cnt;
	MoveList movelist[24], *iter = movelist;
	UINT64 moves = CreateMoves(bk, wh, &move_cnt);
	PVLINE line;

	if (move_cnt == 0)
	{
		// �łĂȂ�����
		if (passed)
		{
			max = GetExactScore(bk, wh, empty);
			pline->cmove = 0;
		}
		else
		{
			max = -AB_SearchExact(wh, bk, blank, empty,
				color ^ 1, -beta, -alpha, 1, pline);
		}
	}
	else
	{
		// �����flip-bit�����߂�move�\���̂ɕۑ�
		StoreMovelist(movelist, bk, wh, moves);
		if (move_cnt > 1) SortPotentionalFastfirst(movelist, bk, wh, blank);

		for (iter = iter->next; alpha < beta && iter != NULL; iter = iter->next)
		{
			pos = iter->move.pos;
			pos_bit = 1ULL << pos;
			rev = iter->move.rev;

			eval = -AB_SearchWinLoss(wh ^ rev, bk ^ (pos_bit | rev),
				blank ^ pos_bit, empty - 1, color ^ 1, -beta, -alpha, 0, &line);

			if (beta <= eval)
			{
				return eval;
			}

			/* ���܂ł��ǂ��ǖʂ�������΍őP��̍X�V */
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

	return max;
}