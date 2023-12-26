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
  char* sep = strchr(p+1, '/');
  while(sep != NULL)
  {
    *sep = '\0';
    if (mkdir(p, 0755) && errno != EEXIST)
    {
      fprintf(stderr, "error while trying to create %s\n", p);
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

  FILE* fp = fopen_mkdir(path, "w+b");
  if (fp == NULL)
  {
    perror("bad1");
  }

  int fd = fileno(fp);
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  init->path = path;
  init->fd = fd;
  init->fp = fp;
  init->len = 0ul;

  *out = init;
  return 0;
}

int f_lookup_file_append(f_lookup_file* db, size_t offset)
{
  // f_log(F_LOG_FINE, "writing offset, %zu %zu", offset, sizeof(size_t));
  int rc = fwrite(&offset, sizeof(size_t), 1, db->fp);
  if (rc < 1)
  {
    perror("didn't write");
    return -1;
  }

  return 0;
}

int f_lookup_file_from_chunk(f_lookup_file** out, f_chunk* chunk, char* path, bool first, bool last)
{
  f_lookup_file* init;

  if (first)
  {
    f_lookup_file_init(&init, path);
  }
  else
  {
    init = *out;
  }

  f_bytes_node* current = chunk->first;
  unsigned int len = chunk->line_count;
  unsigned int i = len;
  // size_t loff = 0ul;

  f_log(F_LOG_INFO, "starting write to file");
  while (current != NULL)
  { 
    // if (loff != 0 && loff < current->bytes->offset)
    // {
    //   f_log(F_LOG_WARN, "failed offset: %zu is less than %zu", loff, current->bytes->offset);
    //   exit(1);
    // }

    f_lookup_file_append(init, current->bytes->offset);
    // loff = current->bytes->offset;
    i--;

    f_bytes_node* tmp = current;
    current = current->next;
    // let's try to free everything while we are making 1 pass
    free(tmp->bytes);
    free(tmp);
  }

  if (last)
  {
    f_lookup_file_append(init, 0ul);
    init->len += len + 1;
  }
  else
  {
    init->len += len;
  }

  if (fflush(init->fp) != 0)
  {
    perror("didn't flush");
  }
  
  f_log(F_LOG_INFO, "finished write to file");

  free(chunk);
  *out = init;
  return 0;
}

void f_lookup_file_free(f_lookup_file** lookupref)
{
  f_lookup_file* lookup = *lookupref;
  fclose(lookup->fp);
  remove(lookup->path);
  free(lookup->path);
  free(lookup);
  *lookupref = NULL;
}

#endif
