/**
 * @file backend.h
 * @brief Internal backend interface used by the logger.
 *
 * A backend is a pluggable sink that receives already-formatted log messages.
 * Implementations must provide a vtable with:
 * - start(): allocate/open resources
 * - stop(): flush/close resources (idempotent)
 * - log(): emit a message (msg is already formatted)
 * - destroy(): free resources and the backend object
 *
 * Ownership:
 * - The creator returns a heap-allocated logger_backend_t*.
 * - Whoever owns the backend must call vtbl->destroy().
 */
#ifndef LOGGER_BACKEND_H
#define LOGGER_BACKEND_H

#include "logger.h"

/**
 * @brief Backend instance (opaque context + vtable).
 *
 * @param vtbl Table of function pointers (must not be NULL).
 * @param ctx  Implementation-defined context (may be NULL).
 */
typedef struct logger_backend logger_backend_t;

/**
 * @brief Backend virtual table (vtable).
 *
 * Implementations must provide these callbacks. The logger will call them
 * through the vtable, so all function pointers must be valid (non-NULL)
 * unless explicitly documented otherwise by the implementation.
 *
 * Conventions:
 * - start(): prepare resources (open files, init clients, etc.)
 * - stop(): flush/close resources; should be safe to call multiple times
 * - log(): emit a single already-formatted message
 * - destroy(): free all resources (must tolerate partially-started objects)
 */
typedef struct logger_backend_vtbl {
  /**
   * @brief Starts the backend.
   * @param backend Backend instance.
   * @return 0 on success, non-zero on failure (implementation-defined).
   */
  int (*start)(struct logger_backend *backend);

  /**
   * @brief Stops the backend and releases runtime resources.
   * @param backend Backend instance.
   * @return 0 on success, non-zero on failure (implementation-defined).
   */
  int (*stop)(struct logger_backend *backend);

  /**
   * @brief Emits a log message.
   *
   * The message is already formatted by the logger (printf-style formatting
   * has already been applied). Implementations should not assume @p msg
   * outlives the call.
   *
   * @param backend Backend instance.
   * @param level Log level of the message.
   * @param file Source file where the log was emitted.
   * @param line Source line where the log was emitted.
   * @param msg  Formatted message.
   */
  void (*log)(struct logger_backend *backend, logger_level_t level,
              const char *file, int line, const char *msg);

  /**
   * @brief Destroys the backend and frees all resources.
   * @param backend Backend instance.
   */
  void (*destroy)(struct logger_backend *backend);
} logger_backend_vtbl_t;

/**
 * @brief Backend instance (opaque context + vtable).
 *
 * Ownership:
 * - Backend instances are heap-allocated by their factory functions
 *   (e.g., logger_backend_console_create()).
 * - The owner must call backend->vtbl->destroy(backend) to release it.
 */
typedef struct logger_backend {
  const logger_backend_vtbl_t *vtbl; /**< Backend implementation callbacks. */
  void *ctx;                         /**< Implementation-defined context. */
} logger_backend_t;

#endif
