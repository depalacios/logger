/**
 * @file quill_backend.h
 * @brief Quill backend (C++), async logging with optional console/file sinks.
 *
 * Build:
 * - Compile this backend only when USE_QUILL is defined.
 *
 * @param file_path If NULL or empty, file sink is disabled.
 * @param enable_console 0/1 to enable console sink.
 *
 * @return NULL if no sinks were enabled or on allocation failure.
 *
 * Ownership:
 * - Returned backend must be destroyed via vtbl->destroy().
 */
#ifndef QUILL_BACKEND_H
#define QUILL_BACKEND_H

#include "backend.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates a Quill backend.
 *
 * This backend uses Quill (C++) to provide asynchronous logging with
 * optional console and file sinks.
 *
 * @param file_path Path to the log file.
 *        If NULL or empty (""), the file sink is not created.
 * @param enable_console Enable console output (stdout).
 *        Use 1 to enable, 0 to disable.
 *
 * @return Pointer to a logger_backend_t instance on success.
 *         Returns NULL if no sink is enabled or if allocation fails.
 *
 * @note This backend is only available when compiled with USE_QUILL enabled.
 */
logger_backend_t *logger_backend_quill_create(const char *file_path,
                                              int enable_console);

#ifdef __cplusplus
}
#endif

#endif
