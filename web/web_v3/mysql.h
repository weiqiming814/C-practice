#ifndef MYSQL_H_
#define MYSQL_H_

#ifdef _cplusplus
extern "C" {
#endif

void handle_get_request(int client_fd, char *uri);

#ifdef _cplusplus
}
#endif

#endif  // MYSQL_H_

