/*
 * Description: Web client.
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

#define MAX_LINE 4096

typedef struct _URL {
    char protocol[8];
    char host[64];
    char port[8];
    char path[64];
}URL;

static void perr_exit(const char *s) {
    perror(s);
    exit(1);
}

URL url_parse(char url[]) {
    char port[8] = {0};
    char host[64] = {0};
    char path[64] = {0};
    char protocol[8] = {0};
    char buffer[4096] = {0};
    char *p;

    strcpy(buffer, url);

    if ((p = strstr(buffer, "://")) != NULL) {
        memcpy(protocol, buffer, strlen(buffer) - strlen(p));
        memmove(p, p + 3, strlen(p) - 2);
        strcpy(buffer, p);
    } else {
        strcpy(protocol, "http");
    }

    if ((p = strstr (buffer, "/")) != NULL) {
        memcpy(host, buffer, strlen(buffer) - strlen(p));
        strcpy(path, p);
    } else {
        strcpy(host, buffer);
        strcpy(path, "/");
    }

    if ((p = strstr(host, ":")) != NULL) {
        memmove(p, p + 1, strlen(p));
        strcpy(port, p);
        host[strlen(host) - strlen(p)] = '\0';
    } else {
        strcpy(port, "80");
    }

    URL u;
    strcpy(u.host, host);
    strcpy(u.port, port);
    strcpy(u.path, path);
    strcpy(u.protocol, protocol);

    return u;
}

void get_web_info(URL url) {
    int sockfd, num;
    char buf[MAX_LINE];
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
    servaddr.sin_port = htons(atoi(url.port));
    servaddr.sin_addr = *((struct in_addr *)he->h_addr);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perr_exit("connect error");
    }

    sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url.path, url.host);

    if (send(sockfd, request, strlen(request), 0) == -1) {
        perr_exit("send error");
    }

    while ((num = recv(sockfd, buf, MAX_LINE, 0)) > 0) {
        buf[num] = '\0';
        printf("%s\n", buf);
    }

    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s host\n", argv[0]);
        exit(1);
    }

    URL url = url_parse(argv[1]);

    get_web_info(url);

    return 0;
}

