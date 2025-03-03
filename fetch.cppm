module;
#include <string.h>

export module fetch;
import jute;
import popen;

export jute::heap fetch(char * api);

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
