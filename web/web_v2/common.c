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

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <zlog.h>
#include <stdarg.h>
#include "common.h"

zlog_category_t *c;

void init_zlog() {
    int rc = zlog_init(LOG_FILE);
    if (rc) {
        printf("init failed, please check file %s.\n", LOG_FILE);
        exit(EXIT_FAILURE);
    }

    c = zlog_get_category("GetIoT");
    if (!c) {
        printf("get cat fail\n");
        zlog_fini();
        exit(EXIT_FAILURE);
    }
}

void error_exit(const char *s) {
    zlog_error(c, "%s", s);
    zlog_fini();
    exit(EXIT_FAILURE);
}

void zlog_print(const char* format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);

    zlog_info(c, "%s", buffer);

    va_end(args);
}

int file_exist(const char* filename) {
    FILE *file;
    if ((file = fopen(filename, "r")) != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
}

int is_executable(const char *buf) {
    char *ext = strstr(buf, ".");
    if (ext == NULL) {
        return 0;
    }
    if (strcmp(ext, ".cgi") == 0) {
        return 0;
    } else {
        return 1;
    }
}

void do_cat(int fd, FILE *file) {
    int c;

    while ((c = getc(file)) != EOF) {
        write(fd, &c, 1);
    }
}

int is_exec(const char *f) {
    struct stat st;

    if (stat(f, &st) < 0) {
        error_exit("stat no executable permission");
    }

    return st.st_mode & S_IEXEC;
}

void do_exec(int client_fd, char *path) {
    pid_t pid = fork();
    if (pid < 0) {
        error_exit("creat pid failed");
    } else if (pid > 0) {
        waitpid(-1, NULL, WNOHANG);
    } else {
        dup2(client_fd, STDOUT_FILENO);
        dup2(client_fd, STDERR_FILENO);
        close(client_fd);
        char *argv[] = {path, NULL};
        char *envp[] = {NULL};

        if (execve(path, argv, envp) < 0) {
            error_exit("executable file failed");
        }
    }
}

