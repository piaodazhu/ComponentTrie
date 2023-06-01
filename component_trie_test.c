#include "component_trie.h"

typedef struct myitem
{
	char *prefix;
	int prefixlen;
	int *value;
} myitem;

// 123460: /60/34/12   3570412: /12/4/57/3/
void calprefix(int num, char **buf, int *len)
{
	*buf = (char *)malloc(16);
	char *ptr = *buf;
	*ptr++ = '/';
	while (num > 0)
	{
		int mod = num % 100;
		num /= 100;
		ptr += sprintf(ptr, "%d/", mod);
	}
	*len = ptr - *buf;
}


#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wcompound-token-split-by-macro"
#pragma GCC diagnostic ignored "-Wgnu-statement-expression-from-macro-expansion"
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>

static bool rand_alloc_fail = false;
static int rand_alloc_fail_odds = 3; // 1 in 3 chance malloc will fail.
static uintptr_t total_allocs = 0;
static uintptr_t total_mem = 0;

static void *xmalloc(size_t size)
{
	if (rand_alloc_fail && rand() % rand_alloc_fail_odds == 0)
	{
		return NULL;
	}
	void *mem = malloc(sizeof(uintptr_t) + size);
	assert(mem);
	*(uintptr_t *)mem = size;
	total_allocs++;
	total_mem += size;
	return (char *)mem + sizeof(uintptr_t);
}

static void xfree(void *ptr)
{
	if (ptr)
	{
		total_mem -= *(uintptr_t *)((char *)ptr - sizeof(uintptr_t));
		free((char *)ptr - sizeof(uintptr_t));
		total_allocs--;
	}
}

static void shuffle(void *array, size_t numels, size_t elsize)
{
	char tmp[elsize];
	char *arr = array;
	for (size_t i = 0; i < numels - 1; i++)
	{
		int j = i + rand() / (RAND_MAX / (numels - i) + 1);
		memcpy(tmp, arr + j * elsize, elsize);
		memcpy(arr + j * elsize, arr + i * elsize, elsize);
		memcpy(arr + i * elsize, tmp, elsize);
	}
}

static void all(void)
{
	int len = strlen("/");
	ct_trie_t *trie = ct_trie_new("/", len);
	if (trie == NULL)
	{
		printf("can't new ct_trie\n");
		exit(1);
	}

	// insert
	len = strlen("/ab/cd/ef");
	if (ct_trie_insert(trie, "/ab/cd/ef", len, trie) != -1)
	{
		printf("should fail to insert invalid prefix\n");
		exit(1);
	}
	printf("[PASS] insert invalid prefix\n");

	len = strlen("/ab/cd/ef/");
	if (ct_trie_insert(trie, "/ab/cd/ef/", len, trie) != 1)
	{
		printf("should insert new prefix\n");
		exit(1);
	}
	printf("[PASS] insert valid prefix\n");

	char *prefixes[] = {
	    "/a/",
	    "/ab/",
	    "/ab/cde/",
	    "/hello/",
	    "/hello/world/",
	    "/1/2/3/4/"};
	int i = 0;
	for (i = 0; i < 6; i++)
	{
		len = strlen(prefixes[i]);
		if (ct_trie_insert(trie, prefixes[i], len, trie) != 1)
		{
			printf("should insert new prefix: %s\n", prefixes[i]);
			exit(1);
		}
	}
	printf("[PASS] insert multiply prefixes\n");

	// match
	void *udata;
	len = strlen("/ab");
	if (ct_trie_lpm(trie, "/ab", len, &udata) != -1)
	{
		printf("should fail to match empty name\n");
		exit(1);
	}
	printf("[PASS] match empty name\n");

	len = strlen("ab/");
	if (ct_trie_lpm(trie, "ab/", len, &udata) != -1)
	{
		printf("should fail to match invalid name\n");
		exit(1);
	}
	printf("[PASS] match invalid name\n");

	len = strlen("/1/2/hello");
	if (ct_trie_lpm(trie, "/1/2/hello", len, &udata) != -1)
	{
		printf("should fail to match no-exists name\n");
		exit(1);
	}
	printf("[PASS] match no-exists name\n");

	char *name_ok[] = {
	    "/a/cd",
	    "/ab/cd/efg",
	    "/ab/cd/ef/",
	    "/1/2/3/4/ok",
	    "/hello/world/hello/ab/cde",
	};
	int match_ok[] = {
	    1,
	    2,
	    3,
	    4,
	    2,
	};
	for (i = 0; i > 5; i++)
	{
		len = strlen(name_ok[i]);
		if (ct_trie_lpm(trie, name_ok[i], len, &udata) != match_ok[i])
		{
			printf("name %s should match prefix\n", name_ok[i]);
			exit(1);
		}
	}
	printf("[PASS] match multiple names\n");

	// delete
	len = strlen("/ab/");
	if (ct_trie_remove(trie, "/ab/", len, &udata) != 1)
	{
		printf("should remove prefix /ab/\n");
		exit(1);
	}
	printf("[PASS] remove valid prefix\n");

	len = strlen("/ab/cd/");
	if (ct_trie_remove(trie, "/ab/cd/", len, &udata) != -1)
	{
		printf("should fail to remove prefix /ab/cd/\n");
		exit(1);
	}
	printf("[PASS] remove invalid prefix\n");

	len = strlen("/ab/cd/efg");
	if (ct_trie_lpm(trie, "/ab/cd/efg", len, &udata) != -1)
	{
		printf("should fail to match no-exists name \"/ab/cd/efg\"\n");
		exit(1);
	}
	printf("[PASS] match remove prefix\n");

	printf("[PASS] ALL PASS\n");
}

