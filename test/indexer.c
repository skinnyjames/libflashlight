TEST test_indexer_threads(void)
{
  f_indexer_threads* indexer;
  f_indexer_threads_init(&indexer, 2, 2000, 1000, 0ul);

  ASSERT_EQ_FMT(2, indexer->len, "%d");

  ASSERT_EQ_FMT(0ul, indexer->threads[0].from, "%zu");
  ASSERT_EQ_FMT(1000ul, indexer->threads[0].to, "%zu");
  ASSERT_EQ_FMT(1000ul, indexer->threads[0].buffer_size, "%zu");

  ASSERT_EQ_FMT(1001ul, indexer->threads[1].from, "%zu");
  ASSERT_EQ_FMT(2000ul, indexer->threads[1].to, "%zu");
  ASSERT_EQ_FMT(1000ul, indexer->threads[1].buffer_size, "%zu");

  f_indexer_threads_free(indexer);
  PASS();
}

TEST test_indexer_threads_remainder(void)
{
  f_indexer_threads* indexer;
  f_indexer_threads_init(&indexer, 2, 1800, 1000, 0ul);

  ASSERT_EQ_FMT(2, indexer->len, "%d");

  ASSERT_EQ_FMT(0ul, indexer->threads[0].from, "%zu");
  ASSERT_EQ_FMT(900ul, indexer->threads[0].to, "%zu");
  ASSERT_EQ_FMT(1000ul, indexer->threads[0].buffer_size, "%zu");

  ASSERT_EQ_FMT(901ul, indexer->threads[1].from, "%zu");
  ASSERT_EQ_FMT(1800ul, indexer->threads[1].to, "%zu");
  ASSERT_EQ_FMT(1000ul, indexer->threads[1].buffer_size, "%zu");

  f_indexer_threads_free(indexer);
  PASS();
}

TEST test_indexer_threads_bigger_buffer_than_bytes(void)
{
  f_indexer_threads* indexer;
  f_indexer_threads_init(&indexer, 3, 1000, 8000, 0ul);

  ASSERT_EQ_FMT(1, indexer->len, "%d");

  ASSERT_EQ_FMT(0ul, indexer->threads[0].from, "%zu");
  ASSERT_EQ_FMT(1000ul, indexer->threads[0].to, "%zu");
  ASSERT_EQ_FMT(8000ul, indexer->threads[0].buffer_size, "%zu");


  f_indexer_threads_free(indexer);
  PASS();
}

/**
* TODO: possibly revisit this behavior.
* buffer_size is greater than from - to in this case.
*/
TEST test_indexer_more_threads_than_index(void)
{
  f_indexer_threads* indexer;
  f_indexer_threads_init(&indexer, 3, 2000, 1500, 0ul);

  ASSERT_EQ_FMT(2, indexer->len, "%d");

  ASSERT_EQ_FMT(0ul, indexer->threads[0].from, "%zu");
  ASSERT_EQ_FMT(1000ul, indexer->threads[0].to, "%zu");
  ASSERT_EQ_FMT(1500ul, indexer->threads[0].buffer_size, "%zu");

  ASSERT_EQ_FMT(1001ul, indexer->threads[1].from, "%zu");
  ASSERT_EQ_FMT(2000ul, indexer->threads[1].to, "%zu");
  ASSERT_EQ_FMT(1500ul, indexer->threads[1].buffer_size, "%zu");

  f_indexer_threads_free(indexer);
  PASS();
}

TEST test_indexer_more_chunks_than_threads(void)
{
  f_indexer_threads* indexer;
  f_indexer_threads_init(&indexer, 3, 1000, 100, 0ul);

  ASSERT_EQ_FMT(3, indexer->len, "%d");
  
  ASSERT_EQ_FMT(0ul, indexer->threads[0].from, "%zu");
  ASSERT_EQ_FMT(333ul, indexer->threads[0].to, "%zu");
  ASSERT_EQ_FMT(100ul, indexer->threads[0].buffer_size, "%zu");

  ASSERT_EQ_FMT(334ul, indexer->threads[1].from, "%zu");
  ASSERT_EQ_FMT(667ul, indexer->threads[1].to, "%zu");
  ASSERT_EQ_FMT(100ul, indexer->threads[1].buffer_size, "%zu");

  ASSERT_EQ_FMT(668ul, indexer->threads[2].from, "%zu");
  ASSERT_EQ_FMT(1000ul, indexer->threads[2].to, "%zu");
  ASSERT_EQ_FMT(100ul, indexer->threads[2].buffer_size, "%zu");

  f_indexer_threads_free(indexer);
  PASS();
}

TEST test_indexer_chunks(void)
{
  f_indexer_chunks* ic;
  f_indexer_chunks_init(&ic, 2, 100, 200, 0, 200);

  ASSERT_EQ_FMT(2, ic->len, "%d");
  ASSERT_EQ_FMT(2, ic->concurrency, "%d");

  ASSERT_EQ_FMT(0ul, ic->chunks[0]->index, "%lu");
  ASSERT_EQ_FMT(0ul, ic->chunks[0]->from, "%zu");
  ASSERT_EQ_FMT(100ul, ic->chunks[0]->count, "%zu");

  ASSERT_EQ_FMT(1ul,  ic->chunks[1]->index, "%lu");
  ASSERT_EQ_FMT(100ul, ic->chunks[1]->from, "%zu");
  ASSERT_EQ_FMT(100ul, ic->chunks[1]->count, "%zu");

  f_indexer_chunks_free(ic);
  PASS();
}

