#pragma once

// Defines common macros, logging and assertion
// All nasty macros will be stored here (To contain the damage!)
//
// Options:
//  - DZ_ENABLE_DEBUGBREAK  - enables debug breaking
//  - DZ_ENABLE_LOGS        - Allows the program to log
//  - DZ_ENABLE_ASSERTS     - Allows the program to assert conditions
//
// These are all enabled when debugging is enabled

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _DEBUG
#define DZ_ENABLE_ASSERTS
#define DZ_ENABLE_DEBUGBREAK
#define DZ_ENABLE_LOGS
#endif

// Common util macros
#define max(a, b) (a > b ? a : b)
#define min(a, b) (a > b ? b : a)
#define array_len(a) (sizeof(a) / sizeof(a[0]))

extern bool str_eq(const char *s1, const char *s2, size_t n);

typedef enum DzErrorLevel {
  DzErrorLevel_INFO,
  DzErrorLevel_TRACE,
  DzErrorLevel_WARN,
  DzErrorLevel_ERROR,
} DzErrorLevel;

// Asserts with file, line number, and condition information.
// Optionally takes a message. You can leave it as NULL if you don't
// want one
extern void dz_impl_assert_msg(const char *filename,
                               const int line_number,
                               const char *condition_string,
                               bool condition, const char *msg, ...);

extern void dz_impl_log(FILE *stream, DzErrorLevel error_level,
                        bool show_errno, const char *msg, ...);

// Util Macros
#define DZ_EXPAND_MACRO(x) x
#define DZ_STRINGIFY(x) #x

// Logging
#ifdef DZ_ENABLE_LOGS
#define DZ_TRACE(...) \
  dz_impl_log(stdout, DzErrorLevel_TRACE, false, __VA_ARGS__)
#define DZ_INFO(...) \
  dz_impl_log(stdout, DzErrorLevel_INFO, false, __VA_ARGS__)
#define DZ_WARN(...) \
  dz_impl_log(stdout, DzErrorLevel_WARN, false, __VA_ARGS__)
#define DZ_WARNNO(...) \
  dz_impl_log(stdout, DzErrorLevel_WARN, true, __VA_ARGS__)
#else
#define DZ_TRACE(...)
#define DZ_INFO(...)
#define DZ_WARN(...)
#define DZ_WARNNO(...)
#endif
#define DZ_ERROR(...) \
  dz_impl_log(stderr, DzErrorLevel_ERROR, false, __VA_ARGS__)
#define DZ_ERRORNO(...) \
  dz_impl_log(stderr, DzErrorLevel_ERROR, true, __VA_ARGS__)

#ifdef DZ_ENABLE_DEBUGBREAK
#include <signal.h>
#define DZ_DEBUGBREAK(...) raise(SIGTRAP)
#else
#define DZ_DEBUGBREAK(...)
#endif

// Assertions
#ifdef DZ_ENABLE_ASSERTS
// Asserts a condition.
// Arguments:
// Condition - The condition to assert
// Message (Optional) - A message to display when assertion fails
#define DZ_ASSERT(...) \
  DZ_EXPAND_MACRO(     \
      DZ_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__))
// Internal macro impl
#define DZ_INTERNAL_ASSERT_WITH_MSG(type, check, ...)                \
  dz_impl_assert_msg(__FILE__, __LINE__, DZ_STRINGIFY(check), check, \
                     __VA_ARGS__)
#define DZ_INTERNAL_ASSERT_NO_MSG(type, check)                       \
  dz_impl_assert_msg(__FILE__, __LINE__, DZ_STRINGIFY(check), check, \
                     NULL)
#define DZ_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) \
  macro
#define DZ_INTERNAL_ASSERT_GET_MACRO(...)            \
  DZ_EXPAND_MACRO(DZ_INTERNAL_ASSERT_GET_MACRO_NAME( \
      __VA_ARGS__, DZ_INTERNAL_ASSERT_WITH_MSG,      \
      DZ_INTERNAL_ASSERT_NO_MSG))
#else
#define DZ_ASSERT(...)
#endif
