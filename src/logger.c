#include "logger.h"

#include "backend.h"
#include "composite_backend.h"
#include "console_backend.h"
#include "file_backend.h"
#include "tracy_backend.h"

// #define USE_QUILL

#ifdef USE_QUILL
#include "quill_backend.h"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Todo, implement mutex
#include <pthread.h>
*/

/**
 * @internal
 * @brief String lookup table for logger_status_t values.
 *
 * This table maps each logger_status_t enumeration value to its textual
 * representation. It is used internally by logger_status_to_string() to
 * provide human-readable error messages.
 *
 * The table is indexed directly by the enum value, therefore the order of
 * elements must match the logger_status_t definition exactly.
 *
 * This symbol is private to the module and must not be accessed directly
 * by library users.
 */
static const char *logger_status_str[LOGGER_STATUS_COUNT] = {
    [LOGGER_OK] = "LOGGER_OK",
    [LOGGER_NO_EXIST] = "LOGGER_NO_EXIST",
    [LOGGER_FILE_IS_ALREADY_OPEN] = "LOGGER_FILE_IS_ALREADY_OPEN",
    [LOGGER_INVALID_PATH] = "LOGGER_INVALID_PATH",
    [LOGGER_OUT_OF_MEMORY] = "LOGGER_OUT_OF_MEMORY",
    [LOGGER_UNABLE_TO_OPEN_FILE] = "LOGGER_UNABLE_TO_OPEN_FILE",
    [LOGGER_UNKOWN_ERROR] = "LOGGER_UNKOWN_ERROR"};

logger_handle_t *base_logger = {0};

struct logger_handle {
  logger_level_t level;
  int started;

  int console_enabled;

  int file_enabled;
  char *file_path;

  int tracy_enabled;

  logger_backend_t *backend;

  /* pthread_mutex_t mutex; */
};

static logger_backend_t *make_backend() {
  logger_backend_t *composite = logger_backend_composite_create();
  if (!composite)
    return NULL;

  int added = 0;

#ifdef USE_QUILL
  // If QUILL selected will be the only backend
  logger_backend_t *q = logger_backend_quill_create(
      base_logger->file_enabled ? base_logger->file_path : NULL,
      base_logger->console_enabled);

  if (!q)
    goto fail;

  logger_backend_composite_add(composite, q);
  added = 1;

  // Tracy can go also
  if (base_logger->tracy_enabled) {
    logger_backend_t *t = logger_backend_tracy_create();
    if (!t)
      goto fail;
    logger_backend_composite_add(composite, t);
  }

  return composite;
#endif

  /* ---- Fallback default logs: backends C ---- */
  if (base_logger->console_enabled) {
    logger_backend_t *c = logger_backend_console_create();
    if (!c)
      goto fail;
    logger_backend_composite_add(composite, c);
    added = 1;
  }

  /* file */
  if (base_logger->file_enabled && base_logger->file_path &&
      base_logger->file_path[0] != '\0') {
    logger_backend_t *f = logger_backend_file_create(base_logger->file_path);
    if (!f)
      goto fail;
    logger_backend_composite_add(composite, f);
    added = 1;
  }

  /* tracy */
  if (base_logger->tracy_enabled) {
    logger_backend_t *t = logger_backend_tracy_create();
    if (!t)
      goto fail;
    logger_backend_composite_add(composite, t);
    added = 1;
  }

  if (!added) {
    composite->vtbl->destroy(composite);
    return NULL;
  }

  return composite;

fail:
  composite->vtbl->destroy(composite);
  return NULL;
}

logger_status_t logger_init(void) {
  logger_handle_t *h = (logger_handle_t *)calloc(1, sizeof(*h));
  if (!h)
    return LOGGER_OUT_OF_MEMORY;

  h->level = LOGGER_LEVEL_INFO;
  h->started = 0;

  h->console_enabled = 1; /* default console on */

  h->file_enabled = 0;
  h->file_path = NULL;

  h->tracy_enabled = 0;

  h->backend = NULL;

  /* pthread_mutex_init(&h->mutex, NULL); */
  base_logger = h;

  return LOGGER_OK;
}

logger_status_t logger_set_level(logger_level_t level) {
  if (!base_logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */
  base_logger->level = level;
  /* pthread_mutex_unlock(&logger->mutex); */

  return LOGGER_OK;
}

logger_status_t logger_start(logger_level_t level) {
  if (!base_logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */

  base_logger->level = level;

  /* rebuild backend on start */
  if (base_logger->backend) {
    base_logger->backend->vtbl->stop(base_logger->backend);
    base_logger->backend->vtbl->destroy(base_logger->backend);
    base_logger->backend = NULL;
  }

  base_logger->backend = make_backend();
  if (!base_logger->backend) {
    /* pthread_mutex_unlock(&logger->mutex); */
    return LOGGER_UNKOWN_ERROR;
  }

  logger_status_t st = base_logger->backend->vtbl->start(base_logger->backend);
  if (st != LOGGER_OK) {
    base_logger->backend->vtbl->destroy(base_logger->backend);
    base_logger->backend = NULL;
    /* pthread_mutex_unlock(&logger->mutex); */
    return st;
  }

  base_logger->started = 1;

  /* pthread_mutex_unlock(&logger->mutex); */
  return LOGGER_OK;
}

logger_status_t logger_stop(void) {
  if (!base_logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */

  base_logger->started = 0;

  if (base_logger->backend) {
    logger_status_t st = base_logger->backend->vtbl->stop(base_logger->backend);

    /* IMPORTANTE: destruir aquí para no dejar file/socket abierto si hacen stop
     * sin destroy */
    base_logger->backend->vtbl->destroy(base_logger->backend);
    base_logger->backend = NULL;

    /* pthread_mutex_unlock(&logger->mutex); */
    return st;
  }

  /* pthread_mutex_unlock(&logger->mutex); */
  return LOGGER_OK;
}

logger_status_t logger_destroy(void) {
  if (!base_logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */

  if (base_logger->backend) {
    base_logger->backend->vtbl->stop(base_logger->backend);
    base_logger->backend->vtbl->destroy(base_logger->backend);
    base_logger->backend = NULL;
  }

  free(base_logger->file_path);
  base_logger->file_path = NULL;

  /* pthread_mutex_unlock(&logger->mutex); */
  /* pthread_mutex_destroy(&logger->mutex); */

  free(base_logger);
  return LOGGER_OK;
}

/* config */
logger_status_t logger_enable_file_output(const char *path) {
  if (!base_logger)
    return LOGGER_NO_EXIST;
  if (!path || !path[0])
    return LOGGER_INVALID_PATH;

  /* pthread_mutex_lock(&logger->mutex); */

  char *copy = (char *)malloc(strlen(path) + 1);
  if (!copy) {
    /* pthread_mutex_unlock(&logger->mutex); */
    return LOGGER_OUT_OF_MEMORY;
  }

  strcpy(copy, path);
  free(base_logger->file_path);

  base_logger->file_path = copy;
  base_logger->file_enabled = 1;

  /* pthread_mutex_unlock(&logger->mutex); */
  return LOGGER_OK;
}

logger_status_t logger_disable_file_output() {
  if (!base_logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */
  base_logger->file_enabled = 0;
  /* pthread_mutex_unlock(&logger->mutex); */

  return LOGGER_OK;
}

logger_status_t logger_enable_tracy() {
  if (!base_logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */
  base_logger->tracy_enabled = 1;
  /* pthread_mutex_unlock(&logger->mutex); */

  return LOGGER_OK;
}

logger_status_t logger_disable_tracy() {
  if (!base_logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */
  base_logger->tracy_enabled = 0;
  /* pthread_mutex_unlock(&logger->mutex); */

  return LOGGER_OK;
}

void logger_log(logger_level_t level, const char *file, int line,
                const char *fmt, ...) {
  if (!base_logger)
    return;
  if (!base_logger->started)
    return;
  if (!base_logger->backend)
    return;
  if (level < base_logger->level)
    return;

  char msg[2048];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  /* pthread_mutex_lock(&logger->mutex); */
  if (base_logger->backend) {
    base_logger->backend->vtbl->log(base_logger->backend, level, file, line,
                                    msg);
  }
  /* pthread_mutex_unlock(&logger->mutex); */
}

const char *logger_status_to_string(logger_status_t status) {
  if (status >= 0 && status < LOGGER_STATUS_COUNT && logger_status_str[status])
    return logger_status_str[status];

  return "LOGGER_STATUS_INVALID";
}