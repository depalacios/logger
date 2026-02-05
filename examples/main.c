#include <logger.h>

int main() {
  logger_handle_t *log = logger_init();

  //   logger_enable_file_output(log, "path_to_file");
  logger_start(log, LOGGER_LEVEL_TRACE);

  LOG_INFO(log, "HI %s", "David");
  LOG_DEBUG(log, "Value=%d", 123);

  logger_stop(log);
  logger_destroy(log);
  return 0;
}