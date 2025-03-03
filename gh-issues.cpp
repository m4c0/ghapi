#pragma leco tool
import jason;
import fetch;
import print;

int main() {
  using namespace jason::ast::nodes;

  auto heap = fetch("/issues?assignee=@me");
  auto json = jason::parse(*heap);
  auto & issues = cast<array>(json);
  if (issues.size() == 0) {
    putln("[No issues]");
    return 0;
  }
  
  for (auto & i : issues) {
    auto & issue = cast<dict>(i);
    auto & title = cast<string>(issue["title"]);
    auto & no = cast<number>(issue["number"]);
    auto & repo = cast<dict>(issue["repository"]);

    auto & fullname = cast<string>(repo["full_name"]);

    putln(fullname.str(), '#', no.integer(), " ", title.str());
  }
}
