/**
 * @file console_backend.h
 * @brief Console backend (stdout/stderr).
 *
 * Behavior:
 * - Levels < ERROR go to stdout
 * - Levels >= ERROR go to stderr
 *
 * Ownership:
 * - Returned backend must be destroyed via vtbl->destroy().
 */
#ifndef CONSOLE_BACKEND_H
#define CONSOLE_BACKEND_H

#include "backend.h"

/**
 * @brief Creates a console backend.
 *
 * This backend writes log messages to the process console.
 * Common behavior is to print low-severity messages to stdout and
 * high-severity messages to stderr (implementation-defined).
 *
 * @return Pointer to a logger_backend_t instance on success.
 *         Returns NULL if allocation fails.
 */
logger_backend_t *logger_backend_console_create(void);

#endif