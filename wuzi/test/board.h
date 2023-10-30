/**
 * Description:  Head file of board.
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
 * @file board.h
 * @brief 实现五子棋的棋盘
 * @details 细节
 * @mainpage 工程概览
 * @author Qiming Wei
 * @email weiqiming814@gmail.com
 * @version 1.0
 * @date 2023-10-30
 */

#ifndef WUZI_TEST_BOARD_H_
#define WUZI_TEST_BOARD_H_

#ifdef _cplusplus
extern "C" {
#endif

#define BOARD_WIDTH  19
#define BOARD_HEIGHT 19

    void init_board();

#ifdef _cplusplus
}
#endif

#endif  // WUZI_TEST_BOARD_H_

