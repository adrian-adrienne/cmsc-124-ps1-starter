/*
 * dt_map.c: Associative arrays for Unit 5, Section E.
 *
 * An array does not store its indices. This map stores its keys.
 * An array calculates a position with one subtraction.
 * The map calculates a hash and then compares keys in one bucket.
 *
 * Hashing turns the key into a bucket number. Compare all keys in that bucket
 * because two keys can select it. Use a linked list for each bucket. Start the
 * unsigned accumulator at 14695981039346656037ULL. For each unsigned byte,
 * exclusive-or the byte into it and multiply by 1099511628211ULL.
 *
 * A separate list stores insertion order for stable output. dt_map_key_at
 * reads this list. Updating a key preserves its position. Removing and
 * reinserting a key moves it to the end.
 */

#include "dt.h"

#include <stdlib.h>
#include <string.h>

#define MAP_INITIAL_BUCKETS 16                  // provides the optimal balance of bitwise optimization and low memory overhead.

typedef struct map_entry {
    char             *key;          // our own copy of the caller's key
    dt_value          value;
    struct map_entry *chain_next;   // next entry in the same bucket 
    struct map_entry *order_prev;   // insertion-order list (doubly linked)
    struct map_entry *order_next;
} map_entry;

struct dt_map {
    map_entry **buckets;             //chains pointer
    size_t      nbuckets;
    size_t      count;
    map_entry  *head;               
    map_entry  *tail;               
};

static unsigned long long fnv1a(const char *key) //from instructions
{
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    return h;
}

static size_t bucket_of(const dt_map *m, const char *key)           // returns the bucket index for a key. The caller must not pass NULL.
{
    return (size_t)(fnv1a(key) % (unsigned long long)m->nbuckets);
}

static map_entry *find_entry(const dt_map *m, const char *key)   // Walk one chain, comparing stored keys. NULL means absence
{
    for (map_entry *e = m->buckets[bucket_of(m, key)]; e != NULL; e = e->chain_next) {
        if (strcmp(e->key, key) == 0) return e;
    }
    return NULL;
}

static void grow(dt_map *m)
{
    if (m->nbuckets > SIZE_MAX / 2 / sizeof(map_entry *)) return;
    size_t newn = m->nbuckets * 2;
    map_entry **nb = calloc(newn, sizeof *nb);
    if (!nb) return;
    free(m->buckets);
    m->buckets = nb;
    m->nbuckets = newn;
    for (map_entry *e = m->head; e != NULL; e = e->order_next) {
        size_t b = bucket_of(m, e->key);
        e->chain_next = m->buckets[b];
        m->buckets[b] = e;
    }
}
/*
 * dt_map_new builds an empty map. It returns NULL after an allocation failure.
 */
dt_map *dt_map_new(void)
{
    /* TODO: Return an allocated empty map. Return NULL after an allocation failure.
       dt_map_new()  -> a map whose dt_map_len is 0
       cases/normal/map_basics.case */
    dt_map *m = malloc(sizeof *m);             
    if (!m){return NULL;};  
    m->buckets = calloc(MAP_INITIAL_BUCKETS, sizeof *m->buckets);       // allocate the bucket array and initialize it to NULL
    if (!m->buckets) { free(m); return NULL; }                          // free the map structure if m is

    m->nbuckets = MAP_INITIAL_BUCKETS;
    m->count = 0;
    m->head = m->tail = NULL;
    return m;
}

/*
 * dt_map_free releases each entry, copied key, order array, and map.
 * It accepts NULL. The environment owns the values.
 */
void dt_map_free(dt_map *m)
{
    /* TODO: Release each entry, copied key, order array, and map.
       Preserve the values. The environment owns them.
       a map holding a string value  -> the nodes and keys go, the string stays
       dt_map_free(NULL)             -> returns, having done nothing
       cases/cleanup/map_churn.case */
    (void)m;
}

/*
 * dt_map_len returns the number of keys in constant time.
 */
size_t dt_map_len(const dt_map *m)
{
    /* TODO: Return the current key count.
       Replacing a value does not change this count.
       after put alpha, beta, gamma:  dt_map_len(m) -> 3
       after put beta again:          dt_map_len(m) -> 3, still
       after del alpha:               dt_map_len(m) -> 2
       cases/normal/map_basics.case */
    (void)m;
    return 0;
}

/*
 * dt_map_put binds v to key.
 * An existing key keeps its insertion position. A new key becomes the last key.
 * Copy each new key because the caller owns the source buffer.
 * Return DT_ERR_CAPACITY after an allocation failure.
 */
