/*
 * Description:  set board.
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

int board[BOARD_WIDTH][BOARD_HEIGHT];

void init_board()
{
	int i;
	int j;
	
	for (i=0; i<BOARD_WIDTH; i++)
	{
		for (j=0; j<BOARD_HEIGHT; j++)
		{
			board[i][j] = BLANK;
		}
	}
}
