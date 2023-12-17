#ifndef FLASHLIGHT_SEARCH
#define FLASHLIGHT_SEARCH

#include "search.h"
pthread_mutex_t search_mutex;

int f_search_result_init(f_search_result** out, unsigned int num)
{
  f_search_result* init = malloc(sizeof(*init));
  if (init == NULL) return -1;

  init->matches_substring_offset = malloc(sizeof(size_t) * num);
  if (init->matches_substring_offset == NULL) return -1;

  init->matches_substring_len = malloc(sizeof(size_t) * num);
  if (init->matches_substring_len == NULL) return -1;
  init->matches_len = num;
  init->line_number = 0;
  init->str = NULL;

  *out = init;
  return 0;
}

void f_search_result_free(f_search_result* res)
{
  free(res->matches_substring_len);
  free(res->matches_substring_offset);
  free(res->str);
  free(res);
}

int f_search_results_init(f_search_results** out)
{
  f_search_results* init = malloc(sizeof(*init));
  if (init == NULL)
  {
    return -1;
  }

  init->result = NULL;
  init->next = NULL;
  *out = init;
  return 0;
}

void f_search_results_prepend(f_search_results** results, f_search_result* addition)
{    
  f_search_result* head = (*results)->result;

  if (head == NULL)
  {
    (*results)->result = addition;
  }
  else
  {
    (*results)->result = addition;
    (*results)->next = head;
  }
}

void f_search_results_free(f_search_results** results)
{
  f_search_results* head = *results;
  while (head != NULL)
  {
    f_search_results* tmp = head->next;
    f_search_result_free((head->result));
    free(head);
    head = tmp;
  }
  *results = NULL;
}

// https://stackoverflow.com/questions/42315585/split-string-into-tokens-in-c-when-there-are-2-delimiters-in-a-row
char* tokenize(char** iptr, char* delim)
{
  char* str = *iptr;
  char* found = NULL;
  int token_length = 0;
  char* token = NULL;

  if (!str) return NULL;

  found = strstr(str, delim);
  if (found)
  {
    token_length = found - str;
  }
  else
  {
    token_length = strlen(str);
  }

  token = malloc(token_length + 1);
  memcpy(token, str, token_length);
  token[token_length] = '\0';

  *iptr = found ? found + strlen(delim) : NULL;
  return token;
}

void* f_index_search_thread(void* payload)
{
  f_searcher_thread* config = payload;
  f_index* index = config->index;
  
  pcre2_match_data* match_data;
  match_data = pcre2_match_data_create_from_pattern(config->regex, NULL);

  PCRE2_SIZE* ovector;  
  int rc;

  for (unsigned int i=config->start; i<config->count + config->start; i+=config->buffer)
  {
    char* lookup;
    int lookup_len;
    
    unsigned int buffer = config->buffer;
    if (i + buffer > config->count + config->start) {
      buffer = (config->count + config->start - i + 1);
    }

    // printf("reading %d buffer at %u - %u\n", buffer, i, config->start);
    pthread_mutex_lock(&search_mutex);

    if (f_index_lookup(&lookup, index, i, buffer, &lookup_len) != 0)
    {
      printf("lookup failed\n");
    }
    pthread_mutex_unlock(&search_mutex);

    /*
      we have 100 lines from disk, so we need to split them up.
    */
    unsigned int line_number = i;
    char* line;
    char* lookupcpy = lookup;

    while (line = tokenize(&lookupcpy, "\n"))
    {
      line_number++;

      if (strlen(line) == 0)
      {
        free(line);
        config->progress = (double) line_number / (config->start + (config->count - 1));
        continue;
      }

      /*
        match regex against lookup.
      */
      rc = pcre2_match(
        config->regex,        /* the compiled pattern */
        line,               /* the subject string */
        strlen(line),           /* the length of the subject */
        0,                    /* start at offset 0 in the subject */
        0,                    /* default options */
        match_data,           /* block for storing the result */
        NULL
      );

      if (rc < 0 && rc != PCRE2_ERROR_NOMATCH)
      {
        free(line);
        free(lookup);
        pcre2_match_data_free(match_data);
        break;
      }
      else
      {
        if (rc == PCRE2_ERROR_NOMATCH) {
          free(line);
          config->progress = (double) line_number / (config->start + (config->count - 1));
          // printf("progress internal %lf, %u\n", config->progress, line_number);
          continue;
        }

        ovector = pcre2_get_ovector_pointer(match_data);
        f_search_result* res;
        f_search_result_init(&res, rc);
        res->line_number = line_number;
        res->str = line;
        res->matches_len = rc;

        // printf("line match: %u - %s\n", line_number, line);

        for (int m = 0; m < rc; m++)
        {
          res->matches_substring_offset[m] = ovector[2*m];
          res->matches_substring_len[m] = ovector[2*m+1] - ovector[2*m];
          // printf("%d match on %u -> %zu %zu:\n%s", rc, line_number, ovector[2*m], ovector[2*m+1] - ovector[2*m], lookup);
        }
        
        if (config->on_result != NULL)
        {
          pthread_mutex_lock(&search_mutex);
          config->on_result(res, config->result_payload);
          pthread_mutex_unlock(&search_mutex);
        }
        
        config->progress = (double) line_number / (config->start + (config->count - 1));
      }
    }

    free(lookup);
  }

  pcre2_match_data_free(match_data);
  return NULL;
}

