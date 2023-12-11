#ifndef FLASHLIGHT_CHUNK_H
#define FLASHLIGHT_CHUNK_H

/** @struct FChunk
* @brief Represents a aggregate of FBytesNode.
* 
* Used to track a partial number of offsets from a concurrent or threaded function.
* FChunk is meant to be aggregated into an array, and it's target array position is tracked on initialization.
* @var FChunk::current
* the current position of this chunk.
* @var FChunk::first
* the first node of this chunk
* @var FChunk::last
* the last node of this chunk
* @var FChunk::empty
* true if the chunk doesn't have an FBytesNode
* @var FChunk::line_count
* the count of offsets used in this chunk
*/
typedef struct FChunk
{
  unsigned long int current;
  f_bytes_node* first;
  f_bytes_node* last;
  bool empty;
  unsigned int line_count;
} f_chunk;

/**
  Initialize a new FChunk
  @param out the FChunk to initialize
  @param current the expected position of this chunk in relation to other chunks
  @param firstref the first FBytesNode of this chunk
  @param lastref the last FBytesNode of this chunk (typically the same as the first)
*/
int f_chunk_new(f_chunk** out, unsigned long int current, f_bytes_node* firstref, f_bytes_node* lastref);

/**
  Free a chunk
  @param chunk the chunk to free
*/
void f_chunk_free(f_chunk** chunk);

/**
  Free a chunk and the FBytesNode in it.
  @param chunk the chunk to free
*/
void f_chunk_free_all(f_chunk* chunk);

/**
  Reverses an array of FChunk into a a single FChunk.
  
  In order to produce a concurrent linked list, each concurrent function 
  needs to return a chunk.
  A chunk contains a linked list with a reference to its tail.
  when we have an array of chunks, we can then link the lists by the tail ref when flattening.

  ```
  [{ first: c -> b -> a, last: a }, { first: f -> e -> d, last: d }, {first: i -> h -> g, last: g }]

  we want to combine these lists to produce the following chunk.

  { first: i -> h -> g -> f -> e -> d -> c -> b -> a, last: a }
  ```
  @param out the new FChunk
  @param idx the expected position of the new FChunk
  @param chunks the array to reduce
  @param len the length of the array to reduce
*/
int f_chunk_array_reverse_reduce(f_chunk** out, int idx, f_chunk** chunks, size_t len);

/**
  Create a new FChunk array
  @param out the new FChunk array
  @param len the length of the array
*/
int f_chunk_array_new(f_chunk*** out, unsigned long len);

/**
  Frees an FChunk array.
  Does not free the FBytesNode in the chunk
  @param chunks the chunk array to free
  @param len the length of the chunk array
*/
void f_chunk_array_free(f_chunk** chunks, size_t len);

/**
  Frees an FChunk array and its contents
  @param chunks the chunk array to free
  @param len the length of the chunk array
*/
void f_chunk_array_free_all(f_chunk** chunks, size_t len);

#endif
