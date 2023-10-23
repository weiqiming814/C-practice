/*
 * Description: Add thousands separator to numbers.
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
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "separator.h"

void perr_exit(const char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

bool check_num(char *str)
{
	int n = 0;
	int m = 0;
	
	if (str[0] == '-')
	{
		n = 1;
	}

	for (int i = n; i < strlen(str); i++)
	{
		if (str[i] == '.')
		{
			m++;
		}
		if (str[i] == '-' || m > 1)
		{
			perr_exit("This number is not legel");
		}
		
		int tmp = (int)str[i];
		if ((tmp >= 48 && tmp <= 57) || tmp == 46)
		{
			continue;
		}
		else
		{
			perr_exit("This number is not legal");
		}
	}
	return true;
}

int find_decimal(char *str)
{
	int n = 0;
	for (int i = 0; i < strlen(str); i++)
	{
		if (str[i] == '.')
		{
			n = i;
		}
	}
	return n;
}

void add_sep(char *str)
{
	int n = 0;
	check_num(str);
	int i = find_decimal(str) - 3;
	if (str[0] == '-')
	{
		n = 1;
	}
	for (; i > n; i -= 3)
	{
		memmove(str + i + 1, str + i, strlen(str) - i + 1);
		str[i] = ',';
	}
}

char *separator(char* dest, const char *src)
{
	strcpy(dest, src);
	add_sep(dest);
	return dest;
}

