/* hash.c
 * Copyright (c) 2006
 * Jeff Nettleton
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>

#include "util.h"
#include "hash.h"

/**
 * create a new hash table
 * @param slots the number of slots to create the hash table with
 */
hash_t *hash_new(unsigned int slots)
{
    hash_t *ht = calloc(1, sizeof *ht);

    if (NULL == ht) {
        ERRF(__FILE__, __LINE__, "allocating memory for new hash_t!\n");
        return NULL;
    }

    hash_init(ht, slots);

    return ht;
}

/**
 * initialise an allocated hash table
 * @param ht the hash table to initialise
 * @param slots the number of slots to start with
 */
void hash_init(hash_t * ht, unsigned int slots)
{
#ifdef DEBUG
    assert(NULL != ht);
#endif

    /* for now, we comment this out.  I need to come up with a better way
     * to round the slots, and this kind of sucks.  But whatever...we'll
     * just take whatever the user provides.
     *
     * ht->slots = (unsigned int) ceil (sqrt (slots) * log (slots));
     */

    ht->slots = slots;
    ht->count = 0;
    ht->unique = 0;

    ht->data = calloc(ht->slots, sizeof *ht->data);
    if (NULL == ht->data) {
        ERRF(__FILE__, __LINE__, "allocating memory for new hash_entry_t!\n");
        exit(1);
    }
}

/**
 * double the size of the hash table
 * @param ht the hash table to make larger
 */
int hash_resize(hash_t * ht)
{
    hash_entry_t **tmp, *he;
    unsigned int index, slots;

#ifdef DEBUG
    assert(NULL != ht);
#endif

    tmp = ht->data;
    ht->data = NULL;
    slots = ht->slots;

    hash_init(ht, slots * 2);

    /* tmp retains the proper pointer for now */
    for (index = 0; index < slots; index++) {
        if (NULL != (he = tmp[index])) {
            while (NULL != he) {
                /* insert into new data table */
                hash_insert(ht, he->key, he->val);

                ht->free_key(he->key);
                ht->free_val(he->val);

                /* "remove" the entry from the linked list */
                if (NULL != he->next) {
                    tmp[index] = he->next;
                    free(he);
                    he = tmp[index];
                } else {
                    free(he);
                    he = tmp[index] = NULL;
                }
            }
        }
    }

    free(tmp);
    return 1;
}

/**
 * insert a key/value pair into the hash table
 * @param ht the hash table to insert into
 * @param key the key to user
 * @param val the value to map to the key
 */
int hash_insert(hash_t * ht, const void *key, const void *val)
{
    hash_entry_t *he;
    unsigned int hash, index;

#ifdef DEBUG
    assert(NULL != ht);
    assert(NULL != key);
    assert(NULL != val);
#endif

    if (NULL == ht->keycpy || NULL == ht->valcpy) {
        ERRF(__FILE__, __LINE__,
             "key/value copying functions are not set properly!\n");
        return 0;
    }

    if (ht->count >= ht->slots) {
        /* we're out of space ! */
        if (!hash_resize(ht)) {
            /* that sucks... errors allocating */
            return 0;
        }
    }

    hash = hash_func(key);
    index = hash % ht->slots;

    if (NULL != (he = ht->data[index])) {
        DEBUGF(__FILE__, __LINE__, "got a duplicate...\n");

        while (NULL != he) {
            if (!ht->keycmp(key, he->key)) {
                /* keys match. free value, and copy new one */
                ht->free_val(he->val);
                he->val = ht->valcpy(val);

                return 1;
            }

            he = he->next;
        }
    } else {
        ht->unique++;
    }

    he = calloc(1, sizeof *he);
    if (NULL == he) {
        ERRF(__FILE__, __LINE__,
             "allocating memory for a new hash_entry_t struct!\n");
        exit(1);
    }

    he->key = ht->keycpy(key);
    he->val = ht->valcpy(val);

    /* do some linked list hax */
    he->next = ht->data[index];
    ht->data[index] = he;

    ++ht->count;

    return 1;
}

/**
 * get an entry in the hash table
 * @param ht the hash table to request the entr from
 * @param key the key to get the value for
 */
void *hash_get(hash_t * ht, const void *key)
{
    unsigned int hash, index;
    hash_entry_t *he;

#ifdef DEBUG
    assert(NULL != ht);
    assert(NULL != key);
#endif

    hash = hash_func(key);
    index = hash % ht->slots;

    if (NULL != (he = ht->data[index])) {
        while (NULL != he) {
            if (!ht->keycmp(key, he->key)) {
                /* k they are equal */
                return he->val;
            }

            he = he->next;
        }
    }

    return NULL;
}

