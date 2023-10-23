/*
 * Description:  Head file of thousend separator.
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

#ifndef separator_H
#define separator_H

#ifdef _cplusplus
extern "C"{
#endif


void perr_exit(const char *s);
bool check_num(char *str);
int find_decimal(char *str);
void add_sep(char *str);
char *separator(char *dest, const char *src);
#ifdef _cplusplus
}
#endif

#endif

