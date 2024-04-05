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
#include <stdint.h>
#include <mysql/mysql.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pcap.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>
#include "common.h"

#define MAX_BUFFER 1024
#define MAX_PARAMS 2

typedef struct _MY_HTTPD_CONF {
  int port;
} MY_HTTPD_CONF;

typedef struct _FuncCall {
  char funcName[64];
  char *params[MAX_PARAMS];
  int paramsCount;
} FuncCall;

void finish_with_error(MYSQL *con) {
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

void resetFuncCall(FuncCall *fc) {
  // 先释放可能已分配的参数内存
  for (int i = 0; i < fc->paramsCount; i++) {
    if (fc->params[i] != NULL) {
      free(fc->params[i]);
      fc->params[i] = NULL;
    }
  }
  // 重置函数名
  memset(fc->funcName, 0, sizeof(fc->funcName));
  // 重置参数计数
  fc->paramsCount = 0;
}

static MY_HTTPD_CONF conf;
void daemonize(void);
int read_config(const char *filename);
static void process_rq(int client_fd);
static void loop(void);
static int make_server_socket(int portnum);
char *execute_function(FuncCall *fc);
char *sign_up_request(char *username, char *password);
char *sign_in_request(char *username, char *password);
int parse_request(const char *request, FuncCall *fc);

int main(int argc, char *argv[]) {
  if (read_config("myhttpd.conf") != 0) {
    error_exit("Failed to read config file");
  }

#ifdef USE_DEAMON
  daemon(1, 1);
#else
  daemonize();
#endif

  loop();

  return 0;
}

void daemonize(void) {
  pid_t  pid;

  if ((pid = fork()) < 0) {
    error_exit("fork");
  } else if (pid != 0) {
    exit(0);
  }

  setsid();

  if (chdir("./") < 0) {
    error_exit("chdir");
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
    }
  }

  fclose(config_file);

  return 0;
}

static void loop(void) {
  int client_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  int fd = make_server_socket(conf.port);
  while (1) {

    if ((client_fd = accept(fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
      error_exit("accept failed");
    }

    process_rq(client_fd);
  }
}

static int make_server_socket(int portnum) {
  int server_fd;
  struct sockaddr_in address;

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    error_exit("socket failed");
  }

  address.sin_family      = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port        = htons(conf.port);

  // 解决close后的wait_time
  int val = 1;
  int ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
  if (ret == -1) {
    error_exit("setsockopt failed");
  }

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    error_exit("bind failed");
  }

  if (listen(server_fd, 3) < 0) {
    error_exit("listen failed");
  }

  return server_fd;
}

static void process_rq(int client_fd) {
  char buffer[MAX_BUFFER];
  int received_length = read(client_fd, buffer, MAX_BUFFER - 1);
  if (received_length < 0) {
    perror("read");
    close(client_fd);
    return;
  }

  buffer[received_length] = '\0'; // 确保字符串结束
  FuncCall fc;
  if (parse_request(buffer, &fc) != 0) {
    close(client_fd);
    return;
  }
  char response[MAX_BUFFER];  
  char *result = execute_function(&fc);
  snprintf(response, MAX_BUFFER,
      "%s", result); // 直接回传收到的JSON正文

  // 发送响应
  write(client_fd, response, strlen(response));
  resetFuncCall(&fc);
  close(client_fd);
}

