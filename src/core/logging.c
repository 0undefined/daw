#include <engine/logging.h>
#include <engine/types.h>
#include <stdlib.h>

char* itoa(i32 x) {
  const i32 size = (((i32)ceil(log10((f64)x))) + 1) * sizeof(char);
  char* retval = (char*)malloc(size);
  if (retval == NULL) {
    perror("Failed to allocate memory for itoa");
    exit(EXIT_FAILURE);
  }
  sprintf(retval, "%d", x);
  return retval;
}

void _log(FILE* stream, const char* prefix, const char* fmt, va_list ap) {
  if (stream == NULL) {
    fprintf(stderr, "_log got NULL in stream argument\n");
    exit(EXIT_FAILURE);
  }
  fputs(prefix, stream);
  vfprintf(stream, fmt, ap);
}

void LOG(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  _log(stdout, "[" TERM_COLOR_BLUE "LOG" TERM_COLOR_RESET "] ", fmt, ap);
  va_end(ap);
  puts("");
}

void INFO_(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  _log(stdout, "[" TERM_COLOR_GREEN "INFO" TERM_COLOR_RESET "] ", fmt, ap);
  va_end(ap);
}

void INFO(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  _log(stdout, "[" TERM_COLOR_GREEN "INFO" TERM_COLOR_RESET "] ", fmt, ap);
  va_end(ap);
  puts("");
}

void __DEBUG(const char* file, const i32 line, const char* func,
             const char* fmt, ...) {
  va_list ap;

  const usize prefix_len = 1024;

  char* prefix = malloc(sizeof(char) * 1024);

  snprintf(prefix, prefix_len,
           "[" TERM_COLOR_YELLOW "DEBUG" TERM_COLOR_RESET "] "
           "%s:%d <%s> ",
           file, line, func);

  va_start(ap, fmt);
  _log(stdout, prefix, fmt, ap);
  va_end(ap);

  free(prefix);
}

void WARN(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  _log(stderr, "[" TERM_COLOR_PURPLE "WARN" TERM_COLOR_RESET "] ", fmt, ap);
  va_end(ap);
  puts("");
}

void ERROR(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  _log(stderr, "[" TERM_COLOR_RED "ERROR" TERM_COLOR_RESET "] ", fmt, ap);
  va_end(ap);
  puts("");
}
