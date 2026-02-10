#ifndef QUILL_BACKEND_H
#define QUILL_BACKEND_H

#include "backend.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Crea un backend Quill.
 * - file_path: si es NULL o "", no crea file handler
 * - enable_console: 0/1 para stdout handler
 *
 * Devuelve NULL si no hay ningún sink habilitado.
 */
logger_backend_t *logger_backend_quill_create(const char *file_path,
                                              int enable_console);

#ifdef __cplusplus
}
#endif

#endif
