/**
 * A test for the ±1 RMQ problem.
 */

#pragma once

#include <assert.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <vector>

namespace rmq_test {

  template<typename impl>
  void test() {
#define EXPECT_RMQ(u, v, expect) do {                                   \
      size_t _ret = im.query(&input[(u)], &input[(v)]);                 \
      std::cout << "rmq(" << u << ", " << v << ") = " << _ret << std::endl; \
      if (input[_ret] != input[(expect)]) {                             \
        std::cout << "(expected " << expect << ")" << std::endl;        \
      }                                                                 \
      assert(input[_ret] == input[(expect)]);                           \
    } while (0)

    {
      int input[] = {
        1,
        2,
        1,
        2,
        1,
        0,
      };

      impl im(&input[0], input + ((sizeof input) / (sizeof *input)));

      EXPECT_RMQ(0, 3, 2);
      EXPECT_RMQ(0, 2, 0);
      EXPECT_RMQ(2, 6, 5);
      EXPECT_RMQ(3, 6, 5);
    }
#undef EXPECT_RMQ
  }

  template<typename impl>
  void vector_test() {
    size_t N = 10000;
    std::vector<int> input(N);
    input[0] = 0;
    for (std::vector<int>::iterator it = input.begin() + 1; it != input.end(); ++it) {
      *it = *(it - 1) + ((std::rand() % 2 == 0) ? -1 : 1);
    }
    auto t0 = std::chrono::high_resolution_clock::now();
    impl im(input.begin(), input.end());
    auto t1 = std::chrono::high_resolution_clock::now();

    std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
    std::cout << "built in " << elapsed.count() << "us" << std::endl;

    size_t K = 100;
    for (size_t i = 0; i < N - K; ++i) {
      size_t len = std::max(size_t(1), size_t(std::abs(std::rand() % K)));
      std::vector<int>::const_iterator expected = std::min_element(input.begin() + i, input.begin() + i + len);
      std::vector<int>::difference_type found = im.query(input.begin() + i, input.begin() + i + len);
      if (*expected != input[found]) {
        std::cout << "error: " << *expected << " at " << expected - input.begin() << " != " << input[found] << " at " << found << std::endl;
        std::cout << "array from " << i << " to " << i + len + 1 << ":" << std::endl;
        std::copy(input.begin() + i, input.begin() + i + len + 1,
                  std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
      }
      assert(*expected == input[found]);
    }
  }

}

#define TEST_IMPL(impl)                                                 \
  int main(int argc, const char *argv[]) {                              \
    rmq_test::test<impl<int *>>();                                      \
    rmq_test::vector_test<impl<std::vector<int>::const_iterator>>();    \
    return 0;                                                           \
  }
