#include "greatest.h"
#include "../src/flashlight.c"
#include "../vendor/btree.c"
#include "node.c"
#include "bytes.c"
#include "chunk.c"
#include "index.c"
#include "indexer.c"
#include "search.c"
#include "log.c"

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
  // f_logger_set_level(F_LOG_ERROR | F_LOG_WARN | F_LOG_INFO | F_LOG_DEBUG);

  srand(time(0));

  GREATEST_MAIN_BEGIN();

  RUN_SUITE(f_node_suite);
  RUN_SUITE(f_bytes_suite);
  RUN_SUITE(f_chunk_suite);
  RUN_SUITE(f_index_suite);
  RUN_SUITE(f_indexer_suite);
  RUN_SUITE(f_search_suite);
  RUN_SUITE(f_log_suite);

  GREATEST_MAIN_END();
}