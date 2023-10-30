/*
 * Description:  convert a string to an integer.
 *
 * Copyright (C) 2023 Qiming Wei
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
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include "my_atoi.h"

static int switch_type(const char *str);

int my_atoi(const char *nptr) {
    assert(nptr != NULL);
    return switch_type(nptr);
}

static int switch_type(const char *str) {
    int s = 0;
    bool falg = false;

    while (isspace(*str)) {
        str++;
    }

    if (*str == '-' || *str == '+') {
        if (*str == '-')
            falg = true;
        str++;
    }

    while (isdigit(*str)) {
        s = s * 10 + *str - '0';
        str++;
        if (s < 0) {
            perror("this number overflowed");
            exit(EXIT_FAILURE);
        }
    }
    return s * (falg?-1:1);
}

