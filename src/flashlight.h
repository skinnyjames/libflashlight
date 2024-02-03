#pragma once

#ifndef CWK_LIBRARY_H
#define CWK_LIBRARY_H

#include <stdbool.h>
#include <stddef.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#define CWK_EXPORT __declspec(dllexport)
#define CWK_IMPORT __declspec(dllimport)
#elif __GNUC__ >= 4
#define CWK_EXPORT __attribute__((visibility("default")))
#define CWK_IMPORT __attribute__((visibility("default")))
#else
#define CWK_EXPORT
#define CWK_IMPORT
#endif

#if defined(CWK_SHARED)
#if defined(CWK_EXPORTS)
#define CWK_PUBLIC CWK_EXPORT
#else
#define CWK_PUBLIC CWK_IMPORT
#endif
#else
#define CWK_PUBLIC
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * A segment represents a single component of a path. For instance, on linux a
 * path might look like this "/var/log/", which consists of two segments "var"
 * and "log".
 */
struct cwk_segment
{
  const char *path;
  const char *segments;
  const char *begin;
  const char *end;
  size_t size;
};

/**
 * The segment type can be used to identify whether a segment is a special
 * segment or not.
 *
 * CWK_NORMAL - normal folder or file segment
 * CWK_CURRENT - "./" current folder segment
 * CWK_BACK - "../" relative back navigation segment
 */
enum cwk_segment_type
{
  CWK_NORMAL,
  CWK_CURRENT,
  CWK_BACK
};

/**
 * @brief Determines the style which is used for the path parsing and
 * generation.
 */
enum cwk_path_style
{
  CWK_STYLE_WINDOWS,
  CWK_STYLE_UNIX
};

/**
 * @brief Generates an absolute path based on a base.
 *
 * This function generates an absolute path based on a base path and another
 * path. It is guaranteed to return an absolute path. If the second submitted
 * path is absolute, it will override the base path. The result will be
 * written to a buffer, which might be truncated if the buffer is not large
 * enough to hold the full path. However, the truncated result will always be
 * null-terminated. The returned value is the amount of characters which the
 * resulting path would take if it was not truncated (excluding the
 * null-terminating character).
 *
 * @param base The absolute base path on which the relative path will be
 * applied.
 * @param path The relative path which will be applied on the base path.
 * @param buffer The buffer where the result will be written to.
 * @param buffer_size The size of the result buffer.
 * @return Returns the total amount of characters of the new absolute path.
 */
CWK_PUBLIC size_t cwk_path_get_absolute(const char *base, const char *path,
  char *buffer, size_t buffer_size);

/**
 * @brief Generates a relative path based on a base.
 *
 * This function generates a relative path based on a base path and another
 * path. It determines how to get to the submitted path, starting from the
 * base directory. The result will be written to a buffer, which might be
 * truncated if the buffer is not large enough to hold the full path. However,
 * the truncated result will always be null-terminated. The returned value is
 * the amount of characters which the resulting path would take if it was not
 * truncated (excluding the null-terminating character).
 *
 * @param base_directory The base path from which the relative path will
 * start.
 * @param path The target path where the relative path will point to.
 * @param buffer The buffer where the result will be written to.
 * @param buffer_size The size of the result buffer.
 * @return Returns the total amount of characters of the full path.
 */
CWK_PUBLIC size_t cwk_path_get_relative(const char *base_directory,
  const char *path, char *buffer, size_t buffer_size);

/**
 * @brief Joins two paths together.
 *
 * This function generates a new path by combining the two submitted paths. It
 * will remove double separators, and unlike cwk_path_get_absolute it permits
 * the use of two relative paths to combine. The result will be written to a
 * buffer, which might be truncated if the buffer is not large enough to hold
 * the full path. However, the truncated result will always be
 * null-terminated. The returned value is the amount of characters which the
 * resulting path would take if it was not truncated (excluding the
 * null-terminating character).
 *
 * @param path_a The first path which comes first.
 * @param path_b The second path which comes after the first.
 * @param buffer The buffer where the result will be written to.
 * @param buffer_size The size of the result buffer.
 * @return Returns the total amount of characters of the full, combined path.
 */
