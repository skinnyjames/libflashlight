#ifndef FLASHLIGHT_INDEX
#define FLASHLIGHT_INDEX
#define _FILE_OFFSET_BITS == 64
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

int f_index_lookup(char** out, f_index* index, size_t start, size_t count)
{
  enum F_LOG_LEVEL log_level = f_logger_get_level();

  if (start > index->flookup->len - 1)
  {
    f_log(F_LOG_WARN, "start %u is greater than max %u", start, index->flookup->len - 1);
    *out = NULL;
    return 0;
  }
  else if (start + count > index->flookup->len - 1)
  {
    size_t newcount = index->flookup->len - 1 - start;
    f_log(F_LOG_DEBUG, "truncating lookup len [%zu %zu] -> [%zu %zu]", start, count, start, newcount);
    count = newcount;
  }

  off_t zero_offset = ((index->flookup->len - 1) * sizeof(size_t));
  off_t start_offset = (zero_offset - (start * sizeof(size_t)));
  off_t count_offset = (count * sizeof(size_t));
  off_t end_offset;

  if ((long) start_offset - (long) count_offset <= 0)
  {
    f_log(F_LOG_DEBUG, "start offset - count offset is less than 0"); 
    end_offset = 0ul;
  }
  else
  {
    end_offset = start_offset - count_offset;
  }

  f_log(F_LOG_FINE, "size of zero offset: %zu %zu", zero_offset, sizeof(size_t));

  size_t buffer_size = sizeof(size_t);
  size_t* start_bytes = malloc(buffer_size);
  size_t* end_bytes = malloc(buffer_size);

  if (pread(index->flookup->fd, start_bytes, buffer_size, start_offset) < 0)
  {
    perror("read failed");
    f_log(F_LOG_ERROR, "index read at %u returned 0 bytes", start);
    *out = NULL;

    free(start_bytes);
    free(end_bytes);
    return 0;
  }

  if (pread(index->flookup->fd, end_bytes, buffer_size, end_offset) < 0)
  {
    perror("read failed");
    f_log(F_LOG_ERROR, "index read at %u returned 0 bytes", start);
    *out = NULL;

    free(start_bytes);
    free(end_bytes);
    return 0;
  }

  // Check the zero offset for logging
  if (log_level & F_LOG_FINE)
  {
    size_t* zero_bytes = malloc(buffer_size);
    if (pread(index->flookup->fd, zero_bytes, buffer_size, (off_t) 0) < 0)
    {
      perror("read failed");
      f_log(F_LOG_ERROR, "index read at %u returned 0 bytes", 0);
      *out = NULL;

      free(zero_bytes);
      free(start_bytes);
      free(end_bytes);
      return 0;
    }

    f_log(F_LOG_FINE, "Zero bytes offset from index is %zu", *zero_bytes);
    free(zero_bytes);
  }

  if (*start_bytes > *end_bytes)
  {
    f_log(F_LOG_ERROR, "something went wrong - (zo: %zu) start: %ld, count: %ld, (start_offset: %zu, end_offset %zu, count offset: %zu) %zu, %zu", zero_offset, start, count, start_offset, end_offset, count_offset, start_bytes, end_bytes);
    *out = NULL;
    
    free(start_bytes);
    free(end_bytes);
    return -1;
  }

  size_t bytes = *end_bytes - *start_bytes;
  char* buffer = malloc(sizeof(char) * bytes);
  if (buffer == NULL)
  {
    perror("failed to allocate buffer for lookup");
    *out = NULL;

    free(start_bytes);
    free(end_bytes);
    return -1;
  }

  off_t starting_bytes = (off_t) *start_bytes;
  ssize_t bytes_read = pread(index->fd, buffer, bytes, starting_bytes);

  if (bytes_read < 0)
  {
    perror("bytes_read is negative");

    f_log(F_LOG_ERROR, "invalid - bytes: %zu  start bytes: %zu, end_bytes: %zu offt (%ld)\n", bytes, *start_bytes, *end_bytes, starting_bytes);
    f_log(F_LOG_DEBUG, "[debug] bytes read: %d lines are: %zu, %zu, [allocation %zu]", bytes_read, start, start + count, bytes);
    f_log(F_LOG_DEBUG, "Start off: %ld - Count off %ld, End off: %ld zero off: %ld", start_offset, count_offset, end_offset, zero_offset);

    free(buffer);
    free(start_bytes);
    free(end_bytes);
    *out = NULL;
    return -1;
  }

  char* string = malloc(sizeof(char) * (bytes_read + 1));
  if (string == NULL)
  {
    f_log(F_LOG_ERROR, "Couldn't allocate string!");
    free(buffer);
    free(start_bytes);
    free(end_bytes);
    *out = NULL;
    return -1;
  }

  memcpy(string, buffer, bytes_read);
  string[bytes_read] = 0;

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
