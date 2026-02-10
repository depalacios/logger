#include "tracy_backend.h"
#include <stdlib.h>
#include <string.h>

#ifdef TRACY_ENABLE
#include "tracy/TracyC.h"
#endif

typedef struct tracy_ctx {
  int enabled;
} tracy_ctx_t;

static logger_status_t t_start(logger_backend_t *self) {
  (void)self;
  return LOGGER_OK;
}

static logger_status_t t_stop(logger_backend_t *self) {
  (void)self;
  return LOGGER_OK;
}

static void t_log(logger_backend_t *self, logger_level_t level,
                  const char *file, int line, const char *msg) {
  (void)level;
  (void)file;
  (void)line;

  tracy_ctx_t *ctx = (tracy_ctx_t *)self->ctx;
  if (!ctx || !ctx->enabled)
    return;

#ifdef TRACY_ENABLE
  /* Enviar mensaje a Tracy (live) */
  TracyCMessage(msg, (uint16_t)strlen(msg));
#else
  (void)msg;
#endif
}

static void t_destroy(logger_backend_t *self) {
  if (!self)
    return;
  free(self->ctx);
  free(self);
}

static const logger_backend_vtbl_t V = {
    .start = t_start, .stop = t_stop, .log = t_log, .destroy = t_destroy};

logger_backend_t *logger_backend_tracy_create(void) {
  logger_backend_t *b = (logger_backend_t *)calloc(1, sizeof(*b));
  if (!b)
    return NULL;

  tracy_ctx_t *ctx = (tracy_ctx_t *)calloc(1, sizeof(*ctx));
  if (!ctx) {
    free(b);
    return NULL;
  }

  ctx->enabled = 1;

  b->vtbl = &V;
  b->ctx = ctx;
  return b;
}
