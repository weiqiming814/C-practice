/*
 * Description: test file.
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
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "my_atoi.h"

bool num_equal(const int n1, const int n2)
{
	bool ret = false;
	ret = n1 == n2;
	if (ret)
	{
		printf("test correct\n");
	}

	return ret;
}

int main(void)
{
	char *a = "2131314";
	char *b = "-1234.12";
	char *c = "    -1234.12";
	char *d = "		-1234.12";
	assert(num_equal(my_atoi(a), atoi(a)));
	assert(num_equal(my_atoi(b), atoi(b)));
	assert(num_equal(my_atoi(c), atoi(c)));
	assert(num_equal(my_atoi(d), atoi(d)));

	return 0;
}

