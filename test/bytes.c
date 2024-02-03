TEST test_f_bytes_new(void)
{
  f_bytes* bytes;
  if (f_bytes_new(&bytes, true, 3ul) == -1)
    FAIL();
  
  ASSERT_EQ_FMT(3ul, bytes->offset, "%zu");
  ASSERT_EQ_FMT(true, bytes->atomic, "%d");

  f_bytes_free(&bytes);

  ASSERT_EQ_FMT(NULL, bytes, "%p");

  PASS();
}

TEST test_f_bytes_new_with_null(void)
{
  f_bytes* bytes = NULL;
  f_bytes_new(&bytes, true, 3ul);

  ASSERT_EQ_FMT(3ul, bytes->offset, "%zu");
  ASSERT_EQ_FMT(true, bytes->atomic, "%d");

  f_bytes_free(&bytes);

  ASSERT_EQ_FMT(NULL, bytes, "%p");

  PASS();
  PASS();
}

TEST test_f_bytes_node_default(void)
{
  f_bytes* bytes;
  if (f_bytes_new(&bytes, true, 1ul) == -1)
    FAIL();

  f_bytes_node* node;
  if (f_bytes_node_new(&node, bytes) == -1)
    FAIL();

  ASSERT_EQ_FMT(NULL, node->next, "%p");

  f_bytes_node_free(&node);
  
  PASS();
}

TEST test_f_bytes_node_multiple(void)
{
  f_bytes* bytes;
  if (f_bytes_new(&bytes, true, 1ul) == -1)
    FAIL();

  f_bytes* next_bytes;
  if (f_bytes_new(&next_bytes, false, 2ul) == -1)
    FAIL();

  f_bytes_node* node;
  if (f_bytes_node_new(&node, bytes) == -1)
    FAIL();

  f_bytes_node* next;
  if (f_bytes_node_new(&next, next_bytes) == -1)
    FAIL();

  node->next = next;

  ASSERT_EQ_FMT(true, node->bytes->atomic, "%d");
  ASSERT_EQ_FMT(1ul, node->bytes->offset, "%zu");
  ASSERT_EQ_FMT(false, node->next->bytes->atomic, "%d");
  ASSERT_EQ_FMT(2ul, node->next->bytes->offset, "%zu");
  ASSERT_EQ_FMT(NULL, node->next->next, "%p");
  
  f_bytes_node_free(&node);

  ASSERT_EQ_FMT(NULL, node, "%p");

  PASS();
}

TEST test_f_bytes_node_prepend(void)
{
  PASS();
}

SUITE(f_bytes_suite)
{
  RUN_TEST(test_f_bytes_new);
  RUN_TEST(test_f_bytes_new_with_null);
  RUN_TEST(test_f_bytes_node_multiple);
  RUN_TEST(test_f_bytes_node_default);
  RUN_TEST(test_f_bytes_node_prepend);
}