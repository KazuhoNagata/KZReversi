/***************************************************************************
* Name  : book.cpp
* Brief : �ǖʂƒ�΃f�[�^���Ƃ炵���킹�Ē���
* Date  : 2016/02/01
****************************************************************************/
#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include "bit64.h"
#include "move.h"
#include "eval.h"
#include "board.h"
#include "book.h"

INT32 max_change_num[2];

/***************************************************************************
*
* Global
*
****************************************************************************/
/* �w����̉�]�E�Ώ̕ϊ��t���O */
int TRANCE_MOVE;

typedef UINT64 BitBoard;

typedef struct node
{
	struct node *child;
	struct node *next;
	BitBoard bk;
	BitBoard wh;
	int eval;
	short move;
	short depth;
}BooksNode;

BooksNode bookTree;


/***************************************************************************
* 
* ProtoType(private)
* 
****************************************************************************/
ULONG SearchBooks(BooksNode *book_root, BitBoard bk, BitBoard wh,
	ULONG color, ULONG change, ULONG turn);
BooksNode *SearchBookInfo(BooksNode *book_header, BooksNode *before_book_header,
	UINT64 bk, UINT64 wh, ULONG turn);
INT32 book_alphabeta(BooksNode *book_header, ULONG depth, LONG alpha, LONG beta,
	ULONG color, ULONG change, ULONG turn);
VOID SortBookNode(BooksNode *best_node[], INT32 e_list[], INT32 cnt);
INT32 SelectNode(INT32 e_list[], INT32 cnt, ULONG change, ULONG turn);