#define bench(name, N, code)                                                             \
	{                                                                                \
		{                                                                        \
			if (strlen(name) > 0)                                            \
			{                                                                \
				printf("%-14s ", name);                                  \
			}                                                                \
			size_t tmem = total_mem;                                         \
			size_t tallocs = total_allocs;                                   \
			uint64_t bytes = 0;                                              \
			clock_t begin = clock();                                         \
			for (int i = 0; i < N; i++)                                      \
			{                                                                \
				(code);                                                  \
			}                                                                \
			clock_t end = clock();                                           \
			double elapsed_secs = (double)(end - begin) / CLOCKS_PER_SEC;    \
			double bytes_sec = (double)bytes / elapsed_secs;                 \
			printf("%d ops in %.3f secs, %.0f ns/op, %.0f op/sec",           \
			       N, elapsed_secs,                                          \
			       elapsed_secs / (double)N * 1e9,                           \
			       (double)N / elapsed_secs);                                \
			if (bytes > 0)                                                   \
			{                                                                \
				printf(", %.1f GB/sec", bytes_sec / 1024 / 1024 / 1024); \
			}                                                                \
			if (total_mem > tmem)                                            \
			{                                                                \
				size_t used_mem = total_mem - tmem;                      \
				printf(", %.2f bytes/op", (double)used_mem / N);         \
			}                                                                \
			if (total_allocs > tallocs)                                      \
			{                                                                \
				size_t used_allocs = total_allocs - tallocs;             \
				printf(", %.2f allocs/op", (double)used_allocs / N);     \
			}                                                                \
			printf("\n");                                                    \
		}                                                                        \
	}

struct thread_arg {
	ct_trie_t *trie;
	myitem *items;
	int N;
	pthread_t *tid;
};

void *randomop(void *args) {
	struct thread_arg *arg = args;
	printf("tid %lu start...\n", *arg->tid);
	while (1) {
		int op = rand() % 3;
		int i = rand() % arg->N;

		switch(op) {
		case 0:
			ct_trie_insert(arg->trie, arg->items[i].prefix, arg->items[i].prefixlen, (void *)arg->items[i].value);
		case 1:
			ct_trie_lpm(arg->trie, arg->items[i].prefix, arg->items[i].prefixlen, NULL);
		case 2:
			ct_trie_remove(arg->trie, arg->items[i].prefix, arg->items[i].prefixlen, NULL);
		}
	}
}

static void threadtest(void)
{
	int N = getenv("N") ? atoi(getenv("N")) : 5000000;
	int M = getenv("M") ? atoi(getenv("M")) : 4;
	printf("count=%d, thread_num=%d, item_size=%zu\n", N, M, sizeof(myitem));

	myitem *items = xmalloc(N * sizeof(myitem));
	for (int i = 0; i < N; i++)
	{
		calprefix(i + 1, &items[i].prefix, &items[i].prefixlen);
		items[i].value = (int *)malloc(sizeof(int));
		*items[i].value = i + 1;
	}

	ct_trie_t *trie;
	
	trie = ct_trie_new("/", 1);
	shuffle(items, N, sizeof(myitem));
	for (int i = 0; i < N/2; i++) {
		ct_trie_insert(trie, items[i].prefix, items[i].prefixlen, (void *)items[i].value);
	}

	pthread_t *tid = (pthread_t*)malloc(sizeof(pthread_t) * M);
	struct thread_arg *targ = (struct thread_arg*)malloc(sizeof(struct thread_arg) * M);

	for (int i = 0; i < M; i++) {
		targ[i] = (struct thread_arg){.trie=trie, .items=items, .N=N, .tid=&tid[i]};
		pthread_create(&tid[i], NULL, randomop, &targ[i]);
	}
	for (int i = 0; i < M; i++) {
		pthread_join(tid[i], NULL);
	}
	
	xfree(items);
}


static void benchmarks(void)
{
	int N = getenv("N") ? atoi(getenv("N")) : 5000000;
	printf("count=%d, item_size=%zu\n", N, sizeof(myitem));

	myitem *items = xmalloc(N * sizeof(myitem));
	for (int i = 0; i < N; i++)
	{
		calprefix(i + 1, &items[i].prefix, &items[i].prefixlen);
		items[i].value = (int *)malloc(sizeof(int));
		*items[i].value = i + 1;
	}

	shuffle(items, N, sizeof(myitem));

	ct_trie_t *trie;
	shuffle(items, N, sizeof(myitem));

	trie = ct_trie_new("/", 1);
	bench("insert", N, {
		const int ret = ct_trie_insert(trie, items[i].prefix, items[i].prefixlen, (void *)items[i].value);
		assert(ret == 1);
	}) shuffle(items, N, sizeof(myitem));
	bench("match", N, {
		int *num;
		const int ret = ct_trie_lpm(trie, items[i].prefix, items[i].prefixlen, (void**)&num);
		assert(ret > 0 && *num == *items[i].value);
	}) shuffle(items, N, sizeof(myitem));
	bench("remove", N, {
		int *num;
		const int ret = ct_trie_remove(trie, items[i].prefix, items[i].prefixlen, (void**)&num);
		assert(ret == 1 && *num == *items[i].value);
	}) ct_trie_free(trie);

	xfree(items);
}

int main(void)
{
	hashmap_set_allocator(xmalloc, xfree);

	if (getenv("BENCH"))
	{
		printf("Running component_trie benchmarks...\n");
		benchmarks();
	} 
	else if (getenv("THREAD")) 
	{
		printf("Running component_trie thread safe test...\n");
		threadtest();
	}
	else
	{
		printf("Running component_trie tests...\n");
		all();
	}
}
