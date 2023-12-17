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
  unsigned int start;
  unsigned int count;
  unsigned int buffer;
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
