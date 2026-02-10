/**
 * @file file_backend.h
 * @brief File backend that appends logs to a file path.
 *
 * Notes:
 * - The backend opens the file in append mode ("a") during create().
 * - Messages are formatted like: [LEVEL] file:line | message
 *
 * Ownership:
 * - Returned backend must be destroyed via vtbl->destroy().
 */
#ifndef FILE_BACKEND_H
#define FILE_BACKEND_H

#include "backend.h"

/**
 * @brief Creates a file backend.
 *
 * This backend appends log messages to a file at the provided path.
 *
 * @param path Output file path. Must be non-NULL and non-empty.
 *
 * @return Pointer to a logger_backend_t instance on success.
 *         Returns NULL if @p path is invalid or if allocation/open fails.
 *
 * @note The file is typically opened in append mode. Exact behavior is
 *       implementation-defined.
 */
logger_backend_t *logger_backend_file_create(const char *path);

#endif