CWK_PUBLIC size_t cwk_path_join(const char *path_a, const char *path_b,
  char *buffer, size_t buffer_size);

/**
 * @brief Joins multiple paths together.
 *
 * This function generates a new path by joining multiple paths together. It
 * will remove double separators, and unlike cwk_path_get_absolute it permits
 * the use of multiple relative paths to combine. The last path of the
 * submitted string array must be set to NULL. The result will be written to a
 * buffer, which might be truncated if the buffer is not large enough to hold
 * the full path. However, the truncated result will always be
 * null-terminated. The returned value is the amount of characters which the
 * resulting path would take if it was not truncated (excluding the
 * null-terminating character).
 *
 * @param paths An array of paths which will be joined.
 * @param buffer The buffer where the result will be written to.
 * @param buffer_size The size of the result buffer.
 * @return Returns the total amount of characters of the full, combined path.
 */
CWK_PUBLIC size_t cwk_path_join_multiple(const char **paths, char *buffer,
  size_t buffer_size);

/**
 * @brief Determines the root of a path.
 *
 * This function determines the root of a path by finding its length. The
 * root always starts at the submitted path. If the path has no root, the
 * length will be set to zero.
 *
 * @param path The path which will be inspected.
 * @param length The output of the root length.
 */
CWK_PUBLIC void cwk_path_get_root(const char *path, size_t *length);

/**
 * @brief Changes the root of a path.
 *
 * This function changes the root of a path. It does not normalize the result.
 * The result will be written to a buffer, which might be truncated if the
 * buffer is not large enough to hold the full path. However, the truncated
 * result will always be null-terminated. The returned value is the amount of
 * characters which the resulting path would take if it was not truncated
 * (excluding the null-terminating character).
 *
 * @param path The original path which will get a new root.
 * @param new_root The new root which will be placed in the path.
 * @param buffer The output buffer where the result is written to.
 * @param buffer_size The size of the output buffer where the result is
 * written to.
 * @return Returns the total amount of characters of the new path.
 */
CWK_PUBLIC size_t cwk_path_change_root(const char *path, const char *new_root,
  char *buffer, size_t buffer_size);

/**
 * @brief Determine whether the path is absolute or not.
 *
 * This function checks whether the path is an absolute path or not. A path is
 * considered to be absolute if the root ends with a separator.
 *
 * @param path The path which will be checked.
 * @return Returns true if the path is absolute or false otherwise.
 */
CWK_PUBLIC bool cwk_path_is_absolute(const char *path);

/**
 * @brief Determine whether the path is relative or not.
 *
 * This function checks whether the path is a relative path or not. A path is
 * considered to be relative if the root does not end with a separator.
 *
 * @param path The path which will be checked.
 * @return Returns true if the path is relative or false otherwise.
 */
CWK_PUBLIC bool cwk_path_is_relative(const char *path);

/**
 * @brief Gets the basename of a file path.
 *
 * This function gets the basename of a file path. A pointer to the beginning
 * of the basename will be returned through the basename parameter. This
 * pointer will be positioned on the first letter after the separator. The
 * length of the file path will be returned through the length parameter. The
 * length will be set to zero and the basename to NULL if there is no basename
 * available.
 *
 * @param path The path which will be inspected.
 * @param basename The output of the basename pointer.
 * @param length The output of the length of the basename. This may be
 * null if not required.
 */
CWK_PUBLIC void cwk_path_get_basename(const char *path, const char **basename,
  size_t *length);

/**
 * @brief Changes the basename of a file path.
 *
 * This function changes the basename of a file path. This function will not
 * write out more than the specified buffer can contain. However, the
 * generated string is always null-terminated - even if not the whole path is
 * written out. The function returns the total number of characters the
 * complete buffer would have, even if it was not written out completely. The
 * path may be the same memory address as the buffer.
 *
 * @param path The original path which will be used for the modified path.
 * @param new_basename The new basename which will replace the old one.
 * @param buffer The buffer where the changed path will be written to.
 * @param buffer_size The size of the result buffer where the changed path is
 * written to.
 * @return Returns the size which the complete new path would have if it was
 * not truncated.
 */
