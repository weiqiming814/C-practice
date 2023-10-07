/*
 * Description: Web server.
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
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BUFFER 4096

int main(int argc, char *argv[])
{
	int server_fd, client_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	char buffer[MAX_BUFFER] = {0};
	char response[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"
		"<html>\n"
		"<body>\n"
		"<h1>Hello!</h1>\n"
		"</body>\n"
		"</html>\n";

	FILE * config_file = fopen("config.txt", "r");
	if (config_file == NULL)
	{
		printf("Could not open config file.\n");
		return -1;
	}

	int port;
	fscanf(config_file, "%d", &port);
	fclose(config_file);



	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port        = htons(port);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0)
	{
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	printf("Server listening on port %d\n", port);

	while (1)
	{
		printf("\nWaiting for a connection...\n");

		if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
		{
			perror("accept failed");
			exit(EXIT_FAILURE);
		}

		read(client_fd, buffer, MAX_BUFFER);
		printf("%s\n", buffer);

		write(client_fd, response, strlen(response));
		close(client_fd);
		
		// 解决close后的wait_time
		int val = 1;
		int ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
		if (ret == -1)
		{
			printf("setsockopt");
			exit(1);
		}
	}

	return 0;
}

