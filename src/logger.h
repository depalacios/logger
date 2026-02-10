/**
 * @file logger.h
 * @brief Public C API for the logger.
 *
 * Typical usage:
 *  1) logger_handle_t* log = logger_init();
 *  2) Optional configuration (file/tracy/etc.)
 *  3) logger_start(log, LOGGER_LEVEL_INFO);
 *  4) LOG_INFO(log, "..."), LOG_ERROR(log, "...")
 *  5) logger_stop(log);
 *  6) logger_destroy(log);
 *
 * Notes:
 * - The logger drops messages if it is not started.
 * - Filtering rule: a message is emitted if (message_level >=
 * configured_level).
 */

#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Status codes returned by logger API functions.
 *
 * Conventions:
 * - LOGGER_OK indicates success.
 * - Any other value indicates an error or an exceptional condition.
 *
 * Notes:
 * - Functions that return logger_status_t never crash on NULL; they return
 *   LOGGER_NO_EXIST when the logger handle is NULL.
 */
typedef enum logger_status {
  LOGGER_OK = 0,               /**< Operation completed successfully. */
  LOGGER_NO_EXIST,             /**< Logger handle is NULL / does not exist. */
  LOGGER_FILE_IS_ALREADY_OPEN, /**< File output is already enabled/open. */
  LOGGER_INVALID_PATH,         /**< Provided file path is NULL or empty. */
  LOGGER_OUT_OF_MEMORY,        /**< Memory allocation failed. */
  LOGGER_UNABLE_TO_OPEN_FILE,  /**< fopen() failed for the provided path. */
  LOGGER_UNKOWN_ERROR, /**< Unknown/unclassified error (avoid if possible). */
} logger_status_t;

/**
 * @brief Log severity levels.
 *
 * Ordering matters: lower values are more verbose.
 *
 * Filtering rule used by logger_log():
 * - A message is emitted if (message_level >= configured_level).
 *
 * Example:
 * - configured = INFO => prints INFO/WARN/ERROR/FATAL
 * - configured = TRACE => prints everything
 */
typedef enum logger_level {
  LOGGER_LEVEL_TRACE = 0, /**< Most verbose. */
  LOGGER_LEVEL_DEBUG,     /**< Debug information. */
  LOGGER_LEVEL_INFO,      /**< General informational messages. */
  LOGGER_LEVEL_WARN,      /**< Warnings that are not fatal. */
  LOGGER_LEVEL_ERROR,     /**< Errors. */
  LOGGER_LEVEL_FATAL      /**< Fatal errors. */
} logger_level_t;

/**
 * @brief Opaque logger handle.
 *
 * The internal structure is private to the library.
 * Consumers must create/destroy it via the provided API.
 */
typedef struct logger_handle logger_handle_t;

// --- Logger API --- //
/**
 * @brief Allocate and initialize a logger instance.
 *
 * Default configuration:
 * - level: LOGGER_LEVEL_INFO
 * - started: false
 * - file output: disabled
 * - tracy: disabled
 *
 * @return Pointer to logger handle on success, NULL on allocation failure.
 */
logger_handle_t *logger_init(void);

/**
 * @brief Set the minimum log level to be emitted.
 *
 * @param logger Logger handle.
 * @param level  Minimum severity to emit.
 * @return LOGGER_OK on success, LOGGER_NO_EXIST if logger is NULL.
 */
logger_status_t logger_set_level(logger_handle_t *logger, logger_level_t level);

/**
 * @brief Start the logger and optionally set the log level.
 *
 * After start, logger_log() and LOG_* macros will emit messages according to
 * the configured level.
 *
 * @param logger Logger handle.
 * @param level  Minimum severity to emit.
 * @return LOGGER_OK on success, LOGGER_NO_EXIST if logger is NULL.
 */
logger_status_t logger_start(logger_handle_t *logger, logger_level_t level);

