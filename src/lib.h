#ifndef FLASHLIGHT_LIB_H
#define FLASHLIGHT_LIB_H
#define PCRE2_CODE_UNIT_WIDTH 8
#ifdef __linux__ || __gnu_linux__ || linux || __linux || __unix__
#define MTRIM(a) malloc_trim(a)
#else
#define MTRIM(a) printf("not linux\n")
#endif
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pcre2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>

#endif
