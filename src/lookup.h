#ifndef FLASHLIGHT_LOOKUP_H
#define FLASHLIGHT_LOOKUP_H

/** @struct FLookupFile
* @brief an persistent index
* @var path
* the location of the index
* @var fd
* the file descriptor of the index
* @var fp
* the file pointer of the index
* @var len
* the number of lines in the target file
*/
typedef struct FLookupFile
{
  char* path;
  int fd;
  FILE* fp;
  unsigned int len;
} f_lookup_file;

/** @struct FLookupMem
* @brief an in-memory index
* @var len
* the number of lines in the target file
* @var values
* an array of line offsets
*/
typedef struct FLookupMem
{
  size_t len;
  size_t* values;
} f_lookup_mem;

/**
  Create a memory lookup from an FChunk
  @param out the lookup to init
  @param chunk the chunk to use
  @return non zero for error
*/
int f_lookup_mem_from_chunk(f_lookup_mem** out, f_chunk* chunk);
void f_lookup_mem_free(f_lookup_mem* lookup);

/**
  Init an persistent index
  @param out the lookup to init
  @param path the filename for the index
  @return non zero for error
*/
int f_lookup_file_init(f_lookup_file** out, char* path);

/**
  Append an offset to the persistent index
  @param db the lookup to append to
  @param offset the offset to append
  @return non zero for error
*/
int f_lookup_file_append(f_lookup_file* db, size_t offset);

/**
  Init a persistent index from an FChunk
  @param out the lookup to use
  @param chunk the chunk to use
  @param path the filename for the index
  @param first if true, create the lookup, else the lookup is expected to be inited
  @param last if true, add a 0 byte offset to represent the beginning of the target file
*/
int f_lookup_file_from_chunk(f_lookup_file** out, f_chunk* chunk, char* path, bool first, bool last);
void f_lookup_file_free(f_lookup_file** lookupref);

#endif
