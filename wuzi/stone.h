/*
 *Description:  Head file of stone.
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

#ifndef STONE_H
#define STONE_H

#ifdef _cplusplus
extern "C"{
#endif

enum Color
{
	WHITE = 1,
	BLANK = 0,
	BLACK = -1
};

enum Turn
{
	BLACK_TURN = 0,
	WHITE_TURN = 1
};

extern int stone_count;

void init_stone();
void get_position(int *x, int *y);
void set_stone(int x, int y, int coloer);

#ifdef _cplusplus
}
#endif

#endif
