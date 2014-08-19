cmake_minimum_required(VERSION 2.8)

project(rmq)

include(CTest)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

add_executable(naive_rmq naive_rmq.cpp)
add_executable(sparse_rmq sparse_rmq.cpp)
add_executable(pm_rmq pm_rmq.cpp)
add_executable(lca lca.cpp)
add_executable(opt_rmq opt_rmq.cpp)

if (BUILD_TESTING)
  add_test(naive_rmq naive_rmq)
  add_test(sparse_rmq sparse_rmq)
  add_test(pm_rmq pm_rmq)
  add_test(lca lca)
  add_test(opt_rmq opt_rmq)
endif (BUILD_TESTING)