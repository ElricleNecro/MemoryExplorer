#ifndef DICT_H_HNTOU0ZV
#define DICT_H_HNTOU0ZV

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef USE_UNICODE
	#define DICT_BASE_ENCODE 16384
#elif defined(USE_ALPHABETIC)
	#define DICT_BASE_ENCODE 26
#else
	#define DICT_BASE_ENCODE 128
#endif

#ifndef DEF_DICT_SIZE
	#define DEF_DICT_SIZE 128
#endif

/**
 * Dictionnary element.
 */
typedef struct _dict_elem {
	char *key;
	void *item;

	struct _dict_elem *next, *prev;
} *Elem;

typedef struct _dict {
	unsigned int (*hash)(const char*, const int);
	unsigned int size;
#ifdef USE_CHAINED_DICT
	Elem first, last;
#else
	Elem *array;
#endif
} *Dict, dict;

Elem Elem_new(const char *key, void *item);
void Elem_free(Elem obj);

Dict Dict_new(const int size);
void* Dict_get(Dict obj, const char *key);
bool Dict_set(Dict obj, const char *key, void *item);
void Dict_free(Dict obj);

#endif /* end of include guard: DICT_H_HNTOU0ZV */
