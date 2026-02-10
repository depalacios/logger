# Backends

## Console backend (C)
- Writes to stdout/stderr depending on level.

## File backend (C)
- Writes to a configured file path.

## Quill backend (C++)
- Provides async logging via Quill.
- Can create:
  - Console sink
  - File sink
- Level mapping:
  - TRACE -> TraceL3
  - DEBUG -> Debug
  - INFO  -> Info
  - WARN  -> Warning
  - ERROR -> Error
  - FATAL -> Critical

Important:
- To see DEBUG/TRACE, the Quill logger must have its minimum level set to Trace/Debug.
  Example: `logger->set_log_level(quill::LogLevel::TraceL3);`

## Tracy backend
- Used for profiling / live diagnostics.
- Tracy output is not printed; it is consumed by the Tracy UI.
