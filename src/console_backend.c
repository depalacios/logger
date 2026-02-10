#include "console_backend.h"
#include <stdio.h>
#include <stdlib.h>

static const char *lvl_to_str(logger_level_t lvl) {
  switch (lvl) {
  case LOGGER_LEVEL_TRACE:
    return "TRACE";
  case LOGGER_LEVEL_DEBUG:
    return "DEBUG";
  case LOGGER_LEVEL_INFO:
    return "INFO";
  case LOGGER_LEVEL_WARN:
    return "WARN";
  case LOGGER_LEVEL_ERROR:
    return "ERROR";
  case LOGGER_LEVEL_FATAL:
    return "FATAL";
  default:
    return "UNKNOWN";
  }
}

static logger_status_t c_start(logger_backend_t *self) {
  (void)self;
  return LOGGER_OK;
}
static logger_status_t c_stop(logger_backend_t *self) {
  (void)self;
  return LOGGER_OK;
}

static void c_log(logger_backend_t *self, logger_level_t lvl, const char *file,
                  int line, const char *msg) {
  (void)self;
  FILE *out = (lvl >= LOGGER_LEVEL_ERROR) ? stderr : stdout;
  fprintf(out, "[%s] %s:%d | %s\n", lvl_to_str(lvl), file, line, msg);
}

static void c_destroy(logger_backend_t *self) { free(self); }

static const logger_backend_vtbl_t V = {
    .start = c_start, .stop = c_stop, .log = c_log, .destroy = c_destroy};

logger_backend_t *logger_backend_console_create(void) {
  logger_backend_t *b = (logger_backend_t *)calloc(1, sizeof(*b));
  if (!b)
    return NULL;
  b->vtbl = &V;
  b->ctx = NULL;
  return b;
}
