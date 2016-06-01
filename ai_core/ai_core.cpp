/***************************************************************************
* Name  : ai_core.cpp
* Brief : DLLエクスポート関数関連
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"
#include "board.h"
#include "move.h"
#include "book.h"
#include "eval.h"
#include "bit64.h"
#include "cpu.h"
#include "rev.h"

#define KZ_EXPORT extern "C" __declspec(dllexport)

BOOL m_BookFlag;

/***************************************************************************
* Name  : KZ_LibInit
* Brief : 初期化処理を行う
* Return: TRUE/FALSE
****************************************************************************/
KZ_EXPORT BOOL KZ_LibInit()
{
	BOOL result;
	
	// DLLのロード
	result = AlocMobilityFunc();

	if (result == TRUE)
	{
		// 定石データと評価テーブルのロード
		result = LoadData();
	}

	return result;
}

/***************************************************************************
* Name  : KZ_EnumGetCpuMove
* Brief : 着手可能手を列挙する
* Return: 着手可能位置のビット列
****************************************************************************/
KZ_EXPORT UINT64 KZ_GetEnumMove(UINT64 bk_p, UINT64 wh_p, UINT32 *p_count_p)
{
	return CreateMoves(bk_p, wh_p, p_count_p);
}

/***************************************************************************
* Name  : KZ_EnumGetCpuMove
* Brief : 変化する箇所を計算し、ビット列にして返却する
* Return: 変化する箇所のビット列
****************************************************************************/
KZ_EXPORT UINT64 KZ_GetBoardChangeInfo(UINT64 bk, UINT64 wh, INT32 move)
{
	return GetRev[move](bk, wh);
}

/***************************************************************************
* Name  : KZ_GetCpuMove
* Brief : 定石や評価値からCPUの着手を計算する
* Args  : bk 黒の盤面情報
*         wh 白の盤面情報
*         cpuConfig CPU設定クラス
* Return: 着手可能位置のビット列
****************************************************************************/
KZ_EXPORT UINT64 KZ_GetCpuMove(UINT64 bk, UINT64 wh, CPUCONFIG *cpuConfig)
{
	UINT64 move = MOVE_NONE;
	UINT32 emptyNum;
	UINT32 turn;

	emptyNum = CountBit(~(bk | wh));
	turn = 60 - emptyNum;

	if (emptyNum == 0)
	{
		return MOVE_PASS;
	}

	if (cpuConfig->bookFlag)
	{
		// 定石データから着手
		m_BookFlag = TRUE;
		move = GetMoveFromBooks(bk, wh, cpuConfig->color, 
			cpuConfig->bookVariability, turn);

	}

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// 定石に該当しない局面の場合
	if (move == MOVE_NONE)
	{
		if (cpuConfig->color == BLACK)
		{
			// 探索開始
			move = GetMoveFromAI(bk, wh, emptyNum, cpuConfig);
		}
		else
		{
			// 探索開始
			move = GetMoveFromAI(wh, bk, emptyNum, cpuConfig);
		}
	}

	return move;
}

/***************************************************************************
* Name  : KZ_GetLastEvaluation
* Brief : 直前にCPUの着手に対応する評価値を取得する
* Return: CPUの算出した直近評価値
****************************************************************************/
KZ_EXPORT INT32 KZ_GetLastEvaluation()
{
	return g_evaluation;
}

/***************************************************************************
* Name  : KZ_SendAbort
* Brief : AIスレッドに中断命令を送信
* Return: 着手可能位置のビット列
****************************************************************************/
KZ_EXPORT void KZ_SendAbort()
{
	SetAbortFlag();
}

/***************************************************************************
* Name  : KZ_CountBit
* Brief : １が立っているビット数を数える
* Args  : bit １が立っているビットを数える対象のビット列
* Return: １が立っているビット数
****************************************************************************/
KZ_EXPORT UINT32 KZ_CountBit(UINT64 bit)
{
	return CountBit(bit);
}