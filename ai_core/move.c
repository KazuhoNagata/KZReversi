/***************************************************************************
* Name  : move.cpp
* Brief : 着手関連の計算を行う
* Date  : 2016/02/01
****************************************************************************/
#include "stdafx.h"
#include "bit64.h"
#include "board.h"
#include "move.h"
#include "rev.h"

typedef void(*INIT_MMX)(void);

BIT_MOB g_bit_mob;
st_bit g_stbit_bk;
st_bit g_stbit_wh;


/***************************************************************************
* Name  : AlocMobilityFunc
* Brief : 着手可能手を列挙する関数をコールするためのDLLを初期化する
* Return: TRUE:DLLロード成功 FALSE:DLLロード失敗
****************************************************************************/
BOOL AlocMobilityFunc()
{
	HMODULE hDLL;

	if ((hDLL = LoadLibrary("mobility.dll")) == NULL)
	{
		return FALSE;
	}

	INIT_MMX init_mmx;
	init_mmx = (INIT_MMX)GetProcAddress(hDLL, "init_mmx");
	g_bit_mob = (BIT_MOB)GetProcAddress(hDLL, "bitboard_mobility");

	(init_mmx)();

	return TRUE;
}

/***************************************************************************
* Name  : CreateMoves
* Args  : bk - 黒の配置ビット列
* wh - 白の配置ビット列
* count - 着手可能数の格納変数
* Brief : 着手可能数と着手可能位置のビット列を計算する
* Return: 着手可能位置のビット列
****************************************************************************/
UINT64 CreateMoves(UINT64 bk_p, UINT64 wh_p, UINT32 *p_count_p)
{
#ifdef __AVX2__	
	UINT64 moves;
	__m256i	PP, mOO, MM, flip_l, flip_r, pre_l, pre_r, shift2;
	__m128i	M;
	const __m256i shift1897 = _mm256_set_epi64x(7, 9, 8, 1);
	const __m256i mflipH = _mm256_set_epi64x(0x7e7e7e7e7e7e7e7e, 0x7e7e7e7e7e7e7e7e, -1, 0x7e7e7e7e7e7e7e7e);

	PP = _mm256_broadcastq_epi64(_mm_cvtsi64_si128(bk_p));
	mOO = _mm256_and_si256(_mm256_broadcastq_epi64(_mm_cvtsi64_si128(wh_p)), mflipH);

	flip_l = _mm256_and_si256(mOO, _mm256_sllv_epi64(PP, shift1897));
	flip_r = _mm256_and_si256(mOO, _mm256_srlv_epi64(PP, shift1897));
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(mOO, _mm256_sllv_epi64(flip_l, shift1897)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(mOO, _mm256_srlv_epi64(flip_r, shift1897)));
	pre_l = _mm256_and_si256(mOO, _mm256_sllv_epi64(mOO, shift1897));
	pre_r = _mm256_srlv_epi64(pre_l, shift1897);
	shift2 = _mm256_add_epi64(shift1897, shift1897);
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(pre_l, _mm256_sllv_epi64(flip_l, shift2)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(pre_r, _mm256_srlv_epi64(flip_r, shift2)));
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(pre_l, _mm256_sllv_epi64(flip_l, shift2)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(pre_r, _mm256_srlv_epi64(flip_r, shift2)));
	MM = _mm256_sllv_epi64(flip_l, shift1897);
	MM = _mm256_or_si256(MM, _mm256_srlv_epi64(flip_r, shift1897));

	M = _mm_or_si128(_mm256_castsi256_si128(MM), _mm256_extracti128_si256(MM, 1));
	M = _mm_or_si128(M, _mm_unpackhi_epi64(M, M));
	moves = _mm_cvtsi128_si64(M) & ~(bk_p | wh_p);
	*p_count_p = CountBit(moves);

	return moves;

#else
	UINT64 moves;

	g_stbit_bk.high = (bk_p >> 32);
	g_stbit_bk.low = (bk_p & 0xffffffff);

	g_stbit_wh.high = (wh_p >> 32);
	g_stbit_wh.low = (wh_p & 0xffffffff);

	*p_count_p = g_bit_mob(g_stbit_bk, g_stbit_wh, &moves);
	return moves;
#endif

}




