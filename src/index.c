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
  if (start > index->flookup->len - 1)
  {
    f_log(F_LOG_WARN, "start %u is greater than max %u", start, index->flookup->len - 1);
    *out = NULL;
    *size = 0;
    return 0;
  }
  else if (start + count > index->flookup->len - 1)
  {
    unsigned int newcount = index->flookup->len - 1 - start;
    f_log(F_LOG_WARN, "truncating lookup len [%u %u] -> [%u %u]", start, count, start, newcount);
    count = newcount;
  }

  size_t zero_offset = ((index->flookup->len - 1) * sizeof(size_t));
  size_t start_offset = (zero_offset - (start * sizeof(size_t)));
  size_t count_offset = (count * sizeof(size_t));
  size_t end_offset;

  if ((long) start_offset - (long) count_offset <= 0)
  {
    f_log(F_LOG_INFO, "soff - coff is less than 0");
    end_offset = 0ul;
  }
  else
  {
    end_offset = start_offset - count_offset;
  }

  f_log(F_LOG_FINE, "size of zero offset: %zu %zu", zero_offset, sizeof(size_t));

  size_t buffer_size = sizeof(size_t);
  size_t* start_bytes = malloc(sizeof(*start_bytes) * buffer_size);
  size_t* end_bytes = malloc(sizeof(*end_bytes) * buffer_size);

  if (pread(index->flookup->fd, start_bytes, buffer_size, start_offset) < 0)
  {
    perror("read failed");
    f_log(F_LOG_ERROR, "index read at %u returned 0 bytes", start);
    *out = NULL;
    *size = 0;
    
    free(start_bytes);
    free(end_bytes);
    return 0;
  }

  if (pread(index->flookup->fd, end_bytes, buffer_size, end_offset) < 0)
  {
    perror("read failed");
    f_log(F_LOG_ERROR, "index read at %u returned 0 bytes", start);
    *out = NULL;
    *size = 0;
    
    free(start_bytes);
    free(end_bytes);
    return 0;
  }

  f_log(F_LOG_DEBUG, "len: %zu, (%u + %u) indexed from offsets %zu, %zu - starting at %zu, ending at %zu", index->flookup->len, start, count, start_offset, end_offset, *start_bytes, *end_bytes);

  if (*start_bytes > *end_bytes)
  {
    f_log(F_LOG_ERROR, "something went wrong - (zo: %zu) start: %ld, count: %ld, (start_offset: %zu, end_offset %zu, count offset: %zu) %zu, %zu", zero_offset, start, count, start_offset, end_offset, count_offset, *start_bytes, *end_bytes);
    return -1;
  }

  size_t bytes = *end_bytes - *start_bytes;

  // f_log(F_LOG_FINE, "allocated %zu bytes to read from %s", bytes, index->flookup->path);
  char* buffer = malloc(sizeof(char) * bytes);
  if (buffer == NULL)
  {
    perror("failed to allocate");
    *out = NULL;
    return -1;
  }


  size_t bytes_read = pread(index->fd, buffer, bytes, (off_t) *start_bytes);
  char* string = malloc(sizeof(char) * (bytes + 1));
  if (string == NULL)
  {
    f_log(F_LOG_ERROR, "Couldn't allocate string!");
    free(buffer);
    free(start_bytes);
    free(end_bytes);
    *out = NULL;
    return -1;
  }

  if (bytes_read < 0 || bytes_read > bytes)
  {
    printf("invalid: %zu %zu\b", bytes, *start_bytes);
    perror("sorry");
  }

  int pos = 0;


  // f_log(F_LOG_FINE, "Byte to populate lookup %zu", bytes_read);

  // memcpy(string, buffer, bytes_read);
  while (pos < bytes_read)
  {
    string[pos] = (char) buffer[pos];
    pos++;
  }
  
  string[bytes_read] = 0;


  // f_log(F_LOG_ERROR, "assigning null terminator %d", string[bytes_read] == '\0');

  // if (size != NULL)
  // {
  //   *size = pos;
  // }

  // f_log(F_LOG_INFO, "out is %s", string);
  *out = string;
  free(buffer);
  free(start_bytes);
  free(end_bytes);
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
