/*
 *Description:  Judge whether won or not.
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
#include "win.h"

extern int stone_count;
extern int board[BOARD_WIDTH][BOARD_HEIGHT];

int judge_line(int x, int y, int x_step, int y_step)
{
	int x1;
	int y1;
	int stone;
	int count;
	int win = 0;
	x1 = x;
	y1 = y;

	if (stone_count >= 9)
	{
		stone = board[x1][y1];
		do
		{
			x1 -= x_step;
			y1 -= y_step;
		}
		while (x1 >= 0 && x1 < BOARD_WIDTH 
					&& y1 >= 0 
					&& y1 < BOARD_HEIGHT 
					&& stone == board[x1][y1]);
		count = 0;
		do
		{
			x1 += x_step;
			y1 += y_step;
			count++;
		}
		while (x1 >= 0 && x1 < BOARD_WIDTH 
					&& y1 >= 0 
					&& y1 < BOARD_HEIGHT 
					&& stone == board[x1][y1]);	
		if (count > 4)
		{
			win = 1;
		}
	}
	
	return win;
}

int is_win(int x, int y)
{
	int win = 0;
	
	win = judge_line(x, y, 1, 0); //horizon line
	if (win)
	{
		return win;
	}
	win = judge_line(x, y, 0, 1); //vertical line
	if (win)
	{
		return win;
	}
	win = judge_line(x, y, 1, -1); //45 degree line
	if (win)
	{
		return win;
	}
	win = judge_line(x, y, 1, 1);// 135 degree line
	if (win)
	{
		return win;
	}	
	return win;
}
