#ifndef LOG_H
#define LOG_H

#define ANSI_RED    "\033[31m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE   "\033[34m"
#define ANSI_RESET  "\033[0m"

#if !defined RELEASE
#  define info(X)    puts(ANSI_BLUE X ANSI_RESET)
#  define warning(X) puts(ANSI_YELLOW X ANSI_RESET)
#  define failure(X) puts(ANSI_RED X ANSI_RESET)
#else
#  define info(_)
#  define warning(_)
#  define failure(X) puts(X)
#endif

#endif  /* LOG_H */
