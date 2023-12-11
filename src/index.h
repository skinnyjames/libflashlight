#ifndef FLASHLIGHT_INDEX_H
#define FLASHLIGHT_INDEX_H

/** @struct FIndex
* @brief an index of a target file that resides on disk
* 
* The index is the main export of this library.
* The index itself can be stored in memory (FLookupMem) 
* or also live on disk (FLookupFile).  It cannot be both.
* @var FIndex::filename
* the target filename
* @var FIndex::filename_len
* the target filename length
* @var FIndex::fd
* the target file descriptor
* @var FIndex::fp
* a file pointer to the target.
* @var FIndex::flookup
* a file lookup (NULL if unused)
* @var FIndex::mlookup
* an in-memory lookup (NULL if unsed)
*/
typedef struct FIndex
{
  char* filename;
  int filename_len;
  int fd;
  FILE* fp;
  f_lookup_file* flookup;
  f_lookup_mem* mlookup;
} f_index;

/**
  Initializes a new index

  @param out the index to initialize
  @param filename the target filename for the index
  @param filename_len the target filename length
  @param flookup a file lookup (NULL if unused)
  @param mlookup an memory lookup (NULL if unused)
  @return non zero for error
*/
int f_index_init(f_index** out, char* filename, int filename_len, f_lookup_file* flookup, f_lookup_mem* mlookup);

/**
  Fetches a portion of the file
  
  @param out the fetched string
  @param index the index to search
  @param start the start line index
  @param count the number of lines to fetch
  @param size an int ref that is populated with the size of the fetched string
  @return non zero for error
*/
int f_index_lookup(char** out, f_index* index, unsigned int start, unsigned int count, int* size);

/**
  Frees an index and it's lookup

  If the lookup is a FLookupFile, the file is deleted
  @param index the index to free
*/
void f_index_free(f_index** index);
typedef void (*lookup_stream_cb)(char* data);

#endif
