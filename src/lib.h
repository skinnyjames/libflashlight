#ifndef FLASHLIGHT_LIB_H
#define FLASHLIGHT_LIB_H
#define PCRE2_CODE_UNIT_WIDTH 8
#ifdef __linux_
#define MTRIM(a) malloc_trim(a)
#else
#define MTRIM(a) do {} while(0)
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