/**
 * the foreach function to use with the hash_clear () macro
 * @param key the key to delete from the hash
 * @param userptr (it will be ht in this case)
 */
int _hash_delete_foreach(const void *key, void *userptr)
{
    hash_delete(userptr, key);

    return 1;
}

/**
 * remove an entry from a hash table
 * @param ht the hash table to remove the entry from
 * @param key the key to remove from the table
 */
int hash_delete(hash_t * ht, const void *key)
{
    unsigned int hash, index;
    hash_entry_t *he;

#ifdef DEBUG
    assert(NULL != ht);
    assert(NULL != key);
#endif

    hash = hash_func(key);
    index = hash % ht->slots;

    if (NULL != (he = ht->data[index])) {
        while (NULL != he) {
            if (!ht->keycmp(key, he->key)) {
                /* k they are equal */
                ht->free_key(he->key);
                ht->free_val(he->val);

                /* "remove" the entry from the linked list */
                if (he->next) {
                    ht->data[index] = he->next;
                    free(he);
                } else {
                    free(he);
                    ht->data[index] = NULL;
                }

                --ht->count;
                return 1;
            }

            he = he->next;
        }
    }

    return 0;
}

/**
 * execute a function for each key in a hash table
 * @param ht the hash table to iterate over
 * @param foreach the function to call for each key
 * @param userptr the additional argument the user can pass (can be NULL)
 */
int hash_foreach(hash_t * ht, int (*foreach) (const void *, void *),
                 void *userptr)
{
    unsigned int i;
    hash_entry_t *he;

#ifdef DEBUG
    assert(NULL != ht);
    assert(NULL != foreach);
#endif

    if (NULL == foreach) {
        /* erm, pointless to execute this function... */
        return 0;
    }

    for (i = 0; i < ht->slots; i++) {
        if (NULL != (he = ht->data[i])) {
            while (NULL != he) {
                if (!foreach(he->key, userptr)) {
                    /* I might change this. If the foreach () function
                     * that the user passes returns 0, do we REALLY want
                     * to exit, or are we just going to ignore this value?
                     * For now, I'm going to have it return 0, but I may
                     * very well change this.
                     */
                    return 0;
                }

                he = he->next;
            }
        }
    }

    return 1;
}

/**
 * Daniel J. Bernstein's hashing function. Pizza says it's good!
 * @param str the string to turn into a number :O O:
 */
unsigned int hash_func(const void *str)
{
    char *c;
    register unsigned int h = 5381;

    for (c = (char *)str; *c != '\0'; c++) {
        h = (h + (h << 5)) + *c;
    }

    return h;
}

/**
 * destroy a hash table
 * @param ht the hash table to destroy
 */
void hash_destroy(hash_t * ht)
{
#ifdef DEBUG
    assert(NULL != ht);
#endif

    hash_clear(ht);

    free(ht->data);

    ht->count = 0;
    ht->slots = 0;
    ht->unique = 0;

    ht->keycmp = NULL;
    ht->valcmp = NULL;
    ht->keycpy = NULL;
    ht->valcpy = NULL;
    ht->free_key = NULL;
    ht->free_val = NULL;
}

/**
 * free a hash table
 * @param ht the hash table to free
 */
void hash_free(hash_t * ht)
{
    hash_destroy(ht);
    free(ht);
}

/**
 * the foreach handler to duplicate the hash table
 * @param key the key to copy on this iteration
 * @param pair the two hash tables
 */
int _hash_dup_foreach(const void *key, void *pair)
{
    hash_insert(*((hash_t **) pair) + 1, key,
                hash_get(*((hash_t **) pair), key));

    return 1;
}

/**
 * duplicate a hash table
 * @param ht the hash table to duplicate
 */
hash_t *hash_dup(hash_t * ht)
{
    hash_t *copy, *pair[2];

    copy = hash_new(ht->slots);

    pair[0] = ht;
    pair[1] = copy;

    hash_foreach(ht, _hash_dup_foreach, pair);

    return copy;
}

/**
 * set the key comparison function
 * @param ht the hash table to set the function for
 * @param keycmp the key comparison function
 */
