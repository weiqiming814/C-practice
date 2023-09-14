/*
 *Description:  Set stone.
 *
 * Copyright (C) Qiming Wei
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include "board.h"
#include "stone.h"

int turn;
int stone_count;

void init_stone()
{
	stone_count = 0;
	turn = BLACK_TURN;
}

void get_position(int *x, int *y)
{
	int position_ok = 0;
	int color;
	int next_turn;
	
	if (turn == BLACK_TURN)
	{
		color = BLACK;
		next_turn = WHITE_TURN;
	}
	else 
	{
		color = WHITE;
		next_turn = BLACK_TURN;
	}
        
	while (!position_ok)
	{ 	
		if (turn == BLACK_TURN){
			printf("请输入黑子下的坐标：");
		}
		else 
		{
			printf("请输入白子下的坐标：");
		}
		scanf("%d %d", x, y);
		if (board[*x][*y] == BLANK)
		{
			board[*x][*y] = color;
			turn = next_turn;
			position_ok = 1;
		}
		else 
		{
			printf("该坐标已有棋子%d\n", board[*x][*y]);
			continue;
		}
	}
}

void set_stone(int x, int y, int color)
{
	board[x][y] = color;
	stone_count++;
}
