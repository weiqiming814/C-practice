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

#include "cJSON.h"

#define MAX_LINE 4096

typedef struct _URL
{
    char protocol[8];
    char host[64];
    char port[8];
    char path[128];
}URL;

static void perr_exit(const char *s)
{
    perror(s);
    exit(1);
}

URL url_parse(char url[])
{
    char port[8] = {0};
    char host[64] = {0};
    char path[64] = {0};
    char protocol[8] = {0};
    char buffer[4096] = {0};
    char *p;

    strcpy(buffer, url);

    if ((p = strstr(buffer, "://")) != NULL)
    {
        strncpy(protocol, buffer, strlen(buffer) - strlen(p));
        memmove(buffer, buffer + strlen(protocol) + 3, strlen(buffer) - strlen(protocol) -2);
    }
    else
    {
        strcpy(protocol, "http");
    }

    if ((p = strstr (buffer, "/")) != NULL)
    {
        strncpy(host, buffer, strlen(buffer) - strlen(p));
        strcpy(path, p);
    }
    else
    {
        strcpy(host, buffer);
        strcpy(path, "/");
    }

    if ((p = strstr(host, ":")) != NULL)
    {
        memmove(p, p + 1, strlen(p));
        strcpy(port, p);
        host[strlen(host) - strlen(p)] = '\0';
    }
    else
    {
        strcpy(port, "80");
    }

    URL u;
    strcpy(u.protocol, protocol);
    strcpy(u.host, host);
    strcpy(u.port, port);
    strcpy(u.path, path);

    return u;
}

void get_web_info(char *buf,URL url)
{
    int sockfd, num;
    struct hostent *he;
    struct sockaddr_in servaddr;
    char request[256];

    if ((he = gethostbyname(url.host)) == NULL)
    {
        perr_exit("gethostbyname error");
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perr_exit("sockfd error");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(url.port));
    servaddr.sin_addr = *((struct in_addr *)he->h_addr);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perr_exit("connect error");
    }

    sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url.path, url.host);

    if (send(sockfd, request, strlen(request), 0) == -1)
    {
        perr_exit("send error");
    }

    while ((num = recv(sockfd, buf, MAX_LINE, 0)) > 0)
    {
        buf[num] = '\0';
    }
    char *p = strstr(buf, "{");
    //printf("%s\n", p);
    strncpy(buf, p, strlen(p));
    buf[strlen(p)] = '\0';
    close(sockfd);
}

void json_parse(char *buf)
{
    cJSON* cjson_test = cJSON_Parse(buf);
    if(cjson_test == NULL)
    {
        printf("parse fail.\n");
        return;
    }
    cJSON *cjson_error_message = cJSON_GetObjectItem(cjson_test, "error_message");
    cJSON *cjson_status = cJSON_GetObjectItem(cjson_test, "status");
    cJSON *cjson_routes = cJSON_GetObjectItem(cjson_test, "routes");
    int routes_array_size = cJSON_GetArraySize(cjson_routes);
    cJSON *arr_item = cjson_routes->child;
    
    printf("error_message: %s\n", cjson_error_message->valuestring);
    printf("routes:[ ");
    for(int i = 0; i <= routes_array_size - 1; i++)
    {
        if (routes_array_size == 0)
        {
            continue;
        }
        else
        {
            printf("%s",cJSON_Print(cJSON_GetObjectItem(arr_item,"test_1")));
            arr_item = arr_item->next;
        }
    } 
    printf("\b]\n");
    printf("status: %s\n", cjson_status->valuestring);
    
	cJSON_Delete(cjson_test);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: %s host\n", argv[0]);
        exit(1);
    }

    URL url = url_parse(argv[1]);
    char buf[MAX_LINE];
    get_web_info(buf, url);
    json_parse(buf);
    return 0;
}