dt_status dt_map_put(dt_map *m, const char *key, dt_value v)
{
    /* TODO: Replace the value for an existing key.
       Add a new entry for a new key. Copy each new key.
       Hash the key. Select its bucket. Search the bucket chain.
       Add a new entry to the chain and insertion list.
       put "beta" -> 2 on an empty map    -> DT_OK, "beta" is last in order
       put "beta" -> 22 on that map       -> DT_OK, same position, new value
       an allocation failure              -> DT_ERR_CAPACITY, map unchanged
       cases/normal/map_basics.case */
    (void)m;
    (void)key;
    (void)v;
    return DT_ERR_CAPACITY;
}

/*
 * dt_map_get writes the value for key to *out.
 * It returns DT_ERR_KEY and does not change *out when the key is absent.
 * An absent key differs from a nil value.
 */
dt_status dt_map_get(const dt_map *m, const char *key, dt_value *out)
{
    /* TODO: Return DT_ERR_KEY when the key is absent.
       Preserve *out after this error. A nil value can be present.
       after put "beta" -> 22:
         dt_map_get(m, "beta", &out)   -> DT_OK, *out is the integer 22
         dt_map_get(m, "ghost", &out)  -> DT_ERR_KEY, *out untouched
       cases/normal/map_basics.case, cases/boundary/map_missing_key.case */
    map_entry *e = find_entry(m, key);
    if (!e) return DT_ERR_KEY;                      // absent is not the same answer as nil; nil can be stored in the map
    *out = e->value;
    return DT_OK;
}

/*
 * dt_map_remove removes key from its bucket and insertion position.
 * It releases the copied key. It returns DT_ERR_KEY when the key is absent.
 */
dt_status dt_map_remove(dt_map *m, const char *key)
{
    /* TODO: Remove the entry from its bucket and insertion position.
       Release the copied key. Return DT_ERR_KEY when the key is absent.
       a map holding alpha, beta, gamma:
         dt_map_remove(m, "alpha")  -> DT_OK, order is now beta, gamma
         dt_map_remove(m, "ghost")  -> DT_ERR_KEY, nothing changes
       reinserting "alpha" appends it after "gamma"
       cases/normal/map_basics.case, cases/boundary/map_remove_missing_key.case */
    map_entry *e = find_entry(m, key);
    if (!e) return DT_ERR_KEY;

    // Remove from hash bucket chain
    size_t bucket = bucket_of(m, key);          
    if (m->buckets[bucket] == e) {                      
        m->buckets[bucket] = e->chain_next;         //mark the next entry in the chain as the new head of the bucket
    } else {
        map_entry *prev = m->buckets[bucket];           
        while (prev && prev->chain_next != e) {         //iterate through the bucket chain to find the entry before e
            prev = prev->chain_next;
        }
        if (prev) {
            prev->chain_next = e->chain_next;           //entry e is removed from the chain by linking the previous entry to the next entry
        }
    }

    // Remove from insertion order list
    if (e->order_prev) {                    //if e has a previous entry in the insertion order list, link that entry to e's next entry
        e->order_prev->order_next = e->order_next;
    } else {
        m->head = e->order_next;   
    }
    if (e->order_next) {                   //if e has a next entry in the insertion order list, link that entry to e's previous entry
        e->order_next->order_prev = e->order_prev;
    } else {
        m->tail = e->order_prev;
    }

    // Release the copied key and free the entry
    free((char *)e->key);
    free(e);
    m->count--;
    return DT_OK;
}

/*
 * dt_map_key_at writes the key at insertion position index to *out.
 * It returns DT_ERR_RANGE and does not change *out for an invalid index.
 */
dt_status dt_map_key_at(const dt_map *m, size_t index, const char **out)
{
    /* TODO: Write the key at the specified insertion position to *out.
       Return DT_ERR_RANGE for an invalid position. Preserve *out after this error.
       The printer uses this order.
       a map holding alpha, beta, gamma:
         dt_map_key_at(m, 0, &out)  -> DT_OK, *out = "alpha"
         dt_map_key_at(m, 3, &out)  -> DT_ERR_RANGE, *out untouched
       cases/normal/map_basics.case */
    if (index >= m->count) {        //Overshooting
        return DT_ERR_RANGE;
    }
    map_entry *e = m->head;         
    for (size_t i = 0; i < index; i++) {            // iterate through the list until reaching the desired index
        e = e->order_next;
    }
    *out = e->key;
    return DT_OK;
}
