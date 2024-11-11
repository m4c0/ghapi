#pragma leco tool
#include <string.h>
#include <time.h>

import jason;
import jute;
import popen;
import print;

int main() try {
  char *args[] {
    strdup("gh"),
    strdup("api"),
    strdup("/search/issues?q=is:pr+author:@me"),
    0
  };
  p::proc proc { args };
  jute::heap heap {};
  while (proc.gets()) {
    auto c = proc.last_line_read();
    heap = heap + jute::view::unsafe(c);
  }

  using namespace jason::ast::nodes;

  auto json = jason::parse(*heap);
  auto & root = cast<dict>(json);
  auto & items = cast<array>(root["items"]);
  auto & first_item = cast<dict>(*items.begin());
  auto & created_at = cast<string>(first_item["created_at"]);

  auto time_cstr = (*created_at.str()).cstr();

  tm t {};
  strptime(time_cstr.begin(), "%FT%T%z", &t);
  auto ts = mktime(&t);
  auto secs = time(nullptr) - ts;
  int days = secs / 86400;
  if (days == 0) return 0;
  if (days > 1) put("😰 ");
  put(days, "d wo PRs");
} catch (...) {
  return 1;
}
