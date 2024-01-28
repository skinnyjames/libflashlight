# libflashlight

A library to build a newline index on files of an arbitrary size.

libflashlight indexes a file in both multiple threads and concurrently.
It is designed to run on machines with multiple cores.
Once a target file is indexed, abitrary lookups based on line numbers can be made.

Supported operating systems are currently MacOSX and Linux.  Windows support may be added in the future.

## Build from source

This library can be built with [xmake](https://xmake.io)

* Run tests: `xmake build test`
* Run valgrind in Ubuntu container (macosx reports false positives): `docker-compose up`
* Configure for release or debug: `xmake f -m release -k <shared|static>`
* Build lib: `xmake build lib`
* Install lib: `xmake install lib`
* Generate the docs using doxygen
  * `git submodule init`
  * `git submodule update` 
  * `xmake doxygen`

Build artifacts are located in `build/<platform>/<arch>/<mode>`

## Usage

```c
#include <flashlight.h>

void on_progress(double progress, void* payload)
{
  printf("loading: %f...\n", progress);
}

int main(void)
{
  f_indexer config = {
    .filename = "/some/file/to/index",
    .lookup_dir = "dir/to/store/.index"
    .threads = 6,
    .concurrency = 50,
    .buffer_size = 10000,
    .max_bytes_per_iteration = 10000000000,
    .on_progress = progress_cb
    .payload = NULL
  };

  // index the target file
  f_index* index = f_index_text_file(config);

  char* lookup
  // lookup 5 lines starting at line 9 million.
  if (f_index_lookup(&lookup, index, 9000000, 5) == -1)
  {
    printf("failure\n");
    return 1;
  };

  // do something with the lookup
  printf("%s\n", lookup);
  free(lookup);

  // free the index when done
  f_index_free(&index);
  return 0;
}

```
### Searching against an index with regex

Searching is possible using PCRE2 regex.

Search results propogate to a callback when they are matched, and the calling code can 
choose to store these how it wishes.

An example using https://github.com/tidwall/btree.c

```c
#include <flashlight.h>
#include <btree.c>

void search_progress(double progress)
{
  printf("Search progress (%lf)\n", progress);
}

int search_result_compare(const void* a, const void* b, void* udata)
{
  f_search_result* sa = a;
  f_search_result* sb = b;
  return sa->line_number > sb->line_number ? 1 : -1;
}

void append_search_result(f_search_result* res, void* payload)
{
  struct btree* results = payload;
  if (btree_set(results, res) != NULL) exit(1);
}

int main(void)
{
  // seed rand so index filenames are random...
  srand(time(0));

  f_indexer i = {
    .filename = "some/file/to/index.txt",
    .lookup_dir = ".flashlight",
    .threads = 6,
    .concurrency = 50,
    .buffer_size = 50000,
    .max_bytes_per_iteration = 10000000000,
    .on_progress = NULL,
    .payload = NULL
  };

  f_index* index = f_index_text_file(i);

  // Search against index
  struct btree* results = btree_new(sizeof(f_search_result), 0, search_result_compare, NULL);

  f_searcher searcher = {
    .regex = "^car",
    .index = index,
    .threads = 6,
    .line_buffer = 1000,
    .on_progress = search_progress,
    .progress_payload = NULL,
    .on_result = append_search_result,
    .result_payload = results
  };

  if (f_index_search(searcher) != 0)
  {
    printf("search failed\n");
  }

  f_search_result** res;
  while (res = btree_pop_min(results))
  {
    printf("[%zu] - %s\n", res->line_number, res->str);
  }

  btree_free(results);
  f_index_free(&index);

  return 0;
}
```

## Development

When adding new files
* the header file can be added to `genheader`
* run `genheader` to rebuild `flashlight.h`