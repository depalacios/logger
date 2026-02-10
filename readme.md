# Logger (C API) — backend-swappable

A small, reusable logging library with a **stable C API** and **interchangeable backends**.
Designed so **executables configure logging once**, while **dependencies only emit logs**
(without knowing which backend is used).

## Features

- Stable **C API** (good for ABI boundaries and mixed C/C++)
- Levels: `TRACE, DEBUG, INFO, WARN, ERROR, FATAL`
- Callsite capture via macros (`__FILE__` / `__LINE__`)
- Backends:
  - **Console** (C) — enabled by default
  - **File** (C)
  - **Tracy** (C wrapper; shows messages in Tracy UI)
  - **Quill** (C++ backend; async; console/file sinks)
- Composite backend (fan-out) for combinations like:
  - `Console + File`
  - `Quill + Tracy` (Quill logs + Tracy profiler)

> Note: Tracy does not print logs to stdout by default. It is visible in the Tracy UI.

---

## Quick start

```c
#include <logger.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  logger_handle_t *log = logger_init();
  if (!log) {
    printf("logger_init failed\n");
    return 1;
  }

  // Optional: file output
  // logger_enable_file_output(log, "app.log");

  // Optional: tracy output (if compiled in)
  logger_enable_tracy(log);

  // Start logger with minimum level
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

## Building

This repo is intentionally build-system-agnostic.

Build modes
- C only: Console/File
- C + Tracy: Tracy backend compiled in
- C++ + Quill: Quill backend compiled in
- C++ + Quill + Tracy: both compiled; runtime composite can fan-out

Compile flags
- -DUSE_QUILL enables the Quill backend translation unit(s)
- -DTRACY_ENABLE enables Tracy compilation/instrumentation (and you must compile TracyClient.cpp into your binary)

See: docs/building.md.

## API

See: docs/api.md.

## License

(TODO).