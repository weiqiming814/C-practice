/*
 *Description:  test file of wuzi game.
 *
 * Copyright (C) Qiming Wei
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
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

int position_h[][2] = {
	{1,1},
	{8,9},
	{1,2},
	{8,8},
	{1,3},
	{7,7},
	{1,4},
	{7,6},
	{1,5}
};
int position_v[][2] = {
	{1,1},
	{8,9},
	{2,1},
	{8,8},
	{3,1},
	{7,7},
	{4,1},
	{7,6},
	{5,1}
};
int position_45[][2] = {
	{1,1},
	{8,9},
	{2,2},
	{8,8},
	{3,3},
	{7,7},
	{4,4},
	{7,6},
	{5,5}
};
int position_135[][2] = {
	{5,1},
	{8,9},
	{4,2},
	{8,8},
	{3,3},
	{7,7},
	{2,4},
	{7,6},
	{1,5}
};

void test(int pos[][2], int len){
	int i =0;
	int x;
	int y;
	int color;
	int win = 0;
	printf("position len=%d\n", len);
	init_board();
	while (i < len)
	{
		if (turn == BLACK_TURN)
		{
			color = BLACK;
		}
		else
		{
			color =WHITE;
		}
		x = position_v[i][0];
		y = position_v[i][1];

		i++;
		set_stone(x, y, color);

		win = is_win(x, y);

		if (win)
		{
			break;
		}
	}
	if (win)
	{
		printf("test OK!\n");
	}
	else
	{
		printf("test failure!\n");
	}
}

int main ()
{
	test(position_v,sizeof(position_v)/sizeof(position_v[0]));
	test(position_h,sizeof(position_h)/sizeof(position_h[0]));
	test(position_45,sizeof(position_45)/sizeof(position_45[0]));
	test(position_135,sizeof(position_135)/sizeof(position_135[0]));

}
