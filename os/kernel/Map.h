#ifndef __MAP_H__
#define __MAP_H__ 1
#include "Set.h"
struct Map {
	struct Set* set;
};
struct Map_pair {
	uintptr key;
	uintptr value;
};
struct Map* Map_create(void) {
	struct Set* set = Set_create();
	struct Map* map = alloc(sizeof(struct Map));
	map->set = set;
	return map;
}
int Map_compare(uintptr suspectedPair, uintptr key) {
	return ((struct Map_pair*) suspectedPair)->key != key;
}
int Map_compareNotEqualRemove(uintptr pair, uintptr arb) {
	dealloc((struct Map_pair*) pair, sizeof(struct Map_pair));
	return 1;
}
void Map_destroy(struct Map* map) {
	Set_containsByCompare(0, Map_compareNotEqualRemove, map->set);
	Set_destroy(map->set);
	dealloc(map, sizeof(struct Map));
	return;
}
int Map_add(uintptr key, uintptr value, struct Map* map) {// Success: 0; Key is already mapped: -1
	if (Set_containsByCompare(key, Map_compare, map->set) != (uintptr) (-1)) {
		return (-1);
	}
	struct Map_pair* m = alloc(sizeof(struct Map_pair));
	m->key = key;
	m->value = value;
	if (Set_add((uintptr) m, map->set)) {
		return (-1);
	}
	return 0;
}
uintptr Map_fetch(uintptr key, struct Map* map) {// Contains the specified key: The associated value; Does not contain the specified key: (uintptr) -1
	uintptr i = Set_containsByCompare(key, Map_compare, map->set);
	if (i == (uintptr) (-1)) {
		return (uintptr) (-1);
	}
	return ((struct Map_pair*) i)->value;
}
uintptr Map_findByCompare(uintptr data, int (*comparator)(uintptr, uintptr), struct Map* map) {// Contains a match: The key of the match; Does not contain a match: (uintptr) (-1)
	uintptr i = Set_containsByCompare(data, comparator, map->set);// comparison is guaranteed to be in the order `comparator(<Map_pair*>, <provided value>)'; comparator(A, B) == 0: A and B match according to the comparator; comparator(A, B) != 0: A and B do not match according to the comparator
	if (i == (uintptr) (-1)) {
		return (-1);
	}
	return ((struct Map_pair*) i)->key;
}
int Map_remove(uintptr key, struct Map* map) {// Success: 0; Key was not already present: -1
	uintptr i = Set_containsByCompare(key, Map_compare, map->set);
	if (i == (uintptr) (-1)) {
		return (-1);
	}
	if (Set_remove(i, map->set)) {
		return (-1);
	}
	dealloc((struct Map_pair*) i, sizeof(struct Map_pair));
	return 0;
}
#endif
