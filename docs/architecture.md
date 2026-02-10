# Architecture

## Public API (C)
- `logger.h` is the stable interface for consumers.
- Consumers call `logger_init()`, configure outputs, then `logger_start()`.
- Log emission uses macros `LOG_*()` that capture `__FILE__` / `__LINE__`.

## Backend interface
Internally, the logger uses:

- `logger_backend_t`
- `logger_backend_vtbl_t` (start/stop/log/destroy)

Backends are created by factory functions (e.g. console/file/quill/tracy).

## Backend selection
`make_backend()` chooses how to wire outputs:
- Without Quill: typically `Console` and/or `File`, optionally `Tracy`
- With Quill: Quill becomes the log backend, optionally `Tracy` added to composite

## Composite backend
Composite aggregates multiple backends (fan-out). This enables combinations like:
- Console + File
- Quill + Tracy