TEST test_indexer_chunks_multiple_pages(void)
{
  f_indexer_chunks* ic;
  f_indexer_chunks_init(&ic, 3, 500, 2000, 0, 2000);

  ASSERT_EQ_FMT(4, ic->len, "%d");
  ASSERT_EQ_FMT(3, ic->concurrency, "%d");

  ASSERT_EQ_FMT(0ul, ic->chunks[0]->index, "%lu");
  ASSERT_EQ_FMT(0ul, ic->chunks[0]->from, "%zu");
  ASSERT_EQ_FMT(500ul, ic->chunks[0]->count, "%zu");

  ASSERT_EQ_FMT(1ul, ic->chunks[1]->index, "%lu");
  ASSERT_EQ_FMT(500ul, ic->chunks[1]->from, "%zu");
  ASSERT_EQ_FMT(500ul, ic->chunks[1]->count, "%zu");

  ASSERT_EQ_FMT(2ul, ic->chunks[2]->index, "%lu");
  ASSERT_EQ_FMT(1000ul, ic->chunks[2]->from, "%zu");
  ASSERT_EQ_FMT(500ul, ic->chunks[2]->count, "%zu");

  ASSERT_EQ_FMT(3ul, ic->chunks[3]->index, "%lu");
  ASSERT_EQ_FMT(1500ul, ic->chunks[3]->from, "%zu");
  ASSERT_EQ_FMT(500ul, ic->chunks[3]->count, "%zu");

  f_indexer_chunks_free(ic);
  PASS();
}

TEST testy(void)
{
  f_indexer_chunks* ic;
  f_indexer_chunks_init(&ic, 3, 100, 2000, 0, 1999);

  size_t expect[20] = {
    0ul, 100ul, 200ul, 300ul, 400ul,
    500ul, 600ul, 700ul, 800ul, 900ul,
    1000ul, 1100ul, 1200ul, 1300ul, 1400ul,
    1500ul, 1600ul, 1700ul, 1800ul, 1900ul
  };

  ASSERT_EQ_FMT(20, ic->len, "%d");

  for (int i=0; i<20; i++)
  {
    ASSERT_EQ_FMT(expect[i], ic->chunks[i]->from, "%zu");

    if (i == 19)
    {
      ASSERT_EQ_FMT(99ul, ic->chunks[i]->count, "%zu");
    }
    else
    {
      ASSERT_EQ_FMT(100ul, ic->chunks[i]->count, "%zu");
    }
  }

  f_indexer_chunks_free(ic);
  PASS();
}

TEST test_text_indexer(void)
{
  char* test = "test/zfixtures/test.txt";
  f_indexer i = {
    .filename = test,
    .lookup_dir = ".flashlight",
    .threads = 2,
    .concurrency = 2,
    .buffer_size = 3,
    .max_bytes_per_iteration = 50000,
    .on_progress = NULL
  };

  f_index* index = f_index_text_file(i);

  char* v;
  if (f_index_lookup(&v, index, 6, 3) == -1)
  {
    FAIL();
  }
  int size = strlen(v);
  char* e = "I\nlike\npie\n";
  ASSERT_EQ_FMT(11, size, "%d");
  ASSERT_STR_EQ(e, v);
  f_index_free(&index);
  PASS();
}

TEST test_index_sequential(void)
{
  char* test = "test/zfixtures/words.txt";
  f_indexer i = {
    .filename = test,
    .lookup_dir = ".flashlight",
    .threads = 2,
    .concurrency = 5,
    .buffer_size = 10,
    .max_bytes_per_iteration = 100,
    .on_progress = NULL
  };

  f_index* index = f_index_text_file(i);
  size_t last_bytes_count = 0;
  size_t previous_last_bytes_count = 0;

  for (long int i = index->flookup->len - 1; i >= 0; i--)
  {
    if (i % 2 == 0) previous_last_bytes_count = last_bytes_count;

    size_t bytes_offset = i * sizeof(size_t);
    size_t bytes_count;
    if (fseek(index->flookup->fp, bytes_offset, SEEK_SET))
    {
      perror("can't seek");
      f_index_free(&index);
      FAIL();
    }

    size_t bread = fread(&bytes_count, sizeof(size_t), 1, index->flookup->fp);
    // printf("bread: %zu, bc: %zu, lbc: %zu, null? %d\n", bread, bytes_count, last_bytes_count, last_bytes_count == NULL);
    if (last_bytes_count != 0 && bytes_count < last_bytes_count)
    {
      printf("[%ld] bad: %zu %zu - prev: %zu\n", i, bytes_count, last_bytes_count, previous_last_bytes_count);
      f_index_free(&index);
      FAIL();
    }

    if (bread > 0) last_bytes_count = bytes_count;
  }

  f_index_free(&index);
  PASS();
}

TEST test_indexer_file_not_exists()
{
    char* test = "test/zfixtures/notexist.txt";
  f_indexer i = {
    .filename = test,
    .lookup_dir = ".flashlight",
    .threads = 2,
    .concurrency = 5,
    .buffer_size = 10,
    .max_bytes_per_iteration = 100,
    .on_progress = NULL
  };

  f_index* index = f_index_text_file(i);
  ASSERT_EQ_FMT(NULL, index, "%p");
  PASS();
}

SUITE(f_indexer_suite)
{
  RUN_TEST(test_indexer_threads);
  RUN_TEST(test_indexer_threads_remainder);
  RUN_TEST(test_indexer_threads_bigger_buffer_than_bytes);
  RUN_TEST(test_indexer_more_chunks_than_threads);
  RUN_TEST(test_indexer_more_threads_than_index);

  RUN_TEST(test_indexer_chunks);
  RUN_TEST(test_indexer_chunks_multiple_pages);
  RUN_TEST(testy);

  RUN_TEST(test_text_indexer);
  RUN_TEST(test_index_sequential);
  RUN_TEST(test_indexer_file_not_exists);
}