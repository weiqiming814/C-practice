/*
 *Description:  Head file of win.
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

#ifndef WIN_H
#define WIN_H

#ifdef _cplusplus

extern "C"
{

#endif

extern int judge_line(int x, int y, int x_step, int y_step);
extern int is_win(int x, int y);

#ifdef _cplusplus

}

#endif

#endif
