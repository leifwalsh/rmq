#include <assert.h>
#include <iostream>
#include <string>

#include "tree.hpp"
#include "lca.hpp"
#include "pm_rmq.hpp"

int main(int argc, const char *argv[]) {
  using std::string;
  std::vector<tree<string> > a_children;
  {
    std::vector<tree<string> > b_children;
    b_children.push_back(tree<string>("c"));
    b_children.push_back(tree<string>("d"));
    b_children.push_back(tree<string>("e"));
    a_children.push_back(tree<string>("b", std::move(b_children)));
  }
  {
    std::vector<tree<string> > f_children;
    {
      std::vector<tree<string> > g_children;
      g_children.push_back(tree<string>("h"));
      f_children.push_back(tree<string>("g", std::move(g_children)));
    }
    f_children.push_back(tree<string>("i"));
    a_children.push_back(tree<string>("f", std::move(f_children)));
  }
  tree<string> input("a", std::move(a_children));
  lca<string, pm_rmq<std::vector<ssize_t>::const_iterator>> lca(input);

#define LCA_TEST(u, v, expect) do {                                     \
    string ret = lca.query(u, v);                                       \
    if (expect != ret) {                                                \
      std::cout << "expected LCA(" << u.id() << ", " << v.id() << ") = " << expect << std::endl; \
      std::cout << "got " << ret << std::endl;                          \
    }                                                                   \
    assert(ret == expect);                                              \
  } while (0)

  LCA_TEST(input, input, "a");
  LCA_TEST(input.children()[0], input.children()[1], "a");
  LCA_TEST(input.children()[0].children()[0], input.children()[0].children()[2], "b");
  LCA_TEST(input.children()[1].children()[0].children()[0], input.children()[1].children()[1], "f");

#undef LCA_TEST

  return 0;
}
