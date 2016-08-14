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
// CPU�ݒ�i�[�p
BOOL g_mpcFlag;
BOOL g_tableFlag;
INT32 g_solveMethod;
INT32 g_empty;
INT32 g_limitDepth;
UINT64 g_casheSize;

// CPU AI���
BOOL g_AbortFlag;
UINT64 g_countNode;
UINT32 g_move;

HashTable *g_hash = NULL;

MPCINFO mpcInfo[22];
double MPC_CUT_VAL;

char g_cordinates_table[64][4];
char g_AiMsg[128];
char g_PVLineMsg[256];
SetMessageToGUI g_set_message_funcptr[2];

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
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline);

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
			sprintf_s(msg, msglen, "guess... move %s [ %+d�`%+d ]", g_cordinates_table[move], eval - 8, eval + 8);
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
		strptr += 3; // 3���������炷
		bufsize -= 3;
	}

	// PV deeper line
	int count = pline->cmove;
	for (; i < count - 1; i++)
	{
		sprintf_s(strptr, bufsize, "%s-", g_cordinates_table[pline->argmove[i]]);
		strptr += 3; // 3���������炷
		bufsize -= 3;
	}

	sprintf_s(strptr, bufsize, "%s", g_cordinates_table[pline->argmove[i]]);
}



void CreatePVLineStr(INT32 *pline, INT32 empty, INT32 score)
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
		strptr += 3; // 3���������炷
		bufsize -= 3;
	}

	sprintf_s(strptr, bufsize, "%s", g_cordinates_table[pline[i]]);
}



BOOL boardMoves(UINT64 *bk, UINT64 *wh, UINT64 move, INT32 pos)
{

	if ((*bk & move) || (*wh & move))
	{
		return FALSE;
	}

	UINT64 rev = GetRev[pos](*bk, *wh);

	if (rev == 0)
	{
		return FALSE;
	}

	*bk ^= (rev | move);
	*wh ^= rev;

	return TRUE;

}



