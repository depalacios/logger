#include <logger.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  logger_status_t ret = logger_init();
  if (ret != LOGGER_OK) {
    printf("logger_init failed ret=%s\n", logger_status_to_string(ret));
    return 1;
  }

  /* enable file */
  ret = logger_enable_file_output("app.log");
  if (ret != LOGGER_OK) {
    printf("error=%s\n", logger_status_to_string(ret));
  }

  /* enable tracy (if compiled) */
  // logger_enable_tracy();

  /* level */
  logger_start(LOGGER_LEVEL_TRACE);

  /* ---- How to use ---- */
  LOG_INFO("Logger started");
  LOG_DEBUG("argc=%d", argc);
  LOG_WARN("This is a warning");
  LOG_ERROR("This is an error");

  /* ---- SHUTDOWN ---- */
  logger_stop();
  logger_destroy();
  return 0;
}
