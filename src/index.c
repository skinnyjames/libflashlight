#ifndef FLASHLIGHT_INDEX
#define FLASHLIGHT_INDEX
#include "index.h"
#include <string.h>

int f_index_init(f_index** out, char* filename, int filename_len, f_lookup_file* flookup, f_lookup_mem* mlookup)
{
  f_index* init = malloc(sizeof(*init));
  if (init == NULL)
  {
    return -1;
  }

  init->filename = filename;
  init->filename_len = filename_len;

  if (mlookup == NULL)
  {
    init->flookup = flookup;
    init->mlookup = NULL;
  }
  else
  {
    init->mlookup = mlookup;
    init->flookup = NULL;
  }
  
  FILE* fp;
  int fd;
  // get filehandle and total bytes. make nonblocking.
  fp = fopen(filename, "rb");
  init->fd = fileno(fp);
  init->fp = fp;

  *out = init;
  return 0;
}

int f_index_lookup(char** out, f_index* index, unsigned int start, unsigned int count, int* size)
{
  size_t start_bytes;
  size_t end_bytes;

  size_t zero_offset = ((index->flookup->len - 1) * sizeof(size_t));
  size_t start_offset = (zero_offset - (start * sizeof(size_t)));
  size_t count_offset = (count * sizeof(size_t));
  size_t end_offset;
  if ((long) start_offset - (long) count_offset <= 0)
  {
    end_offset = 0ul;
  }
  else
  {
    end_offset = start_offset - count_offset;
  }

  if (fseek(index->flookup->fp, start_offset, SEEK_SET) != 0)
  {
    perror("faailed to seek");
    return -1;
  }

  fread(&start_bytes, sizeof(size_t), 1, index->flookup->fp);

  if (fseek(index->flookup->fp, end_offset, SEEK_SET) != 0)
  {
    perror("faailed to seek");
    return -1;
  }

  fread(&end_bytes, sizeof(size_t), 1, index->flookup->fp);

  if (start_bytes > end_bytes)
  {
    printf("something went wrong - (zo: %zu) start: %ld, count: %ld, (start_offset: %zu, end_offset %zu, count offset: %zu) %zu, %zu\n", zero_offset, start, count, start_offset, end_offset, count_offset, start_bytes, end_bytes);
    return -1;
  }

  size_t bytes = end_bytes - start_bytes;
  uint8_t* buffer = malloc(sizeof(*buffer) * bytes);
  if (buffer == NULL)
  {
    perror("failed to allocate");
    return -1;
  }

  char* string = malloc(sizeof(*string) * (bytes + 1));
  if (string == NULL)
  {
    free(buffer);
    return -1;
  }

  int bytes_read;
  bytes_read = pread(index->fd, buffer, bytes, start_bytes);
  if (bytes_read < 0)
  {
    printf("invalid: %zu %zu\b", bytes, start_bytes);
    perror("sorry");
  }

  int pos = 0;

  while (pos < bytes_read)
  {
    string[pos] = (char) buffer[pos];
    pos++;
  }

  string[pos] = '\0';
  free(buffer);
  if (size != NULL)
  {
    *size = pos;
  }
  *out = string;
  return 0;
}

void f_index_free(f_index** index)
{
  f_index* i = *index;
  fclose(i->fp);
  if (i->mlookup == NULL)
  {
    f_lookup_file_free(&i->flookup);
  }
  else
  {
    f_lookup_mem_free(i->mlookup);
  }

  free(*index);
  *index = NULL;
}

#endif