CWK_PUBLIC size_t cwk_path_change_basename(const char *path,
  const char *new_basename, char *buffer, size_t buffer_size);

/**
 * @brief Gets the dirname of a file path.
 *
 * This function determines the dirname of a file path and returns the length
 * up to which character is considered to be part of it. If no dirname is
 * found, the length will be set to zero. The beginning of the dirname is
 * always equal to the submitted path pointer.
 *
 * @param path The path which will be inspected.
 * @param length The length of the dirname.
 */
CWK_PUBLIC void cwk_path_get_dirname(const char *path, size_t *length);

/**
 * @brief Gets the extension of a file path.
 *
 * This function extracts the extension portion of a file path. A pointer to
 * the beginning of the extension will be returned through the extension
 * parameter if an extension is found and true is returned. This pointer will
 * be positioned on the dot. The length of the extension name will be returned
 * through the length parameter. If no extension is found both parameters
 * won't be touched and false will be returned.
 *
 * @param path The path which will be inspected.
 * @param extension The output of the extension pointer.
 * @param length The output of the length of the extension.
 * @return Returns true if an extension is found or false otherwise.
 */
CWK_PUBLIC bool cwk_path_get_extension(const char *path, const char **extension,
  size_t *length);

/**
 * @brief Determines whether the file path has an extension.
 *
 * This function determines whether the submitted file path has an extension.
 * This will evaluate to true if the last segment of the path contains a dot.
 *
 * @param path The path which will be inspected.
 * @return Returns true if the path has an extension or false otherwise.
 */
CWK_PUBLIC bool cwk_path_has_extension(const char *path);

/**
 * @brief Changes the extension of a file path.
 *
 * This function changes the extension of a file name. The function will
 * append an extension if the basename does not have an extension, or use the
 * extension as a basename if the path does not have a basename. This function
 * will not write out more than the specified buffer can contain. However, the
 * generated string is always null-terminated - even if not the whole path is
 * written out. The function returns the total number of characters the
 * complete buffer would have, even if it was not written out completely. The
 * path may be the same memory address as the buffer.
 *
 * @param path The path which will be used to make the change.
 * @param new_extension The extension which will be placed within the new
 * path.
 * @param buffer The output buffer where the result will be written to.
 * @param buffer_size The size of the output buffer where the result will be
 * written to.
 * @return Returns the total size which the output would have if it was not
 * truncated.
 */
CWK_PUBLIC size_t cwk_path_change_extension(const char *path,
  const char *new_extension, char *buffer, size_t buffer_size);

/**
 * @brief Creates a normalized version of the path.
 *
 * This function creates a normalized version of the path within the specified
 * buffer. This function will not write out more than the specified buffer can
 * contain. However, the generated string is always null-terminated - even if
 * not the whole path is written out. The function returns the total number of
 * characters the complete buffer would have, even if it was not written out
 * completely. The path may be the same memory address as the buffer.
 *
 * The following will be true for the normalized path:
 * 1) "../" will be resolved.
 * 2) "./" will be removed.
 * 3) double separators will be fixed with a single separator.
 * 4) separator suffixes will be removed.
 *
 * @param path The path which will be normalized.
 * @param buffer The buffer where the new path is written to.
 * @param buffer_size The size of the buffer.
 * @return The size which the complete normalized path has if it was not
 * truncated.
 */
CWK_PUBLIC size_t cwk_path_normalize(const char *path, char *buffer,
  size_t buffer_size);

/**
 * @brief Finds common portions in two paths.
 *
 * This function finds common portions in two paths and returns the number
 * characters from the beginning of the base path which are equal to the other
 * path.
 *
 * @param path_base The base path which will be compared with the other path.
 * @param path_other The other path which will compared with the base path.
 * @return Returns the number of characters which are common in the base path.
 */
