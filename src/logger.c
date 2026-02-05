#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct logger_handle {
  logger_level_t level;    /* Minimum level to emit */
  int started;             /* 0 = drop logs, 1 = emit logs */
  int tracy_enabled;       /* Future: Tracy backend toggle */
  int file_output_enabled; /* 1 if file is open and enabled */
  char *file_path;         /* Owned copy of path (malloc/free) */
  FILE *file;              /* Open file handle when enabled */
};

// --- LOCAL FUNCTIONS --- //
static const char *level_to_string(logger_level_t lvl) {
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

// --- PUBLIC FUNCTIONS --- //
logger_handle_t *logger_init(void) {
  logger_handle_t *h = (logger_handle_t *)calloc(1, sizeof(logger_handle_t));
  if (!h)
    return NULL;

  h->level = LOGGER_LEVEL_INFO;
  h->started = 0;
  h->tracy_enabled = 0;
  h->file_output_enabled = 0;
  h->file = NULL;
  h->file_path = NULL;

  return h;
}

logger_status_t logger_set_level(logger_handle_t *logger,
                                 logger_level_t level) {
  if (!logger)
    return LOGGER_NO_EXIST;

  logger->level = level;
  return LOGGER_OK;
}

logger_status_t logger_start(logger_handle_t *logger, logger_level_t level) {
  if (!logger)
    return LOGGER_NO_EXIST;

  logger->level = level;
  logger->started = 1;
  return LOGGER_OK;
}

logger_status_t logger_stop(logger_handle_t *logger) {
  if (!logger)
    return LOGGER_NO_EXIST;

  logger->started = 0;

  if (logger->file) {
    fflush(logger->file);
  }

  fflush(stdout);
  fflush(stderr);
  return LOGGER_OK;
}

logger_status_t logger_destroy(logger_handle_t *logger) {
  if (!logger)
    return LOGGER_NO_EXIST;

  if (logger->file) {
    fclose(logger->file);
    logger->file = NULL;
  }

  free(logger->file_path);
  logger->file_path = NULL;

  free(logger);
  return LOGGER_OK;
}

logger_status_t logger_enable_file_output(logger_handle_t *logger,
                                          const char *path) {
  if (!logger)
    return LOGGER_NO_EXIST;

  if (logger->file_output_enabled)
    return LOGGER_FILE_IS_ALREADY_OPEN;

  if (!path || path[0] == '\0') {
    free(logger->file_path);
    logger->file_path = NULL;
    return LOGGER_INVALID_PATH;
  }

  char *copy = (char *)malloc(strlen(path) + 1);
  if (!copy)
    return LOGGER_OUT_OF_MEMORY;

  strcpy(copy, path);
  free(logger->file_path);

  logger->file_path = copy;
  logger->file = fopen(logger->file_path, "a");
  if (!logger->file) {
    logger->file_output_enabled = 0;
    return LOGGER_UNABLE_TO_OPEN_FILE;
  }

  logger->file_output_enabled = 1;
  return LOGGER_OK;
}

logger_status_t logger_disable_file_output(logger_handle_t *logger) {
  if (!logger)
    return LOGGER_NO_EXIST;

  if (logger->file_output_enabled) {
    if (logger->file) {
      fclose(logger->file);
      logger->file = NULL;
    }
    logger->file_output_enabled = 0;
  }

  return LOGGER_OK;
}

logger_status_t logger_enable_tracy(logger_handle_t *logger) {
  if (!logger)
    return LOGGER_NO_EXIST;

  logger->tracy_enabled = 1;
  return LOGGER_OK;
}

logger_status_t logger_disable_tracy(logger_handle_t *logger) {
  if (!logger)
    return LOGGER_NO_EXIST;

  logger->tracy_enabled = 0;
  return LOGGER_OK;
}

void logger_log(logger_handle_t *logger, logger_level_t level, const char *file,
                int line, const char *fmt, ...) {
  if (!logger)
    return;
  if (!logger->started)
    return;
  if (level < logger->level)
    return;

  /* Formateo del mensaje */
  char msg[2048];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  /* stdout/stderr */
  FILE *out = (level >= LOGGER_LEVEL_ERROR) ? stderr : stdout;
  fprintf(out, "[%s] %s:%d | %s\n", level_to_string(level), file, line, msg);

  /* archivo opcional */
  if (logger->file_output_enabled && logger->file) {
    fprintf(logger->file, "[%s] %s:%d | %s\n", level_to_string(level), file,
            line, msg);
    fflush(logger->file);
  }

  /* tracy_enabled: aquí en C puro no hacemos nada.
     Si luego haces bridge a Tracy, aquí llamarías a tracy_log_c(...) */
}
