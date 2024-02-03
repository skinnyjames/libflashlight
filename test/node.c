TEST test_f_node_new(void)
{
  f_node* node;
  if (f_node_new(&node, 35ul) == -1)
  {
    FAIL();
  }

  ASSERT_EQ_FMT(35ul, node->offset, "%zu");
  
  f_node_free(&node);

  ASSERT_EQ_FMT(NULL, node, "%p");
  PASS();
}

TEST test_f_node_new_with_null(void)
{
  f_node* node = NULL;
  f_node_new(&node, 35ul);

  ASSERT_EQ_FMT(35ul, node->offset, "%zu");
  
  f_node_free(&node);

  ASSERT_EQ_FMT(NULL, node, "%p");
  PASS();
}

TEST test_nested_node_free(void)
{
  f_node* node = NULL;
  f_node* inner = NULL;

  f_node_new(&node, 3ul);
  f_node_new(&inner, 1ul);

  node->next = inner;

  f_node_free(&node);

  ASSERT_EQ_FMT(NULL, node, "%p");
  // can't seem to set inner nodes to NULL :(
  // ASSERT_EQ_FMT(NULL, inner, "%p");
  PASS();
}

TEST test_f_node_from_reversed_bytes_node(void)
{
  f_bytes* first_bytes;
  f_bytes* second_bytes;
  f_bytes* third_bytes;
  f_bytes* fourth_bytes;

  if (f_bytes_new(&first_bytes, true, 1ul) == -1)
    FAIL();
  if (f_bytes_new(&second_bytes, false, 2ul) == -1)
    FAIL();
  if (f_bytes_new(&third_bytes, true, 6ul) == -1)
    FAIL();
  if (f_bytes_new(&fourth_bytes, false, 8ul) == -1)
    FAIL();

  f_bytes_node* first_node;
  f_bytes_node* second_node;
  f_bytes_node* third_node;
  f_bytes_node* fourth_node;

  if (f_bytes_node_new(&first_node, first_bytes) == -1)
    FAIL();
  if (f_bytes_node_new(&second_node, second_bytes) == -1)
    FAIL();
  if (f_bytes_node_new(&third_node, third_bytes) == -1)
    FAIL();
  if (f_bytes_node_new(&fourth_node, fourth_bytes) == -1)
    FAIL();

  second_node->next = first_node;
  third_node->next = second_node;
  fourth_node->next = third_node;

  f_node* result;
  f_node_from_reversed_bytes_node(&result, &fourth_node);

  size_t list[3] = {0ul, 3ul, 14ul};

  for (int i=0; i<3; i++)
  {
    size_t val = list[i];
    ASSERT_EQ_FMT(val, result->offset, "%zu");
    result = result->next;
  }
  
  ASSERT_EQ_FMT(NULL, result, "%p");

  f_bytes_node_free(&fourth_node);
  f_node_free(&result);
  PASS();
}

SUITE(f_node_suite)
{
  RUN_TEST(test_f_node_new);
  RUN_TEST(test_f_node_new_with_null);
  RUN_TEST(test_nested_node_free);
  RUN_TEST(test_f_node_from_reversed_bytes_node);
}