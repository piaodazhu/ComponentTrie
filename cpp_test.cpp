#include <iostream>
#include <string>
#include "component_trie.h"

int main() {
	std::string s;
	s = std::string("/");
	ct_trie_t *trie = ct_trie_new(s.c_str(), s.size());
	if (trie == NULL)
	{
		printf("can't new ct_trie\n");
		exit(1);
	}

	// insert
	s = std::string("/ab/cd/ef");
	if (ct_trie_insert(trie, s.c_str(), s.size(), trie) != -1)
	{
		printf("should fail to insert invalid prefix\n");
		exit(1);
	}
	printf("[PASS] insert invalid prefix\n");

	s = std::string("/ab/cd/ef/");
	if (ct_trie_insert(trie, s.c_str(), s.size(), trie) != 1)
	{
		printf("should insert new prefix\n");
		exit(1);
	}
	printf("[PASS] insert valid prefix\n");

	const char *prefixes[] = {
	    "/a/",
	    "/ab/",
	    "/ab/cde/",
	    "/hello/",
	    "/hello/world/",
	    "/1/2/3/4/"};
	int i = 0;
	for (i = 0; i < 6; i++)
	{
		s = std::string(prefixes[i]);
		if (ct_trie_insert(trie, s.c_str(), s.size(), trie) != 1)
		{
			printf("should insert new prefix: %s\n", prefixes[i]);
			exit(1);
		}
	}
	printf("[PASS] insert multiply prefixes\n");

	// match
	void *udata;
	s = std::string("/ab");
	if (ct_trie_lpm(trie, s.c_str(), s.size(), &udata) != -1)
	{
		printf("should fail to match empty name\n");
		exit(1);
	}
	printf("[PASS] match empty name\n");

	s = std::string("ab/");
	if (ct_trie_lpm(trie, s.c_str(), s.size(), &udata) != -1)
	{
		printf("should fail to match invalid name\n");
		exit(1);
	}
	printf("[PASS] match invalid name\n");

	s = std::string("/1/2/hello");
	if (ct_trie_lpm(trie, s.c_str(), s.size(), &udata) != -1)
	{
		printf("should fail to match no-exists name\n");
		exit(1);
	}
	printf("[PASS] match no-exists name\n");

	const char *name_ok[] = {
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
		s = std::string(name_ok[i]);
		if (ct_trie_lpm(trie, s.c_str(), s.size(), &udata) != match_ok[i])
		{
			printf("name %s should match prefix\n", name_ok[i]);
			exit(1);
		}
	}
	printf("[PASS] match multiple names\n");

	// delete
	s = std::string("/ab/");
	if (ct_trie_remove(trie, s.c_str(), s.size(), &udata) != 1)
	{
		printf("should remove prefix /ab/\n");
		exit(1);
	}
	printf("[PASS] remove valid prefix\n");

	s = std::string("/ab/cd/");
	if (ct_trie_remove(trie, s.c_str(), s.size(), &udata) != -1)
	{
		printf("should fail to remove prefix /ab/cd/\n");
		exit(1);
	}
	printf("[PASS] remove invalid prefix\n");

	s = std::string("/ab/cd/efg");
	if (ct_trie_lpm(trie, s.c_str(), s.size(), &udata) != -1)
	{
		printf("should fail to match no-exists name \"/ab/cd/efg\"\n");
		exit(1);
	}
	printf("[PASS] match remove prefix\n");

	printf("[PASS] ALL PASS\n");
}