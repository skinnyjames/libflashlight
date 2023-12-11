TEST test_f_chunk_new(void)
{
  f_bytes* first_bytes;
  f_bytes* second_bytes;
  f_bytes_new(&first_bytes, true, 1ul);
  f_bytes_new(&second_bytes, true, 5ul);

  f_bytes_node* first_node;
  f_bytes_node* second_node;
  f_bytes_node_new(&first_node, first_bytes);
  f_bytes_node_new(&second_node, second_bytes);

  first_node->next = second_node;

  f_chunk* chunk;
  f_chunk_new(&chunk, 1, first_node, second_node);

  ASSERT_EQ_FMT(chunk->current, 1ul, "%lu");
  ASSERT_EQ_FMT(chunk->last, chunk->first->next, "%p");
  ASSERT_EQ_FMT(5ul, chunk->last->bytes->offset, "%zu");
  ASSERT_EQ_FMT(5ul, chunk->first->next->bytes->offset, "%zu");

  f_chunk_free(&chunk);
  ASSERT_EQ_FMT(NULL, chunk, "%p");

  PASS();
}

TEST test_f_chunk_reverse_reduce(void)
{
  f_bytes* first_bytes;
  f_bytes* second_bytes;
  f_bytes* third_bytes;
  f_bytes_new(&first_bytes, true, 1ul);
  f_bytes_new(&second_bytes, true, 5ul);
  f_bytes_new(&third_bytes, false, 6ul);


  f_bytes_node* first_node;
  f_bytes_node* second_node;
  f_bytes_node* third_node;
  f_bytes_node_new(&first_node, first_bytes);
  f_bytes_node_new(&second_node, second_bytes);
  f_bytes_node_new(&third_node, third_bytes);

  first_node->next = NULL;
  second_node->next = NULL;
  third_node->next = second_node;

  f_chunk* first_chunk;
  f_chunk* second_chunk;

  f_chunk_new(&first_chunk, 1, first_node, first_node);
  f_chunk_new(&second_chunk, 2, third_node, second_node);

  /*
    [
      {f: 1 -> NULL, l: 1 -> NULL}
      {f: 6 -> 5 -> NULL, l: 5 -> NULL}
    ]
  */
  int size = 2;
  f_chunk** list;
  f_chunk_array_new(&list, size);

  list[1] = second_chunk;
  list[0] = first_chunk;

  f_chunk* new_chunk;
  f_chunk_array_reverse_reduce(&new_chunk, 0, list, 2);

  f_bytes_node* node = new_chunk->first;
  f_bytes_node* last = new_chunk->last;

  int i = 0;
  while (node->next != NULL)
  {
    i++;

    node = node->next;
    switch(i) 
    {
      case 0 :
        ASSERT_EQ_FMT(6ul, node->bytes->offset, "%zu");
        ASSERT_EQ_FMT(false, node->bytes->atomic, "%d");
        break;
      case 1 :
        ASSERT_EQ_FMT(5ul, node->bytes->offset, "%zu");
        ASSERT_EQ_FMT(true, node->bytes->atomic, "%d");
        break;
      case 2 :
        ASSERT_EQ_FMT(1ul, node->bytes->offset, "%zu");
        ASSERT_EQ_FMT(true, node->bytes->atomic, "%d");
        ASSERT_EQ_FMT(new_chunk->last, node, "%p");
        break;
    }
  }

  ASSERT_EQ_FMT(2, i, "%d");

  f_chunk_free(&new_chunk);
  f_chunk_array_free(list, size);

  ASSERT_EQ_FMT(1ul, first_node->bytes->offset, "%zu");
  ASSERT_EQ_FMT(5ul, second_node->bytes->offset, "%zu");
  ASSERT_EQ_FMT(6ul, third_node->bytes->offset, "%zu");

  PASS();
}

SUITE(f_chunk_suite)
{
  RUN_TEST(test_f_chunk_new);
  RUN_TEST(test_f_chunk_reverse_reduce);
}