void StoreMovelist(MoveList *start, UINT64 bk, UINT64 wh, UINT64 moves)
{
	MoveList *list = start + 1, *previous = start;

	while (moves)
	{
		list->move.pos = CountBit((~moves) & (moves - 1));
		list->move.rev = get_rev(bk, wh, list->move.pos);
		previous = previous->next = list;
		moves ^= 1ULL << list->move.pos;
		list++;
	}
	previous->next = NULL;

}

/**
* @brief Get some potential moves.
*
* @param P bitboard with player's discs.
* @param dir flipping direction.
* @return some potential moves in a 64-bit unsigned integer.
*/
UINT64 GetSomePotentialMoves(const unsigned long long P, const int dir)
{
	return (P << dir | P >> dir);
}

/**
* @brief Get potential moves.
*
* Get the list of empty squares in contact of a player square.
*
* @param P bitboard with player's discs.
* @param O bitboard with opponent's discs.
* @return all potential moves in a 64-bit unsigned integer.
*/
UINT64 GetPotentialMoves(UINT64 P, UINT64 O)
{
	return (GetSomePotentialMoves(O & 0x7E7E7E7E7E7E7E7Eull, 1) // horizontal
		| GetSomePotentialMoves(O & 0x00FFFFFFFFFFFF00ull, 8)   // vertical
		| GetSomePotentialMoves(O & 0x007E7E7E7E7E7E00ull, 7)   // diagonals
		| GetSomePotentialMoves(O & 0x007E7E7E7E7E7E00ull, 9))
		& (~(P | O)); // mask with empties
}



INT32 bit_weighted_count(const UINT64 v)
{
	return CountBit(v) + CountBit(v & 0x8100000000000081ULL);
}

/**
 * @brief Get potential mobility.
 *
 * Count the list of empty squares in contact of a player square.
 *
 * @param P bitboard with player's discs.
 * @param O bitboard with opponent's discs.
 * @return a count of potential moves.
 */
INT32 get_potential_mobility(const UINT64 P, const UINT64 O)
{
	return bit_weighted_count(GetPotentialMoves(P, O));
}

/**
 * @brief Count legal moves.
 *
 * Compute mobility, ie the number of legal moves.
 *
 * @param P bitboard with player's discs.
 * @param O bitboard with opponent's discs.
 * @return a count of all legal moves.
 */
INT32 get_weighted_mobility(const UINT64 P, const UINT64 O)
{
	INT32 temp;
	UINT64 moves = CreateMoves(P, O, &temp);
	return temp + CountBit(moves & 0x8100000000000081ULL);
}


BOOL boardMoves(UINT64 *bk, UINT64 *wh, UINT64 move, INT32 pos)
{

	if ((*bk & move) || (*wh & move))
	{
		return FALSE;
	}

	UINT64 rev = get_rev(*bk, *wh, pos);

	if (rev == 0)
	{
		return FALSE;
	}

	*bk ^= (rev | move);
	*wh ^= rev;

	return TRUE;

}


void GenerateMoveList(MoveList *moves, UINT64 blank)
{
	MoveList *list = moves + 1, *previous = moves;

	while (blank)
	{
		list->move.pos = CountBit((~blank) & (blank - 1));
		previous = previous->next = list;
		blank ^= 1ULL << list->move.pos;
		list++;
	}
	previous->next = NULL;

}


MoveList *UpdateMoveList(MoveList *moves, INT32 pos)
{
	MoveList *iter;
	MoveList *previous;
	MoveList *idx;

	iter = moves->next;
	previous = moves;

	while (iter->move.pos != pos)
	{
		previous = iter;
		iter = iter->next;
	}
	
	idx = previous->next;
	previous->next = iter->next;

	return idx;
}



void RestoreMoveList(MoveList *moves, INT32 pos, MoveList *idx)
{
	MoveList *iter;
	MoveList *previous;

	iter = moves;
	previous = NULL;

	while (iter != NULL && iter < idx)
	{
		previous = iter;
		iter = iter->next;
	}

	if(previous != NULL) previous->next = idx;
	idx->next = iter;
}