int parse_request(const char *request, FuncCall *fc) {
  // 查找HTTP请求正文的开始位置
  char *body = strstr(request, "\r\n\r\n");
  if (body == NULL) {
    printf("Error: Could not find HTTP request body.\n");
    return -1;
  }
  body += 4; // 移过4个字符,即"\r\n\r\n",来到正文的开始
  printf("%s", request);
  cJSON *json = cJSON_Parse(body);
  if (json == NULL) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    cJSON_Delete(json);
    return 0;
  }
  cJSON *funcName = cJSON_GetObjectItemCaseSensitive(json, "funcName");
  if (cJSON_IsString(funcName) && (funcName->valuestring != NULL)) {
    strncpy(fc->funcName, funcName->valuestring, sizeof(fc->funcName) - 1);
    printf("Received funcName: %s\n", funcName->valuestring);
  }

  cJSON *params = cJSON_GetObjectItemCaseSensitive(json, "params");
  if (cJSON_IsArray(params)) {
    int paramsIndex = 0;
    cJSON *param;
    cJSON_ArrayForEach(param, params) {
      if (cJSON_IsString(param) && (param->valuestring != NULL)) {
        if (paramsIndex < MAX_PARAMS) {
          fc->params[paramsIndex] = strdup(param->valuestring);
          fc->paramsCount++;
          printf("Received param: %s\n", fc->params[paramsIndex]);
          paramsIndex++;
        }
      }
    }
  }

  cJSON_Delete(json);
  return 0;
}

char *execute_function(FuncCall *fc) {
  if(strcmp(fc->funcName, "Sign_in") == 0) {
    char *username = fc->params[0];
    char *password = fc->params[1];
    return sign_in_request(username, password);
  } else if(strcmp(fc->funcName, "Sign_up") == 0) {
    char *username = fc->params[0];
    char *password = fc->params[1];
    return sign_up_request(username, password);
  }

  return strdup("request failed");
}

char *sign_in_request(char *username, char *password) {
  static char reply[64];
  reply[0] = '\0';

  MYSQL *con = mysql_init(NULL);

  if (con == NULL) {
    finish_with_error(con);
  }

  if (mysql_real_connect(con, "localhost", "root", "81482523", "user_pas", 0, NULL, 0) == NULL) {
    finish_with_error(con);
  }

  char query[256];
  snprintf(query, sizeof(query), "SELECT password FROM accounts WHERE username='%s'", username);
  if (mysql_query(con, query)) {
    finish_with_error(con);
  }
  MYSQL_RES *result = mysql_store_result(con);
  if (result == NULL) {
    finish_with_error(con);
  }
  if (mysql_num_rows(result) == 0) {
    snprintf(reply, sizeof(reply), "Username does not exist.\n");
    mysql_free_result(result);

    mysql_close(con);
    return reply;
  } else {
    MYSQL_ROW row = mysql_fetch_row(result);
    if (strcmp(row[0], password) == 0) {
      snprintf(reply, sizeof(reply), "successful\n");
      mysql_free_result(result);

      mysql_close(con);
      return reply;
    } else {
      snprintf(reply, sizeof(reply), "Password is incorrect.\n");
      mysql_free_result(result);

      mysql_close(con);
      return reply;
    }
    mysql_free_result(result);

    mysql_close(con);
    exit(0);
  }
}

char *sign_up_request(char *username, char *password) {
  static char reply[64];
  reply[0] = '\0';

  MYSQL *con = mysql_init(NULL);

  if (con == NULL) {
    finish_with_error(con);
  }

  if (mysql_real_connect(con, "localhost", "root", "81482523", "user_pas", 0, NULL, 0) == NULL) {
    finish_with_error(con);
  }

  char query[256];
  snprintf(query, sizeof(query), "SELECT username FROM accounts WHERE username='%s'", username);
  if (mysql_query(con, query)) {
    finish_with_error(con);
  }
  MYSQL_RES *result = mysql_store_result(con);
  if (result == NULL) {
    finish_with_error(con);
  }
  if (mysql_num_rows(result) > 0) {
    snprintf(reply, sizeof(reply), "This user has already been registered.\n");
    mysql_free_result(result);

    mysql_close(con);
    return reply;
  } else {
    snprintf(query, sizeof(query), "INSERT INTO accounts(username, password) VALUES('%s', '%s')", username, password);
    if (mysql_query(con, query)) {
      finish_with_error(con);
    }
    snprintf(reply, sizeof(reply), "successful\n");
    mysql_free_result(result);

    mysql_close(con);
    return reply;
  }
  mysql_free_result(result);
  mysql_close(con);
  exit(0);
}

