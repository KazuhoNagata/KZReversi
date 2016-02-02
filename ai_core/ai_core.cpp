// KzAI_DLL.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "move.h"
#include "book.h"
#include "bit64.h"
#include "cpu.h"

#define KZ_EXPORT extern "C" __declspec(dllexport)

#define MOVE_NONE 0xFF

BOOL m_BookFlag;

KZ_EXPORT BOOL LibInit()
{
	BOOL result;
	
	// DLLのロード
	result = AlocMobilityFunc();

	// 定石データと評価テーブルのロード(TBI)
	//result = LoadData();

	return result;
}

/***************************************************************************
* Name  : EnumGetCpuMove
* Brief : 着手可能手を列挙する
* Return: 着手可能位置のビット列
****************************************************************************/
KZ_EXPORT UINT64 EnumGetMove(UINT64 bk_p, UINT64 wh_p, UINT32 *p_count_p)
{
	return CreateMoves(bk_p, wh_p, p_count_p);
}

/***************************************************************************
* Name  : GetCpuMove
* Brief : 定石や評価値からCPUの着手を計算する
* Return: 着手可能位置のビット列
****************************************************************************/
KZ_EXPORT UINT64 GetCpuMove(UINT64 bk, UINT64 wh, CPUCONFIG *cpuConfig)
{
	UINT64 move;
	UINT32 emptyNum;

	emptyNum = CountBit(~(bk | wh));

	if (cpuConfig->bookFlag)
	{
		// 定石データから着手
		m_BookFlag = TRUE;
		move = GetMoveFromBooks(bk, wh, cpuConfig->color, 
			cpuConfig->bookVariability, emptyNum);

	}

	// 定石に該当しない局面の場合
	if (move == MOVE_NONE)
	{
		// others
		move = GetMoveFromAI(bk, wh, emptyNum, cpuConfig);
	}

	return move;
}