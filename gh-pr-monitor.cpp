#pragma leco tool
#include <stdio.h>
#include <string.h>
#include <time.h>

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

int main() try {
  using namespace jason::ast::nodes;

  auto heap = fetch_latest_prs();
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
  put('[', days, "d wo PRs]");

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
    adds += cast<number>(root["additions"]).integer();
    dels += cast<number>(root["deletions"]).integer();
  }
  put("[repos: ", repo_count, "][+", adds, '-', dels);
  adds /= items.size();
  dels /= items.size();
  put(" avg: +", adds, "-", dels, "]");
} catch (...) {
  return 1;
}
