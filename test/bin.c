#include "../src/flashlight.c"

void progress_cb(double progress)
{
  printf("Progress (%lf)\n", progress);
}

void on_lookup(char* value)
{
  printf("%s", value);
}

int main(void)
{
  // seed rand so index filenames are random...
  srand(time(0));
  char* test = "test/zfixtures/test.txt";
  char* small = "test/zfixtures/words.txt";
  char* lookupdir = ".flashlight";

  f_indexer i = {
    .filename = small,
    .lookup_dir = lookupdir,
    .threads = 6,
    .concurrency = 50,
    .buffer_size = 50000,
    .on_progress = progress_cb
  };

  f_index* index = f_index_text_file(i);

  size_t start = 0;
  char* hello;
  int size;
  if (f_index_lookup(&hello, index, start, 5, NULL) == -1)
  {
    printf("failure\n");
    return -1;
  };

  printf("%s\n", hello);
  free(hello);

  f_index_free(&index);

  return 0;
}