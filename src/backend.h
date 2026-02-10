#ifndef LOGGER_BACKEND_H
#define LOGGER_BACKEND_H

#include "logger.h"

typedef struct logger_backend logger_backend_t;

typedef struct logger_backend_vtbl {
  logger_status_t (*start)(logger_backend_t *self);
  logger_status_t (*stop)(logger_backend_t *self);
  void (*log)(logger_backend_t *self, logger_level_t level, const char *file,
              int line, const char *msg);
  void (*destroy)(logger_backend_t *self);
} logger_backend_vtbl_t;

struct logger_backend {
  const logger_backend_vtbl_t *vtbl;
  void *ctx;
};

#endif
