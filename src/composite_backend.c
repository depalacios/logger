#include "composite_backend.h"
#include <stdlib.h>

/*
 * Composite backend context:
 * holds a dynamic list of child backends
 */
typedef struct composite_ctx {
  logger_backend_t **items;
  size_t count;
  size_t capacity;
} composite_ctx_t;

/* ---- vtable methods ---- */

static logger_status_t composite_start(logger_backend_t *self) {
  composite_ctx_t *ctx = (composite_ctx_t *)self->ctx;
  if (!ctx)
    return LOGGER_UNKOWN_ERROR;

  for (size_t i = 0; i < ctx->count; ++i) {
    logger_status_t st = ctx->items[i]->vtbl->start(ctx->items[i]);
    if (st != LOGGER_OK)
      return st;
  }
  return LOGGER_OK;
}

static logger_status_t composite_stop(logger_backend_t *self) {
  composite_ctx_t *ctx = (composite_ctx_t *)self->ctx;
  if (!ctx)
    return LOGGER_UNKOWN_ERROR;

  for (size_t i = 0; i < ctx->count; ++i) {
    logger_status_t st = ctx->items[i]->vtbl->stop(ctx->items[i]);
    if (st != LOGGER_OK)
      return st;
  }
  return LOGGER_OK;
}

static void composite_log(logger_backend_t *self, logger_level_t level,
                          const char *file, int line, const char *msg) {
  composite_ctx_t *ctx = (composite_ctx_t *)self->ctx;
  if (!ctx)
    return;

  for (size_t i = 0; i < ctx->count; ++i) {
    ctx->items[i]->vtbl->log(ctx->items[i], level, file, line, msg);
  }
}

static void composite_destroy(logger_backend_t *self) {
  if (!self)
    return;

  composite_ctx_t *ctx = (composite_ctx_t *)self->ctx;
  if (ctx) {
    for (size_t i = 0; i < ctx->count; ++i) {
      ctx->items[i]->vtbl->destroy(ctx->items[i]);
    }
    free(ctx->items);
    free(ctx);
  }
  free(self);
}

/* ---- vtable instance ---- */

static const logger_backend_vtbl_t COMPOSITE_VTBL = {.start = composite_start,
                                                     .stop = composite_stop,
                                                     .log = composite_log,
                                                     .destroy =
                                                         composite_destroy};

logger_backend_t *logger_backend_composite_create(void) {
  logger_backend_t *backend = (logger_backend_t *)calloc(1, sizeof(*backend));
  if (!backend)
    return NULL;

  composite_ctx_t *ctx = (composite_ctx_t *)calloc(1, sizeof(*ctx));
  if (!ctx) {
    free(backend);
    return NULL;
  }

  backend->vtbl = &COMPOSITE_VTBL;
  backend->ctx = ctx;

  return backend;
}

logger_status_t logger_backend_composite_add(logger_backend_t *composite,
                                             logger_backend_t *child) {
  if (!composite || !child)
    return LOGGER_NO_EXIST;

  composite_ctx_t *ctx = (composite_ctx_t *)composite->ctx;
  if (!ctx)
    return LOGGER_UNKOWN_ERROR;

  if (ctx->count == ctx->capacity) {
    size_t new_capacity = (ctx->capacity == 0) ? 4 : ctx->capacity * 2;
    logger_backend_t **new_items = (logger_backend_t **)realloc(
        ctx->items, new_capacity * sizeof(*new_items));
    if (!new_items)
      return LOGGER_OUT_OF_MEMORY;

    ctx->items = new_items;
    ctx->capacity = new_capacity;
  }

  ctx->items[ctx->count++] = child;
  return LOGGER_OK;
}
