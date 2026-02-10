#include "file_backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct file_ctx {
  FILE *f;
  char *path; /* owned */
} file_ctx_t;

static logger_status_t f_start(logger_backend_t *self) {
  (void)self;
  return LOGGER_OK;
}

static logger_status_t f_stop(logger_backend_t *self) {
  file_ctx_t *c = (file_ctx_t *)self->ctx;
  if (c && c->f)
    fflush(c->f);
  return LOGGER_OK;
}

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

static void f_log(logger_backend_t *self, logger_level_t lvl, const char *file,
                  int line, const char *msg) {
  file_ctx_t *c = (file_ctx_t *)self->ctx;
  if (!c || !c->f)
    return;
  fprintf(c->f, "[%s] %s:%d | %s\n", lvl_to_str(lvl), file, line, msg);
  fflush(c->f); /* luego optimizas */
}

static void f_destroy(logger_backend_t *self) {
  file_ctx_t *c = (file_ctx_t *)self->ctx;
  if (c) {
    if (c->f)
      fclose(c->f);
    free(c->path);
    free(c);
  }
  free(self);
}

static const logger_backend_vtbl_t V = {
    .start = f_start, .stop = f_stop, .log = f_log, .destroy = f_destroy};

logger_backend_t *logger_backend_file_create(const char *path) {
  if (!path || !path[0])
    return NULL;

  logger_backend_t *b = (logger_backend_t *)calloc(1, sizeof(*b));
  if (!b)
    return NULL;

  file_ctx_t *c = (file_ctx_t *)calloc(1, sizeof(*c));
  if (!c) {
    free(b);
    return NULL;
  }

  c->path = (char *)malloc(strlen(path) + 1);
  if (!c->path) {
    free(c);
    free(b);
    return NULL;
  }
  strcpy(c->path, path);

  c->f = fopen(c->path, "a");
  if (!c->f) {
    free(c->path);
    free(c);
    free(b);
    return NULL;
  }

  b->vtbl = &V;
  b->ctx = c;
  return b;
}