CWK_PUBLIC size_t cwk_path_get_intersection(const char *path_base,
  const char *path_other);

/**
 * @brief Gets the first segment of a path.
 *
 * This function finds the first segment of a path. The position of the
 * segment is set to the first character after the separator, and the length
 * counts all characters until the next separator (excluding the separator).
 *
 * @param path The path which will be inspected.
 * @param segment The segment which will be extracted.
 * @return Returns true if there is a segment or false if there is none.
 */
CWK_PUBLIC bool cwk_path_get_first_segment(const char *path,
  struct cwk_segment *segment);

/**
 * @brief Gets the last segment of the path.
 *
 * This function gets the last segment of a path. This function may return
 * false if the path doesn't contain any segments, in which case the submitted
 * segment parameter is not modified. The position of the segment is set to
 * the first character after the separator, and the length counts all
 * characters until the end of the path (excluding the separator).
 *
 * @param path The path which will be inspected.
 * @param segment The segment which will be extracted.
 * @return Returns true if there is a segment or false if there is none.
 */
CWK_PUBLIC bool cwk_path_get_last_segment(const char *path,
  struct cwk_segment *segment);

/**
 * @brief Advances to the next segment.
 *
 * This function advances the current segment to the next segment. If there
 * are no more segments left, the submitted segment structure will stay
 * unchanged and false is returned.
 *
 * @param segment The current segment which will be advanced to the next one.
 * @return Returns true if another segment was found or false otherwise.
 */
CWK_PUBLIC bool cwk_path_get_next_segment(struct cwk_segment *segment);

/**
 * @brief Moves to the previous segment.
 *
 * This function moves the current segment to the previous segment. If the
 * current segment is the first one, the submitted segment structure will stay
 * unchanged and false is returned.
 *
 * @param segment The current segment which will be moved to the previous one.
 * @return Returns true if there is a segment before this one or false
 * otherwise.
 */
CWK_PUBLIC bool cwk_path_get_previous_segment(struct cwk_segment *segment);

/**
 * @brief Gets the type of the submitted path segment.
 *
 * This function inspects the contents of the segment and determines the type
 * of it. Currently, there are three types CWK_NORMAL, CWK_CURRENT and
 * CWK_BACK. A CWK_NORMAL segment is a normal folder or file entry. A
 * CWK_CURRENT is a "./" and a CWK_BACK a "../" segment.
 *
 * @param segment The segment which will be inspected.
 * @return Returns the type of the segment.
 */
CWK_PUBLIC enum cwk_segment_type cwk_path_get_segment_type(
  const struct cwk_segment *segment);

/**
 * @brief Changes the content of a segment.
 *
 * This function overrides the content of a segment to the submitted value and
 * outputs the whole new path to the submitted buffer. The result might
 * require less or more space than before if the new value length differs from
 * the original length. The output is truncated if the new path is larger than
 * the submitted buffer size, but it is always null-terminated. The source of
 * the segment and the submitted buffer may be the same.
 *
 * @param segment The segment which will be modifier.
 * @param value The new content of the segment.
 * @param buffer The buffer where the modified path will be written to.
 * @param buffer_size The size of the output buffer.
 * @return Returns the total size which would have been written if the output
 * was not truncated.
 */
CWK_PUBLIC size_t cwk_path_change_segment(struct cwk_segment *segment,
  const char *value, char *buffer, size_t buffer_size);

/**
 * @brief Checks whether the submitted pointer points to a separator.
 *
 * This function simply checks whether the submitted pointer points to a
 * separator, which has to be null-terminated (but not necessarily after the
 * separator). The function will return true if it is a separator, or false
 * otherwise.
 *
 * @param symbol A pointer to a string.
 * @return Returns true if it is a separator, or false otherwise.
 */
CWK_PUBLIC bool cwk_path_is_separator(const char *str);

/**
 * @brief Guesses the path style.
 *
 * This function guesses the path style based on a submitted path-string. The
 * guessing will look at the root and the type of slashes contained in the
 * path and return the style which is more likely used in the path.
 *
 * @param path The path which will be inspected.
 * @return Returns the style which is most likely used for the path.
 */