/***************************************************************************
* Name  : GetMoveFromBooks
* Brief : ��΂���CPU�̒�������肷��
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
UINT64 GetMoveFromBooks(UINT64 bk, UINT64 wh, UINT32 color, UINT32 change, UINT32 turn)
{
	UINT64 move;
	if (turn == 0)
	{
		// ���ڂ̒���͂ǂ��ɒ��肵�Ă������Ȃ̂Ń����_���Ƃ���
		UINT32 cnt;
		UINT64 enumMove = CreateMoves(bk, wh, &cnt);
		int rnd = rand() % cnt;

		while (rnd)
		{
			enumMove &= enumMove - 1;
			rnd--;
		}
		move = CountBit((enumMove & (-(INT64)enumMove)) - 1);
	}
	else
	{
		move = SearchBooks(bookTree.child, bk, wh, color, change, turn);
	}
	
	return 1ULL << move;

}

/***************************************************************************
* Name  : SearchBooks
* Brief : ��΂₩��CPU�̒�������肷��
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
ULONG SearchBooks(BooksNode *book_root, BitBoard bk, BitBoard wh, 
	ULONG color, ULONG change, ULONG turn)
{
	INT32 move = -1;
	ULONG eval = 0;
	BooksNode *book_header;

	srand((unsigned int)time(NULL));

	/* �ǖʂ���Y���̒�΂�T�� */
	book_header = SearchBookInfo(book_root, NULL, bk, wh, turn);

	if (book_header != NULL)
	{
		/* �]���l�ɂ�莟�̎���΂���I�� */
		eval = book_alphabeta(book_header, 0, NEGAMIN, NEGAMAX, color, change, turn);

		/* �w����̑Ώ̉�]�ϊ��̏ꍇ���� */
		switch (TRANCE_MOVE)
		{
		case 0:
			move = book_header->move;
			break;
		case 1:
		{
			BitBoard t_move = rotate_90(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 2:
		{
			BitBoard t_move = rotate_180(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 3:
		{
			BitBoard t_move = rotate_270(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 4:
		{
			BitBoard t_move = symmetry_x(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 5:
		{
			BitBoard t_move = symmetry_y(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 6:
		{
			BitBoard t_move = symmetry_b(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 7:
		{
			BitBoard t_move = symmetry_w(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		default:
			move = book_header->move;
			break;
		}
	}

	return move;
}

/***************************************************************************
* Name  : SearchBookInfo
* Brief : ��΂���CPU�̒�������肷��
* Return: ����\�ʒu�̃r�b�g��
****************************************************************************/
BooksNode *SearchBookInfo(BooksNode *book_header, BooksNode *before_book_header, 
	UINT64 bk, UINT64 wh, ULONG turn)
{
	/* �t�m�[�h�܂Ō������Č�����Ȃ��ꍇ */
	if (book_header == NULL)
	{
		return NULL;
	}
	if (book_header->depth > turn)
	{
		return NULL;
	}
	if (book_header->depth == turn)
	{
		if (turn == 0)
		{
			before_book_header = &bookTree;
		}
		/* �Y���̒�΂𔭌�(��]�E�Ώ̂��l����) */
		if (book_header->bk == bk && book_header->wh == wh)
		{
			/* �w����̉�]�E�Ώ̕ϊ��Ȃ� */
			TRANCE_MOVE = 0;
			return before_book_header;
		}
		/* 90�x�̉�]�` */
		if (book_header->bk == rotate_90(bk) && book_header->wh == rotate_90(wh))
		{
			TRANCE_MOVE = 1;
			return before_book_header;
		}
		/* 180�x�̉�]�` */
		if (book_header->bk == rotate_180(bk) && book_header->wh == rotate_180(wh))
		{
			TRANCE_MOVE = 2;
			return before_book_header;
		}
		/* 270�x�̉�]�` */
		if (book_header->bk == rotate_270(bk) && book_header->wh == rotate_270(wh))
		{
			TRANCE_MOVE = 3;
			return before_book_header;
		}
		/* X���̑Ώ̌` */
		if (book_header->bk == symmetry_x(bk) && book_header->wh == symmetry_x(wh))
		{
			TRANCE_MOVE = 4;
			return before_book_header;
		}
		/* Y���̑Ώ̌` */
		if (book_header->bk == symmetry_y(bk) && book_header->wh == symmetry_y(wh))
		{
			TRANCE_MOVE = 5;
			return before_book_header;
		}
		/* �u���b�N���C���̑Ώ̌` */
		if (book_header->bk == symmetry_b(bk) && book_header->wh == symmetry_b(wh))
		{
			TRANCE_MOVE = 6;
			return before_book_header;
		}
		/* �z���C�g���C���̑Ώ̌` */
		if (book_header->bk == symmetry_w(bk) && book_header->wh == symmetry_w(wh))
		{
			TRANCE_MOVE = 7;
			return before_book_header;
		}
	}

	BooksNode *ret;
	if ((ret = SearchBookInfo(book_header->child, book_header, bk, wh, turn)) != NULL)
	{
		return ret;
	}
	if ((ret = SearchBookInfo(book_header->next, book_header, bk, wh, turn)) != NULL)
	{
		return ret;
	}

	return NULL;
}

/***************************************************************************
* Name  : book_alphabeta
* Brief : ��΂̌���̂����A��X�ɕ]���l���ł������Ȃ���̂��Z�o����
* Return: ��΂̕]���l
****************************************************************************/
INT32 book_alphabeta(BooksNode *book_header, ULONG depth, LONG alpha, LONG beta, 
	ULONG color, ULONG change, ULONG turn)
{
	if (book_header->child == NULL)
	{
		if (color == WHITE)
		{
			return -book_header->eval;
		}
		return book_header->eval;
	}

	int i;

	if (depth == 0)
	{
		int e_list[24] =
		{
			NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN,
			NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN, NEGAMIN
		};
		int eval, max = NEGAMIN - 1;
		BooksNode *best_node[24] =
		{
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
		};

		i = 0;
		do{
			if (i == 0)
			{
				book_header = book_header->child;
			}
			else
			{
				book_header = book_header->next;
			}
			eval = -book_alphabeta(book_header, depth + 1, 
				-beta, -alpha, color ^ 1, change, turn);
			e_list[i] = eval;
			best_node[i] = book_header;
			if (eval > max)
			{
				max = eval;
			}
			i++;
		} while (book_header->next != NULL);

		SortBookNode(best_node, e_list, i);
		/* �m�[�h�ԍ����Z�o */
		INT32 node_num = SelectNode(e_list, i, change, turn);

		return e_list[node_num];
	}
	else
	{
		int eval, max = NEGAMIN - 1;
		i = 0;

		do{
			if (i == 0)
			{
				book_header = book_header->child;
			}
			else
			{
				book_header = book_header->next;
			}
			eval = -book_alphabeta(book_header, depth + 1, -beta, -alpha, 
				color ^ 1, change, turn);
			if (eval > max)
			{
				max = eval;
				if (max > alpha)
				{
					alpha = max;   //�����l���X�V
					/* �A���t�@�J�b�g */
					if (beta <= alpha)
					{
						break;
					}
				}
			}
			i++;
		} while (book_header->next != NULL);

		return max;
	}
}

/***************************************************************************
* Name  : SortBookNode
* Brief : ��΂̌���̂����A�]���l�̍������Ƀ\�[�g
****************************************************************************/
void SortBookNode(BooksNode *best_node[], int e_list[], int cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int swaps;
	BooksNode *temp;
	int int_temp;
	if (cnt == 1){ return; }
	while (1)
	{
		swaps = 0;
		for (i = 0; i + h < cnt; i++)
		{
			if (e_list[i] < e_list[i + h])
			{
				temp = best_node[i];
				best_node[i] = best_node[i + h];
				best_node[i + h] = temp;
				int_temp = e_list[i];
				e_list[i] = e_list[i + h];
				e_list[i + h] = int_temp;
				swaps++;
			}
		}
		if (h == 1)
		{
			if (swaps == 0)
			{
				break;
			}
		}
		else
		{
			h = h * 10 / 13;
		}
	}
}

/***************************************************************************
* Name  : SelectNode
* Brief : ��΂̕ω��x�ɂ���Č�������肷��
* Return: ��Δԍ�
****************************************************************************/
INT32 SelectNode(int e_list[], int cnt, ULONG change, ULONG turn)
{
	int ret;

	if (change == NOT_CHANGE)
	{
		int count;
		int max = e_list[0];
		for (count = 1; count < cnt; count++)
		{
			if (e_list[count] < max)
			{
				break;
			}
		}
		ret = rand() % count;
	}
	/* ���P�肩��-2�ȏ�܂ŋ��� */
	else if (change == CHANGE_LITTLE)
	{
		int count;
		int flag = 0;
		int max = e_list[0];
		for (count = 1; count < cnt; count++)
		{
			/* ��x���P���I��ł���A
			�܂��͎��P�肪���̕]���l���2000�ȏ�Ⴂ�Ƃ���break */
			if (max_change_num[turn] || max - e_list[count] >= 2000)
			{
				break;
			}
			if (e_list[count] < max)
			{
				if (flag)
				{
					break;
				}
				flag++;
			}

		}
		ret = rand() % count;
		/* ���P���I�� */
		if (e_list[ret] != max)
		{
			max_change_num[turn]++;
		}
	}
	else if (change == CHANGE_MIDDLE)
	{
		int count;
		int flag = 0;
		int max = e_list[0];
		for (count = 1; count < cnt; count++)
		{
			/* 2�x���P���I��ł���A
			�܂��͎��P�肪���̕]���l���4000�ȏ�Ⴂ�Ƃ���break */
			if (max_change_num[turn] > 1 || max - e_list[count] >= 4000)
			{
				break;
			}
			if (e_list[count] < max)
			{
				if (flag > 2)
				{
					break;
				}
				flag++;
			}
		}
		ret = rand() % count;
		/* ���P���I�� */
		if (ret != 0 && e_list[ret] != max)
		{
			max_change_num[turn]++;
		}
	}
	else
	{
		// ���S�����_��
		ret = rand() % cnt;
	}

	return ret;
}