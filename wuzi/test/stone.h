/*
 * Description:  Head file of stone.
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
 *
 * @file stone.h
 * @brief 五子棋的棋子
 * @details 细节
 * @mainpage 工程概览
 * @author Qiming Wei
 * @email weiqiming814@gmail.com
 * @version 1.0
 * @date 2023-10-30
 */

#ifndef WUZI_TEST_STONE_H_
#define WUZI_TEST_STONE_H_

#ifdef _cplusplus
extern "C" {
#endif

    enum Color {
        WHITE = 1,
        BLANK = 0,
        BLACK = -1
    };
    typedef enum Color Color;

    enum Turn {
        BLACK_TURN = 0,
        WHITE_TURN = 1
    };
    typedef enum Turn Turn;

    void init_stone();
    void get_position(int *x, int *y);
    void set_stone(int x, int y, int coloer);

#ifdef _cplusplus
}
#endif

#endif  // WUZI_TEST_STONE_H_

