#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "types.h"

#if defined __linux__ || defined __APPLE__
#define TERM_COLOR_RESET  "\033[0m"
#define TERM_COLOR_RED    "\033[31m"
#define TERM_COLOR_GREEN  "\033[32m"
#define TERM_COLOR_YELLOW "\033[33m"
#define TERM_COLOR_BLUE   "\033[34m"
#define TERM_COLOR_PURPLE "\033[35m"
#else
#define TERM_COLOR_RESET
#define TERM_COLOR_RED
#define TERM_COLOR_GREEN
#define TERM_COLOR_YELLOW
#define TERM_COLOR_BLUE
#define TERM_COLOR_PURPLE
#endif

#define STR(a) (#a)

void _log(FILE *stream, const char *prefix, const char *fmt, va_list ap);

void LOG(const char *fmt, ...);

void INFO_(const char *fmt, ...);
void INFO(const char *fmt, ...);

#define _DEBUG(...) __DEBUG(__FILE__,__LINE__, __func__, __VA_ARGS__)
void __DEBUG(const char* file, const i32 line, const char* func, const char *fmt, ...);

void WARN(const char *fmt, ...);

void ERROR(const char *fmt, ...);

#endif
