// void progress_cb(double progress, void* payload) {
//   // printf("progress: %f\n", progress);
// }

// TEST test_f_posix_single_thread_read(void) {
//   f_index_config config = {
//     .filename = "test/zfixtures/words.txt",
//     .threads = 1,
//     .concurrency = 100,
//     .buffer_size = 2000,
//     .progress_cb = progress_cb,
//     .payload = NULL
//   };

//   f_index* index = f_posix_single_thread_read(config);
//   f_index_free(index);
//   PASS();
// }

// SUITE(f_indexing_suite) {
//   RUN_TEST(test_f_posix_single_thread_read);
// }