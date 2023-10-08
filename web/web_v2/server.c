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

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER 4096

typedef struct _MY_HTTPD_CONF {
	int port;
	char root_dir[256];
} MY_HTTPD_CONF;

static MY_HTTPD_CONF conf;

int read_config(const char *filename)
{
	if (filename == NULL)
	{
		printf("No file provided.\n");
		return -1;
	}

	FILE *config_file = fopen(filename, "r");
	if (config_file == NULL)
	{
		printf("No content in config file.\n");
		return -1;
	}

	char line[256];
	char *p;
	while (fgets(line, sizeof(line), config_file))
	{
		if ((p = strstr(line, "Port=")) != NULL)
		{
			memmove(p, p + 5, strlen(p) - 4);
			int port = atoi(p);
			conf.port = port;
		}
		if ((p = strstr(line, "Directory=")) != NULL)
		{
			memmove(p, p + 10, strlen(p) - 9);
			p[strlen(p)-2] = '\0';
			strcpy(conf.root_dir, p);
		}
	}

	fclose(config_file);

	return 0;
}

static void set_server(MY_HTTPD_CONF conf)
{
	int server_fd, client_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	char buffer[MAX_BUFFER] = {0};
	char response[MAX_BUFFER] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
	char index_file[256];

	strcpy(index_file, conf.root_dir);
	strcat(index_file, "/index.html");

	FILE *file = fopen(index_file, "r");
	if (file != NULL)
	{
		fread(response, 1, MAX_BUFFER - strlen(response), file);
		fclose(file);
	}
	else
	{
		strcat(response, "<html><body><h1>404 Not Found</h1></body></html>\n");
	}

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port        = htons(conf.port);

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

	printf("Server listening on port %d\n", conf.port);

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
}

int main(int argc, char *argv[])
{
	if (read_config("myhttpd.conf") != 0)
	{
		return -1;
	}
		
	set_server(conf);

	return 0;
}