int f_search_result_compare(const void* a, const void* b, void* udata)
{
  f_search_result* sa = a;
  f_search_result* sb = b;
  return sa->line_number > sb->line_number;
}

int f_search_compile_term(pcre2_code** re, PCRE2_SPTR pattern)
{
  int error_number;
  PCRE2_SIZE error_offset;
  *re = pcre2_compile(
    pattern,               /* the pattern */
    PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
    0,                     /* default options */
    &error_number,          /* for error number */
    &error_offset,          /* for error offset */
    NULL
  );

  if (re == NULL)
  {
    PCRE2_UCHAR buffer[256];
    pcre2_get_error_message(error_number, buffer, sizeof(buffer));
    printf("PCRE2 compilation failed at offset %d: %s\n", (int)error_offset, buffer);
    return -1;
  }

  return 0;
}

int f_index_search(f_searcher config)
{
  f_index* index = config.index;
  int threads = config.threads;
  int total_lines = index->flookup->len;
  double reported_progress = 0.0f;
  
  pthread_mutex_init(&search_mutex, NULL);

  /*
    Compile PCRE2 Regex to pass to threads.
  */
  pcre2_code* re;
  PCRE2_SPTR pattern = (PCRE2_SPTR) config.regex;
  if (f_search_compile_term(&re, pattern) != 0)
  {
    return -2;
  }

  /*
    Determine how many threads are at play, and the offsets.
  */
  if (threads > total_lines)
  {
    threads = total_lines;
  }

  f_searcher_thread** searcher_threads = malloc(sizeof(*searcher_threads) * threads);
  pthread_t* thread_ids = malloc(sizeof(pthread_t) * threads);

  long int lines_per_thread = (long int) ceil(total_lines / threads);

  for (int i=0; i<threads; i++)
  {
    unsigned long int start_position = i * lines_per_thread;
    
    if (start_position + lines_per_thread > total_lines)
    {
      lines_per_thread = (total_lines - start_position);
    }

    f_searcher_thread* searcher_thread = malloc(sizeof(*searcher_thread));

    /*
      allocate search results buffer.
    */
    searcher_thread->start = start_position;
    searcher_thread->count = lines_per_thread;
    searcher_thread->buffer = config.line_buffer;
    searcher_thread->progress = 0.0f;
    searcher_thread->regex = re;
    searcher_thread->index = index;
    searcher_thread->on_result = config.on_result;
    searcher_thread->result_payload = config.result_payload;
    searcher_threads[i] = searcher_thread;

    pthread_create(&thread_ids[i], NULL, f_index_search_thread, searcher_threads[i]);
  }

  double report = 0.0;
  // join threads.
  for (int i=0; i<threads; i++)
  {
    bool next = false;

    // block i thread join until i thread progress is 1.
    // report progress and any new results in the meantime.
    while (!next)
    {
      next = searcher_threads[i]->progress >= 1;
      double progress = 0.0;

      // if i progresses reached 1, go to next block
      for (int p=0; p<threads; p++)
      {
        progress += (searcher_threads[p]->progress / threads);
      }
      
      if (config.on_progress != NULL)
      {
        report = progress / (double) (threads);
        config.on_progress(report + reported_progress, &config.progress_payload);
      }
    }

    reported_progress += report;

    // join next thread.
    if (pthread_join(thread_ids[i], NULL) != 0)
    {
      perror("can't join pthread");
      exit(1);
    }
    
    free(searcher_threads[i]);
  }

  free(searcher_threads);
  free(thread_ids);
  pcre2_code_free(re);
  pthread_mutex_destroy(&search_mutex);
  return 0;
}

#endif