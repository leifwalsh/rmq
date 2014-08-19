/**
 * A test for the RMQ problem.
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
      if (input[_ret] != input[(expect)]) {                             \
        std::cout << "rmq(" << u << ", " << v << ") = " << _ret << std::endl; \
        std::cout << "(expected " << expect << ")" << std::endl;        \
      }                                                                 \
      assert(input[_ret] == input[(expect)]);                           \
    } while (0)

    if (0) {
      int input[] = {
        1,
      };

      impl im(&input[0], input + ((sizeof input) / (sizeof *input)));

      EXPECT_RMQ(0, 1, 0);
    }

    {
      int input[] = {
        1,
        1,
        1,
        1,
        1,
        1,
      };

      impl im(&input[0], input + ((sizeof input) / (sizeof *input)));

      EXPECT_RMQ(0, 3, 2);
      EXPECT_RMQ(0, 2, 1);
      EXPECT_RMQ(2, 6, 5);
      EXPECT_RMQ(3, 6, 5);
    }

    {
      int input[] = {
        3,
        1,
        2,
        1,
        4,
        5,
      };

      impl im(&input[0], input + ((sizeof input) / (sizeof *input)));

      EXPECT_RMQ(0, 3, 1);
      EXPECT_RMQ(0, 2, 1);
      EXPECT_RMQ(2, 6, 3);
      EXPECT_RMQ(3, 6, 3);
    }

    {
      int input[] = {
        3,
        1,
        1,
        1,
        4,
        5,
      };

      impl im(&input[0], input + ((sizeof input) / (sizeof *input)));

      EXPECT_RMQ(0, 3, 2);
    }

    {
      int input[] = {
        10,
        8,
        9,
        2,
        4,
        5,
        1,
        16,
        4,
        7,
      };

      impl im(&input[0], input + ((sizeof input) / (sizeof *input)));

      EXPECT_RMQ(0, 3, 1);
      EXPECT_RMQ(0, 6, 3);
      EXPECT_RMQ(3, 8, 6);
      EXPECT_RMQ(0, 10, 6);
    }
#undef EXPECT_RMQ
  }

  template<typename impl>
  void vector_test() {
    // Lower this number if you want to run naive_rmq.
    size_t N = 1000000;
    std::vector<int> input(N);
    for (std::vector<int>::iterator it = input.begin(); it != input.end(); ++it) {
      *it = std::rand() % 1000;
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
        std::cout << "array from " << i << " to " << i + len << ":" << std::endl;
        std::copy(input.begin() + i, input.begin() + i + len,
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
