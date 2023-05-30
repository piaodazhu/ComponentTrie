#ifndef _COMPONENT_TRIE_H_
#define _COMPONENT_TRIE_H_

struct ct_node {
	char *component;
	int componentlen;
	
	htable *children;

	bool flag;
	void *udata;
}



#endif