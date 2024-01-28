#ifndef FLASHLIGHT_LOOKUP
#define FLASHLIGHT_LOOKUP
#include "lookup.h"

int f_lookup_mem_from_chunk(f_lookup_mem** out, f_chunk* chunk)
{
  f_bytes_node* current = chunk->first;
  unsigned int len = chunk->line_count;

  f_lookup_mem* init = malloc(sizeof(*init));
  if (init == NULL)
  {
    return -1;
  }

  init->len = len + 2;
  init->values = malloc(sizeof(size_t) * (len + 2));

  if (init->values == NULL)
  {
    free(init);
    return -1;
  }

  int i = len;

  while (current != NULL)
  {
    init->values[i] = current->bytes->offset;
    i--;

    f_bytes_node* tmp = current;
    current = current->next;
    // let's try to free everything while we are making 1 pass
    free(tmp->bytes);
    free(tmp);
  }

  free(chunk);

  init->values[0] = 0ul;
  init->values[len + 1] = init->values[len];

  *out = init;
  return 0;
}

void f_lookup_mem_free(f_lookup_mem* lookup)
{
  free(lookup->values);
  free(lookup);
}

FILE* fopen_mkdir(const char* path, const char* mode)
{
  char* p = strdup(path);
  if (p == NULL)
  {
    return NULL;
  }

  char* sep = strchr(p+1, '/');
  while(sep != NULL)
  {
    *sep = '\0';
    if (mkdir(p, 0755) && errno != EEXIST)
    {
      f_log(F_LOG_ERROR, "error while trying to create %s\n", p);
      return NULL;
    }
    *sep = '/';
    sep = strchr(sep+1, '/');
  }
  free(p);
  return fopen(path, mode);
}

int f_lookup_file_init(f_lookup_file** out, char* path)
{
  f_lookup_file* init = malloc(sizeof(*init));
  if (init == NULL)
  {
    return -1;
  }

  FILE* fp = fopen_mkdir(path, "w+b");
  if (fp == NULL)
  {
    perror("Error opening");
    f_log(F_LOG_ERROR, "Can't open file (w+b) %s", path);
    return -1;
  }

  int fd = fileno(fp);
  if (fd == -1)
  {
    f_log(F_LOG_ERROR, "Can't get file descriptor for lookup");
    return -1;
  }

  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
  {
    return -1;
  }
  
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
  {
    return -1;
  }

  init->path = path;
  init->fd = fd;
  init->fp = fp;
  init->len = 0ul;

  *out = init;
  return 0;
}

int f_lookup_file_append(f_lookup_file* db, size_t offset)
{
  int rc = fwrite(&offset, sizeof(size_t), 1, db->fp);
  if (rc < 1)
  {
    perror("unable to append file lookup");
    return -1;
  }

  return 0;
}

int f_lookup_file_from_chunk(f_lookup_file** out, f_chunk* chunk, char* path, bool first, bool last)
{
  f_lookup_file* init;

  if (first)
  {
    if (f_lookup_file_init(&init, path) == -1)
    {
      f_log(F_LOG_ERROR, "Couldn't init lookup file");
      return -1;
    }
  }
  else
  {
    init = *out;
  }

  f_bytes_node* current = chunk->first;
  unsigned int len = chunk->line_count;
  unsigned int i = len;

  f_log(F_LOG_INFO, "starting write to file");

  while (current != NULL)
  { 
    if (f_lookup_file_append(init, current->bytes->offset) == -1)
    {
      f_log(F_LOG_ERROR, "error appending to file");
      return -1;
    }
    i--;

    f_bytes_node* tmp = current;
    current = current->next;
    // let's try to free everything while we are making 1 pass
    free(tmp->bytes);
    free(tmp);
  }

  F_MTRIM(0);

  if (last)
  {
    if (f_lookup_file_append(init, 0ul) == -1)
    {
      f_log(F_LOG_ERROR, "Can't append to lookup file");
      return -1;
    }
    init->len += len + 1;
  }
  else
  {
    init->len += len;
  }

  if (fflush(init->fp) != 0)
  {
    perror("didn't flush");
    return -1;
  }

  f_log(F_LOG_INFO, "finished write to file");

  free(chunk);
  *out = init;
  return 0;
}

void f_lookup_file_free(f_lookup_file** lookupref)
{
  f_lookup_file* lookup = *lookupref;
  if (fclose(lookup->fp) != 0)
  {
    // not blockiing.
    f_log(F_LOG_WARN, "Cannot close file descriptor");
  }

  if (remove(lookup->path) != 0)
  {
    // it's okay if the index was already deleted
    // or changed perms
    f_log(F_LOG_WARN, "Cannot free lookup path");
  }

  free(lookup->path);
  free(lookup);
  *lookupref = NULL;
}

#endif
