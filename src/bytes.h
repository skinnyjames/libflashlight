#ifndef FLASHLIGHT_BYTES_H
#define FLASHLIGHT_BYTES_H

/** @file bytes.h
* @brief Helpers for tracking partial byte indexes/offsets in a text file
* 
* FBytes offsets are aggregated into FBytesNode linked lists.
* This allows for indexing parts of a file to later be recombined into a single FNode
*/

/** @struct FBytes
* @brief representation of the start of a file unit to index
* @var FBytes::atomic 
* true if the bytes offset decmarate the beginning of a new logical unit (ie. a new line)
* @var FBytes::offset
* the offset in the target file to 
*/
typedef struct FBytes
{
  bool atomic;
  size_t offset;
} f_bytes;


/** @struct FBytesNode
* @brief a linked list of FBytes.
* Typically used to represent an index of newlines in a text file
* @var FBytesNode::bytes
* The bytes for this node
* @var FBytesNode::next
* The next node in the linked list
*/
typedef struct FBytesNode
{
  f_bytes* bytes;
  struct FBytesNode* next;
} f_bytes_node;

/**
  Initializer for FBytes.

  @param out the initialized f_bytes struct
  @param atomic true if the offset captures the beginning of a new line
  @param offset the bytes offset
  @return non zero for an error
*/
int f_bytes_new(f_bytes** out, bool atomic, size_t offset);

/**
  Free an FBytes

  @param bytes the f_bytes to free
*/
void f_bytes_free(f_bytes** bytes);

/**
  Initializer for a new FBytesNode

  @param out the FBytesNode to initialize
  @param bytes the first f_bytes to use in the node
  @return non zero for an error
*/
int f_bytes_node_new(f_bytes_node** out, f_bytes* bytes);


/**
  Clone an FBytesNode

  @param out the cloned FBytesNode
  @param node the node to clone
  @return non zero for an error
*/
int f_bytes_node_clone(f_bytes_node**out, f_bytes_node* node);

/**
  Free an FBytesNode

  Note: Also frees all the FBytes in this node

  @param node the node to free
*/
void f_bytes_node_free(f_bytes_node** node);

/**
  Create an FNode from an FBytesNode

  This function assumes the FBytesNode is in reverse order.
  The FBytesNode is not freed.

  ```
  if the list structure looks like
  {8,false} -> {6,true} -> {2,false} -> {1,true}
  
  the function aggregates non atomic offsets with atomic ones and creates a reversed aggregate
  {0, 3, 14}
  ```

  @param out the FNode to create
  @param bytes_head the FBytesNode to transform
*/
int f_node_from_reversed_bytes_node(f_node** out, f_bytes_node** bytes_head);

/**
  Prepend onto an FBytesNode

  @param head the FBytesNode to prepend
  @param parent the target FBytesNode to prepend onto
*/
int f_bytes_node_prepend(f_bytes_node** head, f_bytes_node** parent);

#endif