void hash_set_keycmp(hash_t * ht, int (*keycmp) (const void *, const void *))
{
#ifdef DEBUG
    assert(NULL != ht);
    assert(NULL != keycmp);
#endif

    ht->keycmp = keycmp;
}

/**
 * set the value comparison function
 * @param ht the hash table to set the function for
 * @param valcmp the value comparison function
 */
void hash_set_valcmp(hash_t * ht, int (*valcmp) (const void *, const void *))
{
#ifdef DEBUG
    assert(NULL != ht);
    assert(NULL != valcmp);
#endif

    ht->valcmp = valcmp;
}

/**
 * set the key copying function
 * @param ht the hash table to set the function for
 * @param keycpy the key copying function
 */
void hash_set_keycpy(hash_t * ht, void *(*keycpy) (const void *))
{
#ifdef DEBUG
    assert(NULL != ht);
    assert(NULL != keycpy);
#endif

    ht->keycpy = keycpy;
}

/**
 * set the value copying function
 * @param ht the hash table to set the function for
 * @param valcpy the value copying function
 */
void hash_set_valcpy(hash_t * ht, void *(*valcpy) (const void *))
{
#ifdef DEBUG
    assert(NULL != ht);
    assert(NULL != valcpy);
#endif

    ht->valcpy = valcpy;
}

/**
 * set the key free'ing function
 * @param ht the hash table to set the function for
 * @param free_key the key free'ing function
 */
void hash_set_free_key(hash_t * ht, void (*free_key) (void *))
{
#ifdef DEBUG
    assert(NULL != ht);
    assert(NULL != free_key);
#endif

    ht->free_key = free_key;
}

/**
 * set the value free'ing function
 * @param ht the hash table to set the function for
 * @param free_val the value free'ing function
 */
void hash_set_free_val(hash_t * ht, void (*free_val) (void *))
{
#ifdef DEBUG
    assert(NULL != ht);
    assert(NULL != free_val);
#endif

    ht->free_val = free_val;
}

/**
 * default key comparison function (strncmp ;))
 * @param key the key to compare against
 * @param str the string to compare with the key
 */
int hash_default_keycmp(const void *key, const void *str)
{
    if (!strncmp((char *)key, (char *)str, strlen((char *)key))) {
        /* key and string are equal, true */
        return 0;
    }

    return 1;
}

/**
 * default value comparison function
 * @param val the vaue to compare against
 * @param str the string to compare with the value
 */
int hash_default_valcmp(const void *val, const void *str)
{
    if (!strncmp((char *)val, (char *)str, strlen((char *)val))) {
        /* value and string are equal, true */
        return 0;
    }

    return 1;
}

/**
 * set the key copying funtion
 * @param key the key to copy
 */
void *hash_default_keycpy(const void *key)
{
    char *tmp;

    if (NULL == (tmp = strdup((char *)key))) {
        ERRF(__FILE__, __LINE__, "strdup() in hash_default_keycpy!\n");
        return NULL;
    }

    return tmp;
}

/**
 * set the value copying function
 * @param val the value to copy
 */
void *hash_default_valcpy(const void *val)
{
    char *tmp;

    if (NULL == (tmp = strdup((char *)val))) {
        ERRF(__FILE__, __LINE__, "strdup() in hash_default_valcpy!\n");
        return NULL;
    }

    return tmp;
}

/**
 * set the key free'ing function
 * @param key the key to free
 */
void hash_default_free_key(void *key)
{
    if (NULL != key) {
        free(key);
    }
}

/**
 * set the value free'ing function
 * @param val the value to free
 */
void hash_default_free_val(void *val)
{
    if (NULL != val) {
        free(val);
    }
}

/**
 * create a hash for string keys and values
 * @param slots the number of slots to give the table
 */
hash_t *hash_new_string(unsigned int slots)
{
    hash_t *ht;

    ht = hash_new(slots);
    hash_init_string(ht, slots);

    return ht;
}

/**
 * initialize a string hash
 * @param ht the string hash to initialize
 * @param slots the number of slots to have
 */
void hash_init_string(hash_t * ht, unsigned int slots)
{
    hash_init(ht, slots);

    hash_set_keycpy(ht, hash_default_keycpy);
    hash_set_valcpy(ht, hash_default_valcpy);
    hash_set_keycmp(ht, hash_default_keycmp);
    hash_set_valcmp(ht, hash_default_valcmp);
    hash_set_free_key(ht, hash_default_free_key);
    hash_set_free_val(ht, hash_default_free_val);
}
