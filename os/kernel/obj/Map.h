#ifndef __MAP_H__
#define __MAP_H__ 1
#include "Set.h"
#include "../kmemman.h"
#define FAILMASK_MAP 0x000c0000
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
	Mutex_acquire(&(map->set->lock));
	Set_containsByCompare(0, Map_compareNotEqualRemove, map->set);
	Set_destroy(map->set);
	dealloc(map, sizeof(struct Map));
	return;
}
int Map_add(uintptr key, uintptr value, struct Map* map) {// Success: 0; Key is already mapped: -1
	Mutex_acquire(&(map->set->lock));
	if (Set_containsByCompare(key, Map_compare, map->set) != (uintptr) (-1)) {
		Mutex_release(&(map->set->lock));
		return (-1);
	}
	struct Map_pair* m = alloc(sizeof(struct Map_pair));
	m->key = key;
	m->value = value;
	if (Set_add((uintptr) m, map->set)) {
		bugCheckNum(0x0001 | FAILMASK_MAP);
	}
	Mutex_release(&(map->set->lock));
	return 0;
}
uintptr Map_fetch(uintptr key, struct Map* map) {// Contains the specified key: The associated value; Does not contain the specified key: (uintptr) -1
	Mutex_acquire(&(map->set->lock));
	uintptr i = Set_containsByCompare(key, Map_compare, map->set);
	if (i == (uintptr) (-1)) {
		Mutex_release(&(map->set->lock));
		return (uintptr) (-1);
	}
	uintptr retVal = ((struct Map_pair*) i)->value;
	Mutex_release(&(map->set->lock));
	return retVal;
}
int Map_cmpStr(uintptr suspect, uintptr str) {
	return strcmp((const char*) (((struct Map_pair*) suspect)->key), (const char*) str);
}
uintptr Map_findByCompare(uintptr data, int (*comparator)(uintptr, uintptr), struct Map* map) {// Contains a match: The key of the match; Does not contain a match: (uintptr) (-1); NO CONCURRENT ACCESS
	Mutex_acquire(&(map->set->lock));
	uintptr i = Set_containsByCompare(data, comparator, map->set);// comparison is guaranteed to be in the order `comparator(<Map_pair*>, <provided value>)'; comparator(A, B) == 0: A and B match according to the comparator; comparator(A, B) != 0: A and B do not match according to the comparator
	if (i == (uintptr) (-1)) {
		Mutex_release(&(map->set->lock));
		return (-1);
	}
	uintptr retVal = ((struct Map_pair*) i)->key;
	Mutex_release(&(map->set->lock));
	return retVal;
}
int Map_remove(uintptr key, struct Map* map) {// Success: 0; Key was not already present: -1
	Mutex_acquire(&(map->set->lock));
	uintptr i = Set_containsByCompare(key, Map_compare, map->set);
	if (i == (uintptr) (-1)) {
		Mutex_release(&(map->set->lock));
		return (-1);
	}
	if (Set_remove(i, map->set)) {
		bugCheckNum(0x0002 | FAILMASK_MAP);
	}
	dealloc((struct Map_pair*) i, sizeof(struct Map_pair));
	Mutex_release(&(map->set->lock));
	return 0;
}
int Map_copyCompareNonmatch(uintptr suspect, uintptr nm) {
	Map_add(((struct Map_pair*) suspect)->key, ((struct Map_pair*) suspect)->value, (struct Map*) nm);
	return 1;
}
struct Map* Map_copy(struct Map* orig) {
	struct Map* nm = Map_create();
	Map_findByCompare((uintptr) nm, Map_copyCompareNonmatch, orig);
	return nm;
}
#endif
