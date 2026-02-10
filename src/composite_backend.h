/**
 * @file composite_backend.h
 * @brief Composite backend that fans out logs to multiple child backends.
 *
 * Use cases:
 * - Console + File
 * - Quill + Tracy
 *
 * Ownership:
 * - After calling logger_backend_composite_add(composite, child),
 *   the composite takes ownership of `child` and will destroy it.
 */
#ifndef COMPOSITE_BACKEND_H
#define COMPOSITE_BACKEND_H

#include "backend.h"

/**
 * @brief Creates an empty composite backend.
 *
 * A composite backend forwards each log call to multiple child backends.
 *
 * @return Pointer to a logger_backend_t instance on success, NULL on failure.
 */
logger_backend_t *logger_backend_composite_create(void);

/**
 * @brief Adds a child backend to the composite.
 *
 * Ownership:
 * - On success, the composite takes ownership of @p child and will destroy it.
 * - The caller must not use or destroy @p child after a successful add.
 *
 * @param composite Composite backend instance.
 * @param child Child backend instance to be owned by the composite.
 *
 * @return LOGGER_OK on success, or a logger_status_t error code.
 */
logger_status_t logger_backend_composite_add(logger_backend_t *composite,
                                             logger_backend_t *child);

#endif
