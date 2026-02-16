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
  logger_status_t ret = logger_init();
  if (ret != LOGGER_OK) {
    printf("logger_init failed ret=%s\n", logger_status_to_string(ret));
    return 1;
  }

  /* enable file */
  ret = logger_enable_file_output("app.log");
  if (ret != LOGGER_OK) {
    printf("error=%s\n", logger_status_to_string(ret));
  }

  /* enable tracy (if compiled) */
  // logger_enable_tracy();

  /* level */
  logger_start(LOGGER_LEVEL_TRACE);

  /* ---- How to use ---- */
  LOG_INFO("Logger started");
  LOG_DEBUG("argc=%d", argc);
  LOG_WARN("This is a warning");
  LOG_ERROR("This is an error");

  /* ---- SHUTDOWN ---- */
  logger_stop();
  logger_destroy();
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