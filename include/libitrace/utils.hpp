#pragma once

#include <stdio.h>
#include <stdlib.h>

#define die(msg)            \
	do {                    \
		perror(msg);        \
		exit(EXIT_FAILURE); \
	} while (0)

#define die2(msg)           \
	do {                    \
		printf("%s", msg);  \
		exit(EXIT_FAILURE); \
	} while (0)
