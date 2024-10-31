#pragma leco tool
#include <string.h>

import jason;
import jute;
import popen;
import print;

int main() {
  char *args[4] {
    strdup("gh"),
    strdup("api"),
    strdup("/notifications"),
  };
  p::proc proc { args };
  jute::heap heap {};
  while (proc.gets()) {
    auto c = proc.last_line_read();
    heap = heap + jute::view::unsafe(c);
  }

  using namespace jason::ast::nodes;

  auto list = jason::parse(*heap);
  for (auto &n : cast<array>(list)) {
    auto &nd = cast<dict>(n);
    auto &subject = cast<dict>(nd["subject"]);
    auto reason = cast<string>(nd["reason"]).str();

    put("\e[38;5;235m", cast<string>(nd["id"]).str(), "\e[39m ");

    if (*reason == "review_requested") put("\e[38;5;124m");
    if (*reason == "team_mention") put("\e[38;5;93m");
    if (*reason == "author") put("\e[38;5;20m");
    if (*reason == "comment") put("\e[38;5;22m");
    if (*reason == "assign") put("\e[38;5;25m");
    put(reason);
    putf("%*s\e[39m", static_cast<int>(20 - reason.size()), "");

    put(cast<string>(subject["title"]).str());
    putln();
  }
}