/**
 * @brief Stop the logger.
 *
 * Behavior:
 * - Disables emission of new log messages (started = false).
 * - Flushes outputs (stdout/stderr and file if enabled).
 *
 * @param logger Logger handle.
 * @return LOGGER_OK on success, LOGGER_NO_EXIST if logger is NULL.
 */
logger_status_t logger_stop(logger_handle_t *logger);

/**
 * @brief Destroy a logger instance and release all associated resources.
 *
 * This closes the log file (if enabled), releases allocated memory, and frees
 * the logger handle itself.
 *
 * @param logger Logger handle.
 * @return LOGGER_OK on success, LOGGER_NO_EXIST if logger is NULL.
 */
logger_status_t logger_destroy(logger_handle_t *logger);

// --- Logger API file config --- //

/**
 * @brief Enable file output and open the log file at the given path.
 *
 * Notes:
 * - The library copies the provided path internally (caller can free/modify
 * theirs).
 * - If file output is already enabled, returns LOGGER_FILE_IS_ALREADY_OPEN.
 * - On success, subsequent logs are also written to the file.
 *
 * @param logger Logger handle.
 * @param path   Path to log file (must be non-NULL and non-empty).
 * @return LOGGER_OK on success, or an error status on failure:
 *         - LOGGER_NO_EXIST
 *         - LOGGER_FILE_IS_ALREADY_OPEN
 *         - LOGGER_INVALID_PATH
 *         - LOGGER_OUT_OF_MEMORY
 *         - LOGGER_UNABLE_TO_OPEN_FILE
 */
logger_status_t logger_enable_file_output(logger_handle_t *logger,
                                          const char *path);

/**
 * @brief Disable file output and close the log file if open.
 *
 * Safe to call even if file output is not enabled.
 *
 * @param logger Logger handle.
 * @return LOGGER_OK on success, LOGGER_NO_EXIST if logger is NULL.
 */
logger_status_t logger_disable_file_output(logger_handle_t *logger);

// --- Logger tracy config --- //
/**
 * @brief Enable Tracy integration.
 *
 *
 * @param logger Logger handle.
 * @return LOGGER_OK on success, LOGGER_NO_EXIST if logger is NULL.
 */
logger_status_t logger_enable_tracy(logger_handle_t *logger);

/**
 * @brief Disable Tracy integration.
 *
 * @param logger Logger handle.
 * @return LOGGER_OK on success, LOGGER_NO_EXIST if logger is NULL.
 */
logger_status_t logger_disable_tracy(logger_handle_t *logger);

// --- Logger --- //
/**
 * @brief Core logging function (printf-style).
 *
 * Filtering:
 * - If logger is NULL, not started, or level < configured level => message is
 * dropped.
 *
 * Output:
 * - Messages with level >= ERROR go to stderr; otherwise stdout.
 * - If file output is enabled, messages are also appended to the configured
 * file.
 *
 * @param logger Logger handle.
 * @param level  Severity of this message.
 * @param file   Source file (usually __FILE__).
 * @param line   Source line (usually __LINE__).
 * @param fmt    printf-style format string.
 * @param ...    Format arguments.
 */
void logger_log(logger_handle_t *logger, logger_level_t level, const char *file,
                int line, const char *fmt, ...);

/**
 * @name Convenience macros
 * @{
 *
 * These macros automatically fill file/line using __FILE__/__LINE__.
 *
 * Note:
 * - Consider wrapping in do { ... } while (0) for safer macro behavior
 *   inside if/else blocks.
 */
#define LOG_TRACE(logger, fmt, ...)                                            \
  logger_log(logger, LOGGER_LEVEL_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_DEBUG(logger, fmt, ...)                                            \
  logger_log(logger, LOGGER_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_INFO(logger, fmt, ...)                                             \
  logger_log(logger, LOGGER_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_WARN(logger, fmt, ...)                                             \
  logger_log(logger, LOGGER_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(logger, fmt, ...)                                            \
  logger_log(logger, LOGGER_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_FATAL(logger, fmt, ...)                                            \
  logger_log(logger, LOGGER_LEVEL_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/** @} */

#ifdef __cplusplus
}
#endif
#endif
