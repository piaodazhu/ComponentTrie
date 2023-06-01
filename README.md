# ComponentTrie
A simple component trie implementation that enables component-granularity longest prefix matching for strings with delimiters.

It is header-only. Just include it.

## run tests
```sh
gcc component_trie_test.c -o ctest
# run testbench (default N=5000000)
BENCH=1 N=1000000 ./ctest
# run multi-thread test (default M=4)
THEAD=1 M=4 ./ctest
# run normal test
./ctest

# or simpler cpp testfile
g++ cpp_test.cpp -o cpptest
./cpptest
```