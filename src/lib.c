#ifndef FLASHLIGHT_LIB
#define FLASHLIGHT_LIB

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "../vendor/cwalk.c"
#include "node.c"
#include "bytes.c"
#include "chunk.c"
#include "debug.c"

#include "lookup.c"
#include "index.c"
#include "indexer.c"
#include "indexers/text_indexer.c"

#endif