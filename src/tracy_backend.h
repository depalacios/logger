/**
 * @file tracy_backend.h
 * @brief Tracy backend (sends log messages to Tracy UI).
 *
 * Requirements:
 * - TRACY_ENABLE must be defined for messages to be sent.
 * - Your application must compile/link TracyClient.cpp (as described in docs).
 *
 * Behavior:
 * - Messages are visible in Tracy UI (not printed to stdout by this backend).
 *
 * Ownership:
 * - Returned backend must be destroyed via vtbl->destroy().
 */
#ifndef TRACY_BACKEND_H
#define TRACY_BACKEND_H

#include "backend.h"

/**
 * @brief Creates a Tracy backend.
 *
 * This backend forwards log messages to the Tracy profiler UI.
 * Messages sent through this backend are not printed to stdout or stderr.
 *
 * @return Pointer to a logger_backend_t instance on success.
 *         Returns NULL if allocation fails or if Tracy support is not
 * available.
 *
 * @note This backend is only functional when compiled with TRACY_ENABLE
 *       and when the application links TracyClient.cpp.
 */
logger_backend_t *logger_backend_tracy_create(void);

#endif
