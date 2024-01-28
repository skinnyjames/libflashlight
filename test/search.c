void test_f_search_result(f_search_result* res, void* payload)
{
  struct btree* results = payload;
  if (btree_set(results, res) != NULL)
  {
    printf("Whatt?\n");
  }
}

int test_search_progress(double progress)
{
  // printf("progress: %lf\n", progress);
}

int test_search_result_compare(const void* a, const void* b, void* udata)
{
  f_search_result* sa = (f_search_result*) a;
  f_search_result* sb = (f_search_result*) b;
  return sa->line_number > sb->line_number ? 1 : -1;
}

f_index* get_index(void)
{
  char* fixture = "test/zfixtures/search.txt";

  f_indexer config = {
    .filename = fixture,
    .lookup_dir = ".flashlight",
    .buffer_size = 400000,
    .concurrency = 50,
    .threads = 5,
    .max_bytes_per_iteration = 500000,
    .on_progress = NULL,
    .payload = NULL
  };

  return f_index_text_file(config);
}

TEST test_f_search_invalid_regex(void)
{
  f_index* index = get_index();
  struct btree* results = btree_new(sizeof(f_search_result), 0, test_search_result_compare, NULL);

  f_searcher searcher = {
    .regex = "car(s",
    .index = index,
    .threads = 2,
    .result_limit = 3,
    .line_buffer = 400u,
    .on_progress = test_search_progress,
    .progress_payload = NULL,
    .on_result = test_f_search_result,
    .result_payload = results
  };
  
  int rc = f_index_search(searcher);
  ASSERT_EQ_FMT(-2, rc, "%d");

  btree_free(results);
  f_index_free(&index);
  PASS();
}

TEST test_f_search(void)
{
  char* fixture = "test/zfixtures/search.txt";

  f_indexer config = {
    .filename = fixture,
    .lookup_dir = ".flashlight",
    .buffer_size = 400000,
    .concurrency = 50,
    .threads = 5,
    .max_bytes_per_iteration = 500000,
    .on_progress = NULL,
    .payload = NULL
  };

  f_index* index = f_index_text_file(config);
  struct btree* results = btree_new(sizeof(f_search_result), 0, test_search_result_compare, NULL);

  f_searcher searcher = {
    .regex = "cars",
    .index = index,
    .threads = 2,
    .result_limit = 3,
    .line_buffer = 400u,
    .on_progress = test_search_progress,
    .progress_payload = NULL,
    .on_result = test_f_search_result,
    .result_payload = results
  };
  
  if (f_index_search(searcher) != 0)
  {
    printf("search failed\n");
  }

  size_t count = btree_count(results);
  ASSERT_EQ_FMT(3ul, count, "%zu");

  f_search_result* res;
  int idx = 0;
  while (res = btree_pop_min(results))
  {
    switch(idx)
    {
      case 0:
      {
        ASSERT_STR_EQ("the box ate cars", res->str);
        ASSERT_EQ_FMT(3ul, res->line_number, "%zu");
        ASSERT_EQ_FMT(1u, res->matches_len, "%u");
        ASSERT_EQ_FMT(12ul, res->matches_substring_offset[0], "%zu");
        ASSERT_EQ_FMT(4ul, res->matches_substring_len[0], "%zu");
        break;
      }
      case 1:
      {
        ASSERT_STR_EQ("cars?", res->str);
        ASSERT_EQ_FMT(4ul, res->line_number, "%zu");
        ASSERT_EQ_FMT(1u, res->matches_len, "%u");
        ASSERT_EQ_FMT(0ul, res->matches_substring_offset[0], "%zu");
        ASSERT_EQ_FMT(4ul, res->matches_substring_len[0], "%zu");
        break;
      }
      case 2:
      {
        ASSERT_STR_EQ("cars", res->str);
        ASSERT_EQ_FMT(9ul, res->line_number, "%zu");
        ASSERT_EQ_FMT(1u, res->matches_len, "%u");
        ASSERT_EQ_FMT(0ul, res->matches_substring_offset[0], "%zu");
        ASSERT_EQ_FMT(4ul, res->matches_substring_len[0], "%zu");
        break;
      }
    }

    // f_search_result_free(res);
    idx++;
  }
  f_index_free(&index);
  PASS();
}

SUITE(f_search_suite)
{
  RUN_TEST(test_f_search_invalid_regex);
  RUN_TEST(test_f_search);
}