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

typedef struct _NUM {
	char symbol[2];
	char integer[32];
	char decimal[32];
} NUM;

static NUM num;

char num1[] = "1234.1234";
char num2[] = "-1234.1234";
char num3[] = "1234";
char num4[] = "123456789.123456";

static void add_sep();
static void parse_num(char *str);

static bool check_num()
{
	char buf[64] = {0};
	strcpy(buf, num.integer);
	strcat(buf, num.decimal);
	for (int i = 0; i < strlen(buf); i++)
	{
		int tmp = (int)buf[i];
		if ((tmp >= 48 && tmp <= 57) || tmp == 46)
		{
			continue;
		}
		else
		{
			perror("This number is not legal");
			exit(EXIT_FAILURE);
		}
	}
	return true;
}

static void test(char *str)
{
	printf("Please enter a number:%s\n", str);
	parse_num(str);
	add_sep();
	printf("%s%s%s\n", num.symbol, num.integer, num.decimal);
}

static void parse_num(char *str)
{
	int n = 0;
	assert(str != NULL);
	if (str[0] == '-')
	{
		strcpy(num.symbol, "-");
		memmove(str, str+1, strlen(str));
	}
	
	for (int i = 0; i < strlen(str); i++)
	{
		if (str[i] == '.')
		{
			n++;
		}
		if (str[i] == '-' || n > 1)
		{
			perror("This number is not legel");
			exit(EXIT_FAILURE);
		}
	}
	
	strcpy(num.decimal, ".00");
	if (strstr(str, ".") != NULL)
	{
		strcpy(num.integer, strtok(str, "."));
		memmove(num.decimal + 1, strtok(str, "."), strlen(strtok(NULL, ".")));
	}
	else
	{
		strcpy(num.integer, str);
	}

	check_num();
}

static void add_sep()
{
	int len = strlen(num.integer);
	int i = len - 3;
	for (; i > 0; i -= 3)
	{
		memmove(num.integer + i + 1, num.integer + i, strlen(num.integer) - i + 1);
		num.integer[i] = ',';
	}
}

int main()
{
	test(num1);
	test(num2);
	test(num3);
	test(num4);
	return 0;
}

