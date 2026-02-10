# Logger (C API) — backend-swappable

A small, reusable logging library with a stable C API and interchangeable backends.
Designed so **executables configure logging once**, while **dependencies only emit logs**
(without knowing which backend is used).

## Features

- Stable **C API** (good for ABI boundaries and mixed C/C++)
- Levels: `TRACE, DEBUG, INFO, WARN, ERROR, FATAL`
- Callsite capture via macros (`__FILE__` / `__LINE__`)
- Backends:
  - **Console** (C)
  - **File** (C)
  - **Tracy** (C wrapper)
  - **Quill** (C++ backend; async; file/console sinks)
- Composite backend (fan-out) for combinations like:
  - `Console + File`
  - `Quill + Tracy` (Quill logs + Tracy profiler)

> Note: Tracy does not print logs to stdout by default. It is visible in the Tracy UI.

---

## Quick start

```c
#include "logger.h"

int main(int argc, char** argv) {
  logger_handle_t* log = logger_init();
  if (!log) return 1;

  // optional outputs (depends on build/backend)
  // logger_enable_file_output(log, "app.log");
  // logger_enable_console_output(log);
  // logger_enable_tracy(log);

  logger_start(log, LOGGER_LEVEL_TRACE);

  LOG_INFO(log,  "Logger started");
  LOG_DEBUG(log, "argc=%d", argc);
  LOG_WARN(log,  "This is a warning");
  LOG_ERROR(log, "This is an error");

  logger_stop(log);
  logger_destroy(log);
  return 0;
}
```

## Build

This repository is intentionally build-system-agnostic.
A typical build script can compile:
- Plain C (Console/File)
- Quill backend (C++)
- Tracy backend (C/C++)

See: docs/BUILDING.md.

## License

(TODO)