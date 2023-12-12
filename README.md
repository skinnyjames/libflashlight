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
  int lookup_size;
  // lookup 5 lines starting at line 9 million.
  if (f_index_lookup(&lookup, index, 9000000, 5, &lookup_size) == -1)
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

## Development

When adding new files
* the header file can be added to `genheader`
* run `genheader` to rebuild `flashlight.h`