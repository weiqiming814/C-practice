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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_BUFFER 1024

typedef struct _MY_HTTPD_CONF {
	int port;
	char root_dir[256];
} MY_HTTPD_CONF;

static MY_HTTPD_CONF conf;

int read_config(const char *filename)
{
	char line[256];

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

	while (fgets(line, sizeof(line), config_file))
	{
		char *key = strtok(line, "=");
		char *value = strtok(NULL, "=");

		if (strcmp(key, "Port") == 0)
		{
			conf.port = atoi(value);
		}
		else if (strcmp(key, "Directory") == 0)
		{
			strncpy(conf.root_dir, value, strlen(value) - 1);
		}
	}

	fclose(config_file);

	return 0;
}

static char *get_url(const char* request)
{
	char buf[MAX_BUFFER] = {0};
	static char url[MAX_BUFFER] = {0};

	strncpy(buf, request, strlen(request) + 1);
	strtok(buf, " ");
	char *url_tmp = strtok(NULL, " ");
	strncpy(url, url_tmp, strlen(url_tmp) + 1);

	if (url[0] == '/' && url[1] == '\0')
	{
		strcpy(url, "/index.html");
	}
	return url;
}

int file_exist(const char* filename)
{
	FILE *file;
	if ((file = fopen(filename, "r")) != NULL)
	{
		fclose(file);
		return 1;
	}
	return 0;
}

static char *get_content_type(const char *url)
{
	char *ext;
	char *content_type;
	ext = strstr(url, ".");
	if (ext == NULL)
	{
		content_type = "text/html";
		return content_type;
	}
	ext = ext + 1;
	if (strcasecmp(ext, "html") == 0)
	{
		content_type = "text/html";
	}
	else if (strcasecmp(ext, "jpg") == 0)
	{
		content_type = "image/jpeg";
	}
	else if (strcasecmp(ext, "ico") == 0)
	{
		content_type = "image/x-icon";
	}
	else if (strcasecmp(ext, "cgi") == 0)
	{
		content_type = "text/html";
	}
	else
	{
		content_type = "text/plain";
	}

	return content_type;
}
	
static long get_content_length(char *filename)
{
	FILE *fp = NULL;

	if ((fp = fopen(filename, "r")) == NULL)
	{
		printf("获取失败");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fclose(fp);
	return file_size;
}

int is_executable (const char *buf)
{
	char *buf1;
	buf1 = strstr(buf, ".");
	if (buf1 == NULL)
	{
		return 0;
	}
	if (strcmp(buf1, ".cgi") == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

static char *get_head(const char *url)
{
	char index_file[256];
	static char head[MAX_BUFFER] = {0};
	snprintf(index_file, sizeof(index_file), "%s%s", conf.root_dir, url);
	char *content_type = get_content_type(url);
	long content_length = get_content_length(index_file);

	if (file_exist(index_file) && !is_executable(url))
	{
		snprintf(head, MAX_BUFFER, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n"
						"Connection: close\r\n"
						"\r\n",
						content_type);
	}
	else if (file_exist(index_file) && is_executable(url))
	{
		snprintf(head, MAX_BUFFER, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n"
						"Content-Length: %ld\r\n"
						"Connection: close\r\n"
						"\r\n",
						content_type, content_length);
	}
	else
	{
		snprintf(head, MAX_BUFFER, "HTTP/1.1 404 Not Found\r\nContent-Type: %s\r\n"
						"\r\nrequest file not found\r\n"
						"Connection: close\r\n"
						"\r\n",
						content_type);
	}

	return head;
}

void do_cat(int fd, FILE *file)
{
	int c;

	while ((c = getc(file)) != EOF)
	{
		write(fd, &c, 1);
	}
}

static void do_cat_executable(int client_fd,const char *url)
{
	char path[256];
	struct stat file_stat;
	snprintf(path, sizeof(path), "%s%s", conf.root_dir, url);
	if (stat(path, &file_stat) < 0)
	{
		printf("stat erroe\n");
		exit(1);
	}
	if (!(file_stat.st_mode & S_IXUSR) || !(file_stat.st_mode & S_IXGRP) || !(file_stat.st_mode & S_IXOTH))
	{
		printf("no executable permission\n");
		exit(1);
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		printf("creat fork failed\n");
		exit(1);
	}
	else if (pid > 0)
	{
		waitpid(-1, NULL, WNOHANG);
	}
	else
	{
		dup2(client_fd, STDOUT_FILENO);
		dup2(client_fd, STDERR_FILENO);
		close(client_fd);
		char *argv[] = {path, NULL};
		char *envp[] = {NULL};

		if (execve(path, argv, envp) < 0)
		{
			printf("executable file failed\n");
			exit(1);
		}
	}
}

static void start_server(MY_HTTPD_CONF conf)
{
	int server_fd, client_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	char buffer[MAX_BUFFER] = {0};

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// 解决close后的wait_time
	int val = 1;
	int ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
	if (ret == -1)
	{
		printf("setsockopt");
		exit(1);
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
		char *url;
		url = get_url(buffer);
		char *head;
		head = get_head(url);

		printf("%s\n", buffer);
		
		char file_name[256];
		snprintf(file_name, sizeof(file_name), "%s%s", conf.root_dir, url);
		FILE *file = fopen(file_name, "rb");
		if (file != NULL)
		{
			write(client_fd, head, strlen(head));
			if (!is_executable(url))
			{
				do_cat_executable(client_fd,url);
			}
			else
			{
				do_cat(client_fd, file);
			}

			fclose(file);
		}
		else
		{
			write(client_fd, head, strlen(head));
		}
		
		close(client_fd);
	}
}


int main(int argc, char *argv[])
{
	if (read_config("myhttpd.conf") != 0)
	{
		return -1;
	}

	start_server(conf);

	return 0;
}
