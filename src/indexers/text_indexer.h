#ifndef FLASHLIGHT_INDEXERS_TEXT_H
#define FLASHLIGHT_INDEXERS_TEXT_H

/** @struct FTextThread
* @brief a config to pass to each thread
* @var FTextThread::fd
* the file descriptor of the target
* @var FTextThread::from
* the start offset to read from target
* @var FTextThread::to
* the end offset
* @var FTextThread::total_bytes_count
* the total number of bytes in the target
* @var FTextThread::buffer_size
* the computed buffer size
* @var FTextThread::concurrency
* the computed concurrency
* @var FTextThread::thread
* the thread index
* @var FTextThread::progress
* the current progress of this thread
*/
typedef struct FTextThread 
{
  int fd;
  // thread members
  unsigned long int from;
  unsigned long int to;
  unsigned long int total_bytes_count;
  size_t buffer_size;
  int concurrency;
  int thread;
  double progress;
} f_text_thread;


/**
  Indexes a text file of an arbitrary size concurrently, in multiple threads
  @param indexer the configuration for the indexer
  @return an FIndex
*/
f_index* f_index_text_file(f_indexer indexer);

#endif