void StorePVLineToBoard(UINT64 bk, UINT64 wh, INT32 color, PVLINE *pline)
{
	BOOL ret;
	g_pvline_len = pline->cmove;

	for (int i = 0; i < g_pvline_len; i++)
	{
		g_pvline[i] = pline->argmove[i];
		if (color == BLACK)
		{
			g_pvline_board[BLACK][i] = bk;
			g_pvline_board[WHITE][i] = wh;
			ret = boardMoves(&bk, &wh, 1ULL << g_pvline[i], g_pvline[i]);
			color ^= 1;
		}
		else
		{
			g_pvline_board[BLACK][i] = wh;
			g_pvline_board[WHITE][i] = bk;
			ret = boardMoves(&wh, &bk, 1ULL << g_pvline[i], g_pvline[i]);
			color ^= 1;
		}

		if (ret == FALSE)
		{
			i--; // ��蒼��
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
bool search_SC_PVS(UINT64 bk, UINT64 wh, INT32 empty,
	volatile INT32 *alpha, volatile INT32 *beta, INT32 *score)
{
	if (*beta >= PVS_STABILITY_THRESHOLD[empty]) {
		*score = (64 - 2 * get_stability(wh, bk)) * EVAL_ONE_STONE;
		if (*score <= *alpha) {
			return true;
		}
		else if (*score < *beta) *beta = *score;
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
	int key = KEY_HASH_MACRO(bk, wh, cpuConfig->color);

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
	if (g_tableFlag)
	{
		move = 1ULL << (g_hash->entry[key].deepest.bestmove);
	}
	else
	{
		move = 1ULL << g_move;
	}

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
	UINT32 key = KEY_HASH_MACRO(bk, wh, color);
	INT32 move;
	PVLINE line;

	/* ���OAI�ݒ�g���p(���͉����Ȃ�) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	g_empty = emptyNum;
	g_solveMethod = SOLVE_MIDDLE;

	HashClear(g_hash);
	// �����[���[���D��T��
	for (int count = 2; count <= limit; count += 2)
	{
		// PV ��������
		memset(g_pvline, -1, sizeof(g_pvline));
		g_pvline_len = 0;

		eval_b = eval;
		g_limitDepth = count;
		eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, alpha, beta, NO_PASS, &line);

		if (eval == ABORT)
		{
			break;
		}

		// �ݒ肵�������]���l���Ⴂ���H
		if (eval <= alpha)
		{
			// �Ⴂ�Ȃ烿�������ɍĐݒ肵�Č���
			eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, NEGAMIN, NEGAMAX, NO_PASS, &line);
		}
		// �ݒ肵�������]���l���������H
		else if (eval >= beta)
		{
			// �����Ȃ��������ɍĐݒ肵�Č���
			eval = PVS_SearchDeep(bk, wh, count, emptyNum, color, g_hash, NEGAMIN, NEGAMAX, NO_PASS, &line);
		}

		// ���̕����}8�ɂ��Č��� (��,��) ---> (eval - 8, eval + 8)
		alpha = eval - (8 * EVAL_ONE_STONE);
		beta = eval + (8 * EVAL_ONE_STONE);

		// UI�Ƀ��b�Z�[�W�𑗐M
		move = g_move;
		CreateCpuMessage(g_AiMsg, sizeof(g_AiMsg), eval, move, count, ON_MIDDLE);
		g_set_message_funcptr[0](g_AiMsg);

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
	UINT32 key = KEY_HASH_MACRO(bk, wh, color);
	INT32 aVal;
	INT32 bVal;
	INT32 move;

	INT32 pv_depth = g_pvline_len - emptyNum;
	// PV�ʂ�̎�𒅎肵���ꍇ��PVLINE�������Q��
	if (pv_depth > 0 &&
		g_pvline_board[BLACK][pv_depth] == bk &&
		g_pvline_board[WHITE][pv_depth] == wh)
	{
		g_hash->entry[key].deepest.bestmove = g_pvline[pv_depth];

		// CPU�Ƀ��C������ʒm
		CreatePVLineStr(&g_pvline[pv_depth], emptyNum, g_evaluation);
		g_set_message_funcptr[1](g_PVLineMsg);

		return g_evaluation;
	}

#if 1
	g_limitDepth = emptyNum;

	if (g_limitDepth % 2) g_limitDepth--;
	if (g_limitDepth > 24)
	{
		g_limitDepth = 24;
	}

	if (g_limitDepth >= 12)
	{
		// ���O���s�T��
		eval = SearchMiddle(bk, wh, emptyNum, color);

		eval /= EVAL_ONE_STONE;
		eval -= eval % 2;

		// ���炩�ɍ�������ꍇ�͑S�Ŏ�̌���
		if (eval + 8 > 48) eval = 56;
		if (eval - 8 < -48) eval = -56;

		aVal = eval - 8;
		bVal = eval + 8;

		CreateCpuMessage(g_AiMsg, sizeof(g_AiMsg), eval, g_hash->entry[key].deepest.bestmove, -2, ON_EXACT);
		g_set_message_funcptr[0](g_AiMsg);

		// �u���\��΍��T���p�ɏ�����
		FixTableToExact(g_hash);
	}
	else
	{
		HashClear(g_hash);
		aVal = -INF_SCORE;
		bVal = INF_SCORE;
	}

#endif

	// PV ��������
	memset(g_pvline, -1, sizeof(g_pvline));
	g_pvline_len = 0;

	PVLINE line;
	g_solveMethod = SOLVE_EXACT;
	g_empty = emptyNum;
	// PVS�΍��T��
	eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, aVal, bVal, NO_PASS, &line);
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
		move = g_hash->entry[key].deepest.bestmove;
		sprintf_s(g_AiMsg, "guess move %s <= %+d", g_cordinates_table[move], eval);
		g_set_message_funcptr[0](g_AiMsg);
		// ���l����������̂ōĒT��
		eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, -INF_SCORE, eval + 2, NO_PASS, &line);

	}
	else if (eval >= bVal)
	{
		move = g_hash->entry[key].deepest.bestmove;
		sprintf_s(g_AiMsg, "guess move %s >= %+d", g_cordinates_table[move], eval);
		g_set_message_funcptr[0](g_AiMsg);
		// ���l���������̂ōĒT��
		eval = PVS_SearchDeepExact(bk, wh, emptyNum, color, g_hash, eval - 2, INF_SCORE, NO_PASS, &line);

	}

	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT)
	{
		return eval_b;
	}
	else
	{
		// PVLINE�ǖʂ�ۑ�
		StorePVLineToBoard(bk, wh, color, &line);
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
	UINT32 key = KEY_HASH_MACRO(bk, wh, color);

	g_limitDepth = emptyNum;
	if (g_limitDepth % 2) g_limitDepth--;
	if (g_limitDepth > 24)
	{
		g_limitDepth = 24;
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

		// �u���\��΍��T���p�ɏ�����
		FixTableToWinLoss(g_hash);
	}


	// PV ��������
	memset(g_pvline, -1, sizeof(g_pvline));
	g_pvline_len = 0;

	PVLINE line;
	g_solveMethod = SOLVE_WLD;
	g_empty = emptyNum;
	// PVS�΍��T��
	eval = PVS_SearchDeepWinLoss(bk, wh, emptyNum, color, g_hash, LOSS, WIN, NO_PASS, &line);
	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT)
	{
		return eval_b;
	}

	eval_b = eval;


	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT)
	{
		return eval_b;
	}

	return eval;
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
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	if ((g_limitDepth < DEPTH_DEEP_TO_SHALLOW_SEARCH && g_empty != empty) || 
		(g_limitDepth >= DEPTH_DEEP_TO_SHALLOW_SEARCH && depth < DEPTH_DEEP_TO_SHALLOW_SEARCH))
	{
		return AB_Search(bk, wh, depth, empty, color, alpha, beta, passed, pline);
	}

	g_countNode++;

	int score, bestscore, lower, upper, bestmove;
	UINT32 key;
	MoveList movelist[34], *iter;
	Move *move;
	HashInfo *hashInfo;

	bestmove = NOMOVE;
	lower = alpha;
	upper = beta;

	// stability cutoff
	if (search_SC_PVS(bk, wh, empty, &alpha, &beta, &score)) return score;

	/************************************************************
	*
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/

	/* transposition cutoff ? */
	if (g_tableFlag){
		/* �L�[�𐶐� */
		key = KEY_HASH_MACRO(bk, wh, color);
		hashInfo = HashGet(hash, key, bk, wh);
		if (hashInfo != NULL)
		{
			if (hashInfo->empty < empty)
			{
#if 1
				if (upper > hashInfo->upper) {
					upper = hashInfo->upper;
					if (upper <= lower) return upper;
				}
				if (lower < hashInfo->lower) {
					lower = hashInfo->lower;
					if (lower >= upper) return lower;
				}
#endif
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
	if (g_mpcFlag && depth >= MPC_MIN_DEPTH && depth <= 20)
	{
		if (empty <= 24)
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

	UINT32 moveCount;
	UINT64 moves = CreateMoves(bk, wh, &moveCount);
	bool pv_flag = true;

	// �����flip-bit�����߂�move�\���̂ɕۑ�
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
			else
			{
				bestscore = 1280000;
			}

			bestmove = NOMOVE;
			pline->cmove = 0;
		}
		else {
			bestscore = -PVS_SearchDeep(wh, bk, depth, empty, color ^ 1, hash, -upper, -lower, 1, pline);
			bestmove = PASS;
		}
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
				// ��̕��בւ�
				SortMoveListEnd(movelist, bk, wh, hash, empty, alpha, beta, color);
			}
		}

		/* �u���\�ŎQ�Əo�����肩���ɒ��肷�邽�߂Ƀ\�[�g */
		if (bestmove != NOMOVE && bestmove != movelist->next->move.pos)
		{
			SortMoveListTableMoveFirst(movelist, bestmove);
		}

		bestscore = NEGAMIN;
		/* other moves : try to refute the first/best one */
		for (iter = movelist->next; lower < upper && iter != NULL; iter = iter->next)
		{
			move = &(iter->move);
			// PV�\���L���p
			g_pvline[g_empty - empty] = move->pos;

			if (pv_flag)
			{
				score = -PVS_SearchDeep(wh ^ move->rev, bk ^ ((1ULL << move->pos) | move->rev),
					depth - 1, empty - 1, color ^ 1, hash, -upper, -lower, 0, &line);
			}
			else
			{
				score = -PVS_SearchDeep(wh^move->rev, bk ^ ((1ULL << move->pos) | move->rev),
					depth - 1, empty - 1, color ^ 1, hash, -lower - 1, -lower, 0, &line);
				if (lower < score && score < upper)
					score = -PVS_SearchDeep(wh ^ move->rev, bk ^ ((1ULL << move->pos) | move->rev),
					depth - 1, empty - 1, color ^ 1, hash, -upper, -lower, 0, &line);
			}

			if (score >= beta)
			{
				bestscore = score;
				bestmove = move->pos;
				if (depth == g_limitDepth) g_move = bestmove;
				break;
			}

			if (score > bestscore) {
				bestscore = score;
				bestmove = move->pos;
				if (depth == g_limitDepth) g_move = bestmove;
				if (bestscore > lower)
				{
					lower = bestscore;
					pv_flag = false;
					if (line.cmove < 0 || line.cmove > 59)
					{
						line.cmove = 0;
					}
					pline->argmove[0] = bestmove;
					memcpy(pline->argmove + 1, line.argmove, line.cmove);
					pline->cmove = line.cmove + 1;
					if (g_empty - empty <= 2)
					{
						CreatePVLineStr(pline, empty, bestscore * (1 - (2 * ((g_empty - empty) % 2))));
						g_set_message_funcptr[1](g_PVLineMsg);
					}
				}
			}
		}
	}

	/* �u���\�ɓo�^ */
	HashUpdate(hash, key, bk, wh, alpha, beta, bestscore, empty, bestmove, NEGAMAX);

	return bestscore;

}

INT32 AB_Search(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag)
	{
		return ABORT;
	}

	g_countNode++;

	if (depth == 0)
	{
		pline->cmove = 0;
		/* �t�m�[�h(�ǂ݂̌��E�l�̃m�[�h)�̏ꍇ�͕]���l���Z�o */
		InitIndexBoard(bk, wh);
		return Evaluation(g_board, bk, wh, color, 59 - empty);
	}

	int eval;
	// stability cutoff
	if (search_SC_PVS(bk, wh, empty, &alpha, &beta, &eval)) return eval;

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) �t�F�[�Y
	*
	*************************************************************/
#if 1
	if (g_mpcFlag && depth >= MPC_MIN_DEPTH && depth <= 24)
	{
		if (empty <= 24)
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
	int max;                    //���݂̍ō��]���l
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
			/* �ÓI�����Â��i���Ȃ��R�X�g�ő啝�ɍ���������݂����j */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);
			// PV�\���L���p
			g_pvline[g_empty - empty] = pos;

			/* �^�[����i�߂čċA������ */
			eval = -AB_Search(wh ^ rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, empty - 1, color ^ 1, -beta, -alpha, 0, &line);

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

	int eval;
	// stability cutoff
	if (search_SC_PVS(bk, wh, empty, &alpha, &beta, &eval)) return eval;

	/************************************************************
	*
	* Multi-Prob-Cut(MPC) �t�F�[�Y
	*
	*************************************************************/
#if 1
	if (g_mpcFlag && depth >= MPC_MIN_DEPTH && depth <= 24)
	{
		if (empty <= 24)
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
	int max;                    //���݂̍ō��]���l
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
			/* �ÓI�����Â��i���Ȃ��R�X�g�ő啝�ɍ���������݂����j */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);
			/* �^�[����i�߂čċA������ */
			eval = -AB_SearchNoPV(wh ^ rev, bk ^ ((1ULL << pos) | rev),
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