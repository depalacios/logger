#include "logger.h"

#include "backend.h"
#include "composite_backend.h"
#include "console_backend.h"
#include "file_backend.h"
#include "tracy_backend.h"

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

static logger_backend_t *make_backend(logger_handle_t *l) {
  logger_backend_t *composite = logger_backend_composite_create();
  if (!composite)
    return NULL;

  int added = 0;

#ifdef USE_QUILL
  // If QUILL selected will be the only backend
  logger_backend_t *q = logger_backend_quill_create(
      l->file_enabled ? l->file_path : NULL, l->console_enabled);

  if (!q)
    goto fail;

  logger_backend_composite_add(composite, q);
  added = 1;

  // Tracy can go also
  if (l->tracy_enabled) {
    logger_backend_t *t = logger_backend_tracy_create();
    if (!t)
      goto fail;
    logger_backend_composite_add(composite, t);
  }

  return composite;
#endif

  /* ---- Fallback default logs: backends C ---- */
  if (l->console_enabled) {
    logger_backend_t *c = logger_backend_console_create();
    if (!c)
      goto fail;
    logger_backend_composite_add(composite, c);
    added = 1;
  }

  /* file */
  if (l->file_enabled && l->file_path && l->file_path[0] != '\0') {
    logger_backend_t *f = logger_backend_file_create(l->file_path);
    if (!f)
      goto fail;
    logger_backend_composite_add(composite, f);
    added = 1;
  }

  /* tracy */
  if (l->tracy_enabled) {
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

logger_handle_t *logger_init(void) {
  logger_handle_t *h = (logger_handle_t *)calloc(1, sizeof(*h));
  if (!h)
    return NULL;

  h->level = LOGGER_LEVEL_INFO;
  h->started = 0;

  h->console_enabled = 1; /* default console on */

  h->file_enabled = 0;
  h->file_path = NULL;

  h->tracy_enabled = 0;

  h->backend = NULL;

  /* pthread_mutex_init(&h->mutex, NULL); */

  return h;
}

logger_status_t logger_set_level(logger_handle_t *logger,
                                 logger_level_t level) {
  if (!logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */
  logger->level = level;
  /* pthread_mutex_unlock(&logger->mutex); */

  return LOGGER_OK;
}

logger_status_t logger_start(logger_handle_t *logger, logger_level_t level) {
  if (!logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */

  logger->level = level;

  /* rebuild backend on start */
  if (logger->backend) {
    logger->backend->vtbl->stop(logger->backend);
    logger->backend->vtbl->destroy(logger->backend);
    logger->backend = NULL;
  }

  logger->backend = make_backend(logger);
  if (!logger->backend) {
    /* pthread_mutex_unlock(&logger->mutex); */
    return LOGGER_UNKOWN_ERROR;
  }

  logger_status_t st = logger->backend->vtbl->start(logger->backend);
  if (st != LOGGER_OK) {
    logger->backend->vtbl->destroy(logger->backend);
    logger->backend = NULL;
    /* pthread_mutex_unlock(&logger->mutex); */
    return st;
  }

  logger->started = 1;

  /* pthread_mutex_unlock(&logger->mutex); */
  return LOGGER_OK;
}

logger_status_t logger_stop(logger_handle_t *logger) {
  if (!logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */

  logger->started = 0;

  if (logger->backend) {
    logger_status_t st = logger->backend->vtbl->stop(logger->backend);

    /* IMPORTANTE: destruir aquí para no dejar file/socket abierto si hacen stop
     * sin destroy */
    logger->backend->vtbl->destroy(logger->backend);
    logger->backend = NULL;

    /* pthread_mutex_unlock(&logger->mutex); */
    return st;
  }

  /* pthread_mutex_unlock(&logger->mutex); */
  return LOGGER_OK;
}

logger_status_t logger_destroy(logger_handle_t *logger) {
  if (!logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */

  if (logger->backend) {
    logger->backend->vtbl->stop(logger->backend);
    logger->backend->vtbl->destroy(logger->backend);
    logger->backend = NULL;
  }

  free(logger->file_path);
  logger->file_path = NULL;

  /* pthread_mutex_unlock(&logger->mutex); */
  /* pthread_mutex_destroy(&logger->mutex); */

  free(logger);
  return LOGGER_OK;
}

/* config */
logger_status_t logger_enable_file_output(logger_handle_t *logger,
                                          const char *path) {
  if (!logger)
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

  free(logger->file_path);
  logger->file_path = copy;

  logger->file_enabled = 1;

  /* pthread_mutex_unlock(&logger->mutex); */
  return LOGGER_OK;
}

logger_status_t logger_disable_file_output(logger_handle_t *logger) {
  if (!logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */
  logger->file_enabled = 0;
  /* pthread_mutex_unlock(&logger->mutex); */

  return LOGGER_OK;
}

logger_status_t logger_enable_tracy(logger_handle_t *logger) {
  if (!logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */
  logger->tracy_enabled = 1;
  /* pthread_mutex_unlock(&logger->mutex); */

  return LOGGER_OK;
}

logger_status_t logger_disable_tracy(logger_handle_t *logger) {
  if (!logger)
    return LOGGER_NO_EXIST;

  /* pthread_mutex_lock(&logger->mutex); */
  logger->tracy_enabled = 0;
  /* pthread_mutex_unlock(&logger->mutex); */

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
  if (!logger->backend)
    return;

  char msg[2048];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  /* pthread_mutex_lock(&logger->mutex); */
  if (logger->backend) {
    logger->backend->vtbl->log(logger->backend, level, file, line, msg);
  }
  /* pthread_mutex_unlock(&logger->mutex); */
}
