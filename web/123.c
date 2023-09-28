#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _URL
{
	char host[128];
	char port[8];
	char path[128];
	char protocol[8];
} URL;

URL url_parse(char url[])
{
	char port[8] = {0};
	char host[128] = {0};
	char path[128] = {0};
	char protocol[8] = {0};
	int i;
	int j;

	for (i = 0; i < strlen(url); i++)
	{
		if (url[i] == ':')
		{
			memcpy(protocol, url + 0, i);
			j = i + 3;
			break;
		}
	}

	for (i = j; i < strlen(url); i++)
	{
		if (url[i] == ':')
		{
			memcpy(host, url + j, i - j);
			j = i + 1;
			for (; i < strlen(url); i++)
			{
				if (url[i] == '/')
				{
					memcpy(port, url + j, i - j);
					goto EXIT;
				}
			}
		}
		else if (url[i] == '/')
		{
			memcpy(host, url + j, i - j);
			strcpy(port, "80");
			j = i;
			goto EXIT;
		}
	}
	EXIT:
	memcpy(path, url + i, strlen(url) - i);

	URL u;
	strcpy(u.host, host);
	strcpy(u.port, port);
	strcpy(u.path, path);
	strcpy(u.protocol, protocol);

	return u;
}

int main(int argc, char *argv[])
{
	URL url = url_parse(argv[1]);
	printf("protocol = %s, port = %s, host =%s, path = %s\n", url.protocol, url.port, url.host, url.path);

	return 0;
}
