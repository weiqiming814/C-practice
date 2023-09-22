#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAXLINE 128
#define SERV_PORT 80

void perr_exit(const char *s)
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

	char *request = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n";
	
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
