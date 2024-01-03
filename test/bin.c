#include "../src/flashlight.c"
#include "../vendor/btree.c"

void progress_cb(double progress)
{
  printf("Progress (%lf)\n", progress);
}

void search_progress(double progress)
{
  printf("Search progress (%lf)\n", progress);
}

int search_result_compare(const void* a, const void* b, void* udata)
{
  f_search_result* sa = a;
  f_search_result* sb = b;
  return (sa)->line_number > (sb)->line_number ? 1 : -1;
}

void append_search_result(f_search_result* res, void* payload)
{
  struct btree* results = payload;
  if (btree_set(results, res) != NULL) exit(1);
}

int main(void)
{
  f_logger_set_level(F_LOG_ERROR | F_LOG_DEBUG | F_LOG_INFO | F_LOG_WARN);
  // seed rand so index filenames are random...
  srand(time(0));
  char* test = "test/zfixtures/test.txt";
  char* small = "test/zfixtures/words.txt";
  char* lookupdir = ".flashlight";

  f_indexer i = {
    .filename = small,
    .lookup_dir = lookupdir,
    .threads = 6,
    .concurrency = 50,
    .buffer_size = 50000,
    .max_bytes_per_iteration = 10000000000,
    .on_progress = progress_cb
  };

  f_index* index = f_index_text_file(i);

  size_t start = 0;
  char* hello;
  int size;
  if (f_index_lookup(&hello, index, start, 5, NULL) == -1)
  {
    printf("failure\n");
    return -1;
  };

  free(hello);

  // do a search
  struct btree* results = btree_new(sizeof(f_search_result), 0, search_result_compare, NULL);

  f_searcher searcher = {
    .regex = "^car",
    .index = index,
    .threads = 6,
    .line_buffer = 100,
    .result_limit = 20,
    .on_progress = search_progress,
    .progress_payload = NULL,
    .on_result = append_search_result,
    .result_payload = results
  };

  if (f_index_search(searcher) != 0)
  {
    printf("search failed\n");
  }

  f_search_result** res;
  while (res = btree_pop_min(results))
  {
    printf("[%zu] - %s\n", (res)->line_number, (res)->str);
    f_search_result_free(res);
  }

  btree_free(results);
  f_index_free(&index);

  return 0;
}