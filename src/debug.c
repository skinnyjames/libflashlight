#ifndef FLASHLIGHT_DEBUG
#define FLASHLIGHT_DEBUG

void debug_pointer(void* ptr) {
  printf("pointer -> %p\n", ptr);
}

void debug_node(f_node* node) {
  printf("\nDEBUG - {%zu}", node->offset);

  while (node->next != NULL) {
    printf(" {%zu}", node->next->offset);
    node = node->next;
  }
  printf("\n");
}

char* get_bytes_node_type(f_bytes_node* node) {
  if (node->bytes->atomic) {
    return "atomic";
  } else {
    return "continue";
  }
}

void debug_bytes_node(f_bytes_node* node) {
  printf("\nDEBUG - {%zu, %s}", node->bytes->offset, get_bytes_node_type(node));

  int breakon = 10;
  int i = 0;
  while (node->next != NULL) {
    printf( " {%zu, %s}", node->next->bytes->offset, get_bytes_node_type(node->next));
    if (i >= breakon) {
      printf("\n");
      i = 0;
    }
    node = node->next;
    i++;
  }

  printf("\n");
}

void debug_bytes(f_bytes* bytes) {
  if (bytes == NULL) {
    printf("bytes is null\n");
  } else {
    char* type = bytes->atomic ? "atomic" : "continue";
    printf("bytes at %zu (%s)\n", bytes->offset, type);
  }
}

void debug_chunk(f_chunk* chunk) {
  if (chunk == NULL) {
    printf("chunk is null\n");
  } else {
    f_bytes_node* first = chunk->first;
    f_bytes_node* last = chunk->last;

    printf("chunk at (%zu) -> (%zu)\n", first->bytes->offset, last->bytes->offset);
  }
}

void debug_chunk_details(f_chunk* chunk) {
  debug_chunk(chunk);
  f_bytes_node* first = chunk->first;
  debug_bytes_node(first);
}

#endif
