
void test_f_log_level_cb(f_log_message msg, void* payload)
{
  bool* ok = (bool*) payload;

  if ((msg.level & F_LOG_FINE | F_LOG_WARN) && (strcmp(msg.message, "odoyle rules!") == 0) && msg.datetime != NULL)
  {
    *ok = true;
  }
  else
  {
    *ok = false;
  }
}

TEST test_f_log_level()
{
  bool* ok = malloc(sizeof(bool));
  *ok = false;
  f_logger_set_level(F_LOG_FINE | F_LOG_WARN);
  f_logger_set_cb(test_f_log_level_cb, (void*) ok);

  f_log(F_LOG_ERROR, "discarded");
  if (*ok) FAIL();
  
  f_log(F_LOG_WARN, "%s rules!", "odoyle");
  if (!*ok) FAIL();

  f_log(F_LOG_FINE, "odoyle rules!");
  if (!*ok) FAIL();

  f_log(F_LOG_INFO, "more discarded");
  if (*ok) FAIL();

  PASS();
}

SUITE(f_log_suite)
{
  RUN_TEST(test_f_log_level);
}