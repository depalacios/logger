# Logger

Simple, flexible and backend-swappable logging library with a **C API**, designed to be reused across multiple projects while keeping configuration centralized in the executable.

This logger is intended to be linked as a library and used by dependency repositories without exposing internal configuration or backend details.

---

## Features

- C API (ABI-stable, portable)
- Log levels: TRACE, DEBUG, INFO, WARN, ERROR, FATAL
- Macros with `__FILE__` / `__LINE__` for source tracing
- Configurable outputs:
  - Console (stdout / stderr)
  - File output
- (pending) Backend-based internal architecture (swappable)
- (pending) Designed to integrate with:
  - **Quill** (async file/console logging – C++)
  - **Tracy** (live logging & profiling – C/C++)
- (pending) Clear separation between:
  - **Executable repositories** (configure the logger)
  - **Dependency repositories** (only emit logs)

---

## Project Structure

```text
logger/
├─ src/
│  ├─ logger.c                  # Public API implementation
│  ├─ logger.h 
├─ examples/
└─ README.md
