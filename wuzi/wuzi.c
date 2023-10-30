/*
 * Description:  wuzi game.
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

int main(void) {
    int x;
    int y;
    Color color;
    Turn turn;

    init_board();

    while (1) {
        if (color == BLACK_TURN) {
            color = BLACK;
        } else {
            color = WHITE;
        }

        get_position(&x, &y);
        set_stone(x, y, color);

        if (is_win(x, y)) {
            if (turn == BLACK_TURN) {
                printf("黑子获胜\n");
            } else {
                printf("白子获胜\n");
            }
            break;
        }
    }
}

