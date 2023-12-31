/*
 * Description: test file.
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
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "separator.h"

bool str_equal(const char *s1, const char *s2) {
    bool ret = false;
    ret = (strcmp(s1, s2) == 0);
    if (ret) {
        printf("test correct\n");
    }

    return ret;
}

int main() {
    char str[64] = {0};
    assert(str_equal(separator(str, "1234.1234"), "1,234.1234"));
    assert(str_equal(separator(str, "-1234.1234"), "-1,234.1234"));

    return 0;
}

