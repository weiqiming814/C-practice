/*
 * Description:  get student.
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
#include <mysql/mysql.h>
#include <cjson/cJSON.h>

MYSQL *conn_ptr;
unsigned int timeout = 7;

void handle_get_request(int client_fd, char *uri) {
    cJSON *root = cJSON_CreateArray();
    conn_ptr = mysql_init(NULL);
    if (!conn_ptr) {
        printf("mysql_init failed!\n");
        return;
    }

    mysql_options(conn_ptr, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    conn_ptr = mysql_real_connect(conn_ptr, "localhost", "root", NULL, "school", 0, NULL, 0);
    if (conn_ptr) {
        if (mysql_query(conn_ptr, "select * from Student")) {
            printf("Query failed: %s\n", mysql_error(conn_ptr));
            return;
        }

        MYSQL_RES *result = mysql_store_result(conn_ptr);
        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                cJSON *student = cJSON_CreateObject();
                cJSON_AddStringToObject(student, "Sno", row[0]);
                cJSON_AddStringToObject(student, "Sname", row[1]);
                cJSON_AddStringToObject(student, "Ssex", row[2]);
                cJSON_AddStringToObject(student, "Sbirthday", row[3]);
                cJSON_AddStringToObject(student, "Class", row[4]);
                cJSON_AddItemToArray(root, student);
            }
            mysql_free_result(result);
        } else {
            printf("Failed to get result: %s\n", mysql_error(conn_ptr));
        }
        mysql_close(conn_ptr);
    } else {
        printf("Connection failed: %s\n", mysql_error(conn_ptr));
    }

    char *json_str = cJSON_Print(root);
    char response[1024];
    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n%s\n", json_str);
    write(client_fd, response, strlen(response));
    cJSON_Delete(root);
    free(json_str);
}

