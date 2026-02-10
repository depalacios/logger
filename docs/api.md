# Logger API (C)

This document is a reference for the public API exposed by `src/logger.h`.

## Status codes (`logger_status_t`)

Returned by most configuration and lifecycle functions:

- `LOGGER_OK` — success
- `LOGGER_NO_EXIST` — logger handle is NULL / does not exist
- `LOGGER_FILE_IS_ALREADY_OPEN` — reserved (not currently enforced in the C implementation)
- `LOGGER_INVALID_PATH` — file path is NULL or empty
- `LOGGER_OUT_OF_MEMORY` — allocation failed
- `LOGGER_UNABLE_TO_OPEN_FILE` — reserved (file open errors are typically handled by the backend)
- `LOGGER_UNKOWN_ERROR` — unknown/unclassified error

> Note: The current implementation treats file output as “configured” at the logger level.
> The actual file open happens when the file backend starts.

## Log levels (`logger_level_t`)

Ordered from most verbose to least verbose:

- `LOGGER_LEVEL_TRACE`
- `LOGGER_LEVEL_DEBUG`
- `LOGGER_LEVEL_INFO`
- `LOGGER_LEVEL_WARN`
- `LOGGER_LEVEL_ERROR`
- `LOGGER_LEVEL_FATAL`

Filtering rule:
A message is emitted if `(message_level >= configured_level)`.

Example:
- configured `INFO` prints `INFO/WARN/ERROR/FATAL`
- configured `TRACE` prints everything

## Types

### `logger_handle_t`

Opaque logger instance. Create it with `logger_init()` and release it with `logger_destroy()`.

## Lifecycle

### `logger_handle_t* logger_init(void);`

Allocates and initializes a logger instance.

Default configuration:
- level: `LOGGER_LEVEL_INFO`
- started: false
- console output: enabled (default)
- file output: disabled
- tracy: disabled

Returns:
- non-NULL pointer on success
- NULL on allocation failure

### `logger_status_t logger_start(logger_handle_t* logger, logger_level_t level);`

Starts the logger and sets the minimum log level.

Notes:
- The backend graph is (re)built at start.
- After start, `logger_log()` and `LOG_*` macros emit messages according to the configured level.

### `logger_status_t logger_stop(logger_handle_t* logger);`

Stops logging:
- disables emission of new messages
- stops and destroys the current backend (so files/sockets are not left open)

Safe to call multiple times.

### `logger_status_t logger_destroy(logger_handle_t* logger);`

Stops (if needed), releases resources (including file path memory), and frees the logger handle.

---

## Configuration

### `logger_status_t logger_set_level(logger_handle_t* logger, logger_level_t level);`

Sets the minimum emitted log level.

### File output

#### `logger_status_t logger_enable_file_output(logger_handle_t* logger, const char* path);`

Configures file output (path must be non-NULL and non-empty). The path is copied internally.

#### `logger_status_t logger_disable_file_output(logger_handle_t* logger);`

Disables file output. Safe to call even if file output is not enabled.

### Tracy

#### `logger_status_t logger_enable_tracy(logger_handle_t* logger);`
Enables Tracy backend (if compiled in).

#### `logger_status_t logger_disable_tracy(logger_handle_t* logger);`
Disables Tracy backend.

---

## Logging

### `void logger_log(logger_handle_t* logger, logger_level_t level, const char* file, int line, const char* fmt, ...);`

Core logging function (`printf`-style).

A message is dropped if:
- logger is NULL
- logger is not started
- level < configured level
- backend is NULL

The formatted message is forwarded to the active backend(s).

## Convenience macros

These macros capture callsite via `__FILE__` / `__LINE__`:

- `LOG_TRACE(logger, fmt, ...)`
- `LOG_DEBUG(logger, fmt, ...)`
- `LOG_INFO(logger, fmt, ...)`
- `LOG_WARN(logger, fmt, ...)`
- `LOG_ERROR(logger, fmt, ...)`
- `LOG_FATAL(logger, fmt, ...)`

Example:

```c
LOG_INFO(log, "User=%s, id=%d", user, id);
```

## Thread-safety

The current implementation contains TODOs for mutex protection, but mutex usage is not enabled yet.
If you need to log from multiple threads concurrently, consider adding synchronization or enabling
a thread-safe backend (e.g., Quill) and protecting the logger handle configuration calls.