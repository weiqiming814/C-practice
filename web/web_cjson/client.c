/*
 * Description: Parse JSON.
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cjson/cJSON.h>

#define MAX_LINE 4096

typedef struct _URL {
    char protocol[8];
    char host[64];
    int  port;
    char path[128];
} URL;

URL url_build(char *str[]) {
    URL url;

    strncpy(url.protocol, str[0], strlen(str[0]));
    strncpy(url.host, str[1], strlen(str[1]));
    url.port = atoi(str[2]);
    strncpy(url.path, str[3], strlen(str[3]));
    return url;
}

static void perr_exit(const char *s) {
    perror(s);
    exit(1);
}

void get_web_info(char *buf, URL url) {
    int sockfd, num;
    struct hostent *he;
    struct sockaddr_in servaddr;
    char request[256];

    if ((he = gethostbyname(url.host)) == NULL) {
        perr_exit("gethostbyname error");
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perr_exit("sockfd error");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(url.port);
    servaddr.sin_addr = *((struct in_addr *) he->h_addr);

    if (connect(sockfd, (struct sockaddr *) &servaddr,
                sizeof(servaddr)) == -1) {
        perr_exit("connect error");
    }

    snprintf(request, sizeof(request),
            "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
            url.path, url.host);

    if (send(sockfd, request, strlen(request), 0) == -1) {
        perr_exit("send error");
    }

    while ((num = recv(sockfd, buf, MAX_LINE, 0)) > 0) {
        buf[num] = '\0';
    }

    char *p = strstr(buf, "{");
    strncpy(buf, p, strlen(p));
    buf[strlen(p)] = '\0';
    close(sockfd);
}

void json_parse(char *buf) {
    cJSON *cjson = cJSON_Parse(buf);
    if (cjson == NULL) {
        printf("parse fail.\n");
        return;
    }

    if (cJSON_GetObjectItem(cjson, "message") != NULL) {
        cJSON *cjson_message = cJSON_GetObjectItem(cjson, "message");
        cJSON *cjson_nu = cJSON_GetObjectItem(cjson, "nu");
        cJSON *cjson_ischeck = cJSON_GetObjectItem(cjson, "ischeck");
        cJSON *cjson_com = cJSON_GetObjectItem(cjson, "com");
        cJSON *cjson_status = cJSON_GetObjectItem(cjson, "status");
        cJSON *cjson_condition = cJSON_GetObjectItem(cjson, "condition");
        cJSON *cjson_state = cJSON_GetObjectItem(cjson, "state");
        cJSON *cjson_data = cJSON_GetObjectItem(cjson, "data");
        int routes_array_size = cJSON_GetArraySize(cjson_data);
        cJSON *arr_item = cjson_data->child;

        printf("message: %s\n", cjson_message->valuestring);
        printf("nu: %s\n", cjson_nu->valuestring);
        printf("ischeck: %s\n", cjson_ischeck->valuestring);
        printf("com: %s\n", cjson_com->valuestring);
        printf("status: %s\n", cjson_status->valuestring);
        printf("condition: %s\n", cjson_condition->valuestring);
        printf("state: %s\n", cjson_state->valuestring);
        printf("routes:[ ");
        for (int i = 0; i <= routes_array_size - 1; i++) {
            if (routes_array_size == 0) {
                continue;
            } else {
                printf("time:%s\n", cJSON_Print(cJSON_GetObjectItem(arr_item, "time")));
                printf("context:%s\n", cJSON_Print(cJSON_GetObjectItem(arr_item, "context")));
                printf("ftime:%s", cJSON_Print(cJSON_GetObjectItem(arr_item, "ftime")));
                arr_item = arr_item->next;
            }
        }
        printf("]\n");
    } else {
        cJSON *cjson_time = cJSON_GetObjectItem(cjson, "time");
        cJSON *cjson_context = cJSON_GetObjectItem(cjson, "context");
        cJSON *cjson_ftime = cJSON_GetObjectItem(cjson, "ftime");
        cJSON *cjson_location = cJSON_GetObjectItem(cjson, "location");

        printf("time: %s\n", cjson_time->valuestring);
        printf("context: %s\n", cjson_context->valuestring);
        printf("ftime: %s\n", cjson_ftime->valuestring);
        printf("location: %s\n", cjson_location->valuestring);
    }
    cJSON_Delete(cjson);
}

int main(void) {
    char buf[MAX_LINE];
    char *params[] = {"http", "www.kuaidi100.com", "80",
        "/query?type=shentong&postid=773248104204550"};

    URL url = url_build(params);

    get_web_info(buf, url);

    json_parse(buf);

    return 0;
}

