#include "core.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Terminal Colours
static const char* KNRM = "\x1B[0m";
static const char* KRED = "\x1B[31m";
static const char* KGRN = "\x1B[32m";
static const char* KYEL = "\x1B[33m";
static const char* KBLU = "\x1B[34m";
static const char* KMAG = "\x1B[35m";
static const char* KCYN = "\x1B[36m";
static const char* KWHT = "\x1B[37m";

static const char* get_error_level_color(
    const DzErrorLevel error_level) {
  switch (error_level) {
    case DzErrorLevel_TRACE:
      return KWHT;
    case DzErrorLevel_INFO:
      return KGRN;
    case DzErrorLevel_ERROR:
      return KRED;
    case DzErrorLevel_WARN:
      return KYEL;
  }
}

static const char* get_error_level_string(
    const DzErrorLevel error_level) {
  switch (error_level) {
    case DzErrorLevel_TRACE:
      return "Trace";
    case DzErrorLevel_WARN:
      return "Warn";
    case DzErrorLevel_ERROR:
      return "ERROR";
    case DzErrorLevel_INFO:
      return "Info";
  }
}

static void get_formatted_time(char* buffer,
                               const ssize_t buffer_size) {
  time_t rawtime;
  time(&rawtime);
  struct tm* timeinfo = localtime(&rawtime);
  strftime(buffer, buffer_size, "%x - %H:%M:%S", timeinfo);
}

static const char* strrstr(const char* string, const char* find) {
  const ssize_t find_len = strlen(find);
  const ssize_t string_len = strlen(string);
  for (const char* cp = string + string_len - find_len; cp >= string;
       cp--) {
    if (strncmp(cp, find, find_len) == 0) return cp;
  }
  return NULL;
}

bool str_eq(const char* s1, const char* s2, size_t n) {
  return strncmp(s1, s2, n) == 0;
}

void dz_impl_assert_msg(const char* filename, const int line_number,
                        const char* condition_string, bool condition,
                        const char* msg, ...) {
  if (condition) {
    return;
  }
  va_list args;
  va_start(args, msg);
  const char* relative_file = strrstr(filename, "src");
  const char* relative_file_name =
      (relative_file) ? relative_file : filename;
  static char timebuffer[100];
  get_formatted_time(timebuffer, sizeof(timebuffer));
  fprintf(
      stderr,
      "[%s] %s[Assert Error]%s: Assertion \"%s\" at ./%s:%d failed. ",
      timebuffer, KRED, KNRM, condition_string, relative_file_name,
      line_number);
  if (msg) {
    fprintf(stderr, msg, args);
    fprintf(stderr, "\n");
  } else {
    fprintf(stderr, "\n");
  }
  va_end(args);
  DZ_DEBUGBREAK();
}

void dz_impl_log(FILE* stream, const DzErrorLevel error_level,
                 const bool show_errno, const char* msg, ...) {
  static char timebuffer[100];
  get_formatted_time(timebuffer, sizeof(timebuffer));
  if (!show_errno) {
    fprintf(stream, "[%s] %s[%s]%s: ", timebuffer,
            get_error_level_color(error_level),
            get_error_level_string(error_level), KNRM);
  } else {
    fprintf(stream, "[%s] %s[%s, Errno %d]%s: ", timebuffer,
            get_error_level_color(error_level),
            get_error_level_string(error_level), errno, KNRM);
  }
  va_list args;
  va_start(args, msg);
  vfprintf(stream, msg, args);
  fprintf(stream, "\n");
  va_end(args);
}
