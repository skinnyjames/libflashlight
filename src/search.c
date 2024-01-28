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
  f_search_results* head = *results;

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

  if (token_length < 0)
  {
    *iptr = NULL;
    return NULL;
  }

  token = malloc(sizeof(char) * (token_length + 1));
  if (token == NULL)
  {
    f_log(F_LOG_ERROR, "can't allocate token");
    return NULL;
  }
  token[token_length] = '\0';
  memcpy(token, str, token_length);

  *iptr = found ? found + strlen(delim) : NULL;
  return token;
}

void* f_index_search_thread(void* payload)
{
  f_searcher_thread* config = payload;
  f_index* index = config->index;
  
  pcre2_match_data* match_data;
  match_data = pcre2_match_data_create_from_pattern(config->regex, NULL);
  if (match_data == NULL)
  {
    f_log(F_LOG_ERROR, "cant allocate matchdata block");
    pthread_exit(NULL);
  }

  PCRE2_SIZE* ovector;  
  int rc;

  for (size_t i=config->start; i<config->count + config->start; i+=config->buffer)
  {
    if (*config->result_count >= config->result_limit) 
    {
      f_log(F_LOG_INFO, "met result limit");
      config->progress = (double) 1.0f;
      pcre2_match_data_free(match_data);
      pthread_exit(NULL);
    }

    char* lookup;
    size_t buffer = config->buffer;
    if (i + buffer > config->count + config->start)
    {
      buffer = (config->count + config->start - i - 1);
    }

    if (f_index_lookup(&lookup, index, i, buffer) != 0)
    {
      f_log(F_LOG_ERROR, "lookup failed to start: %zu buffer: %zu", i, buffer); 
      config->progress = (double) 1.0f;
      pthread_exit(NULL); 
    }

    if (lookup == NULL)
    {
      f_log(F_LOG_WARN, "lookup is NULL");
      config->progress = (double) 1.0f;
      pthread_exit(NULL);
    }

    /*
      we have 100 lines from disk, so we need to split them up.
    */
    size_t line_number = i;
    char* line;
    char* lookupcpy = lookup;
    double p = 0.0f;

    while ((line = tokenize(&lookupcpy, "\n")))
    {
      p++;
      line_number++;

      if (strlen(line) == 0)
      {
        f_log(F_LOG_DEBUG, "line is len of 0"); 
        free(line);
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
        f_log(F_LOG_ERROR, "bad pcre2 rc %d", rc);
        free(line);
        free(lookup);
        pcre2_match_data_free(match_data);
        config->progress = (double) 1.0f;
        pthread_exit(NULL);
      }
      else
      {
        if (rc == PCRE2_ERROR_NOMATCH) {
          free(line);
          // check result count on no match.
          if (*config->result_count >= config->result_limit) 
          {
            f_log(F_LOG_INFO, "met result limit");
            config->progress = (double) 1.0f;
            free(lookup);
            pcre2_match_data_free(match_data);

            pthread_exit(NULL);
          }

          continue;
        }

        ovector = pcre2_get_ovector_pointer(match_data);
        f_search_result* res;
        if (f_search_result_init(&res, rc) == -1)
        {
          // TODO free allocations.
          f_log(F_LOG_ERROR, "cant init search result");
          pthread_exit(NULL);
        }
        
        res->line_number = line_number;
        res->str = line;
        res->matches_len = rc;

        for (int m = 0; m < rc; m++)
        {
          res->matches_substring_offset[m] = ovector[2*m];
          res->matches_substring_len[m] = ovector[2*m+1] - ovector[2*m];
        }
        
        if (config->on_result != NULL)
        {
          f_log(F_LOG_DEBUG, "locking for result cb");

          pthread_mutex_lock(&search_mutex);
          if (*config->result_count >= config->result_limit) 
          {
            f_log(F_LOG_INFO, "met result limit");
            config->progress = (double) 1.0f;
            f_search_result_free(res);
            free(lookup);
            pcre2_match_data_free(match_data);
            pthread_mutex_unlock(&search_mutex);

            pthread_exit(NULL);
          }

          *config->result_count += 1;
          f_log(F_LOG_DEBUG, "calling on result"); 
          config->on_result(res, config->result_payload);
          f_log(F_LOG_DEBUG, "on result finished");
          pthread_mutex_unlock(&search_mutex);
        }
        else
        {
          // free if there isn't a supplied callback
          f_search_result_free(res);
        }
      }
    }

    free(lookup);
    config->progress = (double) (i - config->start) / (config->count);
  }
  config->progress = (double) 1.0f;
  f_log(F_LOG_DEBUG, "returning from thread");
  pcre2_match_data_free(match_data);
  pthread_exit(NULL);
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

  if (re == NULL || error_number != 100)
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
  
  if(pthread_mutex_init(&search_mutex, NULL) != 0)
  {
    f_log(F_LOG_ERROR, "cant init mutex");
    return -1;
  }

  /*
    Compile PCRE2 Regex to pass to threads.
  */
  pcre2_code* re;
  PCRE2_SPTR pattern = (PCRE2_SPTR) config.regex;
  if (f_search_compile_term(&re, pattern) != 0)
  {
    f_log(F_LOG_WARN, "Supplied regex is invalid");
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
  if (searcher_threads == NULL)
  {
    f_log(F_LOG_ERROR, "cant allocate searcher threads");
    return -1;
  }

  pthread_t* thread_ids = malloc(sizeof(pthread_t) * threads);
  if (thread_ids == NULL)
  {
    f_log(F_LOG_ERROR, "cant allocate thread ids");
    return -1;
  }

  long int lines_per_thread = (long int) ceil(total_lines / threads);
  int* result_count = malloc(sizeof(*result_count));
  if (result_count == NULL)
  {
    f_log(F_LOG_ERROR, "failed to allocate int for result count");
    return -1;
  }
  *result_count = 0;

  for (int i=0; i<threads; i++)
  {
    size_t start_position = i * lines_per_thread;
    
    if (start_position + lines_per_thread > total_lines)
    {
      lines_per_thread = (total_lines - start_position);
    }

    f_searcher_thread* searcher_thread = malloc(sizeof(*searcher_thread));
    if (searcher_thread == NULL)
    {
      f_log(F_LOG_ERROR, "failed to allocate searcher thread");
      return -1;
    }

    /*
      allocate search results buffer.
    */
    searcher_thread->thread = i;
    searcher_thread->start = start_position;
    searcher_thread->count = lines_per_thread;
    searcher_thread->buffer = config.line_buffer;
    searcher_thread->progress = 0.0f;
    searcher_thread->regex = re;
    searcher_thread->index = index;
    searcher_thread->on_result = config.on_result;
    searcher_thread->result_payload = config.result_payload;
    searcher_thread->result_limit = config.result_limit;
    searcher_thread->result_count = result_count;
    searcher_threads[i] = searcher_thread;

    if (pthread_create(&thread_ids[i], NULL, f_index_search_thread, searcher_threads[i]) != 0)
    {
      // kill already created threads.
      for (int ti=0; ti<i; ti++)
      {
        pthread_kill(thread_ids[ti], SIGINT);
      }
      f_log(F_LOG_ERROR, "Couldn't create thread %d", i);
      return NULL;
    }
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
      // if i progresses reached 1, go to next block
      next = searcher_threads[i]->progress >= 0.999;
      double progress = 0.0f;

      for (int p=0; p<threads; p++)
      {
        progress += (searcher_threads[p]->progress / (double) threads);
      }

      if (config.on_progress != NULL)
      {
        config.on_progress(progress, config.progress_payload);
      }
    }

    // join next thread.
    if (pthread_join(thread_ids[i], NULL) != 0)
    {
      perror("can't join searcher thread");
      f_log(F_LOG_WARN, "Can't joint thread %d", i);
    }
  }

  for (int i=0; i<threads; i++)
  {
    free(searcher_threads[i]);
  }

  free(searcher_threads);
  free(thread_ids);
  free(result_count);
  pcre2_code_free(re);
  pthread_mutex_destroy(&search_mutex);
  return 0;
}

#endif