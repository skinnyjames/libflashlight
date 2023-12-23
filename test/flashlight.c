#include "greatest.h"
#include "../src/flashlight.c"
#include "../vendor/btree.c"
#include "node.c"
#include "bytes.c"
#include "chunk.c"
#include "index.c"
#include "indexer.c"
#include "search.c"

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
  srand(time(0));

  GREATEST_MAIN_BEGIN();

  RUN_SUITE(f_node_suite);
  RUN_SUITE(f_bytes_suite);
  RUN_SUITE(f_chunk_suite);
  RUN_SUITE(f_index_suite);
  RUN_SUITE(f_indexer_suite);
  RUN_SUITE(f_search_suite);

  GREATEST_MAIN_END();
}