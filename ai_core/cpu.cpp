/***************************************************************************
* Name  : search.cpp
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
INT32 g_limitDepth;
UINT64 g_casheSize;

// CPU AI���
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

	if (cpuConfig->winLossDepth <= cpuConfig->exactDepth ||
		cpuConfig->color != BLACK && cpuConfig->color != WHITE)
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

	HashClear(g_hash);
	// ���̋ǖʂ̒u���\�����������Ă���
	int key = KEY_HASH_MACRO(bk, wh);

	UINT32 temp;
	// CPU�̓p�X
	if (CreateMoves(bk, wh, &temp) == 0){
		return MOVE_PASS;
	}

	// ���Ղ��ǂ������`�F�b�N
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


	// �u���\���璅����擾
	move = 1ULL << (g_hash->data[key].bestmove);
	
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

	/* ���OAI�ݒ�g���p(���͉����Ȃ�) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	// �����[���[���D��T��
	for (int count = 2; count <= g_limitDepth && eval != ABORT; count += 2)
	{
		eval_b = eval;
		eval = PvSearchMiddle(bk, wh, count, emptyNum, alpha, beta, color, g_hash, NO_PASS);

		// �ݒ肵�������]���l���Ⴂ���H
		if (eval <= alpha)
		{
			// �Ⴂ�Ȃ烿�������ɍĐݒ肵�Č���
			eval = PvSearchMiddle(bk, wh, count, emptyNum, NEGAMIN, eval, color, g_hash, NO_PASS);
		}
		// �ݒ肵�������]���l���������H
		if (eval >= beta)
		{
			// �����Ȃ��������ɍĐݒ肵�Č���
			eval = PvSearchMiddle(bk, wh, count, emptyNum, eval, NEGAMAX, color, g_hash, NO_PASS);
		}

		// ���̕����}8�ɂ��Č��� (��,��) ---> (eval - 8, eval + 8)
		alpha = eval - (8 * EVAL_ONE_STONE);
		beta = eval + (8 * EVAL_ONE_STONE);
	}

	// ���f���ꂽ�̂Œ��߂̊m��]���l��ԋp
	if (eval == ABORT){
		g_AbortFlag = FALSE;
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
	INT32 eval;

	/* ���OAI�ݒ�g���p(���͉����Ȃ�) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	eval = PvSearchMiddle(bk, wh, g_limitDepth, emptyNum, NEGAMIN, NEGAMAX, color, g_hash, NO_PASS);

	return eval;
}

/***************************************************************************
* Name  : SearchExact
* Brief : CPU�̒����΍��T���ɂ���Č��肷��
* Args  : bk        : ���̃r�b�g��
*         wh        : ���̃r�b�g��
*         empty     : �󔒃}�X�̐�
*         cpuConfig : CPU�̐ݒ�
* Return: ����]���l
****************************************************************************/
INT32 SearchExact(UINT64 bk, UINT64 wh, UINT32 emptyNum, UINT32 color)
{
	INT32 eval;

	/* ���OAI�ݒ�g���p(���͉����Ȃ�) */
	if (g_limitDepth > (INT32)emptyNum)
	{
		g_limitDepth = emptyNum;
	}

	eval = PvSearchMiddle(bk, wh, g_limitDepth, emptyNum, NEGAMIN, NEGAMAX, color, g_hash, NO_PASS);

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
INT32 PvSearchMiddle(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, 
	INT32 alpha, INT32 beta, UINT32 color, HashTable *hash, UINT32 pass_cnt)
{

	/* �A�{�[�g���� */
	if (g_AbortFlag == TRUE)
	{
		return ABORT;
	}

	g_countNode++;

	if (depth < 5 && g_limitDepth > 4)
	{
		// �t�ɋ߂��T��
		return AlphaBetaSearch(bk, wh, depth, 60 - empty, alpha, beta, color, pass_cnt);
	}
	else if (depth == 0){
		/* �t�m�[�h(�ǂ݂̌��E�l�̃m�[�h)�̏ꍇ�͕]���l���Z�o */
		InitIndexBoard(bk, wh);
		return Evaluation(g_board, bk, wh, color, 60 - empty);
	}

	BOOL entry_flag;
	int ret;
	int lower, upper;
	HashInfo hash_info;

	/************************************************************
	*
	* �u���\�J�b�g�I�t�t�F�[�Y
	*
	*************************************************************/
	/* �L�[�𐶐� */
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
	* Multi-Prob-Cut(MPC) �t�F�[�Y
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
	* �l�K�A���t�@�T���t�F�[�Y
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
		/* �u���\����O�̒T���ɂ�����őP����擾 */
		p = hash_info.bestmove;
		rev = GetRev[p](bk, wh);
		/* PV�l���擾�ł���ƐM���Ă���Ă݂�(��������ƒx���̂�������Ȃ��E�E�E�]���֐��̐��x�ɂ��) */
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

		// �ȍ~�C�O�̒T���̍őP�肪�őP�ł͂Ȃ��\��������ꍇ�ɒʂ�
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

		// �u���\�̍őP�������
		moves ^= (1ULL << p);
		move_cnt--;

		if (move_cnt != 0)
		{
			// ����̓K���ȏ����t��
			if (move_cnt > 1)
			{
				MoveOrderingMiddle(pos_list, bk, wh, hash, moves, rev_list,
					depth, empty, alpha, beta, color);
			}
			else
			{
				// �c�蒅�肪1�肵���Ȃ��ꍇ
				pos_list[0] = CountBit(moves - 1);
				rev_list[0] = GetRev[pos_list[0]](bk, wh);
			}

			for (int i = 0; i < move_cnt; i++)
			{
				p = pos_list[i];
				rev = rev_list[i];

				if (pv_flag == true)
				{
					// PV�l���擾�ł��Ă���̂�null-window�T��
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
					// PV�l���擾�ł��Ă��Ȃ��̂Œʏ핝�ł̒T��
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
		// �u���\�X�V
		HashUpdate(&hash_info, max_move, depth, max, alpha, beta, lower, upper);
		HashSet(hash, key, &hash_info);
	}
	else
	{
		/* ���@�萶���ƃp�X�̏��� */
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

		// ����̓K���ȏ����t��
		if (move_cnt > 1){
			MoveOrderingMiddle(pos_list, bk, wh, hash, moves, rev_list,
				depth, empty, alpha, beta, color);
		}
		else {
			// �c�蒅�肪1�肵���Ȃ��ꍇ
			pos_list[0] = CountBit(moves - 1);
			rev_list[0] = GetRev[pos_list[0]](bk, wh);
		}

		// �I�[�_�����O�̐擪�̎���őP�Ƃ��ĒT��
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
				// PV�l���擾�ł��Ă���̂�null-window�T��
				eval = -PvSearchMiddle(wh ^ rev, bk ^ ((1ULL << p) | rev), depth - 1, empty - 1,
					-(a_window + 1), -a_window, color ^ 1, hash, 0);
				if (eval > a_window && eval < beta){ // in fail-soft
					// re-search
					eval = -PvSearchMiddle(wh ^ rev, bk ^ ((1ULL << p) | rev), depth - 1, empty - 1,
						-beta, -eval, color ^ 1, hash, 0);
				}
			}
			else {
				// PV�l���擾�ł��Ă��Ȃ��̂Œʏ핝�ł̒T��
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

		/* �u���\�ɓo�^ */
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
	int max;                    //���݂̍ō��]���l
	int eval;                   //�]���l�̕ۑ�
	UINT64 rev;
	UINT64 moves;             //���@��̃��X�g�A�b�v

	/* ���@�萶���ƃp�X�̏��� */
	if ((moves = CreateMoves(bk, wh, (UINT32 *)(&move_cnt))) == 0)
	{
		if (pass_cnt == 1)
		{
			/* ����(1)�ƕ���(-1)����ш�������(0)�ł���΁A���ꑊ���̕]���l��Ԃ� */
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
			/* �ÓI�����Â��i���Ȃ��R�X�g�ő啝�ɍ���������݂����j */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);
			/* �^�[����i�߂čċA������ */
			eval = -AlphaBetaSearch(wh^rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, turn + 1, -beta, -alpha, color ^ 1, 0);
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

INT32 OrderingAlphaBeta(UINT64 bk, UINT64 wh, UINT32 depth,
	INT32 alpha, INT32 beta, UINT32 color, UINT32 turn, UINT32 pass_cnt)
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
		return Evaluation(g_board, bk, wh, color, turn - 1);
	}

	UINT32 move_cnt;
	INT32 max;                    //���݂̍ō��]���l
	INT32 eval;                   //�]���l�̕ۑ�
	UINT64 rev;
	UINT64 moves;             //���@��̃��X�g�A�b�v

	/* ���@�萶���ƃp�X�̏��� */
	if ((moves = CreateMoves(bk, wh, &move_cnt)) == 0){
		if (pass_cnt == 1)
		{
			/* ����(1)�ƕ���(-1)����ш�������(0)�ł���΁A���ꑊ���̕]���l��Ԃ� */
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
			/* �ÓI�����Â��i���Ȃ��R�X�g�ő啝�ɍ���������݂����j */
			pos = GetOrderPosition(moves);
			rev = GetRev[pos](bk, wh);

			eval = -OrderingAlphaBeta(wh^rev, bk ^ ((1ULL << pos) | rev),
				depth - 1, -beta, -alpha, color ^ 1, turn + 1, 0);

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

