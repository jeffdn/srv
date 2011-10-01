/* hash.h
 * Copyright (c) 2006
 * Jeff Nettleton
 */

#ifndef UTIL_HASH_H
#define UTIL_HASH_H

typedef struct _hash_entry_t {
    void *key;
    void *val;

    /* next entry for linked list */
    struct _hash_entry_t *next;
} hash_entry_t;

typedef struct _hash_t {
    unsigned int slots;
    unsigned int count;
    unsigned int unique;

    /* the table */
    struct _hash_entry_t **data;

    /* key comparison/copying/freeing functions */
    int (*keycmp) (const void *, const void *);
    int (*valcmp) (const void *, const void *);

    void *(*keycpy) (const void *);
    void *(*valcpy) (const void *);

    void (*free_key) (void *);
    void (*free_val) (void *);
} hash_t;

/* create a new hash table */
hash_t *hash_new(unsigned int);
/* initialise a hash table */
void hash_init(hash_t *, unsigned int);
/* destroy a hash table */
void hash_destroy(hash_t *);
/* free a hash table */
void hash_free(hash_t *);
/* insert a key/value pair into a hash table */
int hash_insert(hash_t *, const void *, const void *);
/* get a value by key */
void *hash_get(hash_t *, const void *);
/* remove a hash value from the table */
int hash_delete(hash_t *, const void *);
/* execute the provided function for each key */
int hash_foreach(hash_t *, int (*foreach) (const void *, void *), void *);
/* hashing function */
unsigned int hash_func(const void *);
/* double the size of the hash, not enough slots */
int hash_resize(hash_t *);

/* copy a hash table */
hash_t *hash_dup(hash_t *);

/* remove all keys from a hash */
int _hash_delete_foreach(const void *, void *);
#define hash_clear(ht) hash_foreach (ht, _hash_delete_foreach, ht)

/* assign the functions */
void hash_set_keycmp(hash_t *, int (*keycmp) (const void *, const void *));
void hash_set_valcmp(hash_t *, int (*valcmp) (const void *, const void *));
void hash_set_keycpy(hash_t *, void *(*keycpy) (const void *));
void hash_set_valcpy(hash_t *, void *(*valcpy) (const void *));
void hash_set_free_key(hash_t *, void (*free_key) (void *));
void hash_set_free_val(hash_t *, void (*free_val) (void *));

/* some default functions, for a string hash */
int hash_default_keycmp(const void *, const void *);
int hash_default_valcmp(const void *, const void *);
void *hash_default_keycpy(const void *);
void *hash_default_valcpy(const void *);
void hash_default_free_key(void *);
void hash_default_free_val(void *);

/* some user friendly functions! */
/* create a hash for strings */
hash_t *hash_new_string(unsigned int);
/* initialize said hash */
void hash_init_string(hash_t *, unsigned int);

#endif
