#pragma leco tool
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

import hashley;
import jason;
import jute;
import popen;
import print;

static auto fetch(char * api) {
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

static auto fetch_latest_prs() {
  tm t {};
  auto limit = time(nullptr) - 86400 * 7;
  gmtime_r(&limit, &t);

  char cs[128] {};
  strftime(cs, sizeof(cs), "%F", &t);
  char search[1024] {};
  snprintf(search, sizeof(search),
           "/search/issues?q=is:pr+author:@me+created:>%s&per_page=50", cs);
  return fetch(search);
}

static auto put_pad(jason::ast::nodes::string s, int n) {
  constexpr jute::view pad = "                                                                  ";
  auto padded = s.str() + pad;
  put((*padded).subview(n).before);
}

int main() try {
  using namespace jason::ast::nodes;

  bool verbose = isatty(STDOUT_FILENO);

  auto heap = fetch_latest_prs();
  auto json = jason::parse(*heap);
  auto & root = cast<dict>(json);
  auto & items = cast<array>(root["items"]);
  if (items.size() == 0) {
    putln("[No PRs in 7 days]");
    return 0;
  }
  auto & first_item = cast<dict>(*items.begin());
  auto & created_at = cast<string>(first_item["created_at"]);

  auto time_cstr = (*created_at.str()).cstr();

  tm t {};
  strptime(time_cstr.begin(), "%FT%T%z", &t);
  auto ts = mktime(&t);
  auto secs = time(nullptr) - ts;
  int days = secs / 86400;
  if (days > 0) put('[', days, "d wo PRs]");
  put('[', items.size(), " PRs]");
  if (verbose) putln();

  hashley::niamh repos { 31 }; 
  auto repo_count = 0;
  int adds {};
  int dels {};
  for (auto & i : items) {
    auto & item = cast<dict>(i);

    auto & repo = cast<string>(item["repository_url"]);
    auto & r = repos[*repo.str()];
    if (!r) {
      repo_count++;
      r = 1;
    }

    auto & pr = cast<dict>(item["pull_request"]);
    auto url = (*cast<string>(pr["url"]).str()).cstr();
    auto heap = fetch(url.begin());
    auto json = jason::parse(*heap);
    auto & root = cast<dict>(json);
    auto a = cast<number>(root["additions"]).integer();
    auto d = cast<number>(root["deletions"]).integer();

    adds += a;
    dels += d;

    if (verbose) {
      auto & title = cast<string>(root["title"]);
      auto & created_at = cast<string>(root["created_at"]);

      auto & head = cast<dict>(root["head"]);
      auto & repo = cast<dict>(head["repo"]);
      auto & name = cast<string>(repo["name"]);

      put("-- \e[38;5;238m");
      put(created_at.str());
      put(" \e[0m");
      put_pad(name, 25);
      putf(" \e[0;32m+%3d \e[0;31m-%3d \e[0m", a, d);
      if (!cast<boolean>(root["merged"])) put("\e[0;33m[unmerged]\e[0m ");
      putln(title.str());
    }
  }
  put("[repos: ", repo_count, "][+", adds, '-', dels);
  adds /= items.size();
  dels /= items.size();
  put(" avg: +", adds, "-", dels, "]");
  if (verbose) putln();
} catch (...) {
  return 1;
}
