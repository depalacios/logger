#include <logger.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  logger_handle_t *log = logger_init();
  if (!log) {
    printf("logger_init failed\n");
    return 1;
  }

  /* enable file */
  // logger_enable_file_output(log, "app.log");

  /* enable tracy (if compiled) */
  // logger_enable_tracy(log);

  /* level */
  logger_start(log, LOGGER_LEVEL_TRACE);

  /* ---- How to use ---- */
  LOG_INFO(log, "Logger started");
  LOG_DEBUG(log, "argc=%d", argc);
  LOG_WARN(log, "This is a warning");
  LOG_ERROR(log, "This is an error");

  /* ---- SHUTDOWN ---- */
  logger_stop(log);
  logger_destroy(log);
  return 0;
}
