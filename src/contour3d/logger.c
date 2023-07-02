#include <stdio.h>
#include <stdarg.h>
#include "./logger.h"

int logger_error (
    const char * format,
    ...
) {
  FILE * stream = stderr;
  va_list args = {0};
  va_start(args, format);
  fprintf(stream, "[CONTOUR3D ERROR] ");
  vfprintf(stream, format, args);
  fprintf(stream, "\n");
  fflush(stream);
  va_end(args);
  return 0;
}

