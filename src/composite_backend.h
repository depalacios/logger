#ifndef COMPOSITE_BACKEND_H
#define COMPOSITE_BACKEND_H

#include "backend.h"

logger_backend_t *logger_backend_composite_create(void);

logger_status_t logger_backend_composite_add(logger_backend_t *composite,
                                             logger_backend_t *child);

#endif
