TEST test_f_index_new(void)
{
  PASS();
}

TEST test_f_index_new_with_null(void)
{
  PASS();
}

SUITE(f_index_suite)
{
  RUN_TEST(test_f_index_new);
  RUN_TEST(test_f_index_new_with_null);
}