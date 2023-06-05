# ComponentTrie
A simple component trie implementation that enables component-granularity longest prefix matching and exact match for strings with delimiters.

The same as [piaodazhu/ttlHashMap](https://github.com/piaodazhu/ttlHashMap), this stuff uses the hashmap provided by [tidwall/hashmap.c](https://github.com/tidwall/hashmap.c).

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

## APIs

```c
// structure defination of component trie
typedef struct ct_trie {
	ct_node_t *root;
	char delimiter[MAX_DELIMITER_LEN];
	int delimiter_len;
	uint32_t version;
	pthread_rwlock_t lock;
} ct_trie_t;

// some macro defination
#define MAX_COMPONENT_LEN 16
#define MAX_DELIMITER_LEN 4

// new a trie with a specific delimiter. such as: '/', ': ', ', ', '::'
ct_trie_t* ct_trie_new(const char *delimiter, int delimiter_len);

// free a trie. not thread safe.
void ct_trie_free(ct_trie_t *t);

// insert a kv pair item (prefix, data)
int ct_trie_insert(ct_trie_t* t, const char *prefix, int prefix_len, void *udata);

// longest prefix match a prefix and get the corresponding data with udata.
int ct_trie_lpm(ct_trie_t* t, const char *name, int name_len, void **udata);

// exact prefix match a prefix and get the corresponding data with udata.
int ct_trie_em(ct_trie_t* t, const char *name, int name_len, void **udata);

// remove a kv pair and the the removed data with udata.
int ct_trie_remove(ct_trie_t* t, const char *prefix, int prefix_len, void **udata);
```

## License
ComponentTrie source code is available under the MIT License.