#ifndef FLASHLIGHT_INDEXER_H
#define FLASHLIGHT_INDEXER_H

/** @file indexer.h
* @brief Helpers to determine ahead of time what bytes to index in each thread.
* All calculations are done before spawning a thread.
*/

typedef void (*indexer_progress_cb)(double progress, void* payload);


/** @struct FIndexer
* @brief the configuration to supply to the indexer.
* @var FIndexer::filename
* the location of the target file to index.
* @var FIndexer::filename_len
* the length of the target location
* @var FIndexer::lookup_dir
* the directory to store the lookup index
* @var FIndexer::threads
* the number of threads to spawn during indexing.
* @var FIndexer::concurrency
* the number of concurrent calls to make in each thread
* @var FIndexer::buffer_size
* the buffer to use for each concurrent call
* @var on_progress
* a callback to track the progress of the indexing (NULL if unused)
* @var payload
* a payload that is passed to the progress callback (NULL if unused)
*/
typedef struct FIndexer
{
  char* filename;
  int filename_len;
  char* lookup_dir;
  int threads;
  int concurrency;
  size_t buffer_size;
  indexer_progress_cb on_progress;
  void* payload;
} f_indexer;


/** @struct FIndexerThread
* @brief the configuration to pass to each thread
* @var FIndexerThread::from
* the start byte offset to read from target
* @var FIndexerThread::to
* the end byte offset
* @var FIndexerThread::total_bytes_count
* the total bytes count of the target file
* @var FIndexerThread::buffer_size
* the computed buffer size
*/
typedef struct FIndexerThread
{
  unsigned long int from;
  unsigned long int to;
  unsigned long int total_bytes_count;
  size_t buffer_size;
} f_indexer_thread;

/** @struct FIndexerThreads
* @brief An array of thread indexer configs
* @var FIndexerThreads::threads
* the configs
* @var FIndexerThreads::len
* the len of the array
*/
typedef struct FIndexerThreads
{
  f_indexer_thread* threads;
  int len;
} f_indexer_threads;

/** @struct FIndexerChunk
* @brief A config to pass to each concurrent indexing function call
* @var FIndexerChunk::index
* the thread this chunk is for
* @var FIndexerChunk::from
* the start bytes offset for the target file
* @var FIndexerChunk::to
* the count of bytes to read from the offset
*/
typedef struct FIndexerChunk
{
  unsigned long int index;
  size_t from;
  size_t count;
} f_indexer_chunk;

/** @struct FIndexerChunks
* @brief An array of configs
* @var FIndexerChunks::chunks
* the list of FIndexerChunk
* @var FIndexerChunks::concurrency
* the computed concurrency
* @var FIndexerChunks::len
* the length of the array
*/
typedef struct FIndexerChunks
{
  f_indexer_chunk** chunks;
  int concurrency;
  int len;
} f_indexer_chunks;

/**
  Inits a new FIndexerChunks based on concurrency and buffer size

  @param out the chunks to init
  @param concurrency the expected concurrency
  @param buffer_size the expected buffer size
  @param total_bytes the total number of bytes in the target file
  @param from the start byte offset
  @param to the end byte offset
  @return non zero for error
*/
int f_indexer_chunks_init(f_indexer_chunks** out, int concurrency, size_t buffer_size, size_t total_bytes, size_t from, size_t to);

/**
  Frees an FIndexerChunks
  @param index the FIndexerChunks to free
*/
void f_indexer_chunks_free(f_indexer_chunks* index);

/**
  Inits a new FIndexerThreads based on thread count and buffer size
  
  @param out the configs to init
  @param threads the expected thread count
  @param total_bytes_count the total number of bytes in the target file
  @param buffer_size the expected buffer size
  @param offset the start offset of the target file
  @return non zero for error
*/
int f_indexer_threads_init(f_indexer_threads** out, int threads, size_t total_bytes_count, size_t buffer_size, size_t offset);
void f_indexer_threads_free(f_indexer_threads* index);

#endif
