#ifndef FLASHLIGHT_LIB_H
#define FLASHLIGHT_LIB_H

#define PCRE2_CODE_UNIT_WIDTH 8
#if defined(__linux___) || defined(__gnu_linux__) || defined(linux) || defined(__linux) || defined(__unix__)
#define F_MTRIM(a) malloc_trim(a)
#else
#define F_MTRIM(a) do {} while(0)
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
