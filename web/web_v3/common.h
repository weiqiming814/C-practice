/**
 * Description:  Head file of common.
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
 * @file common.h
 * @brief 通用函数
 * @details 细节
 * @mainpage 工程概览
 * @author Qiming Wei
 * @email weiqiming814@gmail.com
 * @version 1.0
 * @date 2023-10-30
 */

#ifndef COMMON_H_
#define COMMON_H_

#ifdef _cplusplus
extern "C" {
#endif

#define LOG_FILE "./zlog.conf"

void zlog_print(const char* format, ...);
void init_zlog();
void error_exit(const char *s);
int file_exist(const char* filename);
int is_executable(const char *buf);
void do_cat(int fd, FILE *file);
int is_exec(const char *f);
void do_exec(int client_fd, char *path);

#ifdef _cplusplus
}
#endif

#endif  // COMMON_H_

