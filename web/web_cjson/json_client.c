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

#define MAX_LINE 1024

typedef struct _URL {
    char *protocol;
    char *host;
    int port;
    char *path;
} URL;

URL url_build(char **params) {
    URL url;
    url.protocol = params[0];
    url.port     = atoi(params[2]);
    url.host     = params[1];
    url.path     = params[3];
    return url;
}

static void perr_exit(const char *s) {
    perror(s);
    exit(1);
}

void get_web_info(char *receive, char *buf, URL url) {
    int sockfd;
    struct hostent *he;
    struct sockaddr_in servaddr;
    char request[256] = {0};

    if ((he = gethostbyname(url.host)) == NULL) {
        perr_exit("gethostbyname error");
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perr_exit("sockfd error");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(url.port);
    servaddr.sin_addr   = *((struct in_addr *) he->h_addr);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perr_exit("send error");
    }

    snprintf(request, sizeof(request), "POST %s HTTP/1.1\r\nHost: %s\r\n"
            "Content-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s",
            url.path, url.host, strlen(buf), buf);

    if (send(sockfd, request, strlen(request), 0) == -1) {
        perr_exit("send error");
    }

    char *p;
    do {
        if (recv(sockfd, receive, sizeof(request), 0) < 0) {
        perr_exit("recv error");
        }
        p = strstr(receive, "{");
    } while (p == NULL);

    strncpy(receive, p, strlen(p));
    request[strlen(p)] = '\0';
    close(sockfd);
}


void json_parse(char *buf) {
    cJSON *cjson = cJSON_Parse(buf);

    if (cjson == NULL) {
        printf("parse fail.\n");
        return;
    }

    cJSON *cjson_resultCode   = cJSON_GetObjectItem(cjson, "resultCode");
    cJSON *cjson_errorCode    = cJSON_GetObjectItem(cjson, "errorCode");
    cJSON *cjson_errorDesc    = cJSON_GetObjectItem(cjson, "errorDesc");
    cJSON *cjson_result       = cJSON_GetObjectItem(cjson, "result");
    printf("resultCode : %s\n", cjson_resultCode->valuestring);
    printf("errorCode: %s\n", cjson_errorCode->valuestring);
    printf("errorDesc: %s\n", cjson_errorDesc->valuestring);
    printf("result: %s\n", cjson_result->valuestring);

    cJSON_Delete(cjson);
}

int main(void) {
    char buf[MAX_LINE];
    char receive[MAX_LINE];
    snprintf(buf, sizeof(buf), "{\"accountNo\":\"%s\",\"password\":\"%s\"}", "your_account_no", "your_password");

    char *params[] = {"http", "smart-mapi-test.iboxpay.com", "80",
        "/login/goodabase-gateway/mbox/authorizer/v1/login.json"};

    URL url = url_build(params);

    get_web_info(receive, buf, url);

    json_parse(receive);

    return 0;
}