CWK_PUBLIC enum cwk_path_style cwk_path_guess_style(const char *path);

/**
 * @brief Configures which path style is used.
 *
 * This function configures which path style is used. The following styles are
 * currently supported.
 *
 * CWK_STYLE_WINDOWS: Use backslashes as a separator and volume for the root.
 * CWK_STYLE_UNIX: Use slashes as a separator and a slash for the root.
 *
 * @param style The style which will be used from now on.
 */
CWK_PUBLIC void cwk_path_set_style(enum cwk_path_style style);

/**
 * @brief Gets the path style configuration.
 *
 * This function gets the style configuration which is currently used for the
 * paths. This configuration determines how paths are parsed and generated.
 *
 * @return Returns the current path style configuration.
 */
CWK_PUBLIC enum cwk_path_style cwk_path_get_style(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
#ifndef FLASHLIGHT_LIB_H
#define FLASHLIGHT_LIB_H

#define PCRE2_CODE_UNIT_WIDTH 8
#if defined(__linux___) || defined(__gnu_linux__) || defined(linux) || defined(__linux) || defined(__unix__)
#include <malloc.h>
#define F_MTRIM(a) malloc_trim(a)
#else
#define F_MTRIM(a) do {} while(0)
#endif
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pcre2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>

#endif
#ifndef FLASHLIGHT_LOG_H
#define FLASHLIGHT_LOG_H

#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

enum F_LOG_LEVEL
{
  F_LOG_ERROR = 1 << 0,
  F_LOG_WARN = 1 << 1,
  F_LOG_INFO = 1 << 2,
  F_LOG_DEBUG = 1 << 3,
  F_LOG_FINE = 1 << 4
};

typedef struct FLogMessage
{
  char* datetime;
  enum F_LOG_LEVEL level;
  char* message;
} f_log_message;

typedef void (*f_logger_cb)(f_log_message message, volatile void* payload);
static volatile f_logger_cb f_log_cb = NULL;
static volatile enum F_LOG_LEVEL f_log_level = F_LOG_ERROR;
static volatile void* f_log_payload = NULL;

void f_logger_set_level(enum F_LOG_LEVEL level);
void f_logger_set_cb(f_logger_cb cb, volatile void* payload);
f_logger_cb f_logger_get_cb();
enum F_LOG_LEVEL f_logger_get_level();
volatile void* f_logger_get_payload();
void f_log(enum F_LOG_LEVEL level, char* fmt, ...);

#endif
#ifndef FLASHLIGHT_NODE_H
#define FLASHLIGHT_NODE_H

/** @struct FNode
* @brief an offset list, useful for in memory indexes
* @var offset
* the offset of the this line
* @var next
* the next FNode / offset
*/
typedef struct FNode
{
  size_t offset;
  struct FNode* next;
} f_node;

/**
  Initializes a new node
  @param out the node to init
  @param offset the offset of the new line
  @return non zero for error
*/
int f_node_new(f_node** out, size_t offset);

/**
  Append a new node to an existing node
  @param parent the existing node
  @param child the new node
*/
void f_node_append(f_node** parent, f_node* child);

/**
  Free a node
  @param node node to free
*/
void f_node_free(f_node** node);

#endif
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
  
  @param out the fetched string, zero terminated
  @param index the index to search
  @param start the start line index
  @param count the number of lines to fetch
  @return non zero for error
*/
int f_index_lookup(char** out, f_index* index, size_t start, size_t count);

/**
  Frees an index and it's lookup

  If the lookup is a FLookupFile, the file is deleted
  @param index the index to free
*/
void f_index_free(f_index** index);
typedef void (*lookup_stream_cb)(char* data);

#endif
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
* @var FIndexer::max_bytes_per_iteration
* a hard limit on how many bytes to index at one time.
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
  size_t max_bytes_per_iteration;
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
#ifndef FLASHLIGHT_SEARCH_H
#define FLASHLIGHT_SEARCH_H

/** @file search.h 
* @brief Utilty for searching a regex against an index.
* 
* Uses PCRE2 for regex implementation.
*/

/** @struct FSearchResult
* @brief a search result
*
* Propogated to the caller in the `on_result` callback
* of FSearcher.
* @var FSearchResult::line_number
* the line the result was found on.
* @var FSearchResult::str
* the full line that matched
* @var FSearchResult::matches_substring_offset
* An array of offsets for each match
* @var FSearchResult::matches_substring_len
* An array of match lengths
* @var FSearchResult::matches_len
* The number of matches
*/
typedef struct FSearchResult {
  size_t line_number;
  char* str;
  size_t* matches_substring_offset;
  size_t* matches_substring_len;
  unsigned int matches_len;
} f_search_result;

/** @struct FSearchResults
* @brief a linked list for collecting results
* 
* Note, these are unused in this lib.
* Recommend a btree implementation like [this](https://github.com/tidwall/btree.c)
* @var FSearchResults::result
* The search result
* @var FSearchResults::next
* The the next list item
*/
typedef struct FSearchResults {
  f_search_result* result;
  struct FSearchResults* next;
} f_search_results;

typedef void (*searcher_progress_cb)(double progress, void* payload);
typedef void (*searcher_cb)(f_search_result* result, void* payload);

/** @struct FSearcher
* @brief a config to pass to f_index_search
*
* @var FSearcher::regex
* A regex str to search on
* @var FSearcher::index
* The index to search against
* @var FSearcher::threads
* How many threads to use while searching
* @var FSearcher::result_limit
* The maximum number of results returned
* @var FSearcher::line_buffer
* How many lines to read from disk on a search iteration
* @var FSearcher::on_progress
* Progress callback
* @var FSearcher::progress_payload
* Payload for progress callback
* @var FSearcher::on_result
* Search result callback
* @var FSearcher::result_payload
* Payload for result callback
*/
typedef struct FSearcher {
  char* regex;
  f_index* index;
  int threads;
  int result_limit;
  unsigned int line_buffer;
  searcher_progress_cb on_progress;
  void* progress_payload;
  searcher_cb on_result;
  void* result_payload;
} f_searcher;

/** @struct FSearcherThread
* @brief search config is passed to each thread
*
* Note this is probably not going to be useful to the caller.
* @var FSearcherThread::regex
* The compiled regex
* @var FSearcherThread::index
* The index to search against
* @var FSearcherThread::start
* The line number to start searching from
* @var FSearcherThread::count
* How many lines to search
* @var FSearcherThread::buffer
* How many lines to read from disk at a time
* @var FSearcherThread::result_limit
* The max number of results
* @var FSearcherThread::result_count
* The current number of results
* @var FSearcherThread::progress
* This threads progress
* @var FSearcherThread::on_result
* Result callback
* @var FSearcherThread::result_payload
* Result payload
*/
typedef struct FSearcherThread {
  pcre2_code* regex;
  f_index* index;
  int thread;
  size_t start;
  size_t count;
  size_t buffer;
  int result_limit;
  int* result_count;
  double progress;
  searcher_cb on_result;
  void* result_payload;
} f_searcher_thread;

/**
  Initializes a search result

  @param out the search result
  @param num the number of matches
  @return non zero for error
*/
int f_search_result_init(f_search_result** out, unsigned int num);

/**
  Free a search result, its match data and substring.

  @param res the search result to free
*/
void f_search_result_free(f_search_result* res);


/**
  Allocates a new search result list
  @param out the search result list
*/
int f_search_results_init(f_search_results** out);

/**
  Prepends a result to the list
  @param results the list
  @param addition the search result to prepend
*/
void f_search_results_prepend(f_search_results** results, f_search_result* addition);

/**
  Frees the results
  @param results the list to free
*/
void f_search_results_free(f_search_results** results);

/**
  Searches an index concurrently

  Note: search results can only be aggregated through
  the `on_result` callback.

  @param out the search results
  @param config the search config
  @return non zero for error, -2 for invalid regex.
*/
int f_index_search(f_searcher config);



#endif
