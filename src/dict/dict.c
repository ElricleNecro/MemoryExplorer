#include "dict/dict.h"

unsigned int to_int(const char *str)
{
	unsigned int k = 0, l_str = strlen(str);

	for(unsigned int i = 0; i < l_str; i++)
	{
		k += str[i] * pow(DICT_BASE_ENCODE, l_str - i - 1);
	}

	return k;
}

unsigned int hasher(const char *key, const int taille)
{
	static double nb_or = 0.618;
	unsigned int k = to_int(key);

	return (
		(unsigned int)floor(
			taille * (
				nb_or * k - floor(nb_or * k)
			)
		)
	) % (
		taille
	);
}

Elem Elem_new(const char *key, void* item)
{
	Elem obj = NULL;

	if( (obj = malloc(sizeof(struct _dict_elem))) == NULL )
		return NULL;

	obj->key = strdup(key);
	obj->item = item;

	obj->next = obj->prev = NULL;

	return obj;
}

void Elem_free(Elem obj)
{
	if( !obj )
		return ;

	while( obj )
	{
		Elem tmp = obj;
		obj = obj->next;
		free(tmp->key);
		free(tmp);
	}
}

Dict Dict_new(const int size)
{
	Dict obj = NULL;

	if( (obj = malloc(sizeof(struct _dict))) == NULL )
		return NULL;

	obj->size = size;
	obj->hash = hasher;

#ifdef USE_CHAINED_DICT
	obj->first = obj->last = NULL;
#else
	obj->array = NULL;

	if( (obj->array = calloc(size, sizeof(struct _dict))) == NULL )
		return NULL;
#endif

	return obj;
}

void* Dict_get(Dict obj, const char *key)
{
	int k = obj->hash(key, obj->size);

	if( !obj->array[k] )
	{
		return NULL;
	}

	for(Elem item = obj->array[k]; item; item = item->next)
	{
		if( !strcmp(item->key, key) )
			return item->item;
	}

	return NULL;
}

bool Dict_set(Dict obj, const char *key, void *item)
{
	int k = obj->hash(key, obj->size);

	if( obj->array[k] == NULL )
	{
		obj->array[k] = Elem_new(key, item);
		if( !obj->array[k] )
			return false;
		return true;
	}

	Elem elem = obj->array[k];

	while( elem->next && strcmp(elem->key, key) )
		elem = elem->next;

	if( elem->next )
		elem->item = item;
	else
	{
		elem->next = Elem_new(key, item);

		if( !elem->next )
			return false;

		elem->next->prev = elem;
	}

	return true;
}

void Dict_free(Dict obj)
{
#ifdef USE_CHAINED_DICT
	Elem_free(obj->fisrt);
#else
	for (unsigned int i = 0; i < obj->size; i++)
	{
		Elem_free(obj->array[i]);

		/* if( obj->array[i] ) */
		/* { */
			/* [> if( obj->array[i]->next ) <] */
			/* [> { <] */
			/* Elem tmp, actual = obj->array[i]; */
			/* while( actual ) */
			/* { */
				/* tmp = actual; */
				/* actual = actual->next; */
				/* free(tmp); */
			/* } */
			/* [> } <] */
			/* [> free(obj->array[i]); <] */
		/* } */
	}
	free(obj->array);
#endif

	free(obj);
}

