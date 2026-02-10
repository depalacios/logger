#include "quill_backend.h"

#include <cstdlib>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <vector>

/* Prevents collision with logger.h macros */
#define QUILL_DISABLE_NON_PREFIXED_MACROS

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/core/PatternFormatterOptions.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"

struct quill_ctx {
  quill::Logger *logger;
};

/* Start Quill backend thread once */
static std::once_flag g_quill_once;

static void ensure_quill_started() {
  std::call_once(g_quill_once, []() { quill::Backend::start(); });
}

/* --- vtable methods --- */
static logger_status_t quill_start(logger_backend_t *) { return LOGGER_OK; }

static logger_status_t quill_stop(logger_backend_t *self) {
  auto *ctx = static_cast<quill_ctx *>(self->ctx);
  if (ctx && ctx->logger) {
    ctx->logger->flush_log(); // Quill v11: flush per logger
  }
  return LOGGER_OK;
}

static void quill_log(logger_backend_t *self, logger_level_t level,
                      const char *file, int line, const char *msg) {
  auto *ctx = static_cast<quill_ctx *>(self->ctx);
  if (!ctx || !ctx->logger)
    return;

  switch (level) {
  case LOGGER_LEVEL_TRACE:
    QUILL_LOG_TRACE_L3(ctx->logger, "[QUILL] {}:{} | {}", file, line, msg);
    break;
  case LOGGER_LEVEL_DEBUG:
    QUILL_LOG_DEBUG(ctx->logger, "[QUILL] {}:{} | {}", file, line, msg);
    break;
  case LOGGER_LEVEL_INFO:
    QUILL_LOG_INFO(ctx->logger, "[QUILL] {}:{} | {}", file, line, msg);
    break;
  case LOGGER_LEVEL_WARN:
    QUILL_LOG_WARNING(ctx->logger, "[QUILL] {}:{} | {}", file, line, msg);
    break;
  case LOGGER_LEVEL_ERROR:
    QUILL_LOG_ERROR(ctx->logger, "[QUILL] {}:{} | {}", file, line, msg);
    break;
  case LOGGER_LEVEL_FATAL:
    QUILL_LOG_CRITICAL(ctx->logger, "[QUILL] {}:{} | {}", file, line, msg);
    break;
  default:
    QUILL_LOG_INFO(ctx->logger, "[QUILL] {}:{} | {}", file, line, msg);
    break;
  }
}

static void quill_destroy(logger_backend_t *self) {
  if (!self)
    return;

  auto *ctx = static_cast<quill_ctx *>(self->ctx);
  if (ctx) {
    if (ctx->logger) {
      ctx->logger->flush_log();
    }
    delete ctx;
  }

  std::free(self);
}

static const logger_backend_vtbl_t QUILL_VTBL = {.start = quill_start,
                                                 .stop = quill_stop,
                                                 .log = quill_log,
                                                 .destroy = quill_destroy};

extern "C" logger_backend_t *logger_backend_quill_create(const char *file_path,
                                                         int enable_console) {
  ensure_quill_started();

  std::vector<std::shared_ptr<quill::Sink>> sinks;

  if (enable_console) {
    sinks.push_back(quill::Frontend::create_or_get_sink<quill::ConsoleSink>(
        "console_sink"));
  }

  if (file_path && file_path[0] != '\0') {
    quill::FileSinkConfig cfg;
    cfg.set_open_mode('a');

    sinks.push_back(quill::Frontend::create_or_get_sink<quill::FileSink>(
        file_path, cfg, quill::FileEventNotifier{}));
  }

  if (sinks.empty())
    return nullptr;

  quill::PatternFormatterOptions logger_pfo;
  logger_pfo.format_pattern = "%(message)";

  quill::Logger *logger =
      quill::Frontend::create_or_get_logger("backend", sinks, logger_pfo);

  logger->set_log_level(quill::LogLevel::TraceL3);

  auto *ctx = new (std::nothrow) quill_ctx{logger};
  if (!ctx)
    return nullptr;

  auto *b =
      static_cast<logger_backend_t *>(std::calloc(1, sizeof(logger_backend_t)));
  if (!b) {
    delete ctx;
    return nullptr;
  }

  b->vtbl = &QUILL_VTBL;
  b->ctx = ctx;
  return b;
}
