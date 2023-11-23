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
#include <fcntl.h>
#include <zlog.h>
#include <signal.h>

#define LOG_FILE "./zlog.conf"

#define MAX_BUFFER 1024

zlog_category_t *c;

typedef struct _MY_HTTPD_CONF {
    int port;
    char root_dir[256];
} MY_HTTPD_CONF;

static MY_HTTPD_CONF conf;
void daemonize(void);
int read_config(const char *filename);
static void process_rq(char *buffer, int client_fd);
static void loop(void);
static int make_server_socket(int portnum);

static void error_exit(const char *s) {
    zlog_error(c, "%s", s);
    zlog_fini();
    exit(EXIT_FAILURE);
}

static char *get_url(const char* request) {
    char buf[MAX_BUFFER] = {0};
    static char url[MAX_BUFFER] = {0};

    strncpy(buf, request, strlen(request) + 1);
    strtok(buf, " ");
    char *url_tmp = strtok(NULL, " ");
    strncpy(url, url_tmp, strlen(url_tmp) + 1);

    if (url[0] == '/' && url[1] == '\0') {
        strcpy(url, "/index.html");
    }
    return url;
}

int file_exist(const char* filename) {
    FILE *file;
    if ((file = fopen(filename, "r")) != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
}

static char *get_content_type(const char *url) {
    char *content_type;
    char *ext = strstr(url, ".") + 1;
    if (ext == NULL) {
        content_type = "text/html";
        return content_type;
    }

    if (strcasecmp(ext, "html") == 0) {
        content_type = "text/html";
    } else if (strcasecmp(ext, "jpg") == 0) {
        content_type = "image/jpeg";
    } else if (strcasecmp(ext, "ico") == 0) {
        content_type = "image/x-icon";
    } else if (strcasecmp(ext, "cgi") == 0) {
        content_type = "text/html";
    } else {
        content_type = "text/plain";
    }

    return content_type;
}

static int64_t get_content_length(char *filename) {
    FILE *fp = NULL;

    if ((fp = fopen(filename, "r")) == NULL) {
        error_exit("get content failed");
    }

    fseek(fp, 0, SEEK_END);
    int64_t file_size = ftell(fp);
    fclose(fp);
    return file_size;
}

int is_executable(const char *buf) {
    char *ext = strstr(buf, ".");
    if (ext == NULL) {
        return 0;
    }

    if (strcmp(ext, ".cgi") == 0) {
        return 0;
    } else {
        return 1;
    }
}

static char *get_head(const char *url) {
    char index_file[256];
    static char head[MAX_BUFFER] = {0};
    snprintf(index_file, sizeof(index_file), "%s%s", conf.root_dir, url);
    char *content_type = get_content_type(url);
    int64_t content_length = get_content_length(index_file);

    if (file_exist(index_file) && !is_executable(url)) {
        snprintf(head, MAX_BUFFER, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n"
                "Connection: close\r\n"
                "\r\n",
                content_type);
    } else if (file_exist(index_file) && is_executable(url)) {
        snprintf(head, MAX_BUFFER, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n"
                "Content-Length: %ld\r\n"
                "Connection: close\r\n"
                "\r\n",
                content_type, content_length);
    } else {
        snprintf(head, MAX_BUFFER, "HTTP/1.1 404 Not Found\r\nContent-Type: %s\r\n"
                "\r\nrequest file not found\r\n"
                "Connection: close\r\n"
                "\r\n",
                content_type);
    }

    return head;
}

void do_cat(int fd, FILE *file) {
    int c;

    while ((c = getc(file)) != EOF) {
        write(fd, &c, 1);
    }
}

static int is_exec(const char *f) {
    struct stat st;

    if (stat(f, &st) < 0) {
        error_exit("stat no executable permission");
    }


    return st.st_mode & S_IEXEC;
}

static void do_exec(int client_fd, char *path) {
    pid_t pid = fork();
    if (pid < 0) {
        error_exit("creat pid failed");
    } else if (pid > 0) {
        waitpid(-1, NULL, WNOHANG);
    } else {
        dup2(client_fd, STDOUT_FILENO);
        dup2(client_fd, STDERR_FILENO);
        close(client_fd);
        char *argv[] = {path, NULL};
        char *envp[] = {NULL};

        if (execve(path, argv, envp) < 0) {
            error_exit("executable file failed");
        }
    }
}

int main(int argc, char *argv[]) {
    int rc = zlog_init(LOG_FILE);
    if (rc) {
        printf("init failed, please check file %s.\n", LOG_FILE);
        return -1;
    }

    c = zlog_get_category("GetIoT");
    if (!c) {
        printf("get cat fail\n");
        zlog_fini();
        return -2;
    }

    if (read_config("myhttpd.conf") != 0) {
        error_exit("Failed to read config file");
    }


    daemonize();
    loop();
    zlog_fini();

    return 0;
}

void daemonize(void)
{
    pid_t  pid;

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    } else if (pid != 0)
        exit(0);
    setsid();

    if (chdir("/") < 0) {
        perror("chdir");
        exit(1);
    }

    close(0);
    open("/dev/null", O_RDWR);
    dup2(0, 1);
    dup2(0, 2);
}

int read_config(const char *filename) {
    char line[256];

    if (filename == NULL) {
        error_exit("open file failed");
    }

    FILE *config_file = fopen(filename, "r");
    if (config_file == NULL) {
        error_exit("file content failed");
    }

    while (fgets(line, sizeof(line), config_file)) {
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");

        if (strcmp(key, "Port") == 0) {
            conf.port = atoi(value);
        } else if (strcmp(key, "Directory") == 0) {
            strncpy(conf.root_dir, value, strlen(value) - 1);
        }
    }

    fclose(config_file);

    return 0;
}

static int make_server_socket(int portnum) {
    int server_fd;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        error_exit("socket failed");
    }

    // 解决close后的wait_time
    int val = 1;
    int ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
    if (ret == -1) {
        error_exit("setsockopt failed");
    }

    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(conf.port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        error_exit("bind failed");
    }

    if (listen(server_fd, 3) < 0) {
        error_exit("listen failed");
    }

    zlog_info(c, "Server listening on port %d\n", conf.port);

    return server_fd;
}

static void process_rq(char *buffer, int client_fd) {
    char *url = get_url(buffer);
    char *head = get_head(url);

    zlog_info(c, "%s\n", buffer);

    char file_name[256];
    snprintf(file_name, sizeof(file_name), "%s%s", conf.root_dir, url);
    FILE *file = fopen(file_name, "rb");
    if (file != NULL) {
        write(client_fd, head, strlen(head));
        if (is_exec(file_name)) {
            do_exec(client_fd, file_name);
        } else {
            do_cat(client_fd, file);
        }

        fclose(file);
    } else {
        write(client_fd, head, strlen(head));
    }

    close(client_fd);
}

static void loop(void) {
    int client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX_BUFFER];
    int fd = make_server_socket(conf.port);
    while (1) {
        zlog_info(c, "\nWaiting for a connection...\n");

        if ((client_fd = accept(fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            error_exit("accept failed");
        }

        read(client_fd, buffer, MAX_BUFFER);
        process_rq(buffer, client_fd);
    }
}

