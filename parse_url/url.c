/*
 * Description:  Parse the path and query string in the URL.
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
#include <stdlib.h>
#include <regex.h>
#include <stdbool.h>

#define MAX_PARAMS 8
#define MAX_PARAM_LEN 64

typedef struct _url_data_t {
    char path[MAX_PARAM_LEN];
    char keys[MAX_PARAMS][MAX_PARAM_LEN];
    char values[MAX_PARAMS][MAX_PARAM_LEN];
    int count;
} url_data_t;

bool check_url(const char str[]) {
    bool result = false;
    regex_t regex;
    int reti;
    reti = regcomp(&regex, "^(http|https)://[-a-zA-Z0-9@:%._\\+~#=]{1,256}\\"
            ".[a-zA-Z0-9()]{1,6}\\b([-a-zA-Z0-9()@:%_\\+.~#?&//=]*)$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }
    reti = regexec(&regex, str, 0, NULL, 0);
    if (!reti) {
        result = true;
    }
    regfree(&regex);
    return result;
}

url_data_t url_parse(const char *url) {
    url_data_t data;
    data.count = 0;

    char *url_copy = strdup(url);

    char *path = strtok(url_copy, "?");
    if (path != NULL) {
        strncpy(data.path, path, MAX_PARAM_LEN);
    }

    char *params = strtok(NULL, "");

    if (params != NULL)  {
        char *save_params;
        char *token = strtok_r(params, "&", &save_params);
        while (token != NULL) {
            char *save_token;
            char *key = strtok_r(token, "=", &save_token);
            char *value = strtok_r(NULL, "=", &save_token);

            strncpy(data.keys[data.count], key, MAX_PARAM_LEN);
            if (value == 0) {
                strncpy(data.values[data.count], "\0", 2);
            } else {
                strncpy(data.values[data.count], value, MAX_PARAM_LEN);
            }
            data.count++;
            token = strtok_r(NULL, "&", &save_params);
        }
    }

    free(url_copy);
    return data;
}

#ifdef URL_PARSE_TEST

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uage: %s url\n", argv[0]);
        return -1;
    }

    url_data_t data = url_parse(argv[1]);

    printf("Path: %s\n", data.path);
    for (int i = 0; i < data.count; i++) {
        printf("Key: %s, Value: %s\n", data.keys[i], data.values[i]);
    }

    return 0;
}

#endif /* URL_PARSE_TEST */

