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
 * Dictionary element.
 */
typedef struct _dict_elem {
	char *key;
	void *item;

	struct _dict_elem *next, *prev;
} *Elem;

/**
 * Dictionary
 */
typedef struct _dict {
	unsigned int (*hash)(const char*, const int);	// <- Hash function: transform a string into an integer index for our array.
	unsigned int size;				// <- Size of the array
#ifdef USE_CHAINED_DICT
	Elem first, last;				// <- Linked list instead of an array, not yet implemented.
#else
	Elem *array;					// <- Array of element.
#endif
} *Dict, dict;

/**
 * Allocate a new element objet.
 *
 * @param[in] *key Key associated to the element.
 * @param[in] *item The item to stock.
 * @return The new element, or NULL if failed.
 */
Elem Elem_new(const char *key, void *item);

/**
 * Free an element.
 *
 * @param[in] obj Element to free, including all next element.
 */
void Elem_free(Elem obj);

/**
 * Dictionary constructor.
 * The parameter size should be as such your element will occupy 2/3 of the array (not very sure, but it's something like that
 * to minimize collisions). Should be a power of two (it's better).
 *
 * @param[in] size Size of the array which will stock the element.
 *
 * @return The new dictionary.
 */
Dict Dict_new(const int size);

/**
 * Get the item associated to the given key.
 *
 * @param obj The dictionnary to add to.
 * @param[in] key The key to get.
 *
 * @return The item associated to the key.
 */
void* Dict_get(Dict obj, const char *key);

/**
 * Set the key 'key' to contain the object 'item'.
 * All the errors in this function can be retrieve using the 'standard' way (errno & co), as only function from stdlib may failed.
 *
 * @param[in] obj The dictionary to add the key.
 * @param[in] key The key.
 * @param[in] item Item to associate to the key.
 *
 * @return true if success, false if failed.
 */
bool Dict_set(Dict obj, const char *key, void *item);

/**
 * Free the dictionnary, removing all element.
 *
 * @param[in] obj dictionnary to free.
 */
void Dict_free(Dict obj);

#endif /* end of include guard: DICT_H_HNTOU0ZV */
