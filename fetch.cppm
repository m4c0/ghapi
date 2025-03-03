module;
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

export module fetch;
import jute;
import popen;

export jute::heap fetch(char * api);
export __attribute__((format(printf, 1, 2))) auto fetch(const char * fmt, ...);

module :private;

jute::heap fetch(char * api) {
  char *args[] {
    strdup("gh"),
    strdup("api"),
    api,
    0
  };
  p::proc proc { args };
  jute::heap heap {};
  while (proc.gets()) {
    auto c = proc.last_line_read();
    heap = heap + jute::view::unsafe(c);
  }
  return heap;
}

auto fetch(const char * fmt, ...) {
  char buf[10240] {};

  va_list arg;
  va_start(arg, fmt);
  vsnprintf(buf, sizeof(buf), fmt, arg);
  va_end(arg);

  return fetch(buf);
}
