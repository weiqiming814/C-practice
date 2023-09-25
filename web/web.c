/*
 * Description: Get web infomation.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAXLINE 4096
#define SERV_PORT 80

static void perr_exit(const char *s)
{
	perror(s);
	exit(1);
}

int main(int argc, char *argv[]) 
{
	int sockfd, num;
	char buf[MAXLINE];
	struct hostent *he;
	struct sockaddr_in servaddr;
	char request[256];

	if(argc == 1)
	{
		printf("usage: ./web host\n");
		exit(1);
	}

	if ((he = gethostbyname(argv[1])) == NULL)
	{
	perr_exit("gethostbyname error");
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perr_exit("sockfd error");
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_addr = *((struct in_addr *)he->h_addr);

	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		perr_exit("connect error");
	}

	sprintf(request, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", argv[1]);

	if (send(sockfd, request, strlen(request), 0) == -1)
	{
		perr_exit("send error");
	}

	while ((num = recv(sockfd, buf, MAXLINE, 0)) > 0)
	{
		buf[num] = '\0';
		printf("%s", buf);
	}

	close(sockfd);

	return 0;
}
