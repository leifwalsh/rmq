rmq
===

Implementations of LCA and RMQ data structures from
[_The LCA Problem Revisited_](http://www.ics.uci.edu/~eppstein/261/BenFar-LCA-00.pdf).

All implementations take some care to work with generic types, and the
code is therefore a bit hard to read, sorry about that.

Generally, iterator types should be `RandomAccessIterators`, difference
types should be signed (they'll be subtracted from one another and
compared) and value types should have sensible comparison and equality
operators, and for the `pm_rmq` structure they need a reasonable
`operator-()` so we can normalize sub blocks.  The values will also be
copied a few times, so avoid huge values if you can.

naive_rmq
---------

Implements the `<O(n^2), O(1)>` naive RMQ algorithm, where the answers to
every possible RMQ query are computed and stored ahead of time.

Warning: the naive implementation uses `O(n^2)` memory and therefore
before running it you should really lower `N` in `rmq_test.hpp` to
something pretty small (10000 works on an 8GB, 64-bit machine), or you'll
run out of memory very quickly.

sparse_rmq
----------

Implements the `<O(n * log(n)), O(1)>` "Sparse Tree" RMQ algorithm, which
improves on the naive algorithm by only storing answers to queries of
sizes that are powers of 2.

pm_rmq
------

Implements the `<O(n), O(1)>` algorithm for RMQ problems that satisfy the
"±1 constraint" that all consecutive elements differ by exactly +1 or -1.

This could use some optimization work.  We currently use a `std::map` and
a `std::vector` where we should really use a statically allocated table
with some fancy bit twiddling.

lca
---

Implements the `<O(n), O(1)>` algorithm for solving the LCA problem by
reduction to the ±1 RMQ problem and using `pm_rmq`.

opt_rmq
-------

Implements the `<O(n), O(1)>` algorithm for general RMQ by constructing
the Cartesian tree of the input to convert it to an LCA problem, which is
then solved in `<O(n), O(1)>`